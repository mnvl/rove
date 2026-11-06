// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file cone.h
 * @brief 3D cone primitive for spatial queries (e.g. spot-light culling)
 *
 * A cone is defined by an apex, a normalized direction, a cosine half-angle,
 * and a range (length along the axis).
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include "scalar.h"
#include "vec.h"
#include "aabb.h"

namespace rove
{

template<int N, class T = scalar>
class [[nodiscard]] cone {
public:
	using vec_type = vec<N, T>;

	vec_type apex;            ///< Tip of the cone
	vec_type direction;       ///< Normalized axis direction
	T cos_half_angle;      ///< Cosine of the half-angle
	T range;               ///< Maximum distance from apex along the axis

	/// Every number zero (it was left uninitialised)
	cone():
		cos_half_angle(0),
		range(0)
	{
	}

	cone(vec_type const &apex, vec_type const &direction, T cos_half_angle, T range):
		apex(apex),
		direction(direction),
		cos_half_angle(cos_half_angle),
		range(range)
	{
	}

	/**
	 * @brief Test if a point is inside the cone
	 *
	 * A point is inside if it is within range of the apex and the angle
	 * between (point - apex) and the cone axis is less than the half-angle.
	 */
	bool contains(vec_type const &point) const
	{
		vec_type delta = point - apex;
		T along = dot(delta, direction);  // dot product
		if (along < T(0) || along > range)
			return false;
		// cos(angle) = along / |delta|
		// cos(angle) >= cos_half_angle
		// along^2 >= cos_half_angle^2 * |delta|^2  (both sides non-negative)
		T dist_sq = delta.length_sq();
		return along * along >= cos_half_angle * cos_half_angle * dist_sq;
	}

	/// @brief Get a conservative axis-aligned bounding box for the cone
	///
	/// The base's radius is range * tan(half angle), and contains() goes as far as it.  A half angle of 90 degrees or more has a
	/// base of no finite size: the box is infinite across the axis.
	aabb<N, T> bounds() const
	{
		T const cos_abs = abs(cos_half_angle);
		T const base_radius = (cos_abs > T(0)
			? range * (sqrt(std::max(T(0), T(1) - cos_half_angle * cos_half_angle)) / cos_abs)
			: std::numeric_limits<T>::infinity());
		vec_type base_center = apex + direction * range;

		vec_type r;
		r.set_all(base_radius);

		aabb<N, T> result(apex);
		result.extend(base_center - r);
		result.extend(base_center + r);
		return result;
	}
};

}
