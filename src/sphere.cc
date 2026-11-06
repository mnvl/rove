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
	// |x-r0|=R
	// x=x0+a*t
	// |x0+a*t-r0|=R
	// (a*t+x0-r0,a*t+x0-r0)=R^2
	// (a*t,a*t+x0-r0)+(x0-r0,a*t+x0-r0)=R^2
	// (a,a)*t^2+(a,x0-r0)*t+(a,x0-r0)*t+(x0-r0,x0-r0)=R^2
	// (a,a)*t^2+(a,x0-r0)*2*t+(x0-r0,x0-r0)-R^2=0
	// A=(a,a)
	// B=(a,x0-r0)*2
	// C=(x0-r0,x0-r0)-R

	T A = r.a.length_sq();
	T B = 2 * dot(r.a, r.r0 - centre);
	T C = (r.r0 - centre).length_sq() - radius * radius;

	return B * B - 4 * A * C >= 0;
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

	T discriminant = square(B) - 4 * A * C;

	if (discriminant < 0) {
		return false;
	}

	if (discriminant < scalar_traits<T>::zero_length_sq) {
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
	vec_type p1, p2;
	T ll = l.length_sq();

	switch (query_intersection(l.to_ray(), p1, p2)) {
		case 0: return false;
		case 1: return (p1 - l.A).length_sq() < ll;
		default: return (p1 - l.A).length_sq() < ll || (p2 - l.A).length_sq() < ll;
	}
}

template<int N, class T> int
sphere<N, T>::query_intersection(segment<N, T> const &l, vec_type &p1, vec_type &p2) const
{
	switch (query_intersection(l.to_ray(), p1, p2)) {
		case 0: return 0;
		case 1: return int(l.contains(p1));
		default: return int(l.contains(p1)) + int(l.contains(p2));
	}
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
