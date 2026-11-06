// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cassert>
#include "aabb.h"
#include "obb.h"
#include "plane.h"

namespace rove
{

template<class T> typename plane<T>::vec_type
plane<T>::origin() const
{
	// Ax + By + Cz + D = 0 => A(x + D/A) + By + Cz = 0
	// A(x - x0) + B(y - y0) + C(z - z0) = 0
	// x0 = -D / A

	if (abs(A) > scalar_traits<T>::zero_length) return vec_type(-D / A, 0, 0);
	if (abs(B) > scalar_traits<T>::zero_length) return vec_type(0, -D / B, 0);
	if (abs(C) > scalar_traits<T>::zero_length) return vec_type(0, 0, -D / C);

	assert(0 && "does not define a plane");

	return vec_type(0, 0, 0);
}

template<class T> bool
plane<T>::query_intersection(plane const &p, ray_type &r) const {
	r.a = cross(vec_type(A,B,C), vec_type(p.A, p.B, p.C));
	vec_type dir2 = r.a * r.a;

	T invdet;
	if (dir2.z > dir2.y && dir2.z > dir2.x && dir2.z > scalar_traits<T>::zero_length_sq)
	{
		/* then get a point on the XY plane */
		invdet = T(1) / r.a.z;
		/* solve < pl1.x * xpt.x + pl1.y * xpt.y = - pl1.w >
		< pl2.x * xpt.x + pl2.y * xpt.y = - pl2.w > */
		r.r0.x = B * p.D - p.B *D;
		r.r0.y = p.A * D - A * p.D;
		r.r0.z = 0;
	}
	else if (dir2.y > dir2.x && dir2.y > scalar_traits<T>::zero_length_sq)
	{
		/* then get a point on the XZ plane */
		invdet = T(1) / r.a.y;
		/* solve < pl1.x * xpt.x + pl1.z * xpt.z = -pl1.w >
		< pl2.x * xpt.x + pl2.z * xpt.z = -pl2.w > */
		r.r0.x = C * p.D - p.C * D;
		r.r0.y = 0;
		r.r0.z = p.A * D -A * p.D;
	}
	else if (dir2.x > scalar_traits<T>::zero_length_sq)
	{
		/* then get a point on the YZ plane */
		invdet = T(1) / r.a.x;
		/* solve < pl1.y * xpt.y + pl1.z * xpt.z = - pl1.w >
		< pl2.y * xpt.y + pl2.z * xpt.z = - pl2.w > */
		r.r0.x = 0;
		r.r0.y = C * p.D - p.C * D;
		r.r0.z = p.B * D -B * p.D;
	}
	else return false;

	r.r0 *= invdet;

	invdet = T(1) / sqrt(dir2.x + dir2.y + dir2.z);
	r.a *= invdet;

	return true;
}

template<class T> typename plane<T>::classification
plane<T>::classify(vec_type const &point, T epsilon) const
{
	T value = apply(point);
	if (abs(value) < epsilon) return intersects;
	return value > 0 ? positive : negative;
}

template<class T> typename plane<T>::classification
plane<T>::classify(aabb<dimensions, T> const &bounds, T epsilon) const
{
	return classify(obb<dimensions, T>(bounds), epsilon);
}

template<class T> typename plane<T>::classification
plane<T>::classify(obb<dimensions, T> const &bounds, T epsilon) const
{
	bool above = false, below = false;

	for (size_t i = 0; i < obb<dimensions, T>::vertices_count; ++i)
	{
		vec_type vertex;
		vertex = bounds.vertex(i);

		switch (classify(vertex, epsilon)) {
		case positive: above = true; break;
		case negative: below = true; break;
		default: return intersects;
		};

		if (above && below) return intersects;
	}

	if (above) return positive;

	assert(below);
	return negative;
}

template class plane<>;
template class plane<double>;

}
