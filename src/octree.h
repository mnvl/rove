// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file octree.h
 * @brief Static 3D octree for broad-phase spatial queries
 *
 * Driven by the traits of traits.h like hash_grid, with the same queries and
 * the same trace(), so a caller can switch between the two.
 */

#pragma once

#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "aabb.h"
#include "contact_info.h"
#include "ray.h"
#include "traits.h"

namespace rove
{

/**
 * @brief Static octree for primitives with AABB bounds and intersection tests
 *
 * The tree is "static": call `insert(...)` to provide items, then `build()` once.
 * Queries do not mutate the tree.  A query before build(), or after an
 * insert() or clear() since, is a std::logic_error.  Items are numbered from
 * 0 in the order of insert(); the number is the item's index for operator[]
 * and contact_info::index.
 */
template<class Primitive, class Payload, class Traits = spatial_traits<Primitive>>
class octree
{
public:
	static_assert(spatial_traits_for<Traits, Primitive>,
		"octree: Traits must name scalar_type, vec_type and aabb_type and give bounds(primitive); is spatial_traits<Primitive> specialized?");

	using primitive_type = Primitive;
	using payload_type = Payload;
	using traits_type = Traits;
	using scalar_type = typename traits_type::scalar_type;
	using vec_type = typename traits_type::vec_type;
	using aabb_type = typename traits_type::aabb_type;
	using ray_type = ray<3, scalar_type>;
	using contact_type = contact_info<3, scalar_type>;
	using index_type = size_t;

	struct value_type
	{
		primitive_type primitive;
		payload_type payload;
	};

	static constexpr size_t subclusters_count = 8;
	static constexpr unsigned x_mask = 1;
	static constexpr unsigned y_mask = 2;
	static constexpr unsigned z_mask = 4;

	/**
	 * @param bounds The box the items go in
	 * @param leaf_items_threshold A cell of more items than this divides
	 * @param min_subcluster_diagonal_sq A cell whose squared diagonal is below this does not divide.  The default of 1 suits a scene
	 *        measured in units of about a metre: a scene under about one unit across never divides, so scale this with it.
	 */
	explicit octree(
		aabb_type const &bounds,
		size_t leaf_items_threshold = 8,
		scalar_type min_subcluster_diagonal_sq = scalar_type(1)):
		bounds_(bounds),
		centre_(bounds.centre()),
		leaf_items_threshold_(leaf_items_threshold),
		min_subcluster_diagonal_sq_(min_subcluster_diagonal_sq)
	{
		// (a box that holds nothing is no error until build() has items to put in it)
	}

	void clear()
	{
		items_.clear();
		root_.reset();
	}

	void reserve(size_t n)
	{
		items_.reserve(n);
	}

	void insert(primitive_type const &primitive, payload_type const &payload)
	{
		items_.push_back(value_type{primitive, payload});
		root_.reset();
	}

	size_t size() const
	{
		return items_.size();
	}

	/// @throws std::out_of_range for an index no insert() made
	value_type const &operator[](index_type i) const
	{
		if (i >= items_.size()) {
			throw std::out_of_range("octree: no item " + std::to_string(i) + " of " + std::to_string(items_.size()));
		}
		return items_[i];
	}

	void build()
	{
		// A box that is empty or infinite never gets smaller: items in it have nowhere to be put
		if (!items_.empty() && (bounds_.is_empty() || !is_finite(bounds_.lo) || !is_finite(bounds_.hi)))
			throw std::invalid_argument("octree::build: items in a box that is empty or not finite");

		root_.reset(new node(bounds_));

		std::vector<size_t> indices(items_.size());
		for (size_t i = 0; i < items_.size(); ++i) {
			indices[i] = i;
		}

		build_node(*root_, indices, 0);
	}

	/// @brief A new tree of the same items and settings, not built: the tree itself holds nodes that cannot be copied
	octree copy_unbuilt() const
	{
		octree made(bounds_, leaf_items_threshold_, min_subcluster_diagonal_sq_);
		made.items_ = items_;
		return made;
	}

	aabb_type const &bounds() const { return bounds_; }

	/// Whether build() or load() made the tree, and no insert() or clear() since: what a query needs
	bool is_built() const
	{
		return root_ != nullptr;
	}

	bool is_leaf() const
	{
		return root_ == nullptr || root_->is_leaf();
	}

	aabb_type subcluster_bounds(unsigned index) const
	{
		return subcluster_bounds(bounds_, index);
	}

	unsigned subcluster_by_point(vec_type const &point) const
	{
		return subcluster_by_point(centre_, point);
	}

	size_t subclusters_by_aabb(aabb_type const &bounds, unsigned subclusters[subclusters_count]) const
	{
		return subclusters_by_aabb(centre_, bounds, subclusters);
	}

	unsigned subclusters_mask_by_aabb(aabb_type const &bounds) const
	{
		unsigned result = 0;
		unsigned subclusters[subclusters_count];
		size_t nsubclusters = subclusters_by_aabb(bounds, subclusters);

		for (size_t i = 0; i < nsubclusters; ++i) {
			result |= 1u << subclusters[i];
		}

		return result;
	}

	/**
	 * @brief Append to @p result the payload of every item the query meets
	 *
	 * Meets as QueryTraits::intersects says; a query of the stored type is
	 * tested by the tree's own traits.  Each item once, in no order.
	 */
	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	void query_intersection(Query const &query, std::vector<payload_type> &result) const
	{
		visit<QueryTraits>(query, [&](index_type i) { result.push_back(items_[i].payload); });
	}

	/// Like query_intersection(): appends the index (operator[]'s) of every
	/// item the query meets, as hash_grid::query_indices does.
	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	void query_indices(Query const &query, std::vector<index_type> &result) const
	{
		visit<QueryTraits>(query, [&](index_type i) { result.push_back(i); });
	}

	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	std::vector<payload_type> query_intersection(Query const &query) const
	{
		std::vector<payload_type> result;
		query_intersection<Query, QueryTraits>(query, result);
		return result;
	}

	/// Like query_intersection(): appends the items.
	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	void query_intersection_items(Query const &query, std::vector<value_type> &result) const
	{
		visit<QueryTraits>(query, [&](index_type i) { result.push_back(items_[i]); });
	}

	template<class Query, class QueryTraits = default_query_traits<primitive_type, traits_type, Query>>
		requires spatial_query_traits_for<QueryTraits, primitive_type, Query>
	std::vector<value_type> query_intersection_items(Query const &query) const
	{
		std::vector<value_type> result;
		query_intersection_items<Query, QueryTraits>(query, result);
		return result;
	}

	/**
	 * @brief The nearest item along @p r with t in [t_min, t_max]
	 *
	 * As hash_grid::trace: @p test is `bool test(value_type const &item,
	 * ray_type const &r, scalar_type t_min, scalar_type t_max, contact_type &ci)`, true
	 * for a hit in [t_min, t_max] with ci.time, ci.position and ci.normal
	 * filled; by default trace_primitive, the primitive's own trace().
	 * The contact comes back with happened, index (the item's) and
	 * penetrated false; on a miss happened is false and nothing else is set.
	 * Nodes are walked nearest first, and none is entered past the nearest
	 * hit found so far.
	 * @throws std::invalid_argument for a NaN or infinite ray, a NaN t, or
	 *         t_min > t_max
	 * @throws std::logic_error on a tree not built
	 */
	template<class Test = trace_primitive>
		requires trace_test<Test, value_type, ray_type, scalar_type, contact_type>
	contact_type trace(ray_type const &r, scalar_type t_min = 0, scalar_type t_max = 1, Test &&test = Test()) const
	{
		require_built("trace");
		r.check_finite("octree: a ray");
		if (!(t_min <= t_max)) {
			throw std::invalid_argument("octree: trace over [" + detail::to_string_of(t_min) + ", " + detail::to_string_of(t_max) + "]");
		}
		contact_type best;
		best.happened = false;
		trace_node(*root_, r, t_min, t_max, t_min, t_max, test, best);
		return best;
	}

	/**
	 * @brief The built tree -- its items and its nodes -- as bytes.
	 *
	 * Binary and native-endian: a tree is read back on the machine that
	 * wrote it (a scene built in one process, loaded in another), not
	 * exchanged. load() rebuilds exactly this tree, without build().
	 */
	std::string save() const
	{
		require_built("save");
		std::string bytes;
		bytes.reserve(64 + items_.size() * (sizeof(primitive_type) + sizeof(payload_type) + 8));
		serial_writer out{bytes};
		out & serial_magic & serial_version
			& static_cast<std::uint32_t>(sizeof(primitive_type))
			& static_cast<std::uint32_t>(sizeof(payload_type))
			& bounds_
			& static_cast<std::uint64_t>(leaf_items_threshold_)
			& min_subcluster_diagonal_sq_
			& static_cast<std::uint64_t>(items_.size());
		for (value_type const &item : items_) {
			out & item.primitive & item.payload;
		}
		save_node(out, *root_);
		return bytes;
	}

	/**
	 * @brief Read a tree save() wrote: all of `bytes`, nothing else.
	 * Throws std::runtime_error on anything else: another format or
	 * version, another primitive or payload type, too few bytes or too
	 * many, or nodes that do not name every item exactly once.
	 */
	static octree load(std::string_view bytes)
	{
		serial_reader in{bytes, 0};
		if (in.template read<std::uint32_t>() != serial_magic) {
			throw std::runtime_error("octree::load: not a saved octree");
		}
		std::uint32_t const version = in.template read<std::uint32_t>();
		if (version != serial_version) {
			throw std::runtime_error("octree::load: unsupported version " + std::to_string(version));
		}
		if (in.template read<std::uint32_t>() != sizeof(primitive_type) ||
			in.template read<std::uint32_t>() != sizeof(payload_type))
		{
			throw std::runtime_error("octree::load: saved for another primitive or payload type");
		}
		aabb_type const bounds = in.template read<aabb_type>();
		if (!all_less_equal(bounds.lo, bounds.hi)) {
			throw std::runtime_error("octree::load: inverted bounds");
		}
		std::uint64_t const leaf_items_threshold = in.template read<std::uint64_t>();
		scalar_type const min_subcluster_diagonal_sq = in.template read<scalar_type>();
		octree result(bounds, static_cast<size_t>(leaf_items_threshold), min_subcluster_diagonal_sq);

		std::uint64_t const count = in.template read<std::uint64_t>();
		if (count > (std::numeric_limits<std::uint32_t>::max)() ||
			count * (sizeof(primitive_type) + sizeof(payload_type)) > bytes.size())
		{
			throw std::runtime_error("octree::load: item count out of range");
		}
		result.items_.resize(static_cast<size_t>(count));
		for (value_type &item : result.items_) {
			in & item.primitive & item.payload;
		}

		std::vector<bool> placed(result.items_.size(), false);
		size_t placed_count = 0;
		result.root_.reset(new node(result.bounds_));
		load_node(in, *result.root_, placed, placed_count, 0);
		if (placed_count != result.items_.size()) {
			throw std::runtime_error("octree::load: nodes do not hold every item");
		}
		if (in.position != bytes.size()) {
			throw std::runtime_error("octree::load: bytes past the saved tree");
		}
		return result;
	}

private:
	struct node;

	static constexpr std::uint32_t serial_magic = 0x544f5652u;  // "RVOT"
	static constexpr std::uint32_t serial_version = 1;
	// Far past what build() reaches: a float box halves ~40 times before
	// min_subcluster_diagonal_sq stops it. Deeper means corrupt bytes.
	static constexpr unsigned serial_max_depth = 128;

	// A binary archive over rove's serialize(Archive &) members: arithmetic
	// values are their bytes, anything else goes through its own
	// serialize(), which names its fields (`archive & lo & hi`).
	struct serial_writer
	{
		std::string &bytes;

		template<class V>
		serial_writer &operator &(V const &value)
		{
			if constexpr (std::is_arithmetic<V>::value) {
				bytes.append(reinterpret_cast<char const *>(&value), sizeof(V));
			} else {
				V copy(value);
				copy.serialize(*this);
			}
			return *this;
		}
	};

	struct serial_reader
	{
		std::string_view bytes;
		size_t position;

		template<class V>
		serial_reader &operator &(V &value)
		{
			if constexpr (std::is_arithmetic<V>::value) {
				if (bytes.size() - position < sizeof(V)) {
					throw std::runtime_error("octree::load: bytes end early");
				}
				std::memcpy(&value, bytes.data() + position, sizeof(V));
				position += sizeof(V);
			} else {
				value.serialize(*this);
			}
			return *this;
		}

		template<class V>
		V read()
		{
			V value;
			*this & value;
			return value;
		}
	};

	// Pre-order: the node's item indices, then a bit per child present, then
	// those children in index order. A child's bounds are not written: they
	// follow from its parent's, as build() derived them.
	static void save_node(serial_writer &out, node const &current)
	{
		out & static_cast<std::uint32_t>(current.item_indices.size());
		for (size_t index : current.item_indices) {
			out & static_cast<std::uint32_t>(index);
		}
		std::uint8_t mask = 0;
		for (size_t i = 0; i < subclusters_count; ++i) {
			if (current.children[i] != nullptr) {
				mask |= static_cast<std::uint8_t>(1u << i);
			}
		}
		out & mask;
		for (size_t i = 0; i < subclusters_count; ++i) {
			if (current.children[i] != nullptr) {
				save_node(out, *current.children[i]);
			}
		}
	}

	static void load_node(serial_reader &in, node &current, std::vector<bool> &placed,
		size_t &placed_count, unsigned depth)
	{
		if (depth > serial_max_depth) {
			throw std::runtime_error("octree::load: nodes nest deeper than any built tree");
		}
		std::uint32_t const count = in.template read<std::uint32_t>();
		if (count > placed.size() - placed_count) {
			throw std::runtime_error("octree::load: a node holds more items than are left");
		}
		current.item_indices.resize(count);
		for (std::uint32_t k = 0; k < count; ++k) {
			std::uint32_t const index = in.template read<std::uint32_t>();
			if (index >= placed.size() || placed[index]) {
				throw std::runtime_error("octree::load: a node names a missing or already placed item");
			}
			placed[index] = true;
			++placed_count;
			current.item_indices[k] = index;
		}
		std::uint8_t const mask = in.template read<std::uint8_t>();
		for (unsigned i = 0; i < subclusters_count; ++i) {
			if (mask & (1u << i)) {
				current.children[i].reset(new node(subcluster_bounds(current.bounds, i)));
				load_node(in, *current.children[i], placed, placed_count, depth + 1);
			}
		}
	}

	struct node
	{
		explicit node(aabb_type const &node_bounds):
			bounds(node_bounds),
			centre(node_bounds.centre())
		{
		}

		bool is_leaf() const
		{
			for (size_t i = 0; i < subclusters_count; ++i) {
				if (children[i] != nullptr) {
					return false;
				}
			}

			return true;
		}

		aabb_type bounds;
		vec_type centre;
		std::vector<size_t> item_indices;
		std::array<std::unique_ptr<node>, subclusters_count> children;
	};

	static aabb_type subcluster_bounds(aabb_type const &bounds, unsigned index)
	{
		vec_type origin = bounds.lo;
		vec_type extents = bounds.diagonal() / scalar_type(2);

		if (index & x_mask) origin.x += extents.x;
		if (index & y_mask) origin.y += extents.y;
		if (index & z_mask) origin.z += extents.z;

		return aabb_type(origin, origin + extents);
	}

	static unsigned subcluster_by_point(vec_type const &centre, vec_type const &point)
	{
		unsigned index = 0;

		if (point.x >= centre.x) index |= x_mask;
		if (point.y >= centre.y) index |= y_mask;
		if (point.z >= centre.z) index |= z_mask;

		return index;
	}

	static size_t subclusters_by_aabb(vec_type const &centre, aabb_type const &bounds, unsigned subclusters[subclusters_count])
	{
		assert(all_less_equal(bounds.lo, bounds.hi));

		unsigned c1 = subcluster_by_point(centre, bounds.lo);
		unsigned c2 = subcluster_by_point(centre, bounds.hi);

		if (c1 == c2) {
			subclusters[0] = c1;
			return 1;
		}

		if (c1 == 0 && c2 == subclusters_count - 1) {
			for (size_t i = 0; i < subclusters_count; ++i) {
				subclusters[i] = static_cast<unsigned>(i);
			}
			return subclusters_count;
		}

		unsigned diff = c1 ^ c2;
		unsigned hi = diff & (diff - 1);
		unsigned lo = diff & ~hi;

		assert((c1 | lo | hi) == c2);

		if (hi) {
			subclusters[0] = c1;
			subclusters[1] = c1 | lo;
			subclusters[2] = c1 | hi;
			subclusters[3] = c2;
			return 4;
		}

		subclusters[0] = c1;
		subclusters[1] = c2;
		return 2;
	}

	/// Past this many halvings a cell is below what its scalar can tell, and dividing it further only recurses: items that no division
	/// separates (coincident ones where min_subcluster_diagonal_sq is 0, NaNs anywhere) stay in the leaf they reach.
	static constexpr int max_depth = 48;

	void build_node(node &current, std::vector<size_t> const &indices, int depth)
	{
		current.item_indices.clear();

		if (indices.empty()) {
			return;
		}

		if (indices.size() < leaf_items_threshold_ ||
			current.bounds.diagonal().length_sq() < min_subcluster_diagonal_sq_ ||
			depth >= max_depth)
		{
			current.item_indices = indices;
			return;
		}

		std::array<std::vector<size_t>, subclusters_count> child_indices;

		for (size_t item_index : indices) {
			aabb_type primitive_bounds = traits_type::bounds(items_[item_index].primitive);
			// A NaN has no place among the children: it stays here (a query's tests then answer for it, and are false)
			if (primitive_bounds.is_empty()) {
				current.item_indices.push_back(item_index);
				continue;
			}
			unsigned subclusters[subclusters_count];
			size_t nsubclusters = subclusters_by_aabb(current.centre, primitive_bounds, subclusters);

			// Keep spanning primitives in the current node to avoid duplicate payloads.
			if (nsubclusters != 1) {
				current.item_indices.push_back(item_index);
			} else {
				child_indices[subclusters[0]].push_back(item_index);
			}
		}

		bool have_children = false;
		for (size_t i = 0; i < subclusters_count; ++i) {
			if (!child_indices[i].empty()) {
				have_children = true;
				break;
			}
		}

		if (!have_children) {
			return;
		}

		for (size_t i = 0; i < subclusters_count; ++i) {
			if (child_indices[i].empty()) {
				continue;
			}

			current.children[i].reset(new node(subcluster_bounds(current.bounds, static_cast<unsigned>(i))));
			build_node(*current.children[i], child_indices[i], depth + 1);
		}
	}

	void require_built(char const *what) const
	{
		if (root_ == nullptr) {
			throw std::logic_error(std::string("octree::") + what + ": build() the tree first");
		}
	}

	template<class QueryTraits, class Query, class F>
	void visit(Query const &query, F &&f) const
	{
		require_built("query");
		visit_node<QueryTraits>(*root_, query, QueryTraits::bounds(query), f);
	}

	template<class QueryTraits, class Query, class F>
	void visit_node(node const &current, Query const &query, aabb_type const &query_bounds, F &f) const
	{
		for (size_t item_index : current.item_indices) {
			if (QueryTraits::intersects(items_[item_index].primitive, query)) {
				f(item_index);
			}
		}

		if (current.is_leaf()) {
			return;
		}

		unsigned subclusters[subclusters_count];
		size_t nsubclusters = subclusters_by_aabb(current.centre, query_bounds, subclusters);

		for (size_t i = 0; i < nsubclusters; ++i) {
			node const *child = current.children[subclusters[i]].get();
			if (child != nullptr) {
				visit_node<QueryTraits>(*child, query, query_bounds, f);
			}
		}
	}

	// Tests the node's items, then walks into its children in the order the
	// ray enters them, over [from, to]: the part of the ray in this node's
	// octant.  An octant is the half-spaces at its ancestors' centres, not
	// the node's box, as build() placed the items (one may reach out of the
	// tree's bounds); each is widened by a hair at its dividing planes, so
	// rounding cannot hide an item lying on one.
	template<class Test>
	void trace_node(node const &current, ray_type const &r, scalar_type t_min, scalar_type t_max,
		scalar_type from, scalar_type to, Test &test, contact_type &best) const
	{
		for (size_t item_index : current.item_indices) {
			contact_type ci;
			ci.happened = true;
			ci.penetrated = false;
			ci.penetration_depth = 0;
			ci.index = item_index;
			if (test(items_[item_index], r, t_min, best.happened ? best.time : t_max, ci) && best.worse_than(ci)) {
				best = ci;
			}
		}

		struct span { scalar_type from, to; unsigned child; };
		span spans[subclusters_count];
		size_t count = 0;
		for (unsigned i = 0; i < subclusters_count; ++i) {
			if (current.children[i] == nullptr) {
				continue;
			}
			span s{from, to, i};
			for (int k = 0; k < 3 && s.from <= s.to; ++k) {
				scalar_type const c = current.centre.i[k];
				scalar_type const slack = detail::constant_of<scalar_type>(1e-5) * (abs(c) + current.bounds.hi.i[k] - current.bounds.lo.i[k]);
				bool const upper = (i >> k) & 1u;          // x >= c - slack, else x <= c + slack
				scalar_type const plane = upper ? c - slack : c + slack;
				scalar_type const o = r.r0.i[k], d = r.a.i[k];
				if (d == 0) {
					if (upper ? o < plane : o > plane) {
						s.to = -std::numeric_limits<scalar_type>::infinity();
					}
				} else if ((d > 0) == upper) {
					s.from = (std::max)(s.from, (plane - o) / d);
				} else {
					s.to = (std::min)(s.to, (plane - o) / d);
				}
			}
			if (s.from <= s.to) {
				spans[count++] = s;
			}
		}
		std::sort(spans, spans + count, [](span const &a, span const &b) { return a.from < b.from; });
		for (size_t k = 0; k < count; ++k) {
			if (best.happened && spans[k].from > best.time) {
				return;
			}
			trace_node(*current.children[spans[k].child], r, t_min, t_max, spans[k].from, spans[k].to, test, best);
		}
	}

	aabb_type bounds_;
	vec_type centre_;
	size_t leaf_items_threshold_;
	scalar_type min_subcluster_diagonal_sq_;
	std::vector<value_type> items_;
	std::unique_ptr<node> root_;      ///< null until build(), and again after insert() or clear()
};

}
