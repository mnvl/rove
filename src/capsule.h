// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include "segment.h"

namespace rove
{

template<int N, class T> class aabb;
template<int N, class T> class obb;
template<int N, class T> class triangle;

template<int N, class T = scalar>
class [[nodiscard]] capsule {
public:
	static constexpr size_t dimensions = N;
	using vec_type = vec<dimensions, T>;
	using segment_type = segment<dimensions, T>;

	segment_type axe;
	T radius;

	/// A capsule of no radius around the origin (it was left uninitialised)
	capsule():
		radius(0)
	{
	}

	capsule(vec_type const &a, vec_type const &b, T r):
		axe(a, b),
		radius(r)
	{
	}

	bool contains(vec_type const &point) const
	{
		return axe.distance(point) <= radius;
	}

	T distance(vec_type const &point) const
	{
		T d = axe.distance(point);
		if (d <= radius) return 0;
		return d - radius;
	}

	/// @brief The box around the capsule
	aabb<N, T> bounds() const;

	bool test_intersection(triangle<dimensions, T> const &triangle) const;
	bool test_intersection(capsule const &other) const;
};

}
