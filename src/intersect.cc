// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "intersect.h"

#include <algorithm>
#include <cmath>

#include "scalar.h"
#include "ray.h"
#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif

namespace rove
{

template<class T>
T point_triangle_distance_sq(vec<3, T> const &point, triangle<3, T> const &tri)
{
	vec<3, T> const ab = tri.B - tri.A;
	vec<3, T> const ac = tri.C - tri.A;
	vec<3, T> const ap = point - tri.A;

	T const d1 = dot(ab, ap);
	T const d2 = dot(ac, ap);
	if (d1 <= T(0) && d2 <= T(0)) {
		return (point - tri.A).length_sq();
	}

	vec<3, T> const bp = point - tri.B;
	T const d3 = dot(ab, bp);
	T const d4 = dot(ac, bp);
	if (d3 >= T(0) && d4 <= d3) {
		return (point - tri.B).length_sq();
	}

	T const vc = d1 * d4 - d3 * d2;
	if (vc <= T(0) && d1 >= T(0) && d3 <= T(0)) {
		T const v = d1 / (d1 - d3);
		vec<3, T> const projection = tri.A + ab * v;
		return (point - projection).length_sq();
	}

	vec<3, T> const cp = point - tri.C;
	T const d5 = dot(ab, cp);
	T const d6 = dot(ac, cp);
	if (d6 >= T(0) && d5 <= d6) {
		return (point - tri.C).length_sq();
	}

	T const vb = d5 * d2 - d1 * d6;
	if (vb <= T(0) && d2 >= T(0) && d6 <= T(0)) {
		T const w = d2 / (d2 - d6);
		vec<3, T> const projection = tri.A + ac * w;
		return (point - projection).length_sq();
	}

	T const va = d3 * d6 - d5 * d4;
	if (va <= T(0) && (d4 - d3) >= T(0) && (d5 - d6) >= T(0)) {
		vec<3, T> const bc = tri.C - tri.B;
		T const w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		vec<3, T> const projection = tri.B + bc * w;
		return (point - projection).length_sq();
	}

	vec<3, T> const normal = cross(ab, ac);
	T const normal_length_sq = normal.length_sq();
	if (normal_length_sq <= scalar_traits<T>::zero_length_sq) {
		T dA = (point - tri.A).length_sq();
		T dB = (point - tri.B).length_sq();
		T dC = (point - tri.C).length_sq();
		return std::min(dA, std::min(dB, dC));
	}

	T const distance_to_plane = dot(normal, ap);
	return (distance_to_plane * distance_to_plane) / normal_length_sq;
}

template<class T>
T segment_segment_distance_sq(segment<3, T> const &lhs, segment<3, T> const &rhs)
{
	vec<3, T> const d1 = lhs.B - lhs.A;
	vec<3, T> const d2 = rhs.B - rhs.A;
	vec<3, T> const r = lhs.A - rhs.A;

	T const a = dot(d1, d1);
	T const e = dot(d2, d2);
	T const f = dot(d2, r);
	T const zero_sq = scalar_traits<T>::zero_length_sq;

	T s = T(0);
	T t = T(0);

	if (a <= zero_sq && e <= zero_sq) {
		return (lhs.A - rhs.A).length_sq();
	}

	if (a <= zero_sq) {
		t = clamp(f / e);
	} else {
		T const c = dot(d1, r);
		if (e <= zero_sq) {
			s = clamp(-c / a);
		} else {
			T const b = dot(d1, d2);
			T const denom = a * e - b * b;
			using std::abs;
			if (abs(denom) > zero_sq) {
				s = clamp((b * f - c * e) / denom);
			}
			T const t_nom = b * s + f;
			if (t_nom < T(0)) {
				t = T(0);
				s = clamp(-c / a);
			} else if (t_nom > e) {
				t = T(1);
				s = clamp((b - c) / a);
			} else {
				t = t_nom / e;
			}
		}
	}

	vec<3, T> const closest_lhs = lhs.A + d1 * s;
	vec<3, T> const closest_rhs = rhs.A + d2 * t;
	return (closest_lhs - closest_rhs).length_sq();
}

template<class T>
bool segment_intersects_triangle(segment<3, T> const &segment, triangle<3, T> const &tri)
{
	vec<3, T> const direction = segment.B - segment.A;
	if (direction.length_sq() <= scalar_traits<T>::zero_length_sq) {
		return tri.contains(segment.A);
	}

	ray<3, T> const ray(segment.A, direction);
	T t;
	vec<3, T> point;
	return tri.trace(ray, t, point, T(0), T(1));
}

template<class T>
bool segment_intersects_triangle(vec<3, T> const &a, vec<3, T> const &b, triangle<3, T> const &tri)
{
	vec<3, T> const direction = b - a;
	if (direction.length_sq() <= scalar_traits<T>::zero_length_sq) {
		return tri.contains(a);
	}

	ray<3, T> const ray(a, direction);
	T t;
	vec<3, T> point;
	return tri.trace(ray, t, point, T(0), T(1));
}

template<class T>
bool capsule_intersects_triangle(capsule<3, T> const &capsule_shape, triangle<3, T> const &tri)
{
	if (capsule_shape.radius < T(0)) {
		return false;
	}

	if (segment_intersects_triangle(capsule_shape.axe, tri)) {
		return true;
	}

	T min_distance_sq = point_triangle_distance_sq(capsule_shape.axe.A, tri);
	min_distance_sq = std::min(min_distance_sq, point_triangle_distance_sq(capsule_shape.axe.B, tri));

	segment<3, T> const edges[3] = {
		segment<3, T>(tri.A, tri.B),
		segment<3, T>(tri.B, tri.C),
		segment<3, T>(tri.C, tri.A),
	};

	for (size_t i = 0; i < 3; ++i) {
		min_distance_sq = std::min(min_distance_sq, segment_segment_distance_sq(capsule_shape.axe, edges[i]));
	}

	vec<3, T> const axis = capsule_shape.axe.B - capsule_shape.axe.A;
	vec<3, T> const normal = cross(tri.B - tri.A, tri.C - tri.A);
	T const normal_length_sq = normal.length_sq();
	if (axis.length_sq() > scalar_traits<T>::zero_length_sq && normal_length_sq > scalar_traits<T>::zero_length_sq) {
		T const dist_a = dot(normal, capsule_shape.axe.A - tri.A);
		T const dist_b = dot(normal, capsule_shape.axe.B - tri.A);
		T t = T(0);
		if (std::abs(dist_a - dist_b) > scalar_traits<T>::zero_length) {
			t = clamp(dist_a / (dist_a - dist_b));
		} else {
			t = T(0.5);
		}

		vec<3, T> const axis_point = capsule_shape.axe.A + axis * t;
		min_distance_sq = std::min(min_distance_sq, point_triangle_distance_sq(axis_point, tri));
	}

	return min_distance_sq <= capsule_shape.radius * capsule_shape.radius;
}

template scalar point_triangle_distance_sq(vec<3, scalar> const &, triangle<3, scalar> const &);
template double point_triangle_distance_sq(vec<3, double> const &, triangle<3, double> const &);

template scalar segment_segment_distance_sq(segment<3, scalar> const &, segment<3, scalar> const &);
template double segment_segment_distance_sq(segment<3, double> const &, segment<3, double> const &);
#ifdef ROVE_DFLOAT
template dfloat segment_segment_distance_sq(segment<3, dfloat> const &, segment<3, dfloat> const &);
#endif

template bool segment_intersects_triangle(segment<3, scalar> const &, triangle<3, scalar> const &);
template bool segment_intersects_triangle(segment<3, double> const &, triangle<3, double> const &);

template bool segment_intersects_triangle(vec<3, scalar> const &, vec<3, scalar> const &, triangle<3, scalar> const &);
template bool segment_intersects_triangle(vec<3, double> const &, vec<3, double> const &, triangle<3, double> const &);

template bool capsule_intersects_triangle(capsule<3, scalar> const &, triangle<3, scalar> const &);
template bool capsule_intersects_triangle(capsule<3, double> const &, triangle<3, double> const &);

}
