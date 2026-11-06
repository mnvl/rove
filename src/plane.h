// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include "scalar.h"
#include "vec.h"
#include "ray.h"

namespace rove {

template<int N, class T> class aabb;
template<int N, class T> class obb;

template<class T = scalar>
class [[nodiscard]] plane {
public:
	static constexpr size_t dimensions = 3;
	using vec_type = vec<dimensions,T>;
	using ray_type = ray<dimensions,T>;

	enum classification {
		positive = 1,
		intersects,
		negative,
	};

	// data
	T A, B, C, D;

	// constructors
	/// All four coefficients zero: no plane (is_correct() is false), until set() (it was left uninitialised)
	plane():
		A(0), B(0), C(0), D(0)
	{
	}

	plane(vec_type const &o, vec_type const &n) {
		set(o, n);
	}

	plane(T d, vec_type const &normal) {
		set(d, normal);
	}

	void set(vec_type const &o, vec_type const &n) {
		A = n.x;	B = n.y;	C = n.z;	D = -dot(o, n);
	}

	void set(T d, vec_type const &normal) {
		A = normal.x;	B = normal.y;	C = normal.z;	D = d;
	}

	T apply(vec_type const &v) const {
		return A*v.x + B*v.y + C*v.z + D;
	}

	bool test_intersection(ray_type const &r, T epsilon = scalar_traits<T>::default_epsilon) const {
		return abs(A*r.a.x + B*r.a.y + C*r.a.z) > epsilon || contains(r, epsilon);
	}

	bool contains(vec_type const &p, T epsilon = scalar_traits<T>::default_epsilon) const {
		return abs(A * p.x + B * p.y + C * p.z + D) < epsilon;
	}

	bool contains(ray_type const &r, T epsilon = scalar_traits<T>::default_epsilon) const {
		return abs(A*r.a.x + B*r.a.y + C*r.a.z) < epsilon && contains(r.r0, epsilon);
	}

	bool parallel(ray_type const &r, T epsilon = scalar_traits<T>::default_epsilon) const {
		return !test_intersection(r, epsilon);
	}

	T trace(ray_type const &r) const {
		T a_n = A * r.a.x + B * r.a.y + C * r.a.z;
		T d = apply(r.r0);
		return -d / a_n;
	}

	// find intersection with ray r
	// r -- the given ray
	// t -- parameter in line equation r = r.origin + r.direction * t
	[[nodiscard]] bool trace(ray_type const &r,T &t,T t_min = 0, T t_max = 1) const {
		t = trace(r);
		return t >= t_min && t <= t_max;
	}

	// find intersection with ray r
	// r -- the given ray
	// t -- parameter in line equation r = r.origin + r.direction * t
	// v -- intersection point
	[[nodiscard]] bool trace(ray_type const &r,T &t,vec_type &v,T t_min = 0, T t_max = 1) const {
		if (!trace(r,t,t_min,t_max)) return false;
		v = r.r0 + r.a * t;
		return true;
	}

	bool is_correct() const { return abs(A) > 0 || abs(B) > 0 || abs(C) > 0; }

	vec_type origin() const;
	vec_type normal() const { return vec_type(A, B, C); }

	// returns false if planes are parallel
	bool query_intersection(plane const &p, ray_type &r) const;

	/// intersects where the point is within epsilon of the plane, as apply() measures it
	classification classify(vec_type const &point, T epsilon = scalar_traits<T>::default_epsilon) const;
	classification classify(aabb<dimensions, T> const &bounds, T epsilon = scalar_traits<T>::default_epsilon) const;
	classification classify(obb<dimensions, T> const &bounds, T epsilon = scalar_traits<T>::default_epsilon) const;
};

}
