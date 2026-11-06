// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <rove/segment.h>
#include <rove/capsule.h>
#include <rove/sphere.h>
#include "test_garbage.h"
#include <rove/ray.h>
#include <rove/aabb.h>
#include "test_tolerance.h"

BOOST_AUTO_TEST_SUITE(test_sphere)

// Construction tests
BOOST_AUTO_TEST_CASE(construct_from_center_radius_3d)
{
	rove::vec<3> center(1, 2, 3);
	rove::scalar radius = 5.0f;
	rove::sphere<3> s(center, radius);

	BOOST_REQUIRE((s.centre - center).length() < TOLERANCE);
	BOOST_REQUIRE(rove::abs(s.radius - radius) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_from_center_radius_2d)
{
	rove::vec<2> center(10, 20);
	rove::scalar radius = 3.0f;
	rove::sphere<2> s(center, radius);

	BOOST_REQUIRE((s.centre - center).length() < TOLERANCE);
	BOOST_REQUIRE(rove::abs(s.radius - radius) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_from_aabb_3d)
{
	rove::aabb<3> box(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::sphere<3> s(box);

	// Center should be at (5, 5, 5)
	BOOST_REQUIRE((s.centre - rove::vec<3>(5, 5, 5)).length() < TOLERANCE);

	// Radius should reach from center to corner
	rove::scalar expected_radius = (rove::vec<3>(10, 10, 10) - rove::vec<3>(5, 5, 5)).length();
	BOOST_REQUIRE(rove::abs(s.radius - expected_radius) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_from_aabb_2d)
{
	rove::aabb<2> box(rove::vec<2>(0, 0), rove::vec<2>(10, 10));
	rove::sphere<2> s(box);

	BOOST_REQUIRE((s.centre - rove::vec<2>(5, 5)).length() < TOLERANCE);
}

// Set method test
BOOST_AUTO_TEST_CASE(set_sphere)
{
	rove::sphere<3> s;
	s.set(rove::vec<3>(1, 2, 3), 4.0f);

	BOOST_REQUIRE((s.centre - rove::vec<3>(1, 2, 3)).length() < TOLERANCE);
	BOOST_REQUIRE(rove::abs(s.radius - 4.0f) < TOLERANCE);
}

// Get AABB tests
BOOST_AUTO_TEST_CASE(get_aabb_3d)
{
	rove::sphere<3> s(rove::vec<3>(5, 5, 5), 3.0f);
	rove::aabb<3> box = s.bounds();

	BOOST_REQUIRE((box.lo - rove::vec<3>(2, 2, 2)).length() < TOLERANCE);
	BOOST_REQUIRE((box.hi - rove::vec<3>(8, 8, 8)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(get_aabb_2d)
{
	rove::sphere<2> s(rove::vec<2>(10, 10), 5.0f);
	rove::aabb<2> box = s.bounds();

	BOOST_REQUIRE((box.lo - rove::vec<2>(5, 5)).length() < TOLERANCE);
	BOOST_REQUIRE((box.hi - rove::vec<2>(15, 15)).length() < TOLERANCE);
}

// Contains point tests
BOOST_AUTO_TEST_CASE(contains_point_inside_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 10.0f);

	BOOST_REQUIRE(s.contains(rove::vec<3>(0, 0, 0))); // center
	BOOST_REQUIRE(s.contains(rove::vec<3>(5, 0, 0))); // inside
	BOOST_REQUIRE(s.contains(rove::vec<3>(7, 7, 0))); // inside
}

BOOST_AUTO_TEST_CASE(contains_point_outside_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 10.0f);

	BOOST_REQUIRE(!s.contains(rove::vec<3>(15, 0, 0)));
	BOOST_REQUIRE(!s.contains(rove::vec<3>(8, 8, 8)));
}

BOOST_AUTO_TEST_CASE(contains_point_on_surface_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 10.0f);

	// Point exactly on surface
	rove::vec<3> surface_point(10, 0, 0);
	// Due to floating point, we use < instead of <=, so this might not be contained
	// but a point slightly inside should be
	rove::vec<3> just_inside(9.99f, 0, 0);
	BOOST_REQUIRE(s.contains(just_inside));
}

BOOST_AUTO_TEST_CASE(contains_point_2d)
{
	rove::sphere<2> s(rove::vec<2>(5, 5), 3.0f);

	BOOST_REQUIRE(s.contains(rove::vec<2>(5, 5)));
	BOOST_REQUIRE(s.contains(rove::vec<2>(6, 6)));
	BOOST_REQUIRE(!s.contains(rove::vec<2>(10, 10)));
}

// Sphere-sphere intersection tests
BOOST_AUTO_TEST_CASE(sphere_sphere_intersection_overlapping_3d)
{
	rove::sphere<3> s1(rove::vec<3>(0, 0, 0), 5.0f);
	rove::sphere<3> s2(rove::vec<3>(5, 0, 0), 5.0f);

	BOOST_REQUIRE(s1.test_intersection(s2));
	BOOST_REQUIRE(s2.test_intersection(s1));
}

BOOST_AUTO_TEST_CASE(sphere_sphere_intersection_touching_3d)
{
	rove::sphere<3> s1(rove::vec<3>(0, 0, 0), 5.0f);
	rove::sphere<3> s2(rove::vec<3>(10, 0, 0), 5.0f);

	BOOST_REQUIRE(s1.test_intersection(s2));
}

BOOST_AUTO_TEST_CASE(sphere_sphere_intersection_separated_3d)
{
	rove::sphere<3> s1(rove::vec<3>(0, 0, 0), 5.0f);
	rove::sphere<3> s2(rove::vec<3>(20, 0, 0), 5.0f);

	BOOST_REQUIRE(!s1.test_intersection(s2));
}

BOOST_AUTO_TEST_CASE(sphere_sphere_intersection_contained_3d)
{
	rove::sphere<3> s1(rove::vec<3>(0, 0, 0), 10.0f);
	rove::sphere<3> s2(rove::vec<3>(0, 0, 0), 5.0f);

	BOOST_REQUIRE(s1.test_intersection(s2));
	BOOST_REQUIRE(s2.test_intersection(s1));
}

BOOST_AUTO_TEST_CASE(sphere_sphere_intersection_2d)
{
	rove::sphere<2> s1(rove::vec<2>(0, 0), 5.0f);
	rove::sphere<2> s2(rove::vec<2>(5, 0), 5.0f);

	BOOST_REQUIRE(s1.test_intersection(s2));
}

// Sphere-ray intersection tests
BOOST_AUTO_TEST_CASE(sphere_ray_intersection_through_center_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::ray<3> r(rove::vec<3>(-10, 0, 0), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(s.test_intersection(r));
}

BOOST_AUTO_TEST_CASE(sphere_ray_intersection_tangent_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::ray<3> r(rove::vec<3>(-10, 5, 0), rove::vec<3>(1, 0, 0));

	// Ray is tangent to sphere
	BOOST_REQUIRE(s.test_intersection(r));
}

BOOST_AUTO_TEST_CASE(sphere_ray_intersection_miss_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::ray<3> r(rove::vec<3>(-10, 10, 0), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(!s.test_intersection(r));
}

BOOST_AUTO_TEST_CASE(sphere_ray_intersection_from_inside_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::ray<3> r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(s.test_intersection(r));
}

BOOST_AUTO_TEST_CASE(sphere_ray_query_intersection_two_points_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::ray<3> r(rove::vec<3>(-10, 0, 0), rove::vec<3>(1, 0, 0));

	rove::vec<3> p1, p2;
	int npoints = s.query_intersection(r, p1, p2);

	BOOST_REQUIRE(npoints == 2);
	// Verify both intersection points are on the sphere surface
	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
	BOOST_REQUIRE(rove::abs((p2 - s.centre).length() - s.radius) < 0.01f);
	// Verify they are distinct points (entry and exit points)
	BOOST_REQUIRE((p1 - p2).length() > 1.0f);
}

BOOST_AUTO_TEST_CASE(sphere_ray_query_intersection_no_points_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::ray<3> r(rove::vec<3>(-10, 10, 0), rove::vec<3>(1, 0, 0));

	rove::vec<3> p1, p2;
	int npoints = s.query_intersection(r, p1, p2);

	BOOST_REQUIRE(npoints == 0);
}

BOOST_AUTO_TEST_CASE(sphere_ray_intersection_2d)
{
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::ray<2> r(rove::vec<2>(-10, 0), rove::vec<2>(1, 0));

	BOOST_REQUIRE(s.test_intersection(r));

	rove::vec<2> p1, p2;
	int npoints = s.query_intersection(r, p1, p2);
	BOOST_REQUIRE(npoints == 2);
}

// Sphere-line intersection tests
BOOST_AUTO_TEST_CASE (test_sphere_line_intersection)
{
	rove::sphere<2> s;
	s.centre.set(1, 0);
	s.radius = 1;

	rove::segment<2> l;
	l.A.set(0, 1);
	l.B.set(0, -1);

	rove::vec<2> p1, p2;
	int npoints = s.query_intersection(l, p1, p2);

	BOOST_REQUIRE (npoints == 1);
	BOOST_REQUIRE ((p1 - rove::vec<2>(0, 0)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(sphere_line_intersection_two_points_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::segment<3> l(rove::vec<3>(-10, 0, 0), rove::vec<3>(10, 0, 0));

	BOOST_REQUIRE(s.test_intersection(l));

	rove::vec<3> p1, p2;
	int npoints = s.query_intersection(l, p1, p2);
	BOOST_REQUIRE(npoints == 2);
}

BOOST_AUTO_TEST_CASE(sphere_line_intersection_miss_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::segment<3> l(rove::vec<3>(-10, 10, 0), rove::vec<3>(10, 10, 0));

	BOOST_REQUIRE(!s.test_intersection(l));

	rove::vec<3> p1, p2;
	int npoints = s.query_intersection(l, p1, p2);
	BOOST_REQUIRE(npoints == 0);
}

BOOST_AUTO_TEST_CASE(sphere_line_intersection_tangent_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::segment<3> l(rove::vec<3>(-10, 5, 0), rove::vec<3>(10, 5, 0));

	BOOST_REQUIRE(s.test_intersection(l));

	rove::vec<3> p1, p2;
	int npoints = s.query_intersection(l, p1, p2);
	BOOST_REQUIRE(npoints >= 1); // tangent should give 1 point
}

BOOST_AUTO_TEST_CASE(sphere_line_partial_intersection_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::segment<3> l(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 0, 0));

	// Line starts at center
	BOOST_REQUIRE(s.test_intersection(l));

	rove::vec<3> p1, p2;
	int npoints = s.query_intersection(l, p1, p2);
	BOOST_REQUIRE(npoints >= 1);
}

// Sphere-capsule intersection tests
BOOST_AUTO_TEST_CASE (test_sphere_capsule_intersection_1)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 0.5f);
	rove::capsule<3> c(rove::vec<3>(0.7f, -1, 0), rove::vec<3>(0.7f, 1, 0), 0.5f);

	BOOST_REQUIRE (s.test_intersection(c) == true);
}

BOOST_AUTO_TEST_CASE (test_sphere_capsule_intersection_2)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 0.5f);
	rove::capsule<3> c(rove::vec<3>(1.7f, -1, 0), rove::vec<3>(1.7f, 1, 0), 0.5f);

	BOOST_REQUIRE (s.test_intersection(c) == false);
}

BOOST_AUTO_TEST_CASE(sphere_capsule_intersection_overlapping_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 5.0f);
	rove::capsule<3> c(rove::vec<3>(-5, 0, 0), rove::vec<3>(5, 0, 0), 2.0f);

	BOOST_REQUIRE(s.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(sphere_capsule_intersection_separated_3d)
{
	rove::sphere<3> s(rove::vec<3>(0, 0, 0), 1.0f);
	rove::capsule<3> c(rove::vec<3>(10, 0, 0), rove::vec<3>(20, 0, 0), 1.0f);

	BOOST_REQUIRE(!s.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(sphere_capsule_intersection_2d)
{
	rove::sphere<2> s(rove::vec<2>(0, 0), 2.0f);
	rove::capsule<2> c(rove::vec<2>(3, 0), rove::vec<2>(5, 0), 1.5f);

	BOOST_REQUIRE(s.test_intersection(c));
}

// ──────────────────────────────────────────────────────────────────────────────
// sphere<2> / ray<2> — comprehensive 2D tests
// ──────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(sphere2_ray_miss)
{
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::ray<2> r(rove::vec<2>(-10, 10), rove::vec<2>(1, 0));

	BOOST_REQUIRE(!s.test_intersection(r));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(r, p1, p2) == 0);
}

BOOST_AUTO_TEST_CASE(sphere2_ray_tangent)
{
	// Ray along y=5 is tangent to unit-5 circle at (0,5)
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::ray<2> r(rove::vec<2>(-10, 5), rove::vec<2>(1, 0));

	BOOST_REQUIRE(s.test_intersection(r));

	rove::vec<2> p1, p2;
	int n = s.query_intersection(r, p1, p2);
	BOOST_REQUIRE(n == 1);
	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
	BOOST_REQUIRE(rove::abs(p1.x) < 0.01f);
	BOOST_REQUIRE(rove::abs(p1.y - 5.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(sphere2_ray_from_inside)
{
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::ray<2> r(rove::vec<2>(0, 0), rove::vec<2>(1, 0));

	BOOST_REQUIRE(s.test_intersection(r));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(r, p1, p2) == 2);
}

BOOST_AUTO_TEST_CASE(sphere2_ray_two_points_on_surface)
{
	// Ray from (-10,0) along +x through circle at origin, radius 5.
	// Entry at (-5,0), exit at (5,0).
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::ray<2> r(rove::vec<2>(-10, 0), rove::vec<2>(1, 0));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(r, p1, p2) == 2);

	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
	BOOST_REQUIRE(rove::abs((p2 - s.centre).length() - s.radius) < 0.01f);
	// Points must be distinct
	BOOST_REQUIRE((p1 - p2).length() > 1.0f);
	// Both lie on the x-axis
	BOOST_REQUIRE(rove::abs(p1.y) < 0.01f);
	BOOST_REQUIRE(rove::abs(p2.y) < 0.01f);
	// x-coordinates are ±5
	BOOST_REQUIRE(rove::abs(rove::abs(p1.x) - 5.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(rove::abs(p2.x) - 5.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(sphere2_ray_offset_circle)
{
	// Circle not at origin
	rove::sphere<2> s(rove::vec<2>(3, 4), 2.0f);
	rove::ray<2> r(rove::vec<2>(-5, 4), rove::vec<2>(1, 0));

	BOOST_REQUIRE(s.test_intersection(r));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(r, p1, p2) == 2);
	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
	BOOST_REQUIRE(rove::abs((p2 - s.centre).length() - s.radius) < 0.01f);
}

// ──────────────────────────────────────────────────────────────────────────────
// sphere<2> / segment<2> — comprehensive 2D tests
// ──────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(sphere2_line_two_points)
{
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::segment<2> l(rove::vec<2>(-10, 0), rove::vec<2>(10, 0));

	BOOST_REQUIRE(s.test_intersection(l));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(l, p1, p2) == 2);
	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
	BOOST_REQUIRE(rove::abs((p2 - s.centre).length() - s.radius) < 0.01f);
}

BOOST_AUTO_TEST_CASE(sphere2_line_miss)
{
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::segment<2> l(rove::vec<2>(-10, 10), rove::vec<2>(10, 10));

	BOOST_REQUIRE(!s.test_intersection(l));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(l, p1, p2) == 0);
}

BOOST_AUTO_TEST_CASE(sphere2_line_too_short)
{
	// Segment from (-10,0) to (-7,0) — sphere at origin r=5 would be entered at
	// (-5,0), but the segment ends before reaching it.
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::segment<2> l(rove::vec<2>(-10, 0), rove::vec<2>(-7, 0));

	BOOST_REQUIRE(!s.test_intersection(l));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(l, p1, p2) == 0);
}

BOOST_AUTO_TEST_CASE(sphere2_line_starts_inside)
{
	// Segment starts at circle centre and exits through the surface — one hit.
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::segment<2> l(rove::vec<2>(0, 0), rove::vec<2>(10, 0));

	BOOST_REQUIRE(s.test_intersection(l));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(l, p1, p2) == 1);
	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
}

BOOST_AUTO_TEST_CASE(sphere2_line_tangent)
{
	// Segment along y=5 is tangent to circle at (0,5)
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f);
	rove::segment<2> l(rove::vec<2>(-10, 5), rove::vec<2>(10, 5));

	BOOST_REQUIRE(s.test_intersection(l));

	rove::vec<2> p1, p2;
	int n = s.query_intersection(l, p1, p2);
	BOOST_REQUIRE(n >= 1);
	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
}

BOOST_AUTO_TEST_CASE(sphere2_line_diagonal_two_points)
{
	// Circle at origin r=5√2, diagonal segment from (-10,-10) to (10,10).
	// Intersections at (5,5) and (-5,-5).
	rove::sphere<2> s(rove::vec<2>(0, 0), 5.0f * std::sqrt(2.0f));
	rove::segment<2> l(rove::vec<2>(-10, -10), rove::vec<2>(10, 10));

	BOOST_REQUIRE(s.test_intersection(l));

	rove::vec<2> p1, p2;
	BOOST_REQUIRE(s.query_intersection(l, p1, p2) == 2);
	BOOST_REQUIRE(rove::abs((p1 - s.centre).length() - s.radius) < 0.01f);
	BOOST_REQUIRE(rove::abs((p2 - s.centre).length() - s.radius) < 0.01f);
}

BOOST_AUTO_TEST_CASE(default_is_a_point_at_the_origin_not_whatever_the_memory_held)
{
	rove::sphere<3> const s = constructed_over_garbage<rove::sphere<3>>();
	BOOST_REQUIRE(s.centre == rove::vec<3>(0, 0, 0));
	BOOST_REQUIRE_EQUAL(s.radius, 0.0f);
}

BOOST_AUTO_TEST_CASE(contains_takes_the_surface_as_inside_as_aabb_and_capsule_do)
{
	rove::sphere<3> const s(rove::vec<3>(0, 0, 0), 10.0f);
	BOOST_REQUIRE(s.contains(rove::vec<3>(10, 0, 0)));
	BOOST_REQUIRE(s.contains(rove::vec<3>(0, -10, 0)));
	BOOST_REQUIRE(!s.contains(rove::vec<3>(10.001f, 0, 0)));
	// a sphere of no radius holds its centre and nothing else, as a box of no size holds its one point
	rove::sphere<2> const point(rove::vec<2>(1, 2), 0.0f);
	BOOST_REQUIRE(point.contains(rove::vec<2>(1, 2)));
	BOOST_REQUIRE(!point.contains(rove::vec<2>(1, 2.001f)));
}

BOOST_AUTO_TEST_SUITE_END()
