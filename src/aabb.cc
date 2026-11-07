// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "ray.h"
#include "aabb.h"
#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif

namespace rove
{

template<int N, class T>
bool aabb<N, T>::trace(ray_type const &r, T *t0, T *t1, T t_min, T t_max) const
{
	*t0 = t_max;
	*t1 = t_min;

	if (is_empty()) return false;

	// The slab of each axis cuts the interval down.  In the box's unit space where it has a size; a box flat on an axis has its one
	// plane there.  (Written so that a NaN in the ray is a miss.)
	T enter = t_min, leave = t_max;
	for (size_t axis = 0; axis < dimensions; ++axis) {
		T const size = hi.i[axis] - lo.i[axis];
		T t_a, t_b;
		if (size > T(0)) {
			T const o = (r.r0.i[axis] - lo.i[axis]) / size, d = r.a.i[axis] / size;
			if (!(abs(d) > scalar_traits<T>::zero_length)) {
				if (!(o >= T(0) && o <= T(1))) return false;
				continue;
			}
			T const inverse = T(1) / d;
			t_a = -o * inverse;
			t_b = inverse + t_a;
			if (t_a > t_b) { T const swap = t_a; t_a = t_b; t_b = swap; }
		} else {
			T const o = r.r0.i[axis] - lo.i[axis], d = r.a.i[axis];
			if (d == T(0)) {
				if (!(o == T(0))) return false;
				continue;
			}
			t_a = t_b = -o / d;
		}
		if (t_a > enter) enter = t_a;
		if (t_b < leave) leave = t_b;
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

#ifdef ROVE_DFLOAT
template class aabb<2, dfloat>;
template class aabb<3, dfloat>;
#endif

}
