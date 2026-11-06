// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file sphere.h
 * @brief N-dimensional sphere (hypersphere) class
 *
 * Provides sphere primitives for collision detection and spatial queries.
 * Commonly used specializations are 2D circles (N=2) and 3D spheres (N=3).
 */

#pragma once

#include <cassert>
#include "scalar.h"
#include "vec.h"
#include "aabb.h"

namespace rove
{

template<int N, class T> class ray;
template<int N, class T> class segment;
template<int N, class T> class capsule;

/**
 * @brief N-dimensional sphere (circle in 2D, sphere in 3D)
 *
 * Represents a sphere defined by a center point and radius.
 * Provides containment tests, intersection tests with other primitives,
 * and AABB conversion.
 *
 * @tparam N Dimensionality (2 for circle, 3 for sphere)
 */
template<int N, class T = scalar>
class [[nodiscard]] sphere {
public:
	static constexpr size_t dimensions = N;  ///< Number of dimensions
	using vec_type = vec<dimensions,T>;  ///< Associated vector type

	vec_type centre;      ///< Center point of the sphere
	T radius;   ///< Radius of the sphere

	/// A sphere of no radius at the origin (it was left uninitialised)
	sphere():
		radius(0)
	{
	}

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & centre & radius;
	}

	/**
	 * @brief Construct bounding sphere from AABB
	 *
	 * Creates the smallest sphere that contains the given axis-aligned bounding box.
	 */
	explicit sphere(aabb<N, T> const &a)
	{
		construct(a);
	}

	/**
	 * @brief Construct sphere from center and radius
	 *
	 * @param c Center point
	 * @param r Radius
	 */
	sphere(vec_type const &c, T r):
		centre(c),
		radius(r)
	{
	}

	/**
	 * @brief Initialize sphere from AABB
	 *
	 * Sets this sphere to be the smallest sphere containing the AABB.
	 * The center is at the AABB center, radius reaches the furthest corner.
	 */
	void construct(aabb<N, T> const &a)
	{
		centre = (a.lo + a.hi) / 2;
		radius = (a.hi - centre).length();
	}

	/**
	 * @brief Set sphere center and radius
	 *
	 * @param c New center point
	 * @param r New radius
	 */
	void set(vec_type const &c, T r)
	{
		centre = c;
		radius = r;
	}

	/**
	 * @brief Get axis-aligned bounding box containing this sphere
	 *
	 * Returns the smallest AABB that fully contains this sphere.
	 *
	 * @return Bounding box
	 */
	aabb<N, T> bounds() const
	{
		assert(radius >= 0);

		vec_type r;
		r.set_all(radius);
		return aabb<N, T>(centre - r, centre + r);
	}

	/**
	 * @brief Test if point is inside sphere
	 *
	 * @return true if point is inside or on the sphere surface
	 */
	bool contains(vec_type const &v) const
	{
		return (v - centre).length_sq() <= radius * radius;
	}

	/**
	 * @brief Test intersection with another sphere
	 *
	 * @return true if spheres intersect or touch
	 */
	bool test_intersection(sphere<N, T> const &s) const
	{
		return (centre - s.centre).length_sq() <= square(radius + s.radius);
	}

	/**
	 * @brief Test intersection with ray
	 *
	 * @return true if ray intersects sphere
	 */
	bool test_intersection(ray<N, T> const &r) const;

	/**
	 * @brief Query intersection points with ray
	 *
	 * @param[out] p1 First intersection point
	 * @param[out] p2 Second intersection point
	 * @return Number of intersection points (0, 1, or 2)
	 */
	int query_intersection(ray<N, T> const &r, vec_type &p1, vec_type &p2) const;

	/**
	 * @brief Test intersection with line segment
	 *
	 * @return true if line segment intersects sphere
	 */
	bool test_intersection(segment<N, T> const &l) const;

	/**
	 * @brief Query intersection points with line segment
	 *
	 * @param[out] p1 First intersection point
	 * @param[out] p2 Second intersection point
	 * @return Number of intersection points (0, 1, or 2)
	 */
	int query_intersection(segment<N, T> const &l, vec_type &p1, vec_type &p2) const;

	/**
	 * @brief Test intersection with capsule
	 *
	 * @return true if sphere intersects capsule
	 */
	bool test_intersection(capsule<N, T> const &c) const;
};

}
