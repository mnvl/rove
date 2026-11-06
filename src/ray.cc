// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "matrix.h"
#include "ray.h"

namespace rove
{

namespace
{

// How far p is from the line of r; a ray of no direction is the point it starts at.
template<int N, class T> T from_line(ray<N, T> const &r, vec<N, T> const &p)
{
	vec<N, T> const d = p - r.r0;
	T const k = r.a.length_sq();
	if (!(k > 0)) return d.length();
	return (d - r.a * (dot(d, r.a) / k)).length();
}

// Parallel lines are as far apart as the start of one is from the other; measured from the longer direction, so that
// a ray of no direction is a point.
template<int N, class T> T parallel_distance(ray<N, T> const &r1, ray<N, T> const &r2)
{
	return r1.a.length_sq() >= r2.a.length_sq() ? from_line(r1, r2.r0) : from_line(r2, r1.r0);
}

template<class T> T distance_between(ray<2,T> const &r1, ray<2,T> const &r2)
{
	// lines in a plane that are not parallel cross: whether they are is a question of the angle between them
	if (!r1.a.is_collinear(r2.a, scalar_traits<T>::zero_length * r1.a.length() * r2.a.length())) return 0;
	return parallel_distance(r1, r2);
}

template<class T> bool closest_aproach(ray<3, T> const &r1, T &t1, ray<3, T> const &r2, T &t2)
{
	vec<3, T> p2_p1 = r2.r0 - r1.r0;
	vec<3, T> a1_x_a2 = cross(r1.a, r2.a);

	T k = a1_x_a2.length_sq();
	// (k is |a1|^2 |a2|^2 sin^2: parallel is a question of the angle, not of how long the directions are)
	if (k <= scalar_traits<T>::zero_length_sq * r1.a.length_sq() * r2.a.length_sq()) return false;

	t1 = dot(cross(p2_p1, r2.a), a1_x_a2) / k;
	t2 = dot(cross(p2_p1, r1.a), a1_x_a2) / k;

	return true;
}

template<class T> T distance_between(ray<3, T> const &r1, ray<3, T> const &r2)
{
	T t1, t2;
	if (!closest_aproach(r1, t1, r2, t2)) return parallel_distance(r1, r2);
	return (r1.apply(t1) - r2.apply(t2)).length();
}

template<class T> bool query_intersection(ray<3, T> const &r1, ray<3, T> const &r2, T &t, T epsilon)
{
	T t2;
	if (!closest_aproach(r1, t, r2, t2)) return 0;
	return (r1.apply(t) - r2.apply(t2)).length_sq() < epsilon * epsilon;
}

// Lines in a plane that are not parallel cross: no distance between them to measure against epsilon.
template<class T> bool query_intersection(ray<2, T> const &r1, ray<2, T> const &r2, T &s, T)
{
	T s2;
	return closest_aproach(ray<3,T>(r1.r0, r1.a), s, ray<3,T>(r2.r0, r2.a), s2);
}

}

template<int N, class T> T
ray<N, T>::distance(ray const &r) const
{
	return distance_between(*this, r);
}

template<int N, class T> bool
ray<N, T>::query_intersection(ray const &r, T &t, T epsilon) const
{
	return rove::query_intersection(*this, r, t, epsilon);
}

template class ray<2>;
template class ray<3>;
template class ray<2, double>;
template class ray<3, double>;

}
