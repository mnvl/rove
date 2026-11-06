// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cstring>
#include "vec.h"

namespace rove
{

template<class T,class S> void
vec<3,T,S>::to_axis_angle(vec &axis, T &angle) const
{
	vec const original(1,0,0);

	axis = normalize_or_zero(cross(original, *this));

	vec basis_y = cross(axis, original);

	vec<2,T> xy;
	xy.x = x / length();
	xy.y = dot(*this, basis_y);

	T t;
	xy.to_polar(t, angle);
}

template<class T,class S> vec<3,T,S>
vec<3,T,S>::perpendicular_xy() const
{
	if (abs(x) + abs(y) < scalar_traits<T>::zero_length) return vec(0, 0, 1);
	return vec(y, -x, 0);
}

template<class T,class S> vec<3,T,S>
vec<3,T,S>::perpendicular_yz() const
{
	if (abs(y) + abs(z) < scalar_traits<T>::zero_length) return vec(1, 0, 0);
	return vec(0, z, -y);
}

template<class T,class S> vec<3,T,S>
vec<3,T,S>::perpendicular_xz() const
{
	if (abs(x) + abs(z) < scalar_traits<T>::zero_length) return vec(0, 1, 0);
	return vec(z, 0, -x);
}

template class vec<2>;
template class vec<3>;
template class vec<4>;

}
