// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include "vec.h"
#include "segment.h"
#include "triangle.h"
#include "capsule.h"

namespace rove
{

template<class T>
T point_triangle_distance_sq(vec<3, T> const &point, triangle<3, T> const &tri);

template<class T>
T segment_segment_distance_sq(segment<3, T> const &lhs, segment<3, T> const &rhs);

template<class T>
bool segment_intersects_triangle(segment<3, T> const &segment, triangle<3, T> const &tri);

template<class T>
bool segment_intersects_triangle(vec<3, T> const &a, vec<3, T> const &b, triangle<3, T> const &tri);

template<class T>
bool capsule_intersects_triangle(capsule<3, T> const &capsule_shape, triangle<3, T> const &tri);

}
