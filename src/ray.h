// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <limits>
#include <stdexcept>
#include <string>
#include "scalar.h"
#include "vec.h"

namespace rove {

template<int N,class T=scalar>
class [[nodiscard]] ray {
public:
	static constexpr size_t dimensions = N;
	using vec_type = vec<dimensions,T>;

	vec_type r0, a;

	ray() {
	}

	ray(vec_type const &origin, vec_type const &direction):
		r0(origin),
		a(direction)
	{
	}

	/// @brief The ray from @p origin along @p direction made unit length, so that its t is a distance
	/// @throws std::invalid_argument for an origin that is not finite, or a direction direction() refuses (not finite, or too short
	///         to have one)
	static ray unit(vec_type const &origin, vec_type const &direction)
	{
		origin.check_finite("rove: ray::unit from an origin");
		return ray(origin, rove::direction(direction));
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where its origin or its direction is not
	ray const &check_finite(char const *what) const
	{
		r0.check_finite(what);
		a.check_finite(what);
		return *this;
	}

	void set(vec_type const &origin,vec_type const &direction) {
		r0 = origin;
		a = direction;
	}

	vec_type apply(T t) const
	{
		return r0 + a * t;
	}

	bool contains(vec_type const &point, T epsilon = scalar_traits<T>::default_epsilon) const
	{
		return (point - r0).is_collinear(a, epsilon);
	}

	T distance_sq(vec_type const &point) const
	{
		T t = dot(point - r0, a) / a.length_sq();
		vec_type projection = apply(t);
		return (projection - point).length_sq();
	}

	T distance(vec_type const &point) const
	{
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		return sqrt(distance_sq(point));
	}

	/// @brief The distance between the two lines: zero where they cross, the closest approach where they are skew,
	/// and how far apart they are where they are parallel; a ray of no direction is the point it starts at
	T distance(ray const &r) const;

	/// @brief Whether the two lines meet: distance(r) < epsilon
	bool test_intersection(ray const &r, T epsilon = scalar_traits<T>::default_epsilon) const
	{
		return distance(r) < epsilon;
	}

	// intersection point is apply(t), where the lines come within epsilon of each other; parallel lines, the same
	// line among them, have no one point and give false
	bool query_intersection(ray const &r, T &t, T epsilon = scalar_traits<T>::default_epsilon) const;
};

}

