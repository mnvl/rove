// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "ray.h"
#include "aabb.h"

namespace rove
{

template<int N, class T>
bool aabb<N, T>::trace(ray_type const &r, T *t0, T *t1, T t_min, T t_max) const
{
	*t0 = t_max;
	*t1 = t_min;

	// The slab of each axis cuts the interval down, in world units: a box flat on an axis has a slab all the same.
	T enter = t_min, leave = t_max;
	for (size_t axis = 0; axis < dimensions; ++axis) {
		T const o = r.r0.i[axis], d = r.a.i[axis];
		// (written so that a NaN in the box or the ray is a miss)
		if (d == T(0)) {
			if (!(o >= lo.i[axis] && o <= hi.i[axis])) return false;
			continue;
		}
		T const near_face = (d > T(0) ? lo.i[axis] : hi.i[axis]);
		T const far_face = (d > T(0) ? hi.i[axis] : lo.i[axis]);
		if (!(lo.i[axis] <= hi.i[axis])) return false;
		T const t_near = (near_face - o) / d, t_far = (far_face - o) / d;
		if (t_near > enter) enter = t_near;
		if (t_far < leave) leave = t_far;
		if (!(enter <= leave)) return false;
	}

	if (!(enter <= leave)) return false;
	*t0 = enter;
	*t1 = leave;
	return true;
}

template<int N, class T>
bool aabb<N, T>::trace(ray_type const &r, T t_min, T t_max) const
{
	T t0, t1;

	return trace(r, &t0, &t1, t_min, t_max);
}

template class aabb<2>;
template class aabb<3>;
template class aabb<2, double>;
template class aabb<3, double>;

}
