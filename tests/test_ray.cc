// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <boost/test/unit_test.hpp>
#include <rove/ray.h>
#include <rove/random.h>
#include "test_tolerance.h"

// The inputs these tests draw: one generator for the file, the same numbers on every platform
namespace
{

rove::pcg32 draw(1);

rove::scalar uniform(rove::scalar lo = 0, rove::scalar hi = 1)
{
	return draw.uniform(lo, hi);
}

}

BOOST_AUTO_TEST_SUITE (ray)

BOOST_AUTO_TEST_CASE (ray_point_distance)
{
	rove::ray<3> r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	BOOST_REQUIRE(rove::abs(r.distance_sq(rove::vec<3>(0, 1, 0)) - 1) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(r.distance_sq(rove::vec<3>(100, 1, 0)) - 1) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (ray_ray_distance)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(100, 1, 0), rove::vec<3>(0, 0, 1));

	BOOST_REQUIRE(rove::abs(r1.distance(r2) - 1) < TOLERANCE);
}

// Parallel lines are as far apart as any point of one is from the other.
BOOST_AUTO_TEST_CASE (ray_ray_distance_2)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(100, 1, 0), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(rove::abs(r1.distance(r2) - 1) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (ray_ray_intersection_1a)
{
	rove::ray<2> r1, r2;
	rove::scalar s;

	r1.r0.set(0, 0);
	r1.a.set(1, 1);

	r2.r0.set(1, 0);
	r2.a.set(-1, 1);

	BOOST_REQUIRE (r1.query_intersection(r2, s) == true);

	rove::vec<2> p1 = r1.apply(s);
	BOOST_REQUIRE (r2.contains(p1));

	BOOST_REQUIRE (rove::abs(s - 0.5f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (ray_ray_intersection_1b)
{
	rove::ray<2> r1, r2;
	rove::scalar s;

	r1.r0.set(0.846187f, 0.436567f);
	r1.a.set(0.990295f, -0.700491f);

	r2.r0.set(-0.39787f, -0.0618f);
	r2.a.set(0.818842f, -0.538987f);

	BOOST_REQUIRE (r1.query_intersection(r2, s) == true);

	rove::vec<2> p1 = r1.apply(s);
	BOOST_REQUIRE (r2.contains(p1, 0.001f));
}

BOOST_AUTO_TEST_CASE (ray_ray_intersection_2)
{
	for (int i = 0; i < 1000; i++)
	{
		rove::ray<2> r1(rove::vec<2>(uniform(-1, 1), uniform(-1, 1)), rove::vec<2>(uniform(-1, 1), uniform(-1, 1)));
		rove::ray<2> r2(rove::vec<2>(uniform(-1, 1), uniform(-1, 1)), rove::vec<2>(uniform(-1, 1), uniform(-1, 1)));

		rove::scalar s;

		if (r1.query_intersection(r2, s))
		{
			rove::vec<2> p1 = r1.apply(s);
			BOOST_REQUIRE (r2.contains(p1, 0.001f));
		}
	}
}

BOOST_AUTO_TEST_CASE (ray_ray_intersection_3)
{
	for (int i = 0; i < 1000; i++)
	{
		rove::ray<3> r1(rove::vec<3>(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1)),
			rove::vec<3>(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1)));

		rove::ray<3> r2(rove::vec<3>(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1)),
			rove::vec<3>(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1)));

		rove::scalar s;

		if (r1.query_intersection(r2, s))
		{
			rove::vec<3> p1 = r1.apply(s);
			BOOST_REQUIRE (r2.contains(p1, 0.001f));
		}
	}
}

// Additional comprehensive tests

// Ray construction and basic operations
BOOST_AUTO_TEST_CASE(construct_ray_3d)
{
	rove::vec<3> origin(1, 2, 3);
	rove::vec<3> direction(4, 5, 6);
	rove::ray<3> r(origin, direction);

	BOOST_REQUIRE((r.r0 - origin).length() < TOLERANCE);
	BOOST_REQUIRE((r.a - direction).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_ray_2d)
{
	rove::vec<2> origin(1, 2);
	rove::vec<2> direction(3, 4);
	rove::ray<2> r(origin, direction);

	BOOST_REQUIRE((r.r0 - origin).length() < TOLERANCE);
	BOOST_REQUIRE((r.a - direction).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(set_ray)
{
	rove::ray<3> r;
	r.set(rove::vec<3>(1, 2, 3), rove::vec<3>(4, 5, 6));

	BOOST_REQUIRE((r.r0 - rove::vec<3>(1, 2, 3)).length() < TOLERANCE);
	BOOST_REQUIRE((r.a - rove::vec<3>(4, 5, 6)).length() < TOLERANCE);
}

// Apply tests
BOOST_AUTO_TEST_CASE(apply_at_t_zero)
{
	rove::ray<3> r(rove::vec<3>(1, 2, 3), rove::vec<3>(4, 5, 6));
	rove::vec<3> p = r.apply(0);

	BOOST_REQUIRE((p - rove::vec<3>(1, 2, 3)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(apply_at_t_one)
{
	rove::ray<3> r(rove::vec<3>(1, 2, 3), rove::vec<3>(4, 5, 6));
	rove::vec<3> p = r.apply(1);

	BOOST_REQUIRE((p - rove::vec<3>(5, 7, 9)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(apply_at_t_negative)
{
	rove::ray<3> r(rove::vec<3>(10, 10, 10), rove::vec<3>(1, 0, 0));
	rove::vec<3> p = r.apply(-5);

	BOOST_REQUIRE((p - rove::vec<3>(5, 10, 10)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(apply_2d)
{
	rove::ray<2> r(rove::vec<2>(0, 0), rove::vec<2>(3, 4));
	rove::vec<2> p = r.apply(2);

	BOOST_REQUIRE((p - rove::vec<2>(6, 8)).length() < TOLERANCE);
}

// Contains tests
BOOST_AUTO_TEST_CASE(contains_point_on_ray)
{
	rove::ray<3> r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(r.contains(rove::vec<3>(0, 0, 0)));
	BOOST_REQUIRE(r.contains(rove::vec<3>(5, 0, 0)));
	BOOST_REQUIRE(r.contains(rove::vec<3>(100, 0, 0)));
}

BOOST_AUTO_TEST_CASE(contains_point_not_on_ray)
{
	rove::ray<3> r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(!r.contains(rove::vec<3>(0, 1, 0)));
	BOOST_REQUIRE(!r.contains(rove::vec<3>(5, 0.1f, 0)));
}

BOOST_AUTO_TEST_CASE(contains_point_on_ray_2d)
{
	rove::ray<2> r(rove::vec<2>(1, 1), rove::vec<2>(1, 1));

	BOOST_REQUIRE(r.contains(rove::vec<2>(1, 1)));
	BOOST_REQUIRE(r.contains(rove::vec<2>(2, 2)));
	BOOST_REQUIRE(r.contains(rove::vec<2>(5, 5)));
}

// Distance to point tests
BOOST_AUTO_TEST_CASE(distance_to_point_on_ray)
{
	rove::ray<3> r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));

	rove::scalar dist = r.distance(rove::vec<3>(5, 0, 0));
	BOOST_REQUIRE(rove::abs(dist) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(distance_to_point_off_ray)
{
	rove::ray<3> r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));

	rove::scalar dist = r.distance(rove::vec<3>(5, 3, 0));
	BOOST_REQUIRE(rove::abs(dist - 3.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(distance_sq_to_point)
{
	rove::ray<3> r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));

	rove::scalar dist_sq = r.distance_sq(rove::vec<3>(5, 4, 0));
	BOOST_REQUIRE(rove::abs(dist_sq - 16.0f) < TOLERANCE);
}

// Ray-ray distance tests
BOOST_AUTO_TEST_CASE(ray_ray_distance_parallel)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(0, 5, 0), rove::vec<3>(1, 0, 0));

	rove::scalar dist = r1.distance(r2);
	BOOST_REQUIRE(rove::abs(dist - 5) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(ray_ray_distance_intersecting)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(5, -5, 0), rove::vec<3>(0, 1, 0));

	rove::scalar dist = r1.distance(r2);
	BOOST_REQUIRE(rove::abs(dist) < 0.01f);
}

BOOST_AUTO_TEST_CASE(ray_ray_distance_skew)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(0, 5, 5), rove::vec<3>(0, 1, 0));

	// x = 0 and z = 5 on the second line: it passes 5 above the first
	rove::scalar dist = r1.distance(r2);
	BOOST_REQUIRE(rove::abs(dist - 5) < TOLERANCE);
}

// Ray-ray intersection tests - edge cases
BOOST_AUTO_TEST_CASE(ray_ray_intersection_same_ray)
{
	rove::ray<2> r1(rove::vec<2>(0, 0), rove::vec<2>(1, 1));
	rove::ray<2> r2(rove::vec<2>(0, 0), rove::vec<2>(1, 1));

	// they meet everywhere and so at no one point: query_intersection has no t
	// to give, but the lines are no distance apart
	rove::scalar s;
	BOOST_REQUIRE(!r1.query_intersection(r2, s));
	BOOST_REQUIRE(r1.distance(r2) == 0);
}

BOOST_AUTO_TEST_CASE(ray_ray_intersection_parallel_no_intersection)
{
	rove::ray<2> r1(rove::vec<2>(0, 0), rove::vec<2>(1, 0));
	rove::ray<2> r2(rove::vec<2>(0, 5), rove::vec<2>(1, 0));

	rove::scalar s;
	BOOST_REQUIRE(!r1.query_intersection(r2, s));
}

BOOST_AUTO_TEST_CASE(ray_ray_intersection_perpendicular)
{
	rove::ray<2> r1(rove::vec<2>(0, 5), rove::vec<2>(1, 0));
	rove::ray<2> r2(rove::vec<2>(5, 0), rove::vec<2>(0, 1));

	rove::scalar s;
	BOOST_REQUIRE(r1.query_intersection(r2, s));

	rove::vec<2> p = r1.apply(s);
	BOOST_REQUIRE((p - rove::vec<2>(5, 5)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(ray_ray_intersection_3d_skew_lines)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(0, 5, 5), rove::vec<3>(0, 1, 0));

	rove::scalar s;
	// Skew lines in 3D don't intersect
	BOOST_REQUIRE(!r1.query_intersection(r2, s));
}

BOOST_AUTO_TEST_CASE(ray_ray_intersection_2d_far_from_the_origin)
{
	// lines in a plane that are not parallel cross wherever rounding puts the point
	rove::ray<2> r1(rove::vec<2>(10000, 10000.5f), rove::vec<2>(1, 0));
	rove::ray<2> r2(rove::vec<2>(10000.3f, 9000), rove::vec<2>(0, 1));

	rove::scalar s;
	BOOST_REQUIRE(r1.query_intersection(r2, s));
	BOOST_TEST(rove::abs(s - 0.3f) < 1e-2f);
}

BOOST_AUTO_TEST_CASE(ray_ray_intersection_3d_takes_its_tolerance)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(0, 0, 0.01f), rove::vec<3>(0, 1, 0));

	rove::scalar s;
	BOOST_REQUIRE(!r1.query_intersection(r2, s));
	BOOST_REQUIRE(r1.query_intersection(r2, s, 0.1f));
	BOOST_TEST(!r1.test_intersection(r2));
	BOOST_TEST(r1.test_intersection(r2, 0.1f));
}

// Test intersection test (boolean)
BOOST_AUTO_TEST_CASE(test_intersection_intersecting)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(5, -5, 0), rove::vec<3>(0, 1, 0));

	BOOST_REQUIRE(r1.test_intersection(r2));
}

BOOST_AUTO_TEST_CASE(test_intersection_not_intersecting)
{
	rove::ray<3> r1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> r2(rove::vec<3>(0, 5, 5), rove::vec<3>(0, 1, 0));

	BOOST_REQUIRE(!r1.test_intersection(r2));
}

// 2D specific tests
BOOST_AUTO_TEST_CASE(ray_2d_basic_operations)
{
	rove::ray<2> r(rove::vec<2>(0, 0), rove::vec<2>(3, 4));

	// Length of direction vector
	rove::scalar len = r.a.length();
	BOOST_REQUIRE(rove::abs(len - 5.0f) < TOLERANCE);

	// Point at t=1
	rove::vec<2> p = r.apply(1);
	BOOST_REQUIRE((p - rove::vec<2>(3, 4)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(ray_2d_contains)
{
	rove::ray<2> r(rove::vec<2>(1, 1), rove::vec<2>(2, 2));

	BOOST_REQUIRE(r.contains(rove::vec<2>(3, 3)));
	BOOST_REQUIRE(!r.contains(rove::vec<2>(3, 4)));
}

// 4D ray test
BOOST_AUTO_TEST_CASE(ray_4d_basic)
{
	rove::ray<4> r(rove::vec<4>(1, 2, 3, 4), rove::vec<4>(1, 1, 1, 1));

	rove::vec<4> p = r.apply(5);
	BOOST_REQUIRE((p - rove::vec<4>(6, 7, 8, 9)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_SUITE_END()

// ray::distance(ray) is the distance between the two lines: zero where they
// cross, the closest approach where they are skew, and where they are
// parallel, the distance from any point of one to the other.
BOOST_AUTO_TEST_SUITE(ray_line_distance)

BOOST_AUTO_TEST_CASE(parallel_lines_are_as_far_apart_as_they_are)
{
	rove::ray<3> const x(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	// the other way along, and a longer direction
	rove::ray<3> const back(rove::vec<3>(7, 3, 4), rove::vec<3>(-3, 0, 0));
	BOOST_TEST(rove::abs(x.distance(back) - 5) < TOLERANCE);
	BOOST_TEST(rove::abs(back.distance(x) - 5) < TOLERANCE);
	// a short direction
	rove::ray<3> const short_one(rove::vec<3>(-2, 0, 2), rove::vec<3>(0.01f, 0, 0));
	BOOST_TEST(rove::abs(x.distance(short_one) - 2) < 1e-5f);
	// parallel to neither axis
	rove::ray<3> const d1(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 0)), d2(rove::vec<3>(0, 2, 0), rove::vec<3>(2, 2, 0));
	BOOST_TEST(rove::abs(d1.distance(d2) - std::sqrt(2.0f)) < 1e-5f);
	// the same line from another origin
	rove::ray<3> const same(rove::vec<3>(-40, 0, 0), rove::vec<3>(2, 0, 0));
	BOOST_TEST(x.distance(same) == 0);
}

BOOST_AUTO_TEST_CASE(parallel_lines_in_2d)
{
	rove::ray<2> const x(rove::vec<2>(0, 0), rove::vec<2>(1, 0));
	rove::ray<2> const above(rove::vec<2>(100, 1), rove::vec<2>(1, 0));
	BOOST_TEST(rove::abs(x.distance(above) - 1) < TOLERANCE);
	BOOST_TEST(rove::abs(above.distance(x) - 1) < TOLERANCE);
	rove::ray<2> const d1(rove::vec<2>(0, 0), rove::vec<2>(2, 2)), d2(rove::vec<2>(0, 2), rove::vec<2>(-1, -1));
	BOOST_TEST(rove::abs(d1.distance(d2) - std::sqrt(2.0f)) < 1e-5f);
	rove::ray<2> const same(rove::vec<2>(5, 5), rove::vec<2>(3, 3));
	BOOST_TEST(d1.distance(same) == 0);
	// lines that are not parallel cross in the plane
	rove::ray<2> const up(rove::vec<2>(50, -3), rove::vec<2>(0, 1));
	BOOST_TEST(x.distance(up) == 0);
	BOOST_TEST(above.distance(up) == 0);
}

BOOST_AUTO_TEST_CASE(the_distance_does_not_depend_on_the_order)
{
	rove::ray<3> const rays[] = {
		rove::ray<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0)),
		rove::ray<3>(rove::vec<3>(0, 5, 5), rove::vec<3>(0, 1, 0)),
		rove::ray<3>(rove::vec<3>(3, 1, -2), rove::vec<3>(-2, 0, 0)),
		rove::ray<3>(rove::vec<3>(1, 2, 3), rove::vec<3>(1, 1, 1)),
	};
	for (auto const &a : rays)
		for (auto const &b : rays)
			BOOST_TEST(rove::abs(a.distance(b) - b.distance(a)) < 1e-5f);
}

// A ray of no direction is the point it starts at.
BOOST_AUTO_TEST_CASE(a_ray_of_no_direction_is_a_point)
{
	rove::ray<3> const x(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	rove::ray<3> const point(rove::vec<3>(9, 3, 4), rove::vec<3>(0, 0, 0));
	BOOST_TEST(rove::abs(x.distance(point) - 5) < TOLERANCE);
	BOOST_TEST(rove::abs(point.distance(x) - 5) < TOLERANCE);
	rove::ray<3> const other(rove::vec<3>(9, 0, 0), rove::vec<3>(0, 0, 0));
	BOOST_TEST(rove::abs(point.distance(other) - 5) < TOLERANCE);

	rove::ray<2> const x2(rove::vec<2>(0, 0), rove::vec<2>(1, 0)), point2(rove::vec<2>(4, -3), rove::vec<2>(0, 0));
	BOOST_TEST(rove::abs(x2.distance(point2) - 3) < TOLERANCE);
	BOOST_TEST(rove::abs(point2.distance(x2) - 3) < TOLERANCE);
	BOOST_TEST(rove::abs(point2.distance(rove::ray<2>(rove::vec<2>(0, 0), rove::vec<2>(0, 0))) - 5) < TOLERANCE);
}

// test_intersection is distance() < epsilon: parallel lines apart do not meet.
BOOST_AUTO_TEST_CASE(parallel_lines_apart_do_not_intersect)
{
	rove::ray<3> const x(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	BOOST_TEST(!x.test_intersection(rove::ray<3>(rove::vec<3>(0, 1, 0), rove::vec<3>(1, 0, 0))));
	BOOST_TEST(x.test_intersection(rove::ray<3>(rove::vec<3>(3, 0, 0), rove::vec<3>(-1, 0, 0))));
	rove::ray<2> const x2(rove::vec<2>(0, 0), rove::vec<2>(1, 0));
	BOOST_TEST(!x2.test_intersection(rove::ray<2>(rove::vec<2>(0, 1), rove::vec<2>(1, 0))));
	BOOST_TEST(x2.test_intersection(rove::ray<2>(rove::vec<2>(0, 1), rove::vec<2>(1, 1))));
}

BOOST_AUTO_TEST_CASE(contains_and_distances_work_on_a_const_ray)
{
	rove::ray<3> const r(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0));
	BOOST_REQUIRE(r.contains(rove::vec<3>(5, 0, 0)));
	BOOST_REQUIRE(rove::abs(r.distance_sq(rove::vec<3>(2, 3, 0)) - 9) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(r.distance(rove::vec<3>(2, 0, 4)) - 4) < TOLERANCE);
}

// ray::unit(origin, direction): the ray from origin along direction(direction), unit length, so that its t is a distance; an origin
// that is not finite, or a direction direction() refuses, is std::invalid_argument
BOOST_AUTO_TEST_CASE(unit_is_a_ray_whose_t_is_a_distance)
{
	rove::vec<3> const origin(1, -2, 3);
	rove::ray<3> const r = rove::ray<3>::unit(origin, rove::vec<3>(0, -3, 4));
	BOOST_TEST((r.r0 == origin));
	BOOST_TEST(rove::abs(r.a.length() - 1) < TOLERANCE);
	BOOST_TEST((r.a - rove::vec<3>(0, -0.6f, 0.8f)).length() < TOLERANCE);
	BOOST_TEST(rove::abs((r.apply(7.5f) - origin).length() - 7.5f) < 1e-5f);

	// the length of the direction given does not matter, short or long
	rove::ray<3> const tiny = rove::ray<3>::unit(origin, rove::vec<3>(0, -3e-5f, 4e-5f));
	BOOST_TEST((tiny.a - r.a).length() < TOLERANCE);
	rove::ray<3> const huge = rove::ray<3>::unit(origin, rove::vec<3>(0, -3e30f, 4e30f));
	BOOST_TEST((huge.a - r.a).length() < TOLERANCE);
	BOOST_TEST((rove::ray<3>::unit(origin, rove::vec<3>(0, 0, 9)).a == rove::vec<3>(0, 0, 1)));

	rove::ray<2, double> const flat = rove::ray<2, double>::unit(rove::vec<2, double>(0, 0), rove::vec<2, double>(-5, 12));
	BOOST_TEST((flat.a - rove::vec<2, double>(-5.0 / 13, 12.0 / 13)).length() < 1e-15);
	static_assert(std::is_same_v<decltype(rove::ray<3>::unit(origin, origin)), rove::ray<3>>);
}

BOOST_AUTO_TEST_CASE(unit_refuses_what_is_no_ray)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	rove::vec<3> const along(1, 0, 0), at(0, 0, 0);
	for (float const bad : {nan, inf, -inf})
		for (int k = 0; k < 3; ++k) {
			rove::vec<3> v = at;
			v.i[k] = bad;
			BOOST_CHECK_THROW((void)rove::ray<3>::unit(v, along), std::invalid_argument);
			BOOST_CHECK_THROW((void)rove::ray<3>::unit(at, v + along), std::invalid_argument);
		}
	BOOST_CHECK_THROW((void)rove::ray<3>::unit(at, at), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::ray<3>::unit(at, rove::vec<3>(0, -0.0f, 0)), std::invalid_argument);
	// too short to tell from rounding noise, as direction() has it
	BOOST_CHECK_THROW((void)rove::ray<3>::unit(at, rove::vec<3>(1e-7f, 0, 0)), std::invalid_argument);
	BOOST_CHECK_THROW((void)(rove::ray<2, double>::unit(rove::vec<2, double>(0, 0), rove::vec<2, double>(0, 1e-13))),
		std::invalid_argument);
}

// r.check_finite(what): its origin and its direction, each as a vector's check_finite.  A direction of no length is
// finite: that is for direction() to refuse.
BOOST_AUTO_TEST_CASE(check_finite_refuses_an_origin_or_a_direction_that_is_not_finite)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	BOOST_CHECK_NO_THROW((void)(rove::ray<3>(rove::vec<3>(1, 2, 3), rove::vec<3>(0, 0, 1)).check_finite("a ray")));
	BOOST_CHECK_NO_THROW((void)(rove::ray<3>(rove::vec<3>(1, 2, 3), rove::vec<3>(0, 0, 0)).check_finite("a ray")));
	BOOST_CHECK_NO_THROW((void)(rove::ray<2, double>(rove::vec<2, double>(1, 2), rove::vec<2, double>(0, 1)).check_finite("a ray")));
	for (float const bad : {nan, inf, -inf})
		for (int k = 0; k < 3; ++k) {
			rove::ray<3> origin(rove::vec<3>(1, 2, 3), rove::vec<3>(0, 0, 1)), direction = origin;
			origin.r0.i[k] = bad;
			direction.a.i[k] = bad;
			BOOST_CHECK_THROW((void)origin.check_finite("a ray"), std::invalid_argument);
			BOOST_CHECK_THROW((void)direction.check_finite("a ray"), std::invalid_argument);
		}
	try {
		(void)(rove::ray<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(0, nan, 1)).check_finite("octree: a ray"));
		BOOST_FAIL("a NaN direction passed");
	} catch (std::invalid_argument const &e) {
		BOOST_TEST(std::string(e.what()) == "octree: a ray that is not finite");
	}
}

BOOST_AUTO_TEST_CASE(a_ray_of_no_direction_is_a_point_to_measure_from)
{
	rove::ray<3> const r(rove::vec<3>(1, 0, 0), rove::vec<3>(0, 0, 0));
	BOOST_TEST(r.distance(rove::vec<3>(1, 3, 4)) == 5.0f);
	BOOST_TEST(r.distance_sq(rove::vec<3>(1, 3, 4)) == 25.0f);
	rove::ray<2> const r2(rove::vec<2>(1, 0), rove::vec<2>(0, 0));
	BOOST_TEST(r2.distance(rove::vec<2>(1, 3)) == 3.0f);
}

BOOST_AUTO_TEST_SUITE_END()

