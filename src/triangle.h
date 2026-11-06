// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <limits>

#include "scalar.h"
#include "vec.h"
#include "ray.h"
#include "segment.h"
#include "plane.h"
#include "aabb.h"
#include "sphere.h"
#include "matrix.h"

namespace rove {

template<int N,class T=scalar> class triangle;

template<class T>
class [[nodiscard]] triangle<3,T> {
public:
	static constexpr size_t dimensions = 3;
	using vec_type = vec<dimensions,T>;
	using ray_type = ray<dimensions,T>;
	using segment_type = segment<dimensions,T>;
	using plane_type = plane<T>;
	using aabb_type = aabb<dimensions,T>;
	using sphere_type = sphere<dimensions, T>;
	using matrix_type = matrix<dimensions+1,dimensions+1,T>;

	vec_type A, B, C;

	triangle() {
	}

	triangle(vec_type const &a,vec_type const &b,vec_type const &c):
		A(a),
		B(b),
		C(c)
	{
	}

	void construct(vec_type const &a,vec_type const &b,vec_type const &c) {
		A=a;
		B=b;
		C=c;
	}

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & A & B & C;
	}

	// Graphic Gems III, ed. David Kirk. V.1 Triangles Revisited, pp. 215-218
	// the normal of the triangle; zero for one of no area (its points on a line), as a mesh may hold
	vec_type normal() const {
		return normalize_or_zero(cross(B-A, C-B));
	}

	/// @brief The box around the triangle
	aabb_type bounds() const {
		aabb_type locus;
		locus.lo = locus.hi = A;
		locus.extend(B);
		locus.extend(C);
		return locus;
	}

	// the plane of the triangle
	plane_type to_plane() const {
		return plane_type(A,normal());
	}

	// the perimeter of the triangle
	T perimeter() const {
		return (B-A).length()+(C-B).length()+(A-C).length();
	}

	// the area of the triangle
	T area() const {
		return T(0.5)*(cross(A, B) + cross(B, C) + cross(C, A)).length();
	}

	// the centre of gravity, i.e. the concurrence of the medians
	vec_type cog() const {
		return (A + B + C) / T(3);
	}

	// the orthocenter, i.e. the concurrence of the altitudes
	vec_type orthocenter() const {
		vec_type a = B - C;
		vec_type b = C - A;
		vec_type c = A - B;

		T d1 = -dot(b, c);
		T d2 = -dot(c, a);
		T d3 = -dot(a, b);

		T e1 = d2 * d3;
		T e2 = d3 * d1;
		T e3 = d1 * d2;

		return (e1 * A + e2 * B + e3 * C) / (e1 + e2 + e3);
	}

	// the circumcenter, i.e. the concurrence of the perpendicular
	// bisectors to the sides
	vec_type circumcenter() const {
		return T(0.5)*(T(3)*cog() - orthocenter());
	}

	// the radius of the circumcircle, i.e. the circle that passes
	// through the three vertices
	T circumcircle_radius() const {
		vec_type a = B - C;
		vec_type b = C - A;
		vec_type c = A - B;

		T ab = a.length();
		T bc = b.length();
		T ca = c.length();

		return (ab * bc * ca) / (T(4) * area());
	}

	sphere_type circumcircle() const {
		return sphere_type(circumcenter(), circumcircle_radius());
	}

	// the incenter, i.e. the concurrence of the angle bisectors
	vec_type incenter() const {
		vec_type a = B - C;
		vec_type b = C - A;
		vec_type c = A - B;

		T ab = a.length();
		T bc = b.length();
		T ca = c.length();

		return (ab*A+bc*B+ca*C)/(ab+bc+ca);
	}

	// the radius of the incircle, i.e. the circle inscribed in the
	// triangle, tangent to the three sides
	T incircle_radius() const {
		vec_type a = B - C;
		vec_type b = C - A;
		vec_type c = A - B;

		T ab=a.length();
		T bc=b.length();
		T ca=c.length();

		return T(2)*area()/(ab+bc+ca);
	}

	sphere_type incircle() const {
		return sphere_type(incenter(), incircle_radius());
	}

	// the excenters and the radii of the excircles, i.e. the circles
	// tangent to one side of the triangle and to the extensions of the
	// other two sides.  With a, b, c the sides opposite A, B, C, the excentre
	// opposite A is (-a A + b B + c C) / (-a + b + c), of radius area / (s - a).
	vec_type excenter_A() const {
		T const a = (B - C).length(), b = (C - A).length(), c = (A - B).length();
		return (b * B + c * C - a * A) / (b + c - a);
	}

	T excircle_A_radius() const {
		T const a = (B - C).length(), b = (C - A).length(), c = (A - B).length();
		return T(2) * area() / (b + c - a);
	}

	vec_type excenter_B() const {
		T const a = (B - C).length(), b = (C - A).length(), c = (A - B).length();
		return (a * A + c * C - b * B) / (a + c - b);
	}

	T excircle_B_radius() const {
		T const a = (B - C).length(), b = (C - A).length(), c = (A - B).length();
		return T(2) * area() / (a + c - b);
	}

	vec_type excenter_C() const {
		T const a = (B - C).length(), b = (C - A).length(), c = (A - B).length();
		return (a * A + b * B - c * C) / (a + b - c);
	}

	T excircle_C_radius() const {
		T const a = (B - C).length(), b = (C - A).length(), c = (A - B).length();
		return T(2) * area() / (a + b - c);
	}

	// the centre and the radius of the Feuerbach circle (nine-point
	// circle), i.e. the circle that passes through the three feet of
	// the altitudes and the middle points of the sides
	vec_type feuerbach_center() const {
		return T(0.5)*(circumcenter()+orthocenter());
	}

	T feuerbach_radius() const {
		return T(0.5)*circumcircle_radius();
	}

	// transform triangle by matrix
	void transform(matrix_type const &tf) {
		detail::mul(A, tf, vec_type(A));
		detail::mul(B, tf, vec_type(B));
		detail::mul(C, tf, vec_type(C));
	}

	bool contains_on_plane(vec_type const &v) const {
		// http://www.blackpawn.com/texts/pointinpoly/default.html
		return (dot(cross(B - A, v - A), cross(B - A, C - A)) >= 0)
			&& (dot(cross(C - B, v - B), cross(C - B, A - B)) >= 0)
			&& (dot(cross(A - C, v - C), cross(A - C, B - C)) >= 0);
	}

	/// @brief Whether v is in the triangle: on its plane, to within what rounding allows at this distance from the origin
	bool contains(vec_type const &v) const {
		T scale = T(0);
		for (vec_type const *p : {&A, &B, &C, &v})
			for (size_t n = 0; n < dimensions; ++n)
				scale = std::max(scale, abs(p->i[n]));
		T const tolerance = std::max(scalar_traits<T>::default_epsilon, scale * 4 * std::numeric_limits<T>::epsilon());
		return to_plane().contains(v, tolerance) && contains_on_plane(v);
	}

	// Moller-Trumbore ray-triangle intersection. Two-sided (does not cull
	// back faces) and uses inclusive edge/vertex boundaries, matching the
	// plane+contains_on_plane approach this replaced.  A ray on an edge or a vertex shared with another triangle hits
	// both, and one on the very edge of a lone triangle may miss it by a rounding error: see `edge_slack`.
	//
	// `epsilon` bounds the determinant, |direction| |edge1| |edge2| times
	// the sine and cosine of the angles involved: below it the ray counts as
	// parallel.  It is absolute: a unit ray's determinant is twice the area
	// times the cosine of incidence, so the float default misses triangles
	// under half a square millimetre (more at a slant); pass
	// std::numeric_limits<T>::min() to miss only exactly parallel rays.
	[[nodiscard]] bool trace(ray_type const &r,T &t,vec_type &v,T t_min = 0, T t_max = 1, T epsilon = scalar_traits<T>::default_epsilon) const {
		// The barycentric bounds are a hair wider than [0, 1]: rounding puts a ray through a shared edge on the outside of
		// both triangles, and a mesh would have a crack there.  The nearest hit wins, so a double hit costs nothing.
		constexpr T edge_slack = T(1e-5);
		vec_type edge1 = B - A;
		vec_type edge2 = C - A;
		vec_type pvec = cross(r.a, edge2);
		T det = dot(edge1, pvec);
		if (abs(det) < epsilon || det == 0) return false; // ray parallel to triangle plane

		T inv_det = T(1) / det;
		vec_type tvec = r.r0 - A;
		T u = dot(tvec, pvec) * inv_det;
		if (u < -edge_slack || u > T(1) + edge_slack) return false;

		vec_type qvec = cross(tvec, edge1);
		T vv = dot(r.a, qvec) * inv_det;
		if (vv < -edge_slack || u + vv > T(1) + edge_slack) return false;

		t = dot(edge2, qvec) * inv_det;
		if (t < t_min || t > t_max) return false;

		v = r.r0 + r.a * t;
		return true;
	}

	[[nodiscard]] bool trace(ray_type const &r,T t_min = 0,T t_max = 1,T epsilon = scalar_traits<T>::default_epsilon) const {
		T t;
		vec_type v;
		return trace(r,t,v,t_min,t_max,epsilon);
	}

	/// The point of the triangle nearest to p.  A triangle without area is
	/// its edges.  Ericson, Real-Time Collision Detection 5.1.5.
	vec_type closest_point(vec_type const &p) const {
		vec_type const ab = B - A, ac = C - A, ap = p - A;
		T const d1 = dot(ab, ap), d2 = dot(ac, ap);
		if (d1 <= 0 && d2 <= 0) return A;
		vec_type const bp = p - B;
		T const d3 = dot(ab, bp), d4 = dot(ac, bp);
		if (d3 >= 0 && d4 <= d3) return B;
		T const vc = d1 * d4 - d3 * d2;
		if (vc <= 0 && d1 >= 0 && d3 <= 0) return A + ab * (d1 / (d1 - d3));
		vec_type const cp = p - C;
		T const d5 = dot(ab, cp), d6 = dot(ac, cp);
		if (d6 >= 0 && d5 <= d6) return C;
		T const vb = d5 * d2 - d1 * d6;
		if (vb <= 0 && d2 >= 0 && d6 <= 0) return A + ac * (d2 / (d2 - d6));
		T const va = d3 * d6 - d5 * d4;
		if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0) return B + (C - B) * ((d4 - d3) / ((d4 - d3) + (d5 - d6)));
		T const sum = va + vb + vc;
		// a NaN anywhere fails every test above and lands here
		if (std::isnan(sum)) return vec_type(sum, sum, sum);
		if (!(sum > 0)) {
			// no area: the nearest of the three edges
			segment_type const edges[3] = {segment_type(A, B), segment_type(B, C), segment_type(C, A)};
			vec_type best = edges[0].closest_point(p);
			for (int i = 1; i < 3; ++i) {
				vec_type const q = edges[i].closest_point(p);
				if ((q - p).length_sq() < (best - p).length_sq()) best = q;
			}
			return best;
		}
		return A + ab * (vb / sum) + ac * (vc / sum);
	}

	/// The nearest pair of points, `on_segment` on s and `on_triangle` on
	/// the triangle; returns their squared distance, 0 when s crosses the
	/// triangle (both points are then the crossing).  s may be a point.
	T closest_points(segment_type const &s, vec_type &on_segment, vec_type &on_triangle) const {
		T t;
		vec_type at;
		if (trace(s.to_ray(), t, at, T(0), T(1), std::numeric_limits<T>::min())) {
			on_segment = on_triangle = at;
			return 0;
		}
		on_segment = s.A;
		on_triangle = closest_point(s.A);
		T best = (on_segment - on_triangle).length_sq();
		auto consider = [&](vec_type const &a, vec_type const &b) {
			T const d = (a - b).length_sq();
			if (d < best) {
				best = d;
				on_segment = a;
				on_triangle = b;
			}
		};
		consider(s.B, closest_point(s.B));
		segment_type const edges[3] = {segment_type(A, B), segment_type(B, C), segment_type(C, A)};
		for (segment_type const &e : edges) {
			vec_type a, b;
			s.closest_points(e, a, b);
			consider(a, b);
		}
		return best;
	}

	bool test_intersection(triangle const &t) const;

	/// @brief The cut between two triangles: the part of the line of their planes inside both.  False for parallel planes (coplanar
	/// triangles have no one line) and where the two have no point of it in common.
	bool query_intersection(triangle const &t,segment_type &ln) const {
		ray_type r;
		if (!to_plane().query_intersection(t.to_plane(),r)) return false;

		T lo0, hi0, lo1, hi1;
		if (!part_of_line_inside(r, t.to_plane(), lo0, hi0)) return false;
		if (!t.part_of_line_inside(r, to_plane(), lo1, hi1)) return false;
		T const lo = std::max(lo0, lo1), hi = std::min(hi0, hi1);
		if (lo > hi) return false;

		ln.construct(r.apply(lo), r.apply(hi));
		return true;
	}

	// find the time when two moving triangles will collide
	// and the collision point
	// tri1 -- the given triangle
	// vel0 -- velocity of this triangle
	// vel1 -- velocity of triangle tri1
	// t -- time when the triangles will collide
	// cp -- collision point
	// cn -- collision normal
	// t_min, t_max -- minimum and maximum calculation time
	bool collide(triangle const &tri1,vec_type const &vel0,vec_type const &vel1,
		T &t,vec_type &cp,vec_type &cn,T t_min = 0, T t_max = 1) const
	{
		vec_type vel = vel1 - vel0, n0 = normal(), n1 = tri1.normal(), cp0;
		T a_n0 = dot(-vel, n0), a_n1 = dot(vel, n1), t0;
		bool found = false;

		if (abs(a_n1) > 0) {
			t0 = dot(A - tri1.A, n1) / a_n1;
			cp0 = A - vel * t0;
			if (t_min <= t0 && t0 <= t_max && (!found || t0 < t) && tri1.contains(cp0)) {
				t = t0;
				cp = A + vel0 * t;
				cn = n1;
				found = true;
			}

			t0 = dot(B - tri1.A, n1) / a_n1;
			cp0 = B - vel * t0;
			if (t_min <= t0 && t0 <= t_max && (!found || t0 < t) && tri1.contains(cp0)) {
				t = t0;
				cp = B + vel0 * t;
				cn = n1;
				found = true;
			}

			t0 = dot(C - tri1.A, n1) / a_n1;
			cp0 = C - vel * t0;
			if (t_min <= t0 && t0 <= t_max && (!found || t0 < t) && tri1.contains(cp0)) {
				t = t0;
				cp = C + vel0 * t;
				cn = n1;
				found = true;
			}
		}

		if (abs(a_n0) > 0) {
			t0 = dot(tri1.A - A, n0) / a_n0;
			cp0 = tri1.A + vel * t0;
			if (t_min <= t0 && t0 <= t_max && (!found || t0 < t) && contains(cp0)) {
				t = t0;
				cp = tri1.A + vel1 * t;
				cn = n0;
				found = true;
			}

			t0 = dot(tri1.B - A, n0) / a_n0;
			cp0 = tri1.B + vel * t0;
			if (t_min <= t0 && t0 <= t_max && (!found || t0 < t) && contains(cp0)) {
				t = t0;
				cp = tri1.B + vel1 * t;
				cn = n0;
				found = true;
			}

			t0 = dot(tri1.C - A, n0) / a_n0;
			cp0 = tri1.C + vel * t0;
			if (t_min <= t0 && t0 <= t_max && (!found || t0 < t) && contains(cp0)) {
				t = t0;
				cp = tri1.C + vel1 * t;
				cn = n0;
				found = true;
			}
		}

		return found;
	}

private:
	/// The part of a line, as its t, that is inside this triangle, the line lying in this triangle's plane and in `other`: where the
	/// sides cross that other plane.
	bool part_of_line_inside(ray_type const &line, plane_type const &other, T &lo, T &hi) const {
		vec_type const *const corners[3] = {&A, &B, &C};
		bool any = false;
		for (int i = 0; i < 3; ++i) {
			vec_type const &p = *corners[i], &q = *corners[(i + 1) % 3];
			T const dp = other.A * p.x + other.B * p.y + other.C * p.z + other.D;
			T const dq = other.A * q.x + other.B * q.y + other.C * q.z + other.D;
			if ((dp > T(0) && dq > T(0)) || (dp < T(0) && dq < T(0))) continue;
			T const k = dp - dq;
			if (k == T(0) && dp != T(0)) continue;
			// (a side lying in the other plane has both its ends on the line)
			T const ks[2] = { k == T(0) ? T(0) : dp / k, k == T(0) ? T(1) : dp / k };
			for (T const f : ks) {
				T const u = dot(p + (q - p) * f - line.r0, line.a);
				lo = any ? std::min(lo, u) : u;
				hi = any ? std::max(hi, u) : u;
				any = true;
			}
		}
		return any;
	}
};

}
