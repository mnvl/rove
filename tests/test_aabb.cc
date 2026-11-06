// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <random>
#include <vector>
#include <rove/ray.h>
#include <rove/aabb.h>
#include <rove/matrix.h>
#include "test_tolerance.h"
#include "test_garbage.h"

BOOST_AUTO_TEST_SUITE(aabb)

// Construction tests
BOOST_AUTO_TEST_CASE(construct_from_point)
{
	rove::vec<3> point(1, 2, 3);
	rove::aabb<3> b(point);

	BOOST_REQUIRE((b.lo - point).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - point).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_from_corners)
{
	rove::vec<3> lo(1, 2, 3);
	rove::vec<3> hi(4, 5, 6);
	rove::aabb<3> b(lo, hi);

	BOOST_REQUIRE((b.lo - lo).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - hi).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_from_center_radius)
{
	rove::vec<3> centre(5, 5, 5);
	rove::scalar radius = 10.0f;
	rove::aabb<3> b(centre, radius);

	rove::vec<3> expected_lo(0, 0, 0);
	rove::vec<3> expected_hi(10, 10, 10);

	BOOST_REQUIRE((b.lo - expected_lo).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - expected_hi).length() < TOLERANCE);
}

// Null and normalize tests
BOOST_AUTO_TEST_CASE(null_initialization)
{
	rove::aabb<3> b;
	b.null();

	// After null(), lo should be max and hi should be min
	BOOST_REQUIRE(b.lo.x > 0);
	BOOST_REQUIRE(b.hi.x < 0);
}

BOOST_AUTO_TEST_CASE(normalize_swaps_corners)
{
	rove::aabb<3> b(rove::vec<3>(5, 5, 5), rove::vec<3>(0, 0, 0));
	b.normalize();

	BOOST_REQUIRE((b.lo - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - rove::vec<3>(5, 5, 5)).length() < TOLERANCE);
}

// Geometric property tests
BOOST_AUTO_TEST_CASE(diagonal_calculation)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(3, 4, 5));
	rove::vec<3> diag = b.diagonal();

	BOOST_REQUIRE((diag - rove::vec<3>(3, 4, 5)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(centre_calculation)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(4, 6, 8));
	rove::vec<3> c = b.centre();

	BOOST_REQUIRE((c - rove::vec<3>(2, 3, 4)).length() < TOLERANCE);
}

// Contains tests
BOOST_AUTO_TEST_CASE(contains_point_inside)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));

	BOOST_REQUIRE(b.contains(rove::vec<3>(5, 5, 5)));
	BOOST_REQUIRE(b.contains(rove::vec<3>(0, 0, 0))); // boundary
	BOOST_REQUIRE(b.contains(rove::vec<3>(10, 10, 10))); // boundary
}

BOOST_AUTO_TEST_CASE(contains_point_outside)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));

	BOOST_REQUIRE(!b.contains(rove::vec<3>(15, 5, 5)));
	BOOST_REQUIRE(!b.contains(rove::vec<3>(-1, 5, 5)));
	BOOST_REQUIRE(!b.contains(rove::vec<3>(5, 11, 5)));
}

BOOST_AUTO_TEST_CASE(contains_aabb_fully_inside)
{
	rove::aabb<3> b1(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::aabb<3> b2(rove::vec<3>(2, 2, 2), rove::vec<3>(8, 8, 8));

	BOOST_REQUIRE(b1.contains(b2));
}

BOOST_AUTO_TEST_CASE(contains_aabb_partially_outside)
{
	rove::aabb<3> b1(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::aabb<3> b2(rove::vec<3>(5, 5, 5), rove::vec<3>(15, 15, 15));

	BOOST_REQUIRE(!b1.contains(b2));
}

// Extend tests
BOOST_AUTO_TEST_CASE(extend_by_point)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(5, 5, 5));
	b.extend(rove::vec<3>(10, 3, 3));

	BOOST_REQUIRE((b.lo - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - rove::vec<3>(10, 5, 5)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(extend_by_point_negative)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(5, 5, 5));
	b.extend(rove::vec<3>(-2, 3, 3));

	BOOST_REQUIRE((b.lo - rove::vec<3>(-2, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - rove::vec<3>(5, 5, 5)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(extend_by_aabb)
{
	rove::aabb<3> b1(rove::vec<3>(0, 0, 0), rove::vec<3>(5, 5, 5));
	rove::aabb<3> b2(rove::vec<3>(3, 3, 3), rove::vec<3>(8, 8, 8));

	b1.extend(b2);

	BOOST_REQUIRE((b1.lo - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b1.hi - rove::vec<3>(8, 8, 8)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(extend_null_aabb)
{
	rove::aabb<3> b;
	b.null();

	b.extend(rove::vec<3>(1, 2, 3));
	b.extend(rove::vec<3>(4, 5, 6));

	BOOST_REQUIRE((b.lo - rove::vec<3>(1, 2, 3)).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - rove::vec<3>(4, 5, 6)).length() < TOLERANCE);
}

// Intersection tests
BOOST_AUTO_TEST_CASE(test_intersection_overlapping)
{
	rove::aabb<3> b1(rove::vec<3>(0, 0, 0), rove::vec<3>(5, 5, 5));
	rove::aabb<3> b2(rove::vec<3>(3, 3, 3), rove::vec<3>(8, 8, 8));

	BOOST_REQUIRE(b1.test_intersection(b2));
	BOOST_REQUIRE(b2.test_intersection(b1));
}

BOOST_AUTO_TEST_CASE(test_intersection_touching)
{
	rove::aabb<3> b1(rove::vec<3>(0, 0, 0), rove::vec<3>(5, 5, 5));
	rove::aabb<3> b2(rove::vec<3>(5, 0, 0), rove::vec<3>(10, 5, 5));

	BOOST_REQUIRE(b1.test_intersection(b2));
}

BOOST_AUTO_TEST_CASE(test_intersection_separated)
{
	rove::aabb<3> b1(rove::vec<3>(0, 0, 0), rove::vec<3>(5, 5, 5));
	rove::aabb<3> b2(rove::vec<3>(10, 10, 10), rove::vec<3>(15, 15, 15));

	BOOST_REQUIRE(!b1.test_intersection(b2));
}

// Ray tracing tests
BOOST_AUTO_TEST_CASE(trace_through_corners)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), 20.0f);
	rove::ray<3> r(rove::vec<3>(-20, -20, -20), rove::vec<3>(40, 40, 40));

	rove::scalar t0, t1;

	BOOST_REQUIRE(b.trace(r, &t0, &t1));

	BOOST_REQUIRE (rove::abs(t0 - 0.25f) < TOLERANCE);
	BOOST_REQUIRE (rove::abs(t1 - 0.75f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(trace_from_inside)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::ray<3> r(rove::vec<3>(5, 5, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1, -100, 100));
}

BOOST_AUTO_TEST_CASE(trace_axis_aligned)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::ray<3> r(rove::vec<3>(-5, 5, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 100));

	// Ray should enter at x=0 (t0=5) and exit at x=10 (t1=15)
	BOOST_REQUIRE(rove::abs(t0 - 5.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(t1 - 15.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(trace_miss)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::ray<3> r(rove::vec<3>(-5, 20, 5), rove::vec<3>(1, 0, 0));

	BOOST_REQUIRE(!b.trace(r));
}

BOOST_AUTO_TEST_CASE(trace_with_t_range)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::ray<3> r(rove::vec<3>(-5, 5, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	// With t_min=0, t_max=1, ray doesn't reach the box
	BOOST_REQUIRE(!b.trace(r, &t0, &t1, 0, 1));

	// With larger t_max, it should hit
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 20));
}

// World to local transformation tests
BOOST_AUTO_TEST_CASE(world_to_local_vector)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 20, 30));
	rove::vec<3> world_vec(5, 10, 15);
	rove::vec<3> local_vec;

	b.world_to_local_vector(local_vec, world_vec);

	BOOST_REQUIRE(rove::abs(local_vec.x - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_vec.y - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_vec.z - 0.5f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(world_to_local_point)
{
	rove::aabb<3> b(rove::vec<3>(10, 10, 10), rove::vec<3>(20, 30, 40));
	rove::vec<3> world_point(15, 20, 25);
	rove::vec<3> local_point;

	b.world_to_local_point(local_point, world_point);

	BOOST_REQUIRE(rove::abs(local_point.x - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_point.y - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_point.z - 0.5f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(world_to_local_ray)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10));
	rove::ray<3> world_ray(rove::vec<3>(5, 5, 5), rove::vec<3>(10, 0, 0));
	rove::ray<3> local_ray;

	b.world_to_local_ray(local_ray, world_ray);

	// Origin should be at (0.5, 0.5, 0.5) in local space
	BOOST_REQUIRE(rove::abs(local_ray.r0.x - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_ray.r0.y - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_ray.r0.z - 0.5f) < TOLERANCE);

	// Direction should be scaled
	BOOST_REQUIRE(rove::abs(local_ray.a.x - 1.0f) < TOLERANCE);
}

// 2D AABB tests
BOOST_AUTO_TEST_CASE(aabb_2d_basic)
{
	rove::aabb<2> b(rove::vec<2>(0, 0), rove::vec<2>(10, 10));

	BOOST_REQUIRE(b.contains(rove::vec<2>(5, 5)));
	BOOST_REQUIRE(!b.contains(rove::vec<2>(15, 5)));

	rove::vec<2> c = b.centre();
	BOOST_REQUIRE((c - rove::vec<2>(5, 5)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(aabb_2d_intersection)
{
	rove::aabb<2> b1(rove::vec<2>(0, 0), rove::vec<2>(5, 5));
	rove::aabb<2> b2(rove::vec<2>(3, 3), rove::vec<2>(8, 8));

	BOOST_REQUIRE(b1.test_intersection(b2));
}

BOOST_AUTO_TEST_CASE(construct_from_centre_and_side_length)
{
	// The scalar argument is a full side length, not a half-extent:
	// side 10 about (1,2,3) reaches 5 in each direction.  Pinned here
	// because the name reads like a radius and invites the other reading.
	rove::vec<3> const centre(1, 2, 3);
	rove::aabb<3> b(centre, rove::scalar(10));

	BOOST_REQUIRE((b.lo - rove::vec<3>(-4, -3, -2)).length() < TOLERANCE);
	BOOST_REQUIRE((b.hi - rove::vec<3>(6, 7, 8)).length() < TOLERANCE);
	BOOST_REQUIRE((b.centre() - centre).length() < TOLERANCE);
	BOOST_REQUIRE((b.diagonal() - rove::vec<3>(10, 10, 10)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_from_centre_and_side_length_keeps_double_precision)
{
	// A float side_length parameter on aabb<3,double> silently rounds the
	// extent: 0.1 rounds differently in float than in double.
	double const side = 0.1;
	rove::aabb<3,double> b(rove::vec<3,double>(0, 0, 0), side);

	BOOST_REQUIRE_EQUAL(b.hi.x, side / 2);
	BOOST_REQUIRE_EQUAL(b.lo.x, -side / 2);
}

BOOST_AUTO_TEST_CASE(closest_point_of_a_point_inside_is_the_point)
{
	rove::aabb<3> const b(rove::vec<3>(-1, 2, -3), rove::vec<3>(4, 5, 6));
	rove::vec<3> const p(0.5f, 4.25f, -2.75f);
	BOOST_TEST((b.closest_point(p) - p).length() == 0);
	BOOST_TEST((b.closest_point(b.lo) - b.lo).length() == 0);
	BOOST_TEST((b.closest_point(b.hi) - b.hi).length() == 0);
}

BOOST_AUTO_TEST_CASE(closest_point_clamps_each_axis_on_its_own)
{
	rove::aabb<3> const b(rove::vec<3>(-1, 2, -3), rove::vec<3>(4, 5, 6));
	// below on x, inside on y, above on z: a face-edge point, not a corner
	BOOST_TEST((b.closest_point(rove::vec<3>(-7, 3, 9)) - rove::vec<3>(-1, 3, 6)).length() == 0);
	// beyond on every axis: the nearest corner
	BOOST_TEST((b.closest_point(rove::vec<3>(10, -10, -10)) - rove::vec<3>(4, 2, -3)).length() == 0);
	// beyond one face only: straight onto it
	BOOST_TEST((b.closest_point(rove::vec<3>(1, 8, 0)) - rove::vec<3>(1, 5, 0)).length() == 0);
}

BOOST_AUTO_TEST_CASE(closest_point_is_nearer_than_any_other_point_of_the_box)
{
	rove::aabb<3> const b(rove::vec<3>(-1, 2, -3), rove::vec<3>(4, 5, 6));
	rove::vec<3> const p(7, -1, 0.5f);
	rove::scalar const best = (b.closest_point(p) - p).length();
	for (int i = 0; i <= 10; ++i)
		for (int j = 0; j <= 10; ++j)
			for (int k = 0; k <= 10; ++k)
			{
				rove::vec<3> const q(-1 + 0.5f * i, 2 + 0.3f * j, -3 + 0.9f * k);
				BOOST_TEST(best <= (q - p).length() + TOLERANCE);
			}
}

BOOST_AUTO_TEST_CASE(closest_point_of_a_flat_box_lies_in_its_plane)
{
	rove::aabb<3> const b(rove::vec<3>(0, 1, 0), rove::vec<3>(2, 1, 2));
	BOOST_TEST((b.closest_point(rove::vec<3>(1, -5, 3)) - rove::vec<3>(1, 1, 2)).length() == 0);
}

BOOST_AUTO_TEST_CASE(closest_point_in_2d)
{
	rove::aabb<2> const b(rove::vec<2>(0, 0), rove::vec<2>(3, 1));
	BOOST_TEST((b.closest_point(rove::vec<2>(5, 0.5f)) - rove::vec<2>(3, 0.5f)).length() == 0);
}

BOOST_AUTO_TEST_CASE(closest_point_passes_nan_through)
{
	// A NaN position is a bug upstream; it must not come out as a box corner.
	rove::aabb<3> const b(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));
	rove::scalar const nan = std::numeric_limits<rove::scalar>::quiet_NaN();
	rove::vec<3> const c = b.closest_point(rove::vec<3>(nan, 0.5f, 2));
	BOOST_TEST(std::isnan(c.x));
	BOOST_TEST(c.y == 0.5f);
	BOOST_TEST(c.z == 1);
}

BOOST_AUTO_TEST_SUITE_END()

// aabb's << is found by argument-dependent lookup, from a namespace that
// declares a << of its own.
namespace elsewhere_aabb
{

struct thing {};
inline std::ostream &operator <<(std::ostream &os, thing) { return os; }

static std::string printed(rove::aabb<2> box) { std::ostringstream os; os << box; return os.str(); }

}

BOOST_AUTO_TEST_SUITE(aabb_lookup)

BOOST_AUTO_TEST_CASE(aabbs_output_is_found_from_another_namespace)
{
	BOOST_TEST(elsewhere_aabb::printed(rove::aabb<2>(rove::vec<2>(1, 2), rove::vec<2>(3, 4))) == "1 2 x 3 4");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(aabb_output)

// Printing a box does not change it, so a const one prints as well.
BOOST_AUTO_TEST_CASE(a_const_aabb_prints)
{
	rove::aabb<3> const box(rove::vec<3>(-1, 0, 2), rove::vec<3>(1, 5, 3));
	std::ostringstream os;
	os << box;
	BOOST_TEST(os.str() == "-1 0 2 x 1 5 3");
	rove::aabb<2, double> const flat(rove::vec<2, double>(0.5, 1), rove::vec<2, double>(2, 4));
	std::ostringstream os2;
	os2 << flat;
	BOOST_TEST(os2.str() == "0.5 1 x 2 4");
}

BOOST_AUTO_TEST_CASE(default_is_empty_not_whatever_the_memory_held)
{
	rove::aabb<3> const b = constructed_over_garbage<rove::aabb<3>>();
	BOOST_REQUIRE(b.is_empty());
	BOOST_REQUIRE(!b.contains(rove::vec<3>(0, 0, 0)));
	BOOST_REQUIRE(!b.contains(b.lo));
	// the first point extended by is the whole box: no garbage corner, no origin dragged in
	rove::aabb<3> grown = b;
	grown.extend(rove::vec<3>(5, 6, 7));
	BOOST_REQUIRE(grown.lo == rove::vec<3>(5, 6, 7));
	BOOST_REQUIRE(grown.hi == rove::vec<3>(5, 6, 7));
	BOOST_REQUIRE(!grown.is_empty());
}

BOOST_AUTO_TEST_CASE(is_empty_is_lo_past_hi_on_some_axis)
{
	BOOST_REQUIRE(!rove::aabb<3>(rove::vec<3>(0, 0, 0)).is_empty());            // a point is a box of no size, not empty
	BOOST_REQUIRE(!rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 1)).is_empty());
	BOOST_REQUIRE(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, -1, 1)).is_empty());
	rove::aabb<2> nulled(rove::vec<2>(0, 0), rove::vec<2>(1, 1));
	nulled.null();
	BOOST_REQUIRE(nulled.is_empty());
	BOOST_REQUIRE((constructed_over_garbage<rove::aabb<2, double>>().is_empty()));
}

BOOST_AUTO_TEST_CASE(a_transformed_box_is_the_box_of_its_transformed_corners)
{
	std::mt19937 rng(41);
	std::uniform_real_distribution<float> u(-3, 3);
	for (int k = 0; k < 300; ++k) {
		rove::aabb<3> b(rove::vec<3>(u(rng), u(rng), u(rng)));
		b.extend(rove::vec<3>(u(rng), u(rng), u(rng)));
		rove::matrix<4, 4> r, s, t;
		r.rotation(rove::vec<3>(u(rng), u(rng), u(rng)), u(rng));
		s.scaling(u(rng), u(rng), u(rng));         // negative scales too
		t.translation(u(rng), u(rng), u(rng));
		rove::matrix<4, 4> const m = t * r * s;
		rove::aabb<3> corners;
		for (int c = 0; c < 8; ++c)
			corners.extend(m * rove::vec<3>(c & 1 ? b.hi.x : b.lo.x, c & 2 ? b.hi.y : b.lo.y, c & 4 ? b.hi.z : b.lo.z));
		rove::aabb<3> const out = rove::transform_aabb(b, m);
		BOOST_REQUIRE((out.lo - corners.lo).length() < 1e-4f);
		BOOST_REQUIRE((out.hi - corners.hi).length() < 1e-4f);
	}
	// nothing transformed is nothing
	BOOST_REQUIRE(rove::transform_aabb(rove::aabb<3>(), rove::matrix<4, 4>()).is_empty());
}

BOOST_AUTO_TEST_CASE(bounds_of_points_and_a_grown_box)
{
	std::vector<rove::vec<3>> const points{rove::vec<3>(1, 5, -2), rove::vec<3>(-3, 0, 4), rove::vec<3>(2, 2, 2)};
	rove::aabb<3> const b = rove::bounds_of(points.begin(), points.end());
	BOOST_REQUIRE(b.lo == rove::vec<3>(-3, 0, -2));
	BOOST_REQUIRE(b.hi == rove::vec<3>(2, 5, 4));
	BOOST_REQUIRE(rove::bounds_of(points.begin(), points.begin()).is_empty());
	rove::aabb<3> const g = b.grown(0.5f);
	BOOST_REQUIRE(g.lo == rove::vec<3>(-3.5f, -0.5f, -2.5f));
	BOOST_REQUIRE(g.hi == rove::vec<3>(2.5f, 5.5f, 4.5f));
	// shrunk past nothing it is empty, not inside out and "full"
	BOOST_REQUIRE(b.grown(-10).is_empty());
	BOOST_REQUIRE(rove::aabb<3>().grown(1).is_empty());
}

// box.check_finite(what): each corner as a vector's check_finite.  Finite is not non-empty: the empty box's corners are
// float's largest, and pass.
BOOST_AUTO_TEST_CASE(check_finite_refuses_a_corner_that_is_not_finite)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	BOOST_CHECK_NO_THROW((void)rove::aabb<3>(rove::vec<3>(-1, -2, -3), rove::vec<3>(1, 2, 3)).check_finite("a box"));
	BOOST_CHECK_NO_THROW((void)(rove::aabb<2, double>(rove::vec<2, double>(4, 5)).check_finite("a box")));
	BOOST_CHECK_NO_THROW((void)rove::aabb<3>().check_finite("a box"));
	for (float const bad : {nan, inf, -inf})
		for (int k = 0; k < 3; ++k) {
			rove::aabb<3> low(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)), high = low;
			low.lo.i[k] = bad;
			high.hi.i[k] = bad;
			BOOST_CHECK_THROW((void)low.check_finite("a box"), std::invalid_argument);
			BOOST_CHECK_THROW((void)high.check_finite("a box"), std::invalid_argument);
		}
	try {
		(void)rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(inf, 1, 1)).check_finite("hash_grid: an item's bounds");
		BOOST_FAIL("an infinite corner passed");
	} catch (std::invalid_argument const &e) {
		BOOST_TEST(std::string(e.what()) == "hash_grid: an item's bounds that is not finite");
	}
}

// A ray that starts inside the box has entered it: the time of entry is t_min, whichever way the ray points
BOOST_AUTO_TEST_CASE(trace_from_the_centre_hits_in_every_direction)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));
	rove::vec<3> const centre(0.5f, 0.5f, 0.5f);
	float const directions[][3] = { {1, -0.2f, 0}, {1, 0.2f, 0}, {-1, 0.2f, 0.1f}, {0.1f, -1, -0.3f}, {0, 0, -1}, {0.3f, 0.3f, 1} };
	for (auto const &d : directions) {
		rove::vec<3> const dir(d[0], d[1], d[2]);
		float t0 = -1, t1 = -1;
		BOOST_REQUIRE(b.trace(rove::ray<3>(centre, dir), &t0, &t1, 0, 100));
		BOOST_TEST(t0 == 0.0f);
		// the far face is where one coordinate of the point first reaches it
		rove::vec<3> const p = centre + dir * t1;
		BOOST_TEST(std::max({std::abs(p.x - 0.5f), std::abs(p.y - 0.5f), std::abs(p.z - 0.5f)}) == 0.5f, boost::test_tools::tolerance(1e-5f));
	}
}

BOOST_AUTO_TEST_CASE(trace_of_a_segment_wholly_inside_hits_for_its_whole_length)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));
	rove::ray<3> const r(rove::vec<3>(0.4f, 0.4f, 0.4f), rove::vec<3>(0.1f, 0.1f, 0));
	float t0 = -1, t1 = -1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1));
	BOOST_TEST(t0 == 0.0f);
	BOOST_TEST(t1 == 1.0f);
	BOOST_TEST(b.trace(r));
}

BOOST_AUTO_TEST_CASE(trace_counts_a_touch_at_exactly_t_max_and_at_exactly_t_min)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));
	rove::ray<3> const r(rove::vec<3>(-2, 0.5f, 0.5f), rove::vec<3>(1, 0, 0));
	float t0, t1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 2));
	BOOST_TEST(t0 == 2.0f);
	BOOST_TEST(t1 == 2.0f);
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 3, 10));
	BOOST_TEST(t0 == 3.0f);
	BOOST_TEST(t1 == 3.0f);
	BOOST_TEST(!b.trace(r, &t0, &t1, 0, 1.999f));
	BOOST_TEST(!b.trace(r, &t0, &t1, 3.001f, 10));
}

// The interval is the part of [t_min, t_max] in the box, for a box that is not the unit box
BOOST_AUTO_TEST_CASE(trace_interval_of_a_scaled_box_along_all_the_axes)
{
	rove::aabb<3> b(rove::vec<3>(10, -20, 30), rove::vec<3>(12, -10, 70));
	rove::ray<3> const r(rove::vec<3>(11, -30, 50), rove::vec<3>(0, 1, 0));
	float t0, t1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 100));
	BOOST_TEST(t0 == 10.0f);
	BOOST_TEST(t1 == 20.0f);
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 12, 15));
	BOOST_TEST(t0 == 12.0f);
	BOOST_TEST(t1 == 15.0f);
}

BOOST_AUTO_TEST_CASE(trace_of_a_ray_with_no_direction_hits_only_inside)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));
	float t0, t1;
	BOOST_TEST(b.trace(rove::ray<3>(rove::vec<3>(0.5f, 0.5f, 0.5f), rove::vec<3>(0, 0, 0)), &t0, &t1));
	BOOST_TEST(t0 == 0.0f);
	BOOST_TEST(t1 == 1.0f);
	BOOST_TEST(!b.trace(rove::ray<3>(rove::vec<3>(2, 0.5f, 0.5f), rove::vec<3>(0, 0, 0)), &t0, &t1));
}

BOOST_AUTO_TEST_CASE(trace_through_a_box_flat_on_one_axis_hits_it)
{
	rove::aabb<3> b(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 1));
	float t0, t1;
	BOOST_REQUIRE(b.trace(rove::ray<3>(rove::vec<3>(0.5f, 1, 0.5f), rove::vec<3>(0, -1, 0)), &t0, &t1, 0, 10));
	BOOST_TEST(t0 == 1.0f);
	BOOST_TEST(t1 == 1.0f);
	BOOST_TEST(!b.trace(rove::ray<3>(rove::vec<3>(2, 1, 0.5f), rove::vec<3>(0, -1, 0)), 0, 10));
}

BOOST_AUTO_TEST_CASE(trace_misses_an_empty_box)
{
	rove::aabb<3> b;
	BOOST_TEST(!b.trace(rove::ray<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)), -1e6f, 1e6f));
}

BOOST_AUTO_TEST_CASE(trace_of_a_rectangle)
{
	rove::aabb<2> b(rove::vec<2>(1, 1), rove::vec<2>(3, 2));
	float t0, t1;
	BOOST_REQUIRE(b.trace(rove::ray<2>(rove::vec<2>(0, 1.5f), rove::vec<2>(1, 0)), &t0, &t1, 0, 10));
	BOOST_TEST(t0 == 1.0f);
	BOOST_TEST(t1 == 3.0f);
	// from inside
	BOOST_REQUIRE(b.trace(rove::ray<2>(rove::vec<2>(2, 1.5f), rove::vec<2>(1, 0)), &t0, &t1, 0, 10));
	BOOST_TEST(t0 == 0.0f);
	BOOST_TEST(t1 == 1.0f);
	BOOST_TEST(!b.trace(rove::ray<2>(rove::vec<2>(0, 3), rove::vec<2>(1, 0)), 0, 10));
	rove::aabb<2, double> d(rove::vec<2, double>(1, 1), rove::vec<2, double>(3, 2));
	BOOST_TEST(d.trace(rove::ray<2, double>(rove::vec<2, double>(0, 1.5), rove::vec<2, double>(1, 0)), 0, 10));
}

BOOST_AUTO_TEST_SUITE_END()

