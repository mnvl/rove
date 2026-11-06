// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif
#include "triangle.h"
#include "intersect.h"

#include <algorithm>
#include <cmath>

namespace rove
{

namespace
{

template<class T>
struct point2
{
	T x;
	T y;
};

template<class T>
aabb<3, T> triangle_bounds(triangle<3, T> const &tri)
{
	aabb<3, T> result(tri.A);
	result.extend(tri.B);
	result.extend(tri.C);
	return result;
}

template<class T>
point2<T> project_to_2d(vec<3, T> const &v, unsigned drop_axis)
{
	if (drop_axis == 0) {
		return point2<T>{v.y, v.z};
	}
	if (drop_axis == 1) {
		return point2<T>{v.x, v.z};
	}
	return point2<T>{v.x, v.y};
}

template<class T>
T orient_2d(point2<T> const &a, point2<T> const &b, point2<T> const &c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

template<class T>
bool on_segment_2d(point2<T> const &a, point2<T> const &b, point2<T> const &p)
{
	T const zero = scalar_traits<T>::zero_length;
	if (abs(orient_2d(a, b, p)) > zero) {
		return false;
	}

	return
		p.x >= std::min(a.x, b.x) - zero &&
		p.x <= std::max(a.x, b.x) + zero &&
		p.y >= std::min(a.y, b.y) - zero &&
		p.y <= std::max(a.y, b.y) + zero;
}

template<class T>
bool segments_intersect_2d(point2<T> const &a, point2<T> const &b, point2<T> const &c, point2<T> const &d)
{
	T const zero = scalar_traits<T>::zero_length;
	T const o1 = orient_2d(a, b, c);
	T const o2 = orient_2d(a, b, d);
	T const o3 = orient_2d(c, d, a);
	T const o4 = orient_2d(c, d, b);

	bool const opposite_1 = (o1 > zero && o2 < -zero) || (o1 < -zero && o2 > zero);
	bool const opposite_2 = (o3 > zero && o4 < -zero) || (o3 < -zero && o4 > zero);
	if (opposite_1 && opposite_2) {
		return true;
	}

	return
		(abs(o1) <= zero && on_segment_2d(a, b, c)) ||
		(abs(o2) <= zero && on_segment_2d(a, b, d)) ||
		(abs(o3) <= zero && on_segment_2d(c, d, a)) ||
		(abs(o4) <= zero && on_segment_2d(c, d, b));
}

template<class T>
bool point_in_triangle_2d(point2<T> const &p, point2<T> const &a, point2<T> const &b, point2<T> const &c)
{
	T const zero = scalar_traits<T>::zero_length;
	T const o1 = orient_2d(a, b, p);
	T const o2 = orient_2d(b, c, p);
	T const o3 = orient_2d(c, a, p);

	bool const has_negative = (o1 < -zero) || (o2 < -zero) || (o3 < -zero);
	bool const has_positive = (o1 > zero) || (o2 > zero) || (o3 > zero);
	return !(has_negative && has_positive);
}

template<class T>
bool coplanar_intersects(triangle<3, T> const &left, triangle<3, T> const &right, vec<3, T> const &normal)
{
	unsigned drop_axis = 0;
	T const abs_x = abs(normal.x);
	T const abs_y = abs(normal.y);
	T const abs_z = abs(normal.z);
	if (abs_y >= abs_x && abs_y >= abs_z) {
		drop_axis = 1;
	} else if (abs_z >= abs_x && abs_z >= abs_y) {
		drop_axis = 2;
	}

	point2<T> const la = project_to_2d(left.A, drop_axis);
	point2<T> const lb = project_to_2d(left.B, drop_axis);
	point2<T> const lc = project_to_2d(left.C, drop_axis);
	point2<T> const ra = project_to_2d(right.A, drop_axis);
	point2<T> const rb = project_to_2d(right.B, drop_axis);
	point2<T> const rc = project_to_2d(right.C, drop_axis);

	if (
		point_in_triangle_2d(la, ra, rb, rc) ||
		point_in_triangle_2d(lb, ra, rb, rc) ||
		point_in_triangle_2d(lc, ra, rb, rc) ||
		point_in_triangle_2d(ra, la, lb, lc) ||
		point_in_triangle_2d(rb, la, lb, lc) ||
		point_in_triangle_2d(rc, la, lb, lc))
	{
		return true;
	}

	point2<T> const left_edges_start[3] = {la, lb, lc};
	point2<T> const left_edges_end[3] = {lb, lc, la};
	point2<T> const right_edges_start[3] = {ra, rb, rc};
	point2<T> const right_edges_end[3] = {rb, rc, ra};

	for (size_t i = 0; i < 3; ++i) {
		for (size_t j = 0; j < 3; ++j) {
			if (segments_intersect_2d(
				left_edges_start[i], left_edges_end[i],
				right_edges_start[j], right_edges_end[j]))
			{
				return true;
			}
		}
	}

	return false;
}

template<class T>
bool triangle_intersects_impl(triangle<3, T> const &left, triangle<3, T> const &right)
{
	T const zero_sq = scalar_traits<T>::zero_length_sq;
	aabb<3, T> const left_bounds = triangle_bounds(left);
	aabb<3, T> const right_bounds = triangle_bounds(right);
	if (!left_bounds.test_intersection(right_bounds)) {
		return false;
	}

	vec<3, T> const left_normal = cross(left.B - left.A, left.C - left.A);
	vec<3, T> const right_normal = cross(right.B - right.A, right.C - right.A);
	if (left_normal.length_sq() <= zero_sq || right_normal.length_sq() <= zero_sq) {
		return left_bounds.test_intersection(right_bounds);
	}

	typename triangle<3, T>::plane_type const left_plane = left.to_plane();
	if (left_plane.contains(right.A) && left_plane.contains(right.B) && left_plane.contains(right.C)) {
		return coplanar_intersects(left, right, left_normal);
	}

	return
		segment_intersects_triangle(left.A, left.B, right) ||
		segment_intersects_triangle(left.B, left.C, right) ||
		segment_intersects_triangle(left.C, left.A, right) ||
		segment_intersects_triangle(right.A, right.B, left) ||
		segment_intersects_triangle(right.B, right.C, left) ||
		segment_intersects_triangle(right.C, right.A, left) ||
		left.contains(right.A) ||
		left.contains(right.B) ||
		left.contains(right.C) ||
		right.contains(left.A) ||
		right.contains(left.B) ||
		right.contains(left.C);
}

} // namespace

template<class T>
bool triangle<3, T>::test_intersection(triangle const &t) const
{
	return triangle_intersects_impl(*this, t);
}

template bool triangle<3, scalar>::test_intersection(triangle<3, scalar> const &t) const;
template bool triangle<3, double>::test_intersection(triangle<3, double> const &t) const;
#ifdef ROVE_DFLOAT
template bool triangle<3, dfloat>::test_intersection(triangle<3, dfloat> const &t) const;
#endif

}
