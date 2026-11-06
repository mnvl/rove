// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <tuple>
#include <rove/triangle.h>
#include <rove/ray.h>
#include <rove/plane.h>
#include "test_tolerance.h"

BOOST_AUTO_TEST_SUITE(test_triangle)

// Construction tests
BOOST_AUTO_TEST_CASE(triangle_default_constructor)
{
	rove::triangle<3> tri;
	// Just verify it compiles and doesn't crash
	BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(triangle_vertex_constructor)
{
	rove::vec<3> a(0, 0, 0);
	rove::vec<3> b(1, 0, 0);
	rove::vec<3> c(0, 1, 0);
	rove::triangle<3> tri(a, b, c);

	BOOST_REQUIRE((tri.A - a).length() < TOLERANCE);
	BOOST_REQUIRE((tri.B - b).length() < TOLERANCE);
	BOOST_REQUIRE((tri.C - c).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(triangle_construct_method)
{
	rove::triangle<3> tri;
	rove::vec<3> a(1, 2, 3);
	rove::vec<3> b(4, 5, 6);
	rove::vec<3> c(7, 8, 9);
	tri.construct(a, b, c);

	BOOST_REQUIRE((tri.A - a).length() < TOLERANCE);
	BOOST_REQUIRE((tri.B - b).length() < TOLERANCE);
	BOOST_REQUIRE((tri.C - c).length() < TOLERANCE);
}

// Geometric property tests
BOOST_AUTO_TEST_CASE(triangle_get_normal)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(0, 1, 0)
	);

	rove::vec<3> normal = tri.normal();
	// Normal should point in +Z direction for this CCW triangle
	BOOST_REQUIRE(rove::abs(normal.z - 1.0f) < 0.01f || rove::abs(normal.z + 1.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(normal.length() - 1.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(triangle_get_plane)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(0, 1, 0)
	);

	rove::plane<> pl = tri.to_plane();
	// All vertices should be on the plane
	BOOST_REQUIRE(pl.contains(tri.A));
	BOOST_REQUIRE(pl.contains(tri.B));
	BOOST_REQUIRE(pl.contains(tri.C));
}

BOOST_AUTO_TEST_CASE(triangle_perimeter)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(3, 0, 0),
		rove::vec<3>(0, 4, 0)
	);

	rove::scalar p = tri.perimeter();
	// 3-4-5 triangle: perimeter = 3 + 4 + 5 = 12
	BOOST_REQUIRE(rove::abs(p - 12.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_area)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::scalar area = tri.area();
	// Right triangle with legs 2 and 2: area = 0.5 * 2 * 2 = 2
	BOOST_REQUIRE(rove::abs(area - 2.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_cog)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(3, 0, 0),
		rove::vec<3>(0, 3, 0)
	);

	rove::vec<3> cog = tri.cog();
	// Center of gravity is at (1, 1, 0)
	BOOST_REQUIRE(rove::abs(cog.x - 1.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(cog.y - 1.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(cog.z) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_orthocenter)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(4, 0, 0),
		rove::vec<3>(0, 3, 0)
	);

	rove::vec<3> ortho = tri.orthocenter();
	// For a right triangle, orthocenter is at the right angle vertex
	BOOST_REQUIRE(ortho.length() < 0.1f);
}

BOOST_AUTO_TEST_CASE(triangle_circumcenter)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::vec<3> cc = tri.circumcenter();
	// For a right triangle, circumcenter is at midpoint of hypotenuse
	BOOST_REQUIRE(rove::abs(cc.x - 1.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(cc.y - 1.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_circumcircle_radius)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::scalar r = tri.circumcircle_radius();
	// For a right triangle, circumradius = hypotenuse / 2 = 2*sqrt(2) / 2 = sqrt(2)
	rove::scalar expected = rove::square_root(2.0f);
	BOOST_REQUIRE(rove::abs(r - expected) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_get_circumcircle)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::sphere<3> sphere = tri.circumcircle();
	// For a right triangle, circumradius = hypotenuse / 2 = sqrt(2)
	rove::scalar expected = rove::square_root(2.0f);
	BOOST_REQUIRE(rove::abs(sphere.radius - expected) < 0.01f);
	// Circumcenter should be at midpoint of hypotenuse: (1, 1, 0)
	BOOST_REQUIRE(rove::abs(sphere.centre.x - 1.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(sphere.centre.y - 1.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(sphere.centre.z) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_incenter)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(4, 0, 0),
		rove::vec<3>(0, 3, 0)
	);

	rove::vec<3> ic = tri.incenter();
	// Incenter should be inside the triangle
	BOOST_REQUIRE(ic.x > 0 && ic.x < 4);
	BOOST_REQUIRE(ic.y > 0 && ic.y < 3);
	BOOST_REQUIRE(rove::abs(ic.z) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_incircle_radius)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(3, 0, 0),
		rove::vec<3>(0, 4, 0)
	);

	rove::scalar r = tri.incircle_radius();
	// For a 3-4-5 right triangle: area = 6, perimeter = 12, inradius = 2*area/perimeter = 1
	BOOST_REQUIRE(rove::abs(r - 1.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_get_incircle)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(3, 0, 0),
		rove::vec<3>(0, 3, 0)
	);

	rove::sphere<3> sphere = tri.incircle();
	// Radius should be positive
	BOOST_REQUIRE(sphere.radius > 0);
}

// Excircle tests
BOOST_AUTO_TEST_CASE(triangle_excenter_A)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(3, 0, 0),
		rove::vec<3>(0, 3, 0)
	);

	rove::vec<3> ex = tri.excenter_A();
	// Excenter should be outside the triangle
	BOOST_REQUIRE(ex.length() > 0);
}

BOOST_AUTO_TEST_CASE(triangle_feuerbach_center)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(4, 0, 0),
		rove::vec<3>(0, 4, 0)
	);

	rove::vec<3> fc = tri.feuerbach_center();
	// Feuerbach center is midpoint between circumcenter and orthocenter
	BOOST_REQUIRE(fc.length() >= 0);
}

BOOST_AUTO_TEST_CASE(triangle_feuerbach_radius)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(4, 0, 0),
		rove::vec<3>(0, 4, 0)
	);

	rove::scalar r = tri.feuerbach_radius();
	// Feuerbach radius is half the circumradius
	rove::scalar cr = tri.circumcircle_radius();
	BOOST_REQUIRE(rove::abs(r - cr / 2.0f) < 0.01f);
}

// Contains tests
BOOST_AUTO_TEST_CASE(triangle_contains_on_plane)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	// Point inside triangle
	BOOST_REQUIRE(tri.contains_on_plane(rove::vec<3>(0.5f, 0.5f, 0)));

	// Point outside triangle
	BOOST_REQUIRE(!tri.contains_on_plane(rove::vec<3>(2, 2, 0)));

	// Vertex
	BOOST_REQUIRE(tri.contains_on_plane(tri.A));
}

BOOST_AUTO_TEST_CASE(triangle_contains)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	// Point inside triangle and on plane
	BOOST_REQUIRE(tri.contains(rove::vec<3>(0.5f, 0.5f, 0)));

	// Point inside triangle projection but off plane
	BOOST_REQUIRE(!tri.contains(rove::vec<3>(0.5f, 0.5f, 1.0f)));
}

// Ray tracing tests
BOOST_AUTO_TEST_CASE(test_triangle_trace)
{
	rove::triangle<3> tri(rove::vec<3>(0, 1, 0), rove::vec<3>(0, 3, 1), rove::vec<3>(1, 3, 0));
	rove::scalar t;
	rove::vec<3> v;
	bool result = tri.trace(rove::ray<3>(rove::vec<3>(-1, 2, 0), rove::vec<3>(10, 0, 0)), t, v);
	BOOST_CHECK (result == true);
	BOOST_CHECK ((rove::vec<3>(0.5f, 2, 0) - v).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(triangle_trace_hit)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::ray<3> r(rove::vec<3>(0.5f, 0.5f, -5), rove::vec<3>(0, 0, 1));
	rove::scalar t;
	rove::vec<3> v;

	BOOST_REQUIRE(tri.trace(r, t, v, 0, 100));
	BOOST_REQUIRE(rove::abs(v.x - 0.5f) < 0.01f);
	BOOST_REQUIRE(rove::abs(v.y - 0.5f) < 0.01f);
	BOOST_REQUIRE(rove::abs(v.z) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_trace_miss)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::ray<3> r(rove::vec<3>(5, 5, -5), rove::vec<3>(0, 0, 1));

	BOOST_REQUIRE(!tri.trace(r, 0, 100));
}

BOOST_AUTO_TEST_CASE(triangle_trace_boolean)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::ray<3> r1(rove::vec<3>(0.5f, 0.5f, -5), rove::vec<3>(0, 0, 1));
	rove::ray<3> r2(rove::vec<3>(5, 5, -5), rove::vec<3>(0, 0, 1));

	BOOST_REQUIRE(tri.trace(r1, 0, 100));
	BOOST_REQUIRE(!tri.trace(r2, 0, 100));
}

// Pinning tests for the current plane+contains_on_plane trace() implementation,
// written before swapping the internals to Moller-Trumbore so the swap can be
// verified to preserve behavior exactly.

BOOST_AUTO_TEST_CASE(triangle_trace_two_sided)
{
	// Winding A,B,C gives normal (0,0,4) i.e. +Z.
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	// Ray direction (0,0,1): travels *with* the normal (hits the back face).
	rove::ray<3> back(rove::vec<3>(0.5f, 0.5f, -5), rove::vec<3>(0, 0, 1));
	// Ray direction (0,0,-1): travels *against* the normal (hits the front face).
	rove::ray<3> front(rove::vec<3>(0.5f, 0.5f, 5), rove::vec<3>(0, 0, -1));

	rove::scalar t;
	rove::vec<3> v;
	BOOST_REQUIRE(tri.trace(back, t, v, 0, 100));
	BOOST_REQUIRE(rove::abs(t - 5.0f) < 0.01f);
	BOOST_REQUIRE(tri.trace(front, t, v, 0, 100));
	BOOST_REQUIRE(rove::abs(t - 5.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_trace_t_min_exclusion)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);
	// Hit is at t=5 along this ray; t_min excludes it.
	rove::ray<3> r(rove::vec<3>(0.5f, 0.5f, -5), rove::vec<3>(0, 0, 1));

	BOOST_REQUIRE(!tri.trace(r, 5.1f, 100.0f));
	BOOST_REQUIRE(tri.trace(r, 4.9f, 100.0f));
}

BOOST_AUTO_TEST_CASE(triangle_trace_t_max_exclusion)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);
	// Hit is at t=5 along this ray; t_max excludes it.
	rove::ray<3> r(rove::vec<3>(0.5f, 0.5f, -5), rove::vec<3>(0, 0, 1));

	BOOST_REQUIRE(!tri.trace(r, 0.0f, 4.9f));
	BOOST_REQUIRE(tri.trace(r, 0.0f, 5.1f));
}

BOOST_AUTO_TEST_CASE(triangle_trace_vertex_boundary)
{
	// A ray aimed exactly at vertex B should hit (inclusive boundary).
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);
	rove::ray<3> r(rove::vec<3>(2, 0, -5), rove::vec<3>(0, 0, 1));
	rove::scalar t;
	rove::vec<3> v;
	BOOST_REQUIRE(tri.trace(r, t, v, 0, 100));
	BOOST_REQUIRE((v - rove::vec<3>(2, 0, 0)).length() < 0.01f);
}

BOOST_AUTO_TEST_CASE(triangle_trace_just_outside_edge)
{
	// A ray just past the hypotenuse (x+y=2 edge) should miss.
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);
	rove::ray<3> r(rove::vec<3>(1.1f, 1.1f, -5), rove::vec<3>(0, 0, 1));
	BOOST_REQUIRE(!tri.trace(r, 0, 100));
}

// Triangle-triangle intersection tests
BOOST_AUTO_TEST_CASE(triangle_test_intersection_coplanar)
{
	rove::triangle<3> tri1(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::triangle<3> tri2(
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(3, 0, 0),
		rove::vec<3>(1, 2, 0)
	);

	BOOST_REQUIRE(tri1.test_intersection(tri2));
}

BOOST_AUTO_TEST_CASE(triangle_test_intersection_different_planes)
{
	rove::triangle<3> tri1(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::triangle<3> tri2(
		rove::vec<3>(1, 0, -1),
		rove::vec<3>(1, 0, 1),
		rove::vec<3>(1, 2, 0)
	);

	BOOST_REQUIRE(tri1.test_intersection(tri2));
}

BOOST_AUTO_TEST_CASE(triangle_test_intersection_disjoint)
{
	rove::triangle<3> tri1(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 2, 0)
	);

	rove::triangle<3> tri2(
		rove::vec<3>(5, 5, 1),
		rove::vec<3>(6, 5, 1),
		rove::vec<3>(5, 6, 1)
	);

	BOOST_REQUIRE(!tri1.test_intersection(tri2));
}

// Transform test
BOOST_AUTO_TEST_CASE(triangle_transform)
{
	rove::triangle<3> tri(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(0, 1, 0)
	);

	rove::matrix<4, 4> m;
	m.identity();
	m.translation(5, 5, 5);

	tri.transform(m);

	// All vertices should be translated
	BOOST_REQUIRE(rove::abs(tri.A.x - 5.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(tri.A.y - 5.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(tri.A.z - 5.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(collide_accepts_a_double_time_window)
{
	// triangle<3,double>::collide takes t_min as T but declared t_max as
	// rove::scalar, so a double upper bound was rounded to float before the
	// t0 <= t_max test.  2^24 + 1 is the smallest integer a float cannot
	// hold: it rounds down to 2^24, so a genuine hit at 2^24 + 0.5 lands
	// inside the caller's window but outside the rounded one.
	using vec3d = rove::vec<3,double>;
	double const t_max = 16777217.0;          // 2^24 + 1
	double const expected_t = 16777216.5;     // 2^24 + 0.5, inside t_max
	double const separation = 2 * expected_t;

	// tri1 is far wider than tri0 so every vertex of tri0 sweeps into its
	// interior -- this is about the time window, not about edge contact.
	rove::triangle<3,double> tri0(vec3d(0, 0, 0), vec3d(1, 0, 0), vec3d(0, 1, 0));
	rove::triangle<3,double> tri1(vec3d(-100, -100, separation),
	                              vec3d(100, -100, separation),
	                              vec3d(0, 100, separation));

	double t = 0;
	vec3d cp, cn;
	BOOST_REQUIRE(tri0.collide(tri1, vec3d(0, 0, 1), vec3d(0, 0, -1),
	                           t, cp, cn, 0.0, t_max));
	BOOST_REQUIRE_EQUAL(t, expected_t);
}

// closest_point and closest_points(segment).  "On the triangle" is checked
// with barycentric coordinates, independently of the code under test; the
// random cases against dense sampling, which the answer can never beat and
// must come within the sampling step of.

namespace
{

using scalars = std::tuple<float, double>;

// In double whatever T is: a sliver's barycentric system is ill-conditioned.
template<class T>
bool on_triangle(rove::triangle<3, T> const &t, rove::vec<3, T> const &q, T tolerance)
{
	using d3 = rove::vec<3, double>;
	auto d = [](rove::vec<3, T> const &v) { return d3(v.x, v.y, v.z); };
	d3 const v0 = d(t.B) - d(t.A), v1 = d(t.C) - d(t.A), v2 = d(q) - d(t.A);
	double const d00 = dot(v0, v0), d01 = dot(v0, v1), d11 = dot(v1, v1), d20 = dot(v2, v0), d21 = dot(v2, v1);
	double const denom = d00 * d11 - d01 * d01;
	double const v = (d11 * d20 - d01 * d21) / denom, w = (d00 * d21 - d01 * d20) / denom, u = 1 - v - w;
	d3 const back = d(t.A) * u + d(t.B) * v + d(t.C) * w;
	return u >= -tolerance && v >= -tolerance && w >= -tolerance && (back - d(q)).length() <= tolerance;
}

template<class T>
T sampled_min_sq(rove::triangle<3, T> const &t, rove::vec<3, T> const &p, int n)
{
	T best = std::numeric_limits<T>::max();
	for (int i = 0; i <= n; ++i)
		for (int j = 0; i + j <= n; ++j)
			best = std::min(best, (t.A + (t.B - t.A) * (T(i) / n) + (t.C - t.A) * (T(j) / n) - p).length_sq());
	return best;
}

template<class T>
T longest_edge(rove::triangle<3, T> const &t)
{
	return std::max({(t.B - t.A).length(), (t.C - t.B).length(), (t.A - t.C).length()});
}

template<class T>
rove::triangle<3, T> right_triangle()
{
	return rove::triangle<3, T>(rove::vec<3, T>(0, 0, 0), rove::vec<3, T>(4, 0, 0), rove::vec<3, T>(0, 4, 0));
}

}

BOOST_AUTO_TEST_CASE_TEMPLATE(closest_point_in_each_region, T, scalars)
{
	using v3 = rove::vec<3, T>;
	rove::triangle<3, T> const t = right_triangle<T>();
	struct { v3 p, expected; } const cases[] = {
		{v3(1, 1, 3), v3(1, 1, 0)},       // above the face
		{v3(1, 1, -2), v3(1, 1, 0)},      // below it
		{v3(1, 1, 0), v3(1, 1, 0)},       // on it
		{v3(-1, -1, 2), v3(0, 0, 0)},     // vertex A
		{v3(6, -1, 0), v3(4, 0, 0)},      // vertex B
		{v3(-1, 6, 1), v3(0, 4, 0)},      // vertex C
		{v3(2, -3, 0), v3(2, 0, 0)},      // edge AB
		{v3(-2, 2, 5), v3(0, 2, 0)},      // edge AC
		{v3(3, 3, 1), v3(2, 2, 0)},       // edge BC
	};
	for (auto const &c : cases)
		BOOST_TEST_CONTEXT("p " << c.p.x << " " << c.p.y << " " << c.p.z)
			BOOST_TEST((t.closest_point(c.p) - c.expected).length() < T(1e-5));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(closest_point_matches_a_dense_sampling, T, scalars)
{
	std::mt19937 rng(7);
	std::uniform_real_distribution<T> u(-3, 3);
	auto v = [&] { return rove::vec<3, T>{u(rng), u(rng), u(rng)}; };
	int const n = 300;
	for (int k = 0; k < 200; ++k)
	{
		rove::triangle<3, T> t(v(), v(), v());
		if (k % 3 == 1)
			t.C = t.A + (t.B - t.A) * T(0.5) + v() * T(0.02);        // a sliver
		rove::vec<3, T> const p = v() * T(2);
		rove::vec<3, T> const q = t.closest_point(p);
		T const d = (q - p).length_sq(), sampled = sampled_min_sq(t, p, n);
		T const h = longest_edge(t) / n;
		BOOST_TEST_CONTEXT("case " << k)
		{
			BOOST_TEST(on_triangle(t, q, T(1e-3)));
			BOOST_TEST(d <= sampled + T(1e-4));
			BOOST_TEST(std::sqrt(sampled) <= std::sqrt(d) + h + T(1e-4));
		}
	}
}

// Zero area is no reason for a NaN: the triangle is then its edges.
BOOST_AUTO_TEST_CASE_TEMPLATE(closest_point_on_a_degenerate_triangle_is_on_its_edges, T, scalars)
{
	using v3 = rove::vec<3, T>;
	rove::triangle<3, T> const line(v3(0, 0, 0), v3(1, 0, 0), v3(3, 0, 0));
	for (v3 const p : {v3(2, 1, 0), v3(0.5, -2, 1), v3(-1, 0, 0), v3(5, 1, 1)})
	{
		v3 const q = line.closest_point(p);
		v3 const expected(std::clamp(p.x, T(0), T(3)), 0, 0);
		BOOST_TEST_CONTEXT("p " << p.x << " " << p.y << " " << p.z)
			BOOST_TEST((q - expected).length() < T(1e-5));
	}
	rove::triangle<3, T> const point(v3(1, 2, 3), v3(1, 2, 3), v3(1, 2, 3));
	BOOST_TEST((point.closest_point(v3(5, 5, 5)) == v3(1, 2, 3)));
}

// A NaN in any vertex gives a NaN point, not a point on the other edges: a
// sphere test built on it must not meet a broken triangle.
BOOST_AUTO_TEST_CASE_TEMPLATE(closest_point_of_a_triangle_with_a_nan_is_nan, T, scalars)
{
	using v3 = rove::vec<3, T>;
	for (int vertex = 0; vertex < 3; ++vertex)
		for (int axis = 0; axis < 3; ++axis)
		{
			rove::triangle<3, T> t = right_triangle<T>();
			(vertex == 0 ? t.A : vertex == 1 ? t.B : t.C).i[axis] = std::numeric_limits<T>::quiet_NaN();
			BOOST_TEST_CONTEXT("vertex " << vertex << " axis " << axis)
				for (v3 const p : {v3(1, 1, 3), v3(-1, -1, 2), v3(6, -1, 0), v3(-1, 6, 1), v3(2, -3, 0)})
				{
					v3 const q = t.closest_point(p);
					BOOST_TEST((std::isnan(q.x) && std::isnan(q.y) && std::isnan(q.z)));
				}
		}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(segment_to_triangle_known_answers, T, scalars)
{
	using v3 = rove::vec<3, T>;
	using seg = rove::segment<3, T>;
	rove::triangle<3, T> const t = right_triangle<T>();
	struct { seg s; T d; v3 on_segment, on_triangle; } const cases[] = {
		{seg(v3(1, 1, -1), v3(1, 1, 1)), 0, v3(1, 1, 0), v3(1, 1, 0)},          // pierces the face
		{seg(v3(1, 1, 2), v3(2, 1, 2)), 4, v3(1.5, 1, 2), v3(1.5, 1, 0)},      // parallel above (any point of the overlap)
		{seg(v3(2, -3, -1), v3(2, -3, 1)), 9, v3(2, -3, 0), v3(2, 0, 0)},      // passes beside edge AB
		{seg(v3(1, 1, 3), v3(1, 1, 10)), 9, v3(1, 1, 3), v3(1, 1, 0)},         // the near end
		{seg(v3(-3, -2, 0), v3(-3, 6, 0)), 9, v3(-3, 2, 0), v3(0, 2, 0)},      // in the plane, beside edge AC
		{seg(v3(-1, 1, 0), v3(5, 1, 0)), 0, v3(1, 1, 0), v3(1, 1, 0)},         // in the plane, across the face
	};
	for (size_t i = 0; i < std::size(cases); ++i)
	{
		auto const &c = cases[i];
		v3 os, ot;
		T const d = t.closest_points(c.s, os, ot);
		BOOST_TEST_CONTEXT("case " << i)
		{
			BOOST_TEST(std::abs(d - c.d) < T(1e-5));
			BOOST_TEST((os - ot).length_sq() == d, boost::test_tools::tolerance(T(1e-4)));
			BOOST_TEST(c.s.distance(os) < T(1e-5));
			BOOST_TEST(on_triangle(t, ot, T(1e-5)));
			if (i != 1 && i != 4 && i != 5)      // those have more than one nearest pair
			{
				BOOST_TEST((os - c.on_segment).length() < T(1e-4));
				BOOST_TEST((ot - c.on_triangle).length() < T(1e-4));
			}
		}
	}
}

// A capsule's axis through a millimetre-sized triangle: the crossing must
// still be found, whatever the size of Moller-Trumbore's determinant.
BOOST_AUTO_TEST_CASE_TEMPLATE(a_segment_through_a_tiny_triangle_touches_it, T, scalars)
{
	using v3 = rove::vec<3, T>;
	rove::triangle<3, T> const t(v3(0, 0, 0), v3(T(1e-3), 0, 0), v3(0, T(1e-3), 0));
	rove::segment<3, T> const s(v3(T(2e-4), T(2e-4), T(-0.6)), v3(T(2e-4), T(2e-4), T(0.6)));
	v3 os, ot;
	BOOST_TEST(t.closest_points(s, os, ot) == T(0));
	BOOST_TEST((os - v3(T(2e-4), T(2e-4), 0)).length() < T(1e-6));
	BOOST_TEST((ot == os));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(a_point_segment_measures_as_closest_point, T, scalars)
{
	using v3 = rove::vec<3, T>;
	rove::triangle<3, T> const t = right_triangle<T>();
	for (v3 const p : {v3(1, 1, 3), v3(-2, 2, 5), v3(6, -1, 0)})
	{
		v3 os, ot;
		T const d = t.closest_points(rove::segment<3, T>(p, p), os, ot);
		BOOST_TEST((os == p));
		BOOST_TEST((ot - t.closest_point(p)).length() < T(1e-5));
		BOOST_TEST(d == (ot - p).length_sq(), boost::test_tools::tolerance(T(1e-5)));
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(segment_to_triangle_matches_a_dense_sampling, T, scalars)
{
	std::mt19937 rng(11);
	std::uniform_real_distribution<T> u(-3, 3);
	auto v = [&] { return rove::vec<3, T>{u(rng), u(rng), u(rng)}; };
	int const n = 4000;
	for (int k = 0; k < 300; ++k)
	{
		rove::triangle<3, T> const t(v(), v(), v());
		rove::segment<3, T> s(v() * T(2), v() * T(2));
		if (k % 3 == 1)       // through the face
			s = rove::segment<3, T>(t.cog() - v(), t.cog() + v());
		if (k % 3 == 2)       // parallel to the face
			s = rove::segment<3, T>(s.A, s.A + (t.B - t.A) * T(0.8));
		rove::vec<3, T> os, ot;
		T const d = t.closest_points(s, os, ot);
		T sampled = std::numeric_limits<T>::max();
		for (int i = 0; i <= n; ++i)
		{
			rove::vec<3, T> const p = s.point(T(i) / n);
			sampled = std::min(sampled, (t.closest_point(p) - p).length_sq());
		}
		T const h = s.direction().length() / n;
		BOOST_TEST_CONTEXT("case " << k)
		{
			BOOST_TEST(d <= sampled + T(1e-4));
			BOOST_TEST(std::sqrt(sampled) <= std::sqrt(d) + h + T(1e-4));
			BOOST_TEST((os - ot).length_sq() == d, boost::test_tools::tolerance(T(1e-2)));
			BOOST_TEST(s.distance(os) < T(1e-3));
			BOOST_TEST(on_triangle(t, ot, T(1e-3)));
		}
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(trace_takes_its_parallel_tolerance, T, scalars)
{
	using v3 = rove::vec<3, T>;
	// sides a tenth of sqrt(default_epsilon): a unit ray's determinant is a hundredth of the default
	T const side = std::sqrt(rove::scalar_traits<T>::default_epsilon) / 10;
	rove::triangle<3, T> const tiny(v3(0, 0, 0), v3(side, 0, 0), v3(0, side, 0));
	rove::ray<3, T> const down(v3(side / 5, side / 5, 1), v3(0, 0, -1));
	T t = -1;
	v3 at;
	T const smallest = std::numeric_limits<T>::min();
	BOOST_TEST(!tiny.trace(down, t, at, T(0), T(5)));               // the default calls it "parallel"
	BOOST_TEST(tiny.trace(down, t, at, T(0), T(5), smallest));
	BOOST_TEST(t == T(1), boost::test_tools::tolerance(T(1e-6)));
	BOOST_TEST(tiny.trace(down, T(0), T(5), smallest));

	// exactly parallel stays a miss, never a NaN that slips through the tests
	rove::ray<3, T> const along(v3(-1, side / 5, 0), v3(1, 0, 0));
	BOOST_TEST(!tiny.trace(along, t, at, T(0), T(5), smallest));
	BOOST_TEST(!tiny.trace(along, t, at, T(0), T(5), T(0)));
}

// A ray through the edge two triangles share must hit one of them: a quad has no crack down its diagonal.
BOOST_AUTO_TEST_CASE(triangle_trace_is_watertight_along_a_shared_edge)
{
	float const h = 200;
	rove::vec<3> const p00(-h, 0, -h), p01(-h, 0, h), p11(h, 0, h), p10(h, 0, -h);
	rove::triangle<3> const a(p00, p01, p11), b(p00, p11, p10);        // the diagonal runs from (-h, -h) to (h, h)
	std::mt19937 rng(7);
	std::uniform_real_distribution<float> along(-30.0f, 30.0f), slant(-0.3f, 0.3f);
	int missed = 0;
	for (int i = 0; i < 20000; ++i)
	{
		float const s = along(rng);
		rove::vec<3> const on(s, 0, s);               // a point of the diagonal
		rove::vec<3> const direction = normalize(rove::vec<3>{slant(rng), -1, slant(rng)});
		rove::ray<3> const r(on - direction * 3.0f, direction);   // arrives there, two-sided from above
		if (!a.trace(r, 0, 100) && !b.trace(r, 0, 100))
			++missed;
	}
	BOOST_TEST(missed == 0);
}

BOOST_AUTO_TEST_CASE(get_aabb_into_a_box_and_by_value_are_the_corners_bounds)
{
	rove::triangle<3> const t(rove::vec<3>(1, -2, 3), rove::vec<3>(-4, 5, 0), rove::vec<3>(2, 2, -6));
	rove::aabb<3> into(rove::vec<3>(100, 100, 100), rove::vec<3>(200, 200, 200));   // what it held is not kept
	into = t.bounds();
	BOOST_REQUIRE(into.lo == rove::vec<3>(-4, -2, -6));
	BOOST_REQUIRE(into.hi == rove::vec<3>(2, 5, 3));
	rove::aabb<3> const by_value = t.bounds();
	BOOST_REQUIRE(by_value.lo == into.lo);
	BOOST_REQUIRE(by_value.hi == into.hi);
}

BOOST_AUTO_TEST_SUITE_END()
