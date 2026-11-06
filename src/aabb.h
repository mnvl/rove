// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file aabb.h
 * @brief N-dimensional axis-aligned bounding box (AABB) class
 *
 * Provides axis-aligned bounding boxes for spatial queries and collision detection.
 * Commonly used specializations are 2D rectangles (N=2) and 3D boxes (N=3).
 */

#pragma once

#include <cassert>
#include <limits>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "scalar.h"
#include "vec.h"
#include "matrix.h"

#undef min
#undef max

namespace rove
{

template<int N,class T> class triangle;
template<int N,class T> class ray;

/**
 * @brief N-dimensional axis-aligned bounding box
 *
 * Represents a box aligned with the coordinate axes, defined by minimum (lo)
 * and maximum (hi) corner points. AABBs are commonly used for:
 * - Broad-phase collision detection
 * - Spatial partitioning (octrees, BVH)
 * - View frustum culling
 *
 * @tparam N Dimensionality (2 for rectangle, 3 for box)
 *
 * @note The lo corner should have smaller coordinates than hi for a valid AABB.
 *       Use normalize() to ensure this invariant after direct manipulation.
 */
template<int N,class T = scalar>
class [[nodiscard]] aabb {
public:
	// types and constants
	using limits_type = std::numeric_limits<T>;  ///< Numeric limits for scalar type
	static constexpr size_t dimensions = N;              ///< Number of dimensions
	using vec_type = vec<dimensions,T>;          ///< Associated vector type
	static constexpr size_t vertices_count = 1 << dimensions;  ///< Number of corner vertices (2^N)
	static constexpr size_t edges_count = dimensions * (1 << (dimensions-1));  ///< Number of edges
	using ray_type = ray<dimensions,T>;          ///< Associated ray type

	vec_type lo;   ///< Minimum corner (lower bounds in all dimensions)
	vec_type hi;   ///< Maximum corner (upper bounds in all dimensions)

	/// An empty box (null()): it contains nothing, and the first extend() makes it that point.  It was left uninitialised.
	aabb()
	{
		null();
	}

	/**
	 * @brief Construct degenerate AABB from single point
	 *
	 * Creates a zero-volume AABB where lo and hi are both set to the given point.
	 *
	 * @param centre The point to use as both corners
	 */
	explicit aabb(vec_type const &centre):
		lo(centre),
		hi(centre)
	{
	}

	/**
	 * @brief Construct AABB from two corner points
	 *
	 * @param l Minimum corner (lo)
	 * @param h Maximum corner (hi)
	 * @note If l > h in any dimension, call normalize() to fix
	 */
	aabb(vec_type const &l,vec_type const &h):
		lo(l),
		hi(h)
	{
	}

	/**
	 * @brief Construct a cube AABB from its centre and side length
	 *
	 * The box spans centre +- side_length / 2 on every axis.  Note this is
	 * a *diameter*, not a radius: aabb(centre, 10) has sides of 10 and a
	 * half-extent of 5.
	 *
	 * @param centre Center point of the AABB
	 * @param side_length Full side length (applied to all dimensions)
	 */
	aabb(vec_type const &centre, T side_length)
	{
		vec_type delta;
		delta.set_all(side_length / 2);

		lo = centre - delta;
		hi = centre + delta;
	}

	/**
	 * @brief Reset AABB to "null" state for accumulation
	 *
	 * Sets lo to maximum possible values and hi to minimum possible values.
	 * This prepares the AABB for building via extend() calls - the first
	 * extend() will set proper bounds, subsequent calls expand as needed.
	 */
	void null() {
		for (size_t n=0; n<dimensions; n++) {
			lo.i[n] = +limits_type::max();
			hi.i[n] = -limits_type::max();
		}
	}

	/// Whether the box holds no point at all: lo past hi on some axis, as null() leaves it.  A box of a single point is not empty.
	bool is_empty() const {
		for (size_t n=0; n<dimensions; n++)
			if (!(lo.i[n] <= hi.i[n])) return true;
		return false;
	}

	/// The box with every side moved out by `margin` (in by a negative one); an empty box stays empty, and one shrunk past nothing
	/// is empty
	aabb grown(T margin) const {
		if (is_empty()) return *this;
		aabb r(*this);
		for (size_t n=0; n<dimensions; n++) {
			r.lo.i[n] -= margin;
			r.hi.i[n] += margin;
		}
		return r;
	}

	/**
	 * @brief Transform vector from world to local AABB coordinates
	 *
	 * Scales a direction vector to AABB-local space where the AABB
	 * has unit dimensions. Does not account for translation.
	 *
	 * @param[out] v Output vector in local coordinates
	 * @param[in] v0 Input vector in world coordinates
	 */
	void world_to_local_vector(vec_type &v,vec_type const &v0) const {
		v = v0 / (hi - lo);
	}

	/**
	 * @brief Transform point from world to local AABB coordinates
	 *
	 * Transforms a point to AABB-local space where lo maps to origin
	 * and hi maps to (1,1,...). Useful for computing barycentric-like
	 * coordinates within the box.
	 *
	 * @param[out] v Output point in local coordinates [0,1]^N for interior points
	 * @param[in] v0 Input point in world coordinates
	 */
	void world_to_local_point(vec_type &v,vec_type const &v0) const {
		world_to_local_vector(v, v0 - lo);
	}

	/**
	 * @brief Transform ray from world to local AABB coordinates
	 *
	 * Transforms both the ray origin and direction to AABB-local space.
	 *
	 * @param[out] r Output ray in local coordinates
	 * @param[in] r0 Input ray in world coordinates
	 */
	void world_to_local_ray(ray_type &r,ray_type const &r0) const {
		world_to_local_point(r.r0,r0.r0);
		world_to_local_vector(r.a,r0.a);
	}

	/**
	 * @brief Ensure lo <= hi in all dimensions
	 *
	 * Swaps lo and hi coordinates where necessary to maintain the
	 * invariant that lo[i] <= hi[i] for all dimensions i.
	 */
	void normalize() {
		minmax(lo,hi);
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where a corner is not
	aabb const &check_finite(char const *what) const {
		lo.check_finite(what);
		hi.check_finite(what);
		return *this;
	}

	/**
	 * @brief Get AABB diagonal vector
	 *
	 * Returns the vector from lo to hi corner.
	 *
	 * @return Vector representing AABB dimensions (hi - lo)
	 */
	vec_type diagonal() const {
		return hi - lo;
	}

	/**
	 * @brief Get AABB center point
	 *
	 * @return Center point of the AABB
	 */
	vec_type centre() const {
		return (lo + hi) / T(2);
	}

	/**
	 * @brief Point of the AABB closest to a point
	 *
	 * Each coordinate clamped to [lo, hi] on its own: @p p itself when it is
	 * inside.  A NaN coordinate stays NaN.
	 *
	 * @param p Point
	 * @return Closest point on or in the AABB
	 */
	vec_type closest_point(vec_type const &p) const {
		assert(all_less_equal(lo, hi));
		vec_type r(p);
		for (size_t n=0; n<dimensions; n++) {
			if (p.i[n] < lo.i[n]) r.i[n] = lo.i[n];
			else if (p.i[n] > hi.i[n]) r.i[n] = hi.i[n];
		}
		return r;
	}

	/**
	 * @brief Extend AABB to include a point
	 *
	 * Grows the AABB if necessary so that it contains the given point.
	 */
	void extend(vec_type const &v) {
		for (size_t n=0; n<dimensions; n++) {
			if (lo.i[n] > v.i[n]) lo.i[n] = v.i[n];
			if (hi.i[n] < v.i[n]) hi.i[n] = v.i[n];
		}
	}

	/**
	 * @brief Extend AABB to include another AABB
	 *
	 * Grows this AABB to be the union of both boxes.  An empty box holds nothing, so it adds nothing.
	 */
	void extend(aabb const &b) {
		if (b.is_empty()) return;
		extend(b.lo);
		extend(b.hi);
	}

	/**
	 * @brief Test if point is inside AABB
	 *
	 * @return true if point is inside or on the boundary
	 */
	bool contains(vec_type const &right) const {
		return all_less_equal(lo, right) && all_less_equal(right, hi);
	}

	/**
	 * @brief Test if another AABB is fully contained
	 *
	 * @return true if right is completely inside this AABB
	 */
	bool contains(aabb const &right) const {
		return contains(right.lo) && contains(right.hi);
	}

	/**
	 * @brief Test intersection with another AABB
	 *
	 * @return true if the AABBs overlap (including touching)
	 */
	bool test_intersection(aabb const &right) const {
		return all_greater_equal(right.hi, lo) && all_greater_equal(hi, right.lo);
	}

	/**
	 * @brief Ray trace against AABB with intersection times
	 *
	 * Finds the part of [t_min, t_max] the ray spends in the box, the faces included.  A ray that starts inside
	 * entered at t_min, and one still inside at t_max leaves at t_max.  A ray with no direction is in the box or not.
	 *
	 * @param[out] t0 Time of entry, t_min or later (t_max on a miss)
	 * @param[out] t1 Time of exit, t_max or sooner (t_min on a miss)
	 * @param t_min Minimum t value to consider (default 0)
	 * @param t_max Maximum t value to consider (default 1)
	 * @return true if ray meets the AABB in [t_min, t_max]
	 */
	[[nodiscard]] bool trace(ray_type const &r, T *t0, T *t1, T t_min = 0, T t_max = 1) const;

	/**
	 * @brief Ray trace against AABB (intersection test only)
	 *
	 * @param t_min Minimum t value to consider (default 0)
	 * @param t_max Maximum t value to consider (default 1)
	 * @return true if ray intersects AABB in [t_min, t_max]
	 */
	[[nodiscard]] bool trace(ray_type const &r, T t_min = 0, T t_max = 1) const;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & lo & hi;
	}
};

/// The box of the points from `first` to `last`; empty for none
template<class It>
auto bounds_of(It first, It last)
{
	using vec_type = std::remove_cvref_t<decltype(*first)>;
	using scalar_type = std::remove_cvref_t<decltype(first->i[0])>;
	aabb<vec_type::dimensions, scalar_type> b;
	for (; first != last; ++first)
		b.extend(*first);
	return b;
}

/**
 * @brief The box of a box moved by an affine matrix: the box of its eight corners, from the matrix's entries (Arvo)
 *
 * An empty box stays empty.  A matrix whose last row is not (0, 0, 0, 1) moves a box into no box: std::invalid_argument.
 */
template<class T>
aabb<3, T> transform_aabb(aabb<3, T> const &box, matrix<4, 4, T> const &m)
{
	using std::abs;
	T const loose = T(1) / T(100000);
	m.check_affine("rove: transform_aabb by a matrix");
	if (box.is_empty()) return box;
	aabb<3, T> r;
	for (int i = 0; i < 3; ++i) {
		r.lo.i[i] = r.hi.i[i] = m.ij[3][i];
		for (int j = 0; j < 3; ++j) {
			T const a = m.ij[j][i] * box.lo.i[j], b = m.ij[j][i] * box.hi.i[j];
			r.lo.i[i] += std::min(a, b);
			r.hi.i[i] += std::max(a, b);
		}
	}
	return r;
}

/**
 * @brief Stream output operator for AABB
 *
 * Outputs AABB in format "lo x hi"
 */
template<int N, class T>
std::ostream &operator <<(std::ostream &lhs, aabb<N,T> const &rhs)
{
	return lhs << rhs.lo << " x " << rhs.hi;
}

}
