// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include "vec.h"
#include "plane.h"
#include "matrix.h"

namespace rove
{

template<int N, class T> class aabb;
template<int N, class T> class obb;
template<int N, class T> class sphere;

// this class is specialized for frustum culling
template<class T = scalar>
class [[nodiscard]] frustum {
public:
	static constexpr size_t dimensions = 3;
	static constexpr size_t planes_count = 6;

	using vec_type = vec<dimensions, T>;
	using aabb_type = aabb<dimensions, T>;
	using obb_type = obb<dimensions, T>;
	using plane_type = plane<T>;
	using matrix_type = matrix<4,4,T>;

	enum {
		left_plane,
		right_plane,
		top_plane,
		bottom_plane,
		near_plane,
		far_plane,
	};

	plane_type planes[planes_count];

	frustum();
	explicit frustum(matrix_type const &tf);
	~frustum();

	void load(matrix_type const &tf);

	bool contains(vec_type const &point) const;
	bool test_intersection(aabb_type const &bounds) const;
	bool test_intersection(obb_type const &bounds) const;
	/// Whether the sphere reaches inside every plane (it may still miss a corner of the frustum, as the box tests may)
	bool test_intersection(sphere<dimensions, T> const &bounds) const;
};

}
