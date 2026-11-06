// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <rove/aabb.h>
#include <rove/obb.h>
#include <rove/plane.h>
#include "test_garbage.h"
#include "test_tolerance.h"

BOOST_AUTO_TEST_SUITE(plane)

BOOST_AUTO_TEST_CASE(get_origin_1)
{
	rove::plane<> p1(rove::vec<3>(1, -3, 5), rove::vec<3>(-7, 11, -13));
	rove::plane<> p2(p1.origin(), p1.normal());

	BOOST_REQUIRE(rove::abs(p1.A - p2.A) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(p1.B - p2.B) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(p1.C - p2.C) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(p1.D - p2.D) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(classify_point_1)
{
	rove::plane<> pl(rove::vec<3>(0, -1, 0), rove::vec<3>(1, 1, 1));

	BOOST_REQUIRE(pl.classify(rove::vec<3>(0, -2, 0)) == rove::plane<>::negative);
	BOOST_REQUIRE(pl.classify(rove::vec<3>(0, 0, 0)) == rove::plane<>::positive);
	BOOST_REQUIRE(pl.classify(rove::vec<3>(0, -1, 0)) == rove::plane<>::intersects);
}

BOOST_AUTO_TEST_CASE(classify_point_2)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(1, -1, 1));

	BOOST_REQUIRE(pl.classify(rove::vec<3>(-2, -1.5, -2)) == rove::plane<>::negative);
	BOOST_REQUIRE(pl.classify(rove::vec<3>(-1, -2, -1)) == rove::plane<>::intersects);
}

BOOST_AUTO_TEST_CASE(classify_aabb_1)
{
	rove::plane<> pl(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1));

	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(-1, -1.5, -1))) == rove::plane<>::negative);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(-1, -1, -1))) == rove::plane<>::intersects);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1))) == rove::plane<>::positive);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1))) == rove::plane<>::intersects);
}

BOOST_AUTO_TEST_CASE(classify_aabb_2)
{
	rove::plane<> pl(rove::vec<3>(-1, -1, -1), rove::vec<3>(-1, -1, -1));

	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(-1, -1.5, -1))) == rove::plane<>::positive);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(-1, -1, -1))) == rove::plane<>::intersects);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1))) == rove::plane<>::negative);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1))) == rove::plane<>::intersects);
}

BOOST_AUTO_TEST_CASE(classify_aabb_3)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(1, -1, 1));

	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(0, 4, 0), rove::vec<3>(1, 5, 1))) == rove::plane<>::negative);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-1, -5, -1), rove::vec<3>(0, -4, 0))) == rove::plane<>::positive);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(0, 0, 0))) == rove::plane<>::intersects);
	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1))) == rove::plane<>::intersects);
}

BOOST_AUTO_TEST_CASE(classify_aabb_4)
{
	rove::plane<> pl(rove::vec<3>(0, 2, 0), rove::vec<3>(1, 1, 1));

	BOOST_REQUIRE(pl.classify(rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(0, 0, 0))) == rove::plane<>::negative);
}

BOOST_AUTO_TEST_CASE(classify_obb_1)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));

	BOOST_REQUIRE(pl.classify(
			rove::obb<3>(rove::vec<3>(0, 1, 0), rove::vec<3>(1, 0, 0), rove::vec<3>(0, 1, 0), rove::vec<3>(0, 0, 1)))
		== rove::plane<>::positive);

	BOOST_REQUIRE(pl.classify(
			rove::obb<3>(rove::vec<3>(-1, -2, -1), rove::vec<3>(1, 0, 0), rove::vec<3>(0, 1, 0), rove::vec<3>(0, 0, 1)))
		== rove::plane<>::negative);
}

BOOST_AUTO_TEST_CASE(classify_obb_2)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));

	BOOST_REQUIRE(pl.classify(
			rove::obb<3>(
				rove::vec<3>(-1, -1, -1),
				rove::vec<3>(11, 10, 10),
				rove::vec<3>(10, 11, 10),
				rove::vec<3>(10, 10, 11)))
		== rove::plane<>::intersects);
}

BOOST_AUTO_TEST_CASE(classify_obb_3)
{
	rove::plane<> pl;
	pl.A =  0.00000000f;
	pl.B = -0.26937401f;
	pl.C = -0.26937401f;
	pl.D =  1.00618340f;

	rove::obb<3> bound;
	bound.origin.set(-5.6245117f, 0.00000000f, 23.716305f);
	bound.tangent.set(2.0000000f, 0.00000000f, 0.00000000f);
	bound.normal.set(0.00000000f, 1.0000000f, 0.00000000f);
	bound.binormal.set(0.00000000f, 0.00000000f, 2.0000000f);

	BOOST_REQUIRE (pl.classify(bound.origin) == rove::plane<>::negative);
	BOOST_REQUIRE (pl.classify(bound.origin + bound.tangent) == rove::plane<>::negative);
	BOOST_REQUIRE (pl.classify(bound.origin + bound.normal) == rove::plane<>::negative);
	BOOST_REQUIRE (pl.classify(bound.origin + bound.binormal) == rove::plane<>::negative);

	BOOST_REQUIRE (pl.classify(bound) == rove::plane<>::negative);
}

BOOST_AUTO_TEST_CASE(raytrace_1)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r(rove::vec<3>(0, 1, 0), rove::vec<3>(0, -10, 0));

	rove::scalar t;
	rove::vec<3> v;

	BOOST_REQUIRE(pl.trace(r, t, v));

	BOOST_REQUIRE((v - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(raytrace_2)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r(rove::vec<3>(7, 1, 11), rove::vec<3>(0, -10, 0));

	rove::scalar t;
	rove::vec<3> v;

	BOOST_REQUIRE(pl.trace(r, t, v));

	BOOST_REQUIRE((v - rove::vec<3>(7, 0, 11)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(raytrace_3)
{
	rove::plane<> pl(rove::vec<3>(1, 3, 5), rove::vec<3>(0, 1, 0));

	rove::ray<3> r(rove::vec<3>(7 + 1, 1 + 3, 11 + 5), rove::vec<3>(0, -10, 0));

	rove::scalar t;
	rove::vec<3> v;

	BOOST_REQUIRE(pl.trace(r, t, v));

	BOOST_REQUIRE((v - rove::vec<3>(7 + 1, 0 + 3, 11 + 5)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(raytrace_4)
{
	rove::plane<> pl(rove::vec<3>(1, 3, 5), rove::vec<3>(1, 1, 1));

	rove::ray<3> r(rove::vec<3>(1, 10, 5), rove::vec<3>(0, -10, 0));

	rove::scalar t;
	rove::vec<3> v;

	BOOST_REQUIRE(pl.trace(r, t, v));

	BOOST_REQUIRE((v - rove::vec<3>(1, 3, 5)).length() < TOLERANCE);
}

// Additional comprehensive tests

// Plane construction tests
BOOST_AUTO_TEST_CASE(construct_from_origin_normal)
{
	rove::vec<3> origin(1, 2, 3);
	rove::vec<3> normal(0, 1, 0);
	rove::plane<> pl(origin, normal);

	rove::vec<3> recovered_normal = pl.normal();
	BOOST_REQUIRE((recovered_normal - normal).length() < TOLERANCE);
	BOOST_REQUIRE(pl.contains(origin));
}

BOOST_AUTO_TEST_CASE(construct_from_d_normal)
{
	rove::vec<3> normal(0, 1, 0);
	rove::scalar d = -5.0f;
	rove::plane<> pl(d, normal);

	BOOST_REQUIRE(rove::abs(pl.A - normal.x) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(pl.B - normal.y) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(pl.C - normal.z) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(pl.D - d) < TOLERANCE);
}

// Apply and contains tests
BOOST_AUTO_TEST_CASE(apply_to_point_on_plane)
{
	rove::plane<> pl(rove::vec<3>(0, 5, 0), rove::vec<3>(0, 1, 0));

	rove::scalar result = pl.apply(rove::vec<3>(0, 5, 0));
	BOOST_REQUIRE(rove::abs(result) < TOLERANCE);

	result = pl.apply(rove::vec<3>(10, 5, 10));
	BOOST_REQUIRE(rove::abs(result) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(contains_point_on_plane)
{
	rove::plane<> pl(rove::vec<3>(0, 5, 0), rove::vec<3>(0, 1, 0));

	BOOST_REQUIRE(pl.contains(rove::vec<3>(0, 5, 0)));
	BOOST_REQUIRE(pl.contains(rove::vec<3>(100, 5, -50)));
	BOOST_REQUIRE(!pl.contains(rove::vec<3>(0, 6, 0)));
}

BOOST_AUTO_TEST_CASE(contains_ray_on_plane)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r_on_plane(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r_not_on_plane(rove::vec<3>(0, 1, 0), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(pl.contains(r_on_plane));
	BOOST_REQUIRE(!pl.contains(r_not_on_plane));
}

// Parallel and intersection tests
BOOST_AUTO_TEST_CASE(parallel_ray)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r_parallel(rove::vec<3>(5, 5, 5), rove::vec<3>(1, 0, 0));
	rove::ray<3> r_not_parallel(rove::vec<3>(5, 5, 5), rove::vec<3>(0, 1, 0));

	BOOST_REQUIRE(pl.parallel(r_parallel));
	BOOST_REQUIRE(!pl.parallel(r_not_parallel));
}

BOOST_AUTO_TEST_CASE(test_intersection_ray)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r_intersects(rove::vec<3>(0, 5, 0), rove::vec<3>(0, -1, 0));
	rove::ray<3> r_parallel(rove::vec<3>(5, 5, 5), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(pl.test_intersection(r_intersects));
	BOOST_REQUIRE(!pl.test_intersection(r_parallel));
}

// Trace with t parameter tests
BOOST_AUTO_TEST_CASE(trace_with_t_in_range)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r(rove::vec<3>(0, 10, 0), rove::vec<3>(0, -1, 0));

	rove::scalar t;
	BOOST_REQUIRE(pl.trace(r, t, 0, 20));
	BOOST_REQUIRE(rove::abs(t - 10.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(trace_with_t_out_of_range)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r(rove::vec<3>(0, 10, 0), rove::vec<3>(0, -1, 0));

	rove::scalar t;
	// t would be 10, but t_max is 5
	BOOST_REQUIRE(!pl.trace(r, t, 0, 5));
}

// Plane-plane intersection
BOOST_AUTO_TEST_CASE(plane_plane_intersection)
{
	rove::plane<> pl1(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));
	rove::plane<> pl2(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));

	rove::ray<3> intersection_line;
	BOOST_REQUIRE(pl1.query_intersection(pl2, intersection_line));

	// The intersection line should lie on both planes
	BOOST_REQUIRE(pl1.contains(intersection_line.r0));
	BOOST_REQUIRE(pl2.contains(intersection_line.r0));
}

BOOST_AUTO_TEST_CASE(plane_plane_parallel)
{
	rove::plane<> pl1(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));
	rove::plane<> pl2(rove::vec<3>(0, 5, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> intersection_line;
	// Parallel planes don't intersect
	BOOST_REQUIRE(!pl1.query_intersection(pl2, intersection_line));
}

// Get origin and normal tests
BOOST_AUTO_TEST_CASE(plane_normal)
{
	rove::vec<3> normal(1, 2, 3);
	rove::plane<> pl(rove::vec<3>(0, 0, 0), normal);

	rove::vec<3> recovered_normal = pl.normal();
	BOOST_REQUIRE((recovered_normal - normal).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(is_correct)
{
	rove::plane<> pl1(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));
	BOOST_REQUIRE(pl1.is_correct());

	rove::plane<> pl2;
	pl2.A = 0;
	pl2.B = 0;
	pl2.C = 0;
	pl2.D = 5;
	BOOST_REQUIRE(!pl2.is_correct());
}

// Edge cases
BOOST_AUTO_TEST_CASE(raytrace_perpendicular)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r(rove::vec<3>(0, 10, 0), rove::vec<3>(0, -1, 0));

	rove::scalar t = pl.trace(r);
	BOOST_REQUIRE(rove::abs(t - 10.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(raytrace_oblique_angle)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	rove::ray<3> r(rove::vec<3>(0, 10, 0), rove::vec<3>(1, -1, 0));

	rove::scalar t;
	rove::vec<3> intersection;
	BOOST_REQUIRE(pl.trace(r, t, intersection, 0, 100));

	// Check that intersection point is on the plane
	BOOST_REQUIRE(pl.contains(intersection));
}

BOOST_AUTO_TEST_CASE(classify_point_far_positive)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));

	BOOST_REQUIRE(pl.classify(rove::vec<3>(0, 100, 0)) == rove::plane<>::positive);
	BOOST_REQUIRE(pl.classify(rove::vec<3>(0, -100, 0)) == rove::plane<>::negative);
}

BOOST_AUTO_TEST_CASE(classify_aabb_completely_on_one_side)
{
	rove::plane<> pl(rove::vec<3>(0, 10, 0), rove::vec<3>(0, 1, 0));

	rove::aabb<3> box_above(rove::vec<3>(-5, 15, -5), rove::vec<3>(5, 20, 5));
	rove::aabb<3> box_below(rove::vec<3>(-5, 0, -5), rove::vec<3>(5, 5, 5));

	BOOST_REQUIRE(pl.classify(box_above) == rove::plane<>::positive);
	BOOST_REQUIRE(pl.classify(box_below) == rove::plane<>::negative);
}

BOOST_AUTO_TEST_CASE(contains_and_classify_take_their_tolerance)
{
	rove::plane<> pl(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));
	rove::vec<3> const near(0, 0.01f, 0);

	BOOST_TEST(!pl.contains(near));
	BOOST_TEST(pl.contains(near, 0.1f));
	BOOST_TEST(pl.classify(near) == rove::plane<>::positive);
	BOOST_TEST(pl.classify(near, 0.1f) == rove::plane<>::intersects);

	// a ray a hair off parallel: crossing by default, parallel within a looser tolerance
	rove::ray<3> const slant(rove::vec<3>(0, 1, 0), rove::vec<3>(1, 0.01f, 0));
	BOOST_TEST(pl.test_intersection(slant));
	BOOST_TEST(pl.parallel(slant, 0.1f));
}

BOOST_AUTO_TEST_CASE(default_is_all_zero_and_so_no_plane_not_whatever_the_memory_held)
{
	rove::plane<> const p = constructed_over_garbage<rove::plane<>>();
	BOOST_REQUIRE_EQUAL(p.A, 0.0f);
	BOOST_REQUIRE_EQUAL(p.B, 0.0f);
	BOOST_REQUIRE_EQUAL(p.C, 0.0f);
	BOOST_REQUIRE_EQUAL(p.D, 0.0f);
	BOOST_REQUIRE(!p.is_correct());
}

BOOST_AUTO_TEST_CASE(two_planes_meet_on_both_for_a_line_mostly_along_y)
{
	// x = 1 and z = 2, the line along y: the branch that had the point mirrored
	rove::plane<> const a(rove::vec<3>(1, 0, 0), rove::vec<3>(1, 0, 0)), b(rove::vec<3>(0, 0, 2), rove::vec<3>(0, 0, 1));
	rove::ray<3> line;
	BOOST_REQUIRE(a.query_intersection(b, line));
	BOOST_TEST(a.contains(line.r0));
	BOOST_TEST(b.contains(line.r0));
	BOOST_TEST(line.r0.x == 1.0f, boost::test_tools::tolerance(1e-5f));
	BOOST_TEST(line.r0.z == 2.0f, boost::test_tools::tolerance(1e-5f));
	BOOST_TEST(std::abs(line.a.y) == 1.0f, boost::test_tools::tolerance(1e-5f));
	// tilted, each branch of the three lies on both planes
	float const normals[][6] = {
		{1, 0.2f, 0.1f, 0.1f, 0.3f, 1},       // along y mostly
		{1, 0.1f, 0.3f, 0.1f, 1, 0.2f},       // along z mostly
		{0.1f, 1, 0.2f, 0.3f, 0.1f, 1},       // along x mostly
	};
	for (auto const &n : normals) {
		rove::plane<> const p(rove::vec<3>(1, 2, 3), normalize(rove::vec<3>(n[0], n[1], n[2])));
		rove::plane<> const q(rove::vec<3>(-1, 4, 2), normalize(rove::vec<3>(n[3], n[4], n[5])));
		rove::ray<3> l;
		BOOST_REQUIRE(p.query_intersection(q, l));
		BOOST_TEST(p.contains(l.r0, 1e-4f));
		BOOST_TEST(q.contains(l.r0, 1e-4f));
	}
}

BOOST_AUTO_TEST_SUITE_END()

