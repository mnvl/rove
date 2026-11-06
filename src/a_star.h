// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file a_star.h
 * @brief Grid-based A* pathfinding on integer 2D grids
 *
 * Header-only, templated on a scorer callable. The scorer returns the
 * traversal cost of a cell (>= 0) or a negative value to mark it blocked.
 *
 * Moves are 8-connected. Entering a cell costs its score, times sqrt(2) for a
 * diagonal step, and a diagonal step is only taken when both cells it passes
 * between are open (no cutting corners). The heuristic is the octile distance
 * to the goal times `min_cell_cost` (and 0.01 %, to break ties), so the path
 * found is a cheapest one to within 0.01 % as long as no cell is cheaper than
 * `min_cell_cost` (default 1).
 *
 * The scorer is called at most once per cell per search. The goal is tested
 * when it is taken off the open list, not when it is first reached.
 *
 * Usage:
 * @code
 *   struct my_scorer {
 *       rove::scalar operator()(rove::vec<2,int> const &p) const {
 *           return is_blocked(p) ? -1.0f : 1.0f;
 *       }
 *   };
 *
 *   rove::a_star<my_scorer> finder(start, goal, my_scorer{});
 *   finder.calculate_path();
 *   if (!finder.have_failed())
 *       auto path = finder.build_path();   // start → goal
 * @endcode
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <vector>
#include "vec.h"

namespace rove
{

/// What a_star needs of a Scorer: a cell's cost from the cell, callable on a const scorer
template<class Scorer>
concept a_star_scorer = std::invocable<Scorer const &, vec<2, int> const &> &&
	std::convertible_to<std::invoke_result_t<Scorer const &, vec<2, int> const &>, scalar>;

/**
 * @brief A* pathfinder on an integer 2D grid with 8-connected neighbours
 *
 * @tparam Scorer Callable with signature `scalar(vec<2,int> const &)`.
 *         Must return the traversal cost of the queried cell (>= 0),
 *         or a negative value if the cell is impassable.
 */
template<class Scorer>
class a_star {
public:
	static_assert(a_star_scorer<Scorer>, "a_star: Scorer must be callable as `scalar(vec<2, int> const &) const`");

	using point = vec<2, int>;

	/**
	 * @brief Construct the pathfinder
	 * @param start Start cell
	 * @param goal  Goal cell
	 * @param scorer Callable returning cell traversal cost (negative = blocked).
	 *        If the start cell is blocked, have_failed() is immediately true
	 *        and build_path()/build_cost_map() return empty results.
	 * @param min_cell_cost A lower bound on every open cell's cost, > 0. The
	 *        heuristic scales with it: a cell cheaper than this can make the
	 *        path found more expensive than the cheapest.
	 */
	a_star(point const &start, point const &goal, Scorer const &scorer, scalar min_cell_cost = 1):
		start_(start),
		goal_(goal),
		scorer_(scorer),
		min_cell_cost_(min_cell_cost),
		failed_(false)
	{
		if (!(min_cell_cost > 0))
			throw std::invalid_argument("a_star: min_cell_cost must be positive");
	}

	/**
	 * @brief Confine the search to the cells lo <= p < hi_exclusive
	 *
	 * Call before calculate_path()/calculate_cost_map(). Cells outside are blocked without asking the scorer, and the search keeps its cells
	 * in a flat array indexed by position instead of a hash table: several times faster on a grid that has bounds, and a search can no
	 * longer run off into unbounded space. The array is one int32 per cell of the box.
	 */
	void restrict_to(point const &lo, point const &hi_exclusive)
	{
		if (!(hi_exclusive.x > lo.x && hi_exclusive.y > lo.y))
			throw std::invalid_argument("a_star::restrict_to: the box must not be empty");
		if (int64_t(hi_exclusive.x - lo.x) * int64_t(hi_exclusive.y - lo.y) > int64_t(1) << 30)
			throw std::invalid_argument("a_star::restrict_to: the box has over 2^30 cells");
		bounded_ = true;
		box_lo_ = lo;
		box_w_ = hi_exclusive.x - lo.x;
		box_h_ = hi_exclusive.y - lo.y;
	}

	/**
	 * @brief How many cells a search with no box (restrict_to()) visits before it gives up, have_failed() true: 2^26 unless set
	 *
	 * That default is a few gigabytes and some seconds.  A box already bounds a search, so this does not apply in one.  0 is
	 * std::invalid_argument.
	 */
	void limit_cells(size_t cells)
	{
		if (cells == 0) throw std::invalid_argument("a_star::limit_cells: a search visits at least its start");
		cell_limit_ = cells;
	}

	/**
	 * @brief Find a cheapest path from start to goal
	 *
	 * After this call, have_failed() / build_path() are valid.
	 *
	 * If the goal is unreachable (blocked or disconnected from start),
	 * have_failed() returns true and build_path() returns an empty vector.  With no box (restrict_to()) a scorer that opens unbounded
	 * space has no end to search, so the search gives up the same way at limit_cells() cells.
	 * build_cost_map() will contain g-values only for the cells that were
	 * reached before the search exhausted the reachable region.
	 */
	void calculate_path()
	{
		start_search(true);
		run(true);
	}

	/**
	 * @brief Flood-fill the entire reachable area (Dijkstra-style)
	 *
	 * Does not stop at the goal: reaches every reachable cell so that
	 * build_cost_map() returns a complete distance field.
	 *
	 * have_failed() is false after this call unless the start cell is blocked.
	 * Unreachable cells simply remain at -1 in build_cost_map().
	 */
	void calculate_cost_map()
	{
		start_search(false);
		run(false);
	}

	/// @brief True if the goal was unreachable (or the start blocked)
	/// Only meaningful after calculate_path().
	bool have_failed() const { return failed_; }

	/// @brief Access the scorer passed at construction
	Scorer const &scorer() const { return scorer_; }

	/// @brief Cells taken off the open list and expanded by the last search
	size_t expanded() const { return expanded_; }

	/**
	 * @brief Reconstruct the path from start to goal
	 * @return Ordered sequence of cells [start, ..., goal].
	 *         Empty if have_failed() is true (goal unreachable or start blocked).
	 */
	std::vector<point> build_path() const
	{
		std::vector<point> result;

		if (failed_ || nodes_.empty()) return result;

		int32_t const goal_index = find(goal_);
		if (goal_index < 0 || nodes_[goal_index].g == infinity()) return result;

		for (int32_t i = goal_index; i >= 0; i = nodes_[i].came_from)
			result.push_back(nodes_[i].p);

		std::reverse(result.begin(), result.end());
		return result;
	}

	/**
	 * @brief Export g-values (cumulative cost from start) as a flat grid
	 * @param w Grid width
	 * @param h Grid height
	 * @return Row-major (y-major) vector of size w*h.
	 *         Element at index [y * w + x] holds the g-value for cell (x, y).
	 *         Cells that were not reached have value -1.
	 *         After calculate_cost_map(), all reachable cells have values >= 0
	 *         and they are exact; after calculate_path(), only cells reached
	 *         before the goal was taken have values >= 0 (not all exact).
	 */
	std::vector<scalar> build_cost_map(int w, int h) const
	{
		std::vector<scalar> cost_map(size_t(w) * size_t(h), scalar(-1));

		for (node const &n : nodes_)
		{
			if (n.blocked || n.g == infinity()) continue;
			if (n.p.x >= 0 && n.p.x < w && n.p.y >= 0 && n.p.y < h)
				cost_map[size_t(n.p.y) * size_t(w) + size_t(n.p.x)] = n.g;
		}

		return cost_map;
	}

	/**
	 * @brief build_cost_map() over the box of restrict_to()
	 * @return Row-major vector of the box's cells: [(y - lo.y) * width + (x - lo.x)]
	 *         holds the g-value of cell (x, y), -1 where none was reached.
	 * @throws std::logic_error when the search is not confined to a box
	 */
	std::vector<scalar> build_cost_map() const
	{
		if (!bounded_)
			throw std::logic_error("a_star::build_cost_map: no box to map, call restrict_to() or give the size");
		std::vector<scalar> cost_map(size_t(box_w_) * size_t(box_h_), scalar(-1));
		for (node const &n : nodes_)
		{
			if (n.blocked || n.g == infinity()) continue;
			cost_map[size_t(n.p.y - box_lo_.y) * size_t(box_w_) + size_t(n.p.x - box_lo_.x)] = n.g;
		}
		return cost_map;
	}

private:
	struct node
	{
		point p;
		int32_t came_from = -1;   ///< index of the predecessor on the cheapest known path
		scalar cost = 0;          ///< what entering the cell costs
		scalar g = 0;             ///< cheapest known cost from start
		bool blocked = false;
		bool closed = false;
	};

	struct open_entry
	{
		scalar f;
		scalar g;                 ///< the node's g when this entry was made: an entry with a larger g than the node's is stale
		int32_t index;
	};

	/// The heuristic is this much over the octile distance: on open ground the cells between start and goal then differ in f by more than
	/// rounding noise, and the search keeps to them instead of a plateau of equal f. The path found costs at most this share more than a cheapest.
	static constexpr scalar tie_break = 1.0001f;

	static constexpr scalar infinity() { return std::numeric_limits<scalar>::infinity(); }

	static constexpr scalar sqrt2 = std::numbers::sqrt2_v<scalar>;

	/// In a confined search, the one blocked node every cell outside the box shares
	static constexpr int32_t OUTSIDE = 0;

	point const start_, goal_;
	Scorer const scorer_;
	scalar const min_cell_cost_;
	bool failed_;
	bool goal_directed_ = true;
	bool bounded_ = false;
	point box_lo_{0, 0};
	int box_w_ = 0, box_h_ = 0;
	size_t expanded_ = 0;

	size_t cell_limit_ = size_t(1) << 26;
	std::vector<node> nodes_;
	std::vector<int32_t> table_;          ///< open addressing: node index or -1, a power of two long
	std::vector<open_entry> open_;        ///< a binary min-heap on f

	static uint64_t mix(point const &p)
	{
		uint64_t z = (uint64_t(uint32_t(p.x)) << 32 | uint32_t(p.y)) + 0x9e3779b97f4a7c15ull;
		z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
		z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
		return z ^ (z >> 31);
	}

	/// @brief The node of a cell, or -1 when the search has not met it
	int32_t find(point const &p) const
	{
		if (bounded_)
		{
			if (!in_box(p)) return OUTSIDE;
			return table_[size_t(p.y - box_lo_.y) * size_t(box_w_) + size_t(p.x - box_lo_.x)];
		}
		if (table_.empty()) return -1;
		size_t const mask = table_.size() - 1;
		for (size_t i = size_t(mix(p)) & mask;; i = (i + 1) & mask)
		{
			int32_t const k = table_[i];
			if (k < 0) return -1;
			if (nodes_[k].p == p) return k;
		}
	}

	bool in_box(point const &p) const { return p.x >= box_lo_.x && p.y >= box_lo_.y && p.x - box_lo_.x < box_w_ && p.y - box_lo_.y < box_h_; }

	void grow()
	{
		std::vector<int32_t> bigger(table_.empty() ? 1024 : table_.size() * 2, -1);
		size_t const mask = bigger.size() - 1;
		for (int32_t k = 0; k < int32_t(nodes_.size()); ++k)
		{
			size_t i = size_t(mix(nodes_[k].p)) & mask;
			while (bigger[i] >= 0) i = (i + 1) & mask;
			bigger[i] = k;
		}
		table_.swap(bigger);
	}

	/// @brief The node of a cell, scoring the cell the first time it is met
	int32_t node_of(point const &p)
	{
		int32_t const found = find(p);
		if (found >= 0) return found;
		node n;
		n.p = p;
		n.cost = scorer_(p);
		n.blocked = n.cost < 0;
		n.g = infinity();
		if (bounded_)
		{
			nodes_.push_back(n);
			table_[size_t(p.y - box_lo_.y) * size_t(box_w_) + size_t(p.x - box_lo_.x)] = int32_t(nodes_.size() - 1);
			return int32_t(nodes_.size() - 1);
		}
		if ((nodes_.size() + 1) * 2 > table_.size()) grow();
		nodes_.push_back(n);
		size_t const mask = table_.size() - 1;
		size_t i = size_t(mix(p)) & mask;
		while (table_[i] >= 0) i = (i + 1) & mask;
		table_[i] = int32_t(nodes_.size() - 1);
		return int32_t(nodes_.size() - 1);
	}

	/// @brief Octile distance to the goal, scaled: what the cheapest way could cost on an empty grid
	scalar heuristic(point const &p) const
	{
		if (!goal_directed_) return 0;
		scalar const dx = scalar(std::abs(p.x - goal_.x)), dy = scalar(std::abs(p.y - goal_.y));
		return tie_break * min_cell_cost_ * (std::max(dx, dy) + (sqrt2 - 1) * std::min(dx, dy));
	}

	/// Cheapest f first; of equal f the one further along (larger g), which keeps a search across open ground to the straight band
	/// between start and goal instead of every cell of equal f.
	static bool later(open_entry const &a, open_entry const &b) { return a.f > b.f || (a.f == b.f && a.g < b.g); }

	void push(open_entry const &e)
	{
		open_.push_back(e);
		std::push_heap(open_.begin(), open_.end(), later);
	}

	open_entry pop()
	{
		std::pop_heap(open_.begin(), open_.end(), later);
		open_entry e = open_.back();
		open_.pop_back();
		return e;
	}

	/// @brief Seed the open list with the start cell
	void start_search(bool goal_directed)
	{
		goal_directed_ = goal_directed;
		failed_ = false;
		expanded_ = 0;
		nodes_.clear();
		table_.clear();
		open_.clear();
		if (bounded_)
		{
			table_.assign(size_t(box_w_) * size_t(box_h_), -1);
			node out;                      // outside the box: blocked, never scored
			out.p = point(0, 0);
			out.blocked = true;
			out.g = infinity();
			nodes_.push_back(out);
		}
		int32_t const s = node_of(start_);
		if (nodes_[s].blocked) { failed_ = true; return; }
		nodes_[s].g = 0;
		push({heuristic(start_), 0, s});
	}

	/**
	 * @brief Main search loop
	 * @param stop_at_goal If true, stop when the goal is taken off the open
	 *                     list (standard A*). If false, reach all reachable cells.
	 */
	void run(bool stop_at_goal)
	{
		if (failed_) return;
		while (!open_.empty())
		{
			open_entry const e = pop();
			if (nodes_[e.index].closed || e.g > nodes_[e.index].g) continue;      // stale: a cheaper way to it was found since

			if (stop_at_goal && nodes_[e.index].p == goal_) return;
			if (!bounded_ && nodes_.size() > cell_limit_) {
				failed_ = true;
				return;
			}

			nodes_[e.index].closed = true;
			++expanded_;
			expand(e.index);
		}

		if (stop_at_goal) failed_ = true;
	}

	/// @brief Relax the 8 neighbours of node @p index
	void expand(int32_t index)
	{
		point const p = nodes_[index].p;
		scalar const g = nodes_[index].g;
		int32_t around[3][3];      // the node of each neighbour, met once (the nodes vector may move as they are added: indices only)
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
				around[dy + 1][dx + 1] = (dx || dy) ? node_of(point(p.x + dx, p.y + dy)) : -1;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0) continue;
				int32_t const n = around[dy + 1][dx + 1];
				if (nodes_[n].blocked) continue;
				bool const diagonal = dx != 0 && dy != 0;
				if (diagonal && (nodes_[around[1][dx + 1]].blocked || nodes_[around[dy + 1][1]].blocked))
					continue;      // no cutting a corner

				scalar const ng = g + nodes_[n].cost * (diagonal ? sqrt2 : scalar(1));
				if (ng < nodes_[n].g)
				{
					nodes_[n].came_from = index;
					nodes_[n].g = ng;
					nodes_[n].closed = false;
					push({ng + heuristic(nodes_[n].p), ng, n});
				}
			}
	}
};

}
