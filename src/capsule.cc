// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "aabb.h"
#include "capsule.h"
#include "intersect.h"
#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif

namespace rove
{

template<int N, class T>
bool capsule<N, T>::test_intersection(triangle<dimensions, T> const &triangle_shape) const
{
	static_assert(N == 3, "capsule::test_intersection(triangle) is only implemented for 3D capsules");
	return capsule_intersects_triangle(*this, triangle_shape);
}

template<int N, class T>
bool capsule<N, T>::test_intersection(capsule const &other) const
{
	static_assert(N == 3, "capsule::test_intersection(capsule) is only implemented for 3D capsules");
	if (radius < T(0) || other.radius < T(0)) return false;
	T combined_radius = radius + other.radius;
	return segment_segment_distance_sq(axe, other.axe) <= combined_radius * combined_radius;
}

template class capsule<3>;
template bool capsule<3, double>::test_intersection(triangle<3, double> const &triangle_shape) const;
template bool capsule<3, double>::test_intersection(capsule<3, double> const &other) const;
#ifdef ROVE_DFLOAT
template bool capsule<3, dfloat>::test_intersection(capsule<3, dfloat> const &other) const;
template bool capsule<3, dfloat>::test_intersection(triangle<3, dfloat> const &triangle_shape) const;
#endif

}
