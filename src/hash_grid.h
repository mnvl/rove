// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file hash_grid.h
 * @brief Unbounded spatial hash of vertical columns for broad-phase
 *        queries over primitives you keep adding to
 *
 * Header-only, templated on the primitive and payload like octree, and
 * driven by the same traits: spatial_traits for an item's bounds,
 * spatial_query_traits for the exact test of a query against an item, so a
 * caller can switch between the two.  Where octree wants its bounds up
 * front and a build() before the first query, a hash grid needs neither:
 * insert() at any time, anywhere, and query straight away.  It suits many
 * small queries over a level that grows (a walker's per-frame probes).
 *
 * The space is cut into square columns of `cell_size` across the two axes
 * other than `up`; along `up` a column is unbounded.  A column holds every
 * item whose bounds reach into it, so an item spanning several columns is
 * listed in each, and a query reports it once.  Queries are const and touch
 * no shared state: several threads may query at once.
 *
 * Usage:
 * @code
 *   rove::hash_grid<rove::triangle<3>, int> grid(2.0f);       // 2 m columns, y up
 *   grid.insert(triangle, 7);
 *   std::vector<rove::hash_grid<rove::triangle<3>, int>::index_type> near;
 *   grid.query_indices(rove::aabb<3>(lo, hi), near);           // bounds meet the box
 *   rove::contact_info<3> const hit = grid.trace(rove::ray<3>(origin, direction * 10), 0, 1);
 *   if (hit.happened)
 *       use(grid[*hit.index].payload, hit.position, hit.normal);
 * @endcode
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "aabb.h"
#include "contact_info.h"
#include "traits.h"
#include "ray.h"
#include "vec.h"

namespace rove
{

/**
 * @brief Columns of `cell_size` square over the two axes other than `up`,
 *        each listing the items whose bounds reach into it
 *
 * @tparam Primitive What is stored; Traits::bounds gives its box.
 * @tparam Payload Carried with each item.
 * @tparam Traits spatial_traits<Primitive> or anything with the same shape.
 *
 * Items are numbered from 0 in the order of insert(); the number is the
 * item's index for operator[] and for query_indices().
 */
template<class Primitive, class Payload, class Traits = spatial_traits<Primitive>>
class hash_grid
{
public:
	static_assert(spatial_traits_for<Traits, Primitive>,
		"hash_grid: Traits must name scalar_type, vec_type and aabb_type and give bounds(primitive); is spatial_traits<Primitive> specialized?");

	using primitive_type = Primitive;
	using payload_type = Payload;
	using traits_type = Traits;
	using scalar_type = typename traits_type::scalar_type;
	using vec_type = typename traits_type::vec_type;
	using aabb_type = typename traits_type::aabb_type;
	using ray_type = ray<3, scalar_type>;
	using contact_type = contact_info<3, scalar_type>;
	using index_type = std::uint32_t;

	struct value_type
	{
		primitive_type primitive;
		payload_type payload;
	};

	/**
	 * @param cell_size Side of a column, finite and > 0
	 * @param up The axis columns run along: 0, 1 or 2
	 * @throws std::invalid_argument for anything else
	 */
	explicit hash_grid(scalar_type cell_size, int up = 1):
		cell_size_(cell_size),
		axes_{(up + 1) % 3, (up + 2) % 3}
	{
		if (!(cell_size > 0) || !std::isfinite(cell_size))
			throw std::invalid_argument("hash_grid: cell_size must be finite and positive, got " + std::to_string(cell_size));
		if (up < 0 || up > 2)
			throw std::invalid_argument("hash_grid: up must be 0, 1 or 2, got " + std::to_string(up));
	}

	scalar_type cell_size() const { return cell_size_; }
	size_t size() const { return items_.size(); }
	bool empty() const { return items_.empty(); }

	/// Room for @p n items in all, as std::vector::reserve.
	void reserve(size_t n)
	{
		items_.reserve(n);
		bounds_of_.reserve(n);
		first_of_.reserve(n);
	}

	/// Remove every item; the next insert() is numbered 0 again.  The columns
	/// keep their memory, so a grid refilled every frame does not allocate.
	void clear()
	{
		items_.clear();
		bounds_of_.clear();
		first_of_.clear();
		for (auto &c : cells_)
			c.second.clear();
	}

	/// @throws std::out_of_range for an index insert() never returned
	value_type const &operator[](index_type i) const
	{
		if (i >= items_.size())
			throw std::out_of_range("hash_grid: no item " + std::to_string(i) + " of " + std::to_string(items_.size()));
		return items_[i];
	}

	/// The union of the items' bounds.  @throws std::logic_error when empty
	aabb_type const &bounds() const
	{
		if (items_.empty())
			throw std::logic_error("hash_grid: an empty grid has no bounds");
		return bounds_;
	}

	/**
	 * @brief Add an item; returns its index
	 * @throws std::invalid_argument for bounds that are not finite
	 * @throws std::out_of_range for bounds too far out to number their
	 *         columns (beyond 2^30 columns from the origin)
	 * @throws std::length_error past 2^32 - 1 items
	 * On a throw nothing is added.
	 */
	index_type insert(primitive_type const &primitive, payload_type const &payload)
	{
		aabb_type const b = traits_type::bounds(primitive);
		b.check_finite("hash_grid: an item's bounds");
		for (int a : axes_)
			if (!(std::abs(b.lo.i[a] / cell_size_) < limit && std::abs(b.hi.i[a] / cell_size_) < limit))
				throw std::out_of_range("hash_grid: an item beyond 2^30 columns from the origin");
		if (items_.size() >= (std::numeric_limits<index_type>::max)())
			throw std::length_error("hash_grid: full");

		index_type const index = index_type(items_.size());
		cell const lo = cell_of(b.lo), hi = cell_of(b.hi);
		items_.push_back(value_type{primitive, payload});
		bounds_of_.push_back(b);
		first_of_.push_back(lo);
		if (index == 0)
			bounds_ = b;
		else
			bounds_.extend(b);
		for (int32_t u = lo.u; u <= hi.u; ++u)
			for (int32_t v = lo.v; v <= hi.v; ++v)
				cells_[cell{u, v}].push_back(index);
		return index;
	}

	/**
	 * @brief Append to @p out the index of every item the query meets
	 *
	 * Meets as QueryTraits::intersects says, after a bounds test; a query of
	 * the stored type is tested by the grid's own traits, and against
	 * triangles traits.h says what meets what.  Each item once, in no order.
	 * @throws std::invalid_argument for query bounds that are NaN
	 */
	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	void query_indices(Query const &query, std::vector<index_type> &out) const
	{
		visit<Query, QueryTraits>(query, [&](index_type i) { out.push_back(i); });
	}

	/// Like octree::query_intersection: appends the payloads.
	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	void query_intersection(Query const &query, std::vector<payload_type> &out) const
	{
		visit<Query, QueryTraits>(query, [&](index_type i) { out.push_back(items_[i].payload); });
	}

	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	std::vector<payload_type> query_intersection(Query const &query) const
	{
		std::vector<payload_type> out;
		query_intersection<Query, QueryTraits>(query, out);
		return out;
	}

	/// Like octree::query_intersection_items: appends the items.
	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	void query_intersection_items(Query const &query, std::vector<value_type> &out) const
	{
		visit<Query, QueryTraits>(query, [&](index_type i) { out.push_back(items_[i]); });
	}

	/**
	 * @brief The nearest item along @p r with t in [t_min, t_max]
	 *
	 * Walks the columns the ray crosses in order and stops at the first
	 * column past the nearest hit, so a long ray costs the columns it
	 * crosses, not the box around it.  Items whose bounds miss the box of
	 * the ray's [t_min, t_max] are passed over; @p test decides the rest:
	 * `bool test(value_type const &item, ray_type const &r, scalar_type t_min,
	 * scalar_type t_max, contact_type &ci)` returns true for a hit in [t_min,
	 * t_max] and fills ci.time, ci.position and ci.normal; by default
	 * trace_primitive, the primitive's own trace().  The contact
	 * comes back with happened, index (the item's), and penetrated false;
	 * on a miss happened is false and nothing else is set.  An item is
	 * tested in every column it shares with the ray; on equal time the
	 * first found is kept.
	 * @throws std::invalid_argument for a NaN or infinite ray, a NaN t, or
	 *         t_min > t_max
	 */
	template<class Test = trace_primitive>
		requires trace_test<Test, value_type, ray_type, scalar_type, contact_type>
	contact_type trace(ray_type const &r, scalar_type t_min = 0, scalar_type t_max = 1, Test &&test = Test()) const
	{
		r.check_finite("hash_grid: a ray");
		if (!(t_min <= t_max))
			throw std::invalid_argument("hash_grid: trace over [" + std::to_string(t_min) + ", " + std::to_string(t_max) + "]");
		contact_type best;
		best.happened = false;
		if (items_.empty())
			return best;

		// the part of [t_min, t_max] over the grid's columns
		scalar_type t0 = t_min, t1 = t_max;
		for (int a : axes_)
		{
			scalar_type const o = r.r0.i[a], d = r.a.i[a];
			if (d == 0)
			{
				if (o < bounds_.lo.i[a] || o > bounds_.hi.i[a])
					return best;
				continue;
			}
			scalar_type ta = (bounds_.lo.i[a] - o) / d, tb = (bounds_.hi.i[a] - o) / d;
			if (ta > tb)
				std::swap(ta, tb);
			t0 = (std::max)(t0, ta);
			t1 = (std::min)(t1, tb);
		}
		if (t0 > t1)
			return best;

		// the box of [t_min, t_max]; a direction of 0 keeps the origin (0 * inf is NaN)
		aabb_type span;
		for (int k = 0; k < 3; ++k)
		{
			scalar_type const o = r.r0.i[k], d = r.a.i[k];
			scalar_type const a = d == 0 ? o : o + d * t_min, b = d == 0 ? o : o + d * t_max;
			span.lo.i[k] = (std::min)(a, b);
			span.hi.i[k] = (std::max)(a, b);
		}

		cell const first = cell_of(r.apply(t0)), lo = cell_of(bounds_.lo), hi = cell_of(bounds_.hi);
		int32_t at[2] = {std::clamp(first.u, lo.u, hi.u), std::clamp(first.v, lo.v, hi.v)};
		int32_t const last[2][2] = {{lo.u, hi.u}, {lo.v, hi.v}};
		for (;;)
		{
			// where the ray leaves this column, per axis
			scalar_type leave[2];
			for (int k = 0; k < 2; ++k)
			{
				scalar_type const o = r.r0.i[axes_[k]], d = r.a.i[axes_[k]];
				leave[k] = d > 0 ? ((at[k] + 1) * cell_size_ - o) / d
					: d < 0 ? (at[k] * cell_size_ - o) / d
					: std::numeric_limits<scalar_type>::infinity();
			}
			auto const found = cells_.find(cell{at[0], at[1]});
			if (found != cells_.end())
				for (index_type i : found->second)
				{
					if (!bounds_of_[i].test_intersection(span))
						continue;
					contact_type ci;
					ci.happened = true;
					ci.penetrated = false;
					ci.penetration_depth = 0;
					ci.index = i;
					if (test(items_[i], r, t_min, best.happened ? best.time : t_max, ci) && best.worse_than(ci))
						best = ci;
				}
			int const k = leave[0] <= leave[1] ? 0 : 1;
			if ((best.happened && best.time <= leave[k]) || leave[k] >= t1)
				return best;
			at[k] += r.a.i[axes_[k]] > 0 ? 1 : -1;
			if (at[k] < last[k][0] || at[k] > last[k][1])
				return best;
		}
	}

private:
	static constexpr scalar_type limit = scalar_type(1 << 30);

	struct cell
	{
		int32_t u, v;
		bool operator==(cell const &o) const { return u == o.u && v == o.v; }
	};
	struct cell_hash
	{
		size_t operator()(cell const &c) const
		{
			return std::hash<uint64_t>()((uint64_t(uint32_t(c.u)) << 32) | uint32_t(c.v));
		}
	};

	scalar_type cell_size_;
	int axes_[2];
	std::vector<value_type> items_;
	std::vector<aabb_type> bounds_of_;
	std::vector<cell> first_of_;      // the column of each item's bounds.lo
	aabb_type bounds_;
	std::unordered_map<cell, std::vector<index_type>, cell_hash> cells_;

	cell cell_of(vec_type const &p) const
	{
		return cell{int32_t(std::floor(p.i[axes_[0]] / cell_size_)), int32_t(std::floor(p.i[axes_[1]] / cell_size_))};
	}

	// Calls f once for each item the query meets.  An item listed in several
	// columns is reported from one of them only: the first column, per axis,
	// that both it and the query reach, max(item's first, query's first).
	template<class Query, class QueryTraits, class F>
	void visit(Query const &query, F &&f) const
	{
		aabb_type q = QueryTraits::bounds(query);
		for (int k = 0; k < 3; ++k)
			if (std::isnan(q.lo.i[k]) || std::isnan(q.hi.i[k]))
				throw std::invalid_argument("hash_grid: a query with NaN bounds");
		if (items_.empty() || !q.test_intersection(bounds_))
			return;
		// only the grid's part of the query: keeps the column numbers in range
		for (int a : axes_)
		{
			q.lo.i[a] = (std::max)(q.lo.i[a], bounds_.lo.i[a]);
			q.hi.i[a] = (std::min)(q.hi.i[a], bounds_.hi.i[a]);
		}
		cell const lo = cell_of(q.lo), hi = cell_of(q.hi);
		auto column = [&](cell const &c, std::vector<index_type> const &items) {
			for (index_type i : items)
			{
				aabb_type const &b = bounds_of_[i];
				if (!b.test_intersection(q))
					continue;
				cell const &first = first_of_[i];
				if (c.u != (std::max)(first.u, lo.u) || c.v != (std::max)(first.v, lo.v))
					continue;
				if (QueryTraits::intersects(items_[i].primitive, query))
					f(i);
			}
		};
		// a query over more columns than are occupied walks the occupied ones
		double const columns = (double(hi.u) - lo.u + 1) * (double(hi.v) - lo.v + 1);
		if (columns > double(cells_.size()))
		{
			for (auto const &[c, items] : cells_)
				if (c.u >= lo.u && c.u <= hi.u && c.v >= lo.v && c.v <= hi.v)
					column(c, items);
			return;
		}
		for (int32_t u = lo.u; u <= hi.u; ++u)
			for (int32_t v = lo.v; v <= hi.v; ++v)
			{
				auto const found = cells_.find(cell{u, v});
				if (found != cells_.end())
					column(found->first, found->second);
			}
	}
};

}
