// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "ray.h"
#include "segment.h"
#include "capsule.h"
#include "sphere.h"
#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif

namespace rove
{

template<int N, class T> bool sphere<N, T>::test_intersection(ray<N, T> const &r) const
{
	// The nearest point of the ray to the centre: at its origin when it points away or has no direction.
	vec_type const to_centre = centre - r.r0;
	T const along = dot(r.a, to_centre), a_sq = r.a.length_sq();
	vec_type const nearest = (along > T(0) && a_sq > T(0)) ? to_centre - r.a * (along / a_sq) : to_centre;
	return nearest.length_sq() <= square(radius);
}

template<int N, class T> int
sphere<N,T>::query_intersection(ray<N, T> const &r, vec_type &p1, vec_type &p2) const
{
	// p = r.direction * t + r.origin
	// |p - centre| ^ 2 = radius ^ 2
	// |r.direction * t + r.origin - centre| ^ 2 = radius ^ 2
	// delta = r.origin - centre
	// r.direction.length_sq() * t + 2 * (r.direction & delta) * t + delta.length_sq() - radius ^ 2 = 0

	vec_type delta = r.r0 - centre;
	T A = r.a.length_sq();
	T B = T(2) * dot(r.a, delta);
	T C = delta.length_sq() - square(radius);

	if (!(A > T(0))) return 0;          // no direction: no line to cross

	T discriminant = square(B) - 4 * A * C;

	if (discriminant < 0) {
		return 0;
	}

	// (the discriminant is 4 A (R^2 - d^2), d how far the line is from the centre: tangent is a question of those two)
	if (discriminant < 4 * A * square(radius) * scalar_traits<T>::zero_length_sq) {
		T t = -B / (2 * A);
		p1 = p2 = r.a * t + r.r0;
		return 1;
	}

	T discriminant_sqrt = sqrt(discriminant);

	T t1 = (-B + discriminant_sqrt) / (2 * A);
	p1 = r.a * t1 + r.r0;

	T t2 = (-B - discriminant_sqrt) / (2 * A);
	p2 = r.a * t2 + r.r0;

	return 2;
}

template<int N, class T> bool
sphere<N, T>::test_intersection(segment<N, T> const &l) const
{
	// A sphere is solid: a segment wholly inside it meets it.
	return (l.closest_point(centre) - centre).length_sq() <= square(radius);
}

template<int N, class T> int
sphere<N, T>::query_intersection(segment<N, T> const &l, vec_type &p1, vec_type &p2) const
{
	// The same roots as the ray's, kept by where on the segment they are (the segment's A + (B - A) t).
	vec_type const direction = l.direction(), delta = l.A - centre;
	T const A = direction.length_sq();
	if (!(A > T(0))) return 0;
	T const B = T(2) * dot(direction, delta);
	T const C = delta.length_sq() - square(radius);
	T const discriminant = square(B) - 4 * A * C;
	if (discriminant < 0) return 0;

	if (discriminant < 4 * A * square(radius) * scalar_traits<T>::zero_length_sq) {
		T const t = -B / (2 * A);
		if (!(t >= T(0) && t <= T(1))) return 0;
		p1 = p2 = l.A + direction * t;
		return 1;
	}

	T const discriminant_sqrt = sqrt(discriminant);
	T const t1 = (-B + discriminant_sqrt) / (2 * A), t2 = (-B - discriminant_sqrt) / (2 * A);
	bool const on1 = t1 >= T(0) && t1 <= T(1), on2 = t2 >= T(0) && t2 <= T(1);
	if (on1 && on2) {
		p1 = l.A + direction * t1;
		p2 = l.A + direction * t2;
		return 2;
	}
	if (on1 || on2) {
		p1 = p2 = l.A + direction * (on1 ? t1 : t2);
		return 1;
	}
	return 0;
}

template<int N, class T> bool
sphere<N, T>::test_intersection(capsule<N, T> const &c) const
{
	return c.distance(centre) <= radius;
}

template class sphere<2>;
template class sphere<3>;
template class sphere<2, double>;
template class sphere<3, double>;
#ifdef ROVE_DFLOAT
template class sphere<2, dfloat>;
template class sphere<3, dfloat>;
#endif

}
