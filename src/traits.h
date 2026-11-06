// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file traits.h
 * @brief How a spatial index (octree, hash_grid) sees a primitive
 *
 * spatial_traits<P> gives a stored primitive's box and tests two of them;
 * spatial_query_traits<S, Q> gives a query's box and tests it against a
 * stored primitive.  The indices prune by boxes and ask the query traits for
 * the final answer.  Specialize either for a primitive of your own; both
 * indices take them as template arguments.
 *
 * Against a stored triangle: a box meets it when the box meets the
 * triangle's bounds, a segment or a sphere when it touches the triangle
 * itself, a cone when it may (never missing a triangle that has a point in
 * the cone, sometimes reporting one that has none).
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <limits>
#include <type_traits>
#include "aabb.h"
#include "cone.h"
#include "segment.h"
#include "sphere.h"
#include "triangle.h"
#include "vec.h"

namespace rove
{

template<class Primitive>
struct spatial_traits;

template<class StoredPrimitive, class QueryPrimitive>
struct spatial_query_traits;

/// What an index (octree, hash_grid) needs of the traits of what it stores: the types, and the box of a primitive
template<class Traits, class Primitive>
concept spatial_traits_for = requires(Primitive const &primitive) {
	typename Traits::scalar_type;
	typename Traits::vec_type;
	typename Traits::aabb_type;
	{ Traits::bounds(primitive) } -> std::convertible_to<typename Traits::aabb_type>;
};

/// What a query of an index needs of its traits: the query's box, and whether it meets a stored primitive
template<class QueryTraits, class StoredPrimitive, class Query>
concept spatial_query_traits_for = requires(StoredPrimitive const &stored, Query const &query) {
	QueryTraits::bounds(query);
	{ QueryTraits::intersects(stored, query) } -> std::convertible_to<bool>;
};

/// The types every 3D spatial_traits names
template<class T>
struct spatial_traits_3 {
	using scalar_type = T;
	using vec_type = vec<3, T>;
	using aabb_type = aabb<3, T>;
};

template<class T>
struct spatial_traits<aabb<3, T>>: spatial_traits_3<T> {
	using primitive_type = aabb<3, T>;

	static aabb<3, T> bounds(primitive_type const &primitive)
	{
		return primitive;
	}

	static bool intersects(primitive_type const &left, primitive_type const &right)
	{
		return left.test_intersection(right);
	}
};

template<class T>
struct spatial_traits<sphere<3, T>>: spatial_traits_3<T> {
	using primitive_type = sphere<3, T>;

	static aabb<3, T> bounds(primitive_type const &primitive)
	{
		return primitive.bounds();
	}

	static bool intersects(primitive_type const &left, primitive_type const &right)
	{
		return left.test_intersection(right);
	}
};

template<class T>
struct spatial_traits<triangle<3, T>>: spatial_traits_3<T> {
	using primitive_type = triangle<3, T>;

	/// Min and max per axis; NaN on an axis where a vertex is NaN (aabb::extend
	/// would drop a NaN in B or C and hand the triangle a plausible box).
	static aabb<3, T> bounds(primitive_type const &primitive)
	{
		aabb<3, T> result;
		for (int k = 0; k < 3; ++k) {
			T const a = primitive.A.i[k], b = primitive.B.i[k], c = primitive.C.i[k];
			if (detail::isnan_of(a) || detail::isnan_of(b) || detail::isnan_of(c)) {
				result.lo.i[k] = result.hi.i[k] = std::numeric_limits<T>::quiet_NaN();
				continue;
			}
			result.lo.i[k] = std::fmin(std::fmin(a, b), c);
			result.hi.i[k] = std::fmax(std::fmax(a, b), c);
		}
		return result;
	}

	static bool intersects(primitive_type const &left, primitive_type const &right)
	{
		return left.test_intersection(right);
	}
};

template<class Primitive>
struct spatial_query_traits<Primitive, Primitive> {
	using primitive_traits_type = spatial_traits<Primitive>;
	using aabb_type = typename primitive_traits_type::aabb_type;

	static aabb_type bounds(Primitive const &query)
	{
		return primitive_traits_type::bounds(query);
	}

	static bool intersects(Primitive const &stored, Primitive const &query)
	{
		return primitive_traits_type::intersects(stored, query);
	}
};

template<class T>
struct spatial_query_traits<triangle<3, T>, aabb<3, T>> {
	static aabb<3, T> bounds(aabb<3, T> const &query)
	{
		return query;
	}

	static bool intersects(triangle<3, T> const &stored, aabb<3, T> const &query)
	{
		return spatial_traits<triangle<3, T>>::bounds(stored).test_intersection(query);
	}
};

template<class T>
struct spatial_query_traits<triangle<3, T>, sphere<3, T>> {
	static aabb<3, T> bounds(sphere<3, T> const &query)
	{
		return query.bounds();
	}

	/// Exact: some point of the triangle within the radius.  A triangle with
	/// a NaN meets no sphere.
	static bool intersects(triangle<3, T> const &stored, sphere<3, T> const &query)
	{
		return (stored.closest_point(query.centre) - query.centre).length_sq() <= query.radius * query.radius;
	}
};

template<class T>
struct spatial_query_traits<triangle<3, T>, segment<3, T>> {
	static aabb<3, T> bounds(segment<3, T> const &query)
	{
		return query.bounds();
	}

	// Exact triangle/segment test (not an AABB approximation): a segment
	// from query.A to query.B, i.e. ray parameter t in [0, 1] reaches
	// query.B exactly. Used for lidar-style bounded raycasts, where the
	// segment length is origin-to-max_distance.
	static bool intersects(triangle<3, T> const &stored, segment<3, T> const &query)
	{
		return stored.trace(query.to_ray(), T(0), T(1));
	}
};

template<class T>
struct spatial_query_traits<triangle<3, T>, cone<3, T>> {
	static aabb<3, T> bounds(cone<3, T> const &query)
	{
		return query.bounds();
	}

	/**
	 * Conservative: false only when the triangle is certainly outside the
	 * cone, behind the apex, past the range, or off to a side.  The side
	 * test is a sphere around the triangle against the plane that touches
	 * the cone along the line nearest the sphere's centre; the cone lies
	 * wholly on one side of that plane.  A triangle with a NaN meets no cone.
	 */
	static bool intersects(triangle<3, T> const &stored, cone<3, T> const &query)
	{
		vec<3, T> const &axis = query.direction;
		T const a = dot(stored.A - query.apex, axis), b = dot(stored.B - query.apex, axis), c = dot(stored.C - query.apex, axis);
		if ((a < 0 && b < 0 && c < 0) || (a > query.range && b > query.range && c > query.range))
			return false;

		vec<3, T> const centre = (stored.A + stored.B + stored.C) / T(3);
		T const radius_sq = (std::max)({(stored.A - centre).length_sq(), (stored.B - centre).length_sq(), (stored.C - centre).length_sq()});
		vec<3, T> const v = centre - query.apex;
		T const along = dot(v, axis), across = (v - axis * along).length();
		// cone::contains squares the cosine: a half-angle of at most a quarter turn
		using std::sqrt;
		T const cos_half = abs(query.cos_half_angle), sin_half = sqrt((std::max)(T(0), T(1) - cos_half * cos_half));
		T const outside = cos_half * across - sin_half * along;      // the centre's distance past the touching plane
		return outside <= 0 || outside * outside <= radius_sq;
	}
};

/// The query traits an index over Primitive, seen through Traits, tests a
/// Query with when given none: Traits itself for a query of the stored
/// type, so an index with traits of its own keeps them for such a query.
template<class Primitive, class Traits, class Query>
using default_query_traits = std::conditional_t<std::is_same_v<Query, Primitive>, Traits, spatial_query_traits<Primitive, Query>>;

/// The test hash_grid::trace and octree::trace take: `bool test(item, ray, t_min, t_max, contact)`, true for a hit in
/// [t_min, t_max] with the contact's time, position and normal filled
template<class Test, class Item, class Ray, class Scalar, class Contact>
concept trace_test = std::invocable<Test &, Item const &, Ray const &, Scalar, Scalar, Contact &> &&
	std::convertible_to<std::invoke_result_t<Test &, Item const &, Ray const &, Scalar, Scalar, Contact &>, bool>;

/**
 * @brief The test hash_grid::trace and octree::trace run on an item when
 *        given none
 *
 * The primitive's own `trace(ray, t, point, t_min, t_max)` and
 * `normal()`, the normal turned to face against the ray.
 */
struct trace_primitive {
	template<class Item, class Ray, class Scalar, class Contact>
	bool operator()(Item const &item, Ray const &along, Scalar t_min, Scalar t_max, Contact &ci) const
	{
		if (!item.primitive.trace(along, ci.time, ci.position, t_min, t_max))
			return false;
		ci.normal = item.primitive.normal();
		if (dot(ci.normal, along.a) > 0)
			ci.normal = -ci.normal;
		return true;
	}
};

}
