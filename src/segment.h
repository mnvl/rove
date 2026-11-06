// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <algorithm>

#include "scalar.h"
#include "vec.h"
#include "ray.h"
#include "aabb.h"

namespace rove
{

/// A line segment from A to B
template<int N,class T=scalar>
class [[nodiscard]] segment {
public:
	static constexpr size_t dimensions = N;
	using vec_type = vec<dimensions, T>;
	using ray_type = ray<dimensions, T>;
	using aabb_type = aabb<dimensions, T>;

	vec_type A, B;

	segment() {
	}

	segment(vec_type const &a,vec_type const &b) {
		construct(a,b);
	}

	void construct(vec_type const &a,vec_type const &b) {
		A=a;
		B=b;
	}

	vec_type direction() const {
		return B - A;
	}

	ray_type to_ray() const {
		return ray_type(A, direction());
	}

	aabb_type bounds() const {
		aabb_type bounds(A, B);
		minmax(bounds.lo, bounds.hi);
		return bounds;
	}

	T length_sq() const {
		return direction().length_sq();
	}

	bool contains(vec_type const &p) const
	{
		// (the cross product is the distance from the line times |v|: the tolerance is one of distance whatever the length of the segment)
		vec_type u(p - A), v(direction());
		T const tolerance = scalar_traits<T>::default_epsilon * std::max(T(1), v.length());
		return u.is_collinear(v, tolerance) && u.length_sq() <= v.length_sq() && dot(u, v) >= 0;
	}

	/// The point A + (B - A) t; exactly A at t = 0 and exactly B at t = 1.
	vec_type point(T t) const
	{
		return A * (1 - t) + B * t;
	}

	/// The t in [0, 1] of the point on the segment nearest to p; 0 when
	/// A == B, where every t names the same point.  NaN in, NaN out.
	T parameter(vec_type const &p) const
	{
		vec_type const dir(direction());
		T const dir_sq = dir.length_sq();
		if (dir_sq == 0) return 0;
		return std::clamp(dot(p - A, dir) / dir_sq, T(0), T(1));
	}

	vec_type closest_point(vec_type const &p) const
	{
		return point(parameter(p));
	}

	T distance_sq(vec_type const &p) const
	{
		return (closest_point(p) - p).length_sq();
	}

	T distance(vec_type const &p) const
	{
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		return sqrt(distance_sq(p));
	}

	/// The nearest pair of points, `on_this` on this segment and `on_other`
	/// on `other`; returns their squared distance.  Either segment may be a
	/// point.  Ericson, Real-Time Collision Detection 5.1.9.
	T closest_points(segment const &other, vec_type &on_this, vec_type &on_other) const
	{
		vec_type const d1 = direction(), d2 = other.direction(), r = A - other.A;
		T const a = dot(d1, d1), e = dot(d2, d2), f = dot(d2, r);
		T s, t;
		if (a == 0 && e == 0)
		{
			s = t = 0;
		}
		else if (a == 0)
		{
			s = 0;
			t = std::clamp(f / e, T(0), T(1));
		}
		else
		{
			T const c = dot(d1, r);
			if (e == 0)
			{
				t = 0;
				s = std::clamp(-c / a, T(0), T(1));
			}
			else
			{
				T const b = dot(d1, d2), denom = a * e - b * b;
				// parallel: any s will do, take A's end
				s = denom > 0 ? std::clamp((b * f - c * e) / denom, T(0), T(1)) : T(0);
				t = (b * s + f) / e;
				if (t < 0)
				{
					t = 0;
					s = std::clamp(-c / a, T(0), T(1));
				}
				else if (t > 1)
				{
					t = 1;
					s = std::clamp((b - c) / a, T(0), T(1));
				}
			}
		}
		on_this = point(s);
		on_other = other.point(t);
		return (on_this - on_other).length_sq();
	}
};

}

