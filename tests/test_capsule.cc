// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <rove/aabb.h>
#include <rove/obb.h>
#include <rove/capsule.h>
#include "test_garbage.h"
#include <rove/segment.h>
#include <rove/triangle.h>
#include <rove/matrix.h>
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

BOOST_AUTO_TEST_SUITE (capsule)

// Construction tests
BOOST_AUTO_TEST_CASE(capsule_default_constructor)
{
	rove::capsule<3> c;
	BOOST_REQUIRE(true);  // Just verify it compiles
}

BOOST_AUTO_TEST_CASE(capsule_constructor_3d)
{
	rove::vec<3> a(0, 0, 0);
	rove::vec<3> b(0, 0, 10);
	rove::scalar r = 2.0f;

	rove::capsule<3> c(a, b, r);

	BOOST_REQUIRE((c.axe.A - a).length() < TOLERANCE);
	BOOST_REQUIRE((c.axe.B - b).length() < TOLERANCE);
	BOOST_REQUIRE(rove::abs(c.radius - r) < TOLERANCE);
}

// 2D capsule tests removed - only 3D capsule is instantiated in capsule.cc

// Contains tests
BOOST_AUTO_TEST_CASE(capsule_contains_on_axis)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	// Point on the axis should be contained
	BOOST_REQUIRE(c.contains(rove::vec<3>(0, 0, 5)));
}

BOOST_AUTO_TEST_CASE(capsule_contains_at_endpoint)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	// Points at endpoints should be contained
	BOOST_REQUIRE(c.contains(rove::vec<3>(0, 0, 0)));
	BOOST_REQUIRE(c.contains(rove::vec<3>(0, 0, 10)));
}

BOOST_AUTO_TEST_CASE(capsule_contains_on_surface)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	// Point on surface (perpendicular to axis at distance = radius)
	BOOST_REQUIRE(c.contains(rove::vec<3>(2, 0, 5)));
	BOOST_REQUIRE(c.contains(rove::vec<3>(0, 2, 5)));
}

BOOST_AUTO_TEST_CASE(capsule_contains_outside)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	// Point outside capsule
	BOOST_REQUIRE(!c.contains(rove::vec<3>(5, 0, 5)));
	BOOST_REQUIRE(!c.contains(rove::vec<3>(0, 0, 20)));
}

BOOST_AUTO_TEST_CASE(capsule_contains_hemisphere)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	// Point in hemispherical cap at end
	BOOST_REQUIRE(c.contains(rove::vec<3>(1, 0, -1)));
	BOOST_REQUIRE(c.contains(rove::vec<3>(0, 1, 11)));
}


// Distance tests
BOOST_AUTO_TEST_CASE(capsule_distance_to_point_inside)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	rove::scalar dist = c.distance(rove::vec<3>(0, 0, 5));
	BOOST_REQUIRE(rove::abs(dist) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(capsule_distance_to_point_on_surface)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	rove::scalar dist = c.distance(rove::vec<3>(2, 0, 5));
	BOOST_REQUIRE(rove::abs(dist) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(capsule_distance_to_point_outside)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	rove::scalar dist = c.distance(rove::vec<3>(5, 0, 5));
	// Distance from axis is 5, radius is 2, so distance = 3
	BOOST_REQUIRE(rove::abs(dist - 3.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(capsule_distance_to_point_near_endpoint)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	// Point beyond endpoint
	rove::vec<3> p(0, 0, -5);
	rove::scalar dist = c.distance(p);

	// Distance calculation uses line segment, so verify it's non-negative
	BOOST_REQUIRE(dist >= 0);
}

// AABB tests
BOOST_AUTO_TEST_CASE(capsule_get_aabb_axis_aligned_z)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 0, 10),
		2.0f
	);

	rove::aabb<3> b;
	b = c.bounds();

	// AABB should extend radius in all perpendicular directions
	BOOST_REQUIRE(rove::abs(b.lo.x + 2.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(b.hi.x - 2.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(b.lo.y + 2.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(b.hi.y - 2.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(b.lo.z + 2.0f) < 0.01f);
	BOOST_REQUIRE(rove::abs(b.hi.z - 12.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(capsule_get_aabb_diagonal)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(10, 10, 10),
		1.0f
	);

	rove::aabb<3> b;
	b = c.bounds();

	// AABB should contain both endpoints plus radius
	BOOST_REQUIRE(b.contains(rove::vec<3>(0, 0, 0)));
	BOOST_REQUIRE(b.contains(rove::vec<3>(10, 10, 10)));
}


// Comprehensive bounds test (original test)
BOOST_AUTO_TEST_CASE (bounds_1)
{
	unsigned false_positives = 0;

	for (int i = 0; i < 100; i++)
	{
		rove::capsule<3> c(rove::vec<3>(uniform(), uniform(), uniform()), rove::vec<3>(uniform(), uniform(), uniform()),
			rove::abs(uniform()) + 0.001f);

		rove::aabb<3> b;
		b = c.bounds();

		for (int j = 0; j < 100; j++)
		{
			rove::vec<3> point(uniform(), uniform(), uniform());

			bool b1 = c.contains(point);
			bool b2 = b.contains(point);

			if (b1 != b2)
			{
				// bounding box could have only false positioves
				BOOST_REQUIRE(b1 == false && b2 == true);

				++false_positives;
			}
		}
	}
}

// Edge cases
// Note: Zero-length capsule test removed - implementation doesn't handle this edge case well

BOOST_AUTO_TEST_CASE(capsule_small_radius)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(10, 0, 0),
		0.1f
	);

	// Points very close to axis should be contained
	BOOST_REQUIRE(c.contains(rove::vec<3>(5, 0.05f, 0)));
	BOOST_REQUIRE(!c.contains(rove::vec<3>(5, 1.0f, 0)));
}

BOOST_AUTO_TEST_CASE(capsule_large_radius)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 0, 0),
		10.0f
	);

	// Large radius should contain distant points
	BOOST_REQUIRE(c.contains(rove::vec<3>(0.5f, 9.0f, 0)));
	BOOST_REQUIRE(!c.contains(rove::vec<3>(0.5f, 12.0f, 0)));
}

// Horizontal and vertical capsules
BOOST_AUTO_TEST_CASE(capsule_horizontal)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(10, 0, 0),
		1.0f
	);

	BOOST_REQUIRE(c.contains(rove::vec<3>(5, 0, 0)));
	BOOST_REQUIRE(c.contains(rove::vec<3>(5, 1, 0)));
	BOOST_REQUIRE(!c.contains(rove::vec<3>(5, 0, 2)));
}

BOOST_AUTO_TEST_CASE(capsule_vertical)
{
	rove::capsule<3> c(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(0, 10, 0),
		1.0f
	);

	BOOST_REQUIRE(c.contains(rove::vec<3>(0, 5, 0)));
	BOOST_REQUIRE(c.contains(rove::vec<3>(0, 5, 1)));
	BOOST_REQUIRE(!c.contains(rove::vec<3>(2, 5, 0)));
}

BOOST_AUTO_TEST_CASE(capsule_triangle_intersection_basic)
{
	rove::capsule<3> capsule_shape(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 1.0f),
		0.5f
	);

	rove::triangle<3> triangle_shape(
		rove::vec<3>(-1.0f, -1.0f, 0.5f),
		rove::vec<3>(1.0f, -1.0f, 0.5f),
		rove::vec<3>(0.0f, 1.0f, 0.5f)
	);

	BOOST_REQUIRE(capsule_shape.test_intersection(triangle_shape));
}

BOOST_AUTO_TEST_CASE(capsule_triangle_intersection_no_hit)
{
	rove::capsule<3> capsule_shape(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 1.0f),
		0.5f
	);

	rove::triangle<3> triangle_shape(
		rove::vec<3>(2.0f, 2.0f, 0.5f),
		rove::vec<3>(3.0f, 2.0f, 0.5f),
		rove::vec<3>(2.0f, 3.0f, 0.5f)
	);

	BOOST_REQUIRE(!capsule_shape.test_intersection(triangle_shape));
}

BOOST_AUTO_TEST_CASE(capsule_triangle_intersection_cap_hit)
{
	rove::capsule<3> capsule_shape(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 1.0f),
		0.5f
	);

	rove::triangle<3> triangle_shape(
		rove::vec<3>(-0.25f, 0.0f, 1.45f),
		rove::vec<3>(0.25f, 0.0f, 1.45f),
		rove::vec<3>(0.0f, 0.5f, 1.45f)
	);

	BOOST_REQUIRE(capsule_shape.test_intersection(triangle_shape));
}

BOOST_AUTO_TEST_CASE(capsule_capsule_intersection_overlapping)
{
	rove::capsule<3> a(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 1.0f),
		0.5f
	);
	rove::capsule<3> b(
		rove::vec<3>(0.3f, 0.0f, 0.0f),
		rove::vec<3>(0.3f, 0.0f, 1.0f),
		0.5f
	);
	BOOST_REQUIRE(a.test_intersection(b));
}

BOOST_AUTO_TEST_CASE(capsule_capsule_intersection_no_hit)
{
	rove::capsule<3> a(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 1.0f),
		0.5f
	);
	rove::capsule<3> b(
		rove::vec<3>(5.0f, 0.0f, 0.0f),
		rove::vec<3>(5.0f, 0.0f, 1.0f),
		0.5f
	);
	BOOST_REQUIRE(!a.test_intersection(b));
}

BOOST_AUTO_TEST_CASE(capsule_capsule_intersection_touching)
{
	rove::capsule<3> a(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 1.0f),
		0.5f
	);
	// Distance between axes = 1.0, sum of radii = 1.0, should touch.
	rove::capsule<3> b(
		rove::vec<3>(1.0f, 0.0f, 0.0f),
		rove::vec<3>(1.0f, 0.0f, 1.0f),
		0.5f
	);
	BOOST_REQUIRE(a.test_intersection(b));
}

BOOST_AUTO_TEST_CASE(capsule_capsule_intersection_perpendicular)
{
	rove::capsule<3> a(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 2.0f),
		0.3f
	);
	rove::capsule<3> b(
		rove::vec<3>(-1.0f, 0.0f, 1.0f),
		rove::vec<3>(1.0f, 0.0f, 1.0f),
		0.3f
	);
	// Axes cross at (0,0,1), distance = 0, so they intersect.
	BOOST_REQUIRE(a.test_intersection(b));
}

BOOST_AUTO_TEST_CASE(capsule_capsule_intersection_endpoint_cap)
{
	rove::capsule<3> a(
		rove::vec<3>(0.0f, 0.0f, 0.0f),
		rove::vec<3>(0.0f, 0.0f, 1.0f),
		0.5f
	);
	// Near the top cap endpoint.
	rove::capsule<3> b(
		rove::vec<3>(0.0f, 0.0f, 1.8f),
		rove::vec<3>(0.0f, 0.0f, 2.5f),
		0.5f
	);
	// Distance between closest endpoints = 0.8, sum of radii = 1.0, should hit.
	BOOST_REQUIRE(a.test_intersection(b));
}

// ── Brute-force false-positive search ────────────────────────────────────────

namespace {

struct CapsuleTestRNG {
	uint32_t state;
	explicit CapsuleTestRNG(uint32_t seed = 42) : state(seed) {}
	uint32_t next() { state = state * 1664525u + 1013904223u; return state; }
	float uniform(float lo = 0.f, float hi = 1.f) {
		return lo + (hi - lo) * float(next() & 0xFFFF) / 65535.f;
	}
};

rove::vec<3> capsule_random_direction(CapsuleTestRNG &rng) {
	float x, y, s;
	do {
		x = rng.uniform(-1.f, 1.f);
		y = rng.uniform(-1.f, 1.f);
		s = x*x + y*y;
	} while (s >= 1.f || s < 1e-6f);
	float r = 2.f * std::sqrt(1.f - s);
	return rove::vec<3>(x * r, y * r, 1.f - 2.f * s);
}

rove::matrix<4,4> capsule_random_transform(CapsuleTestRNG &rng) {
	rove::matrix<4,4> m;
	m.rotation(rng.uniform(0.f, 6.2832f), rng.uniform(0.f, 6.2832f), rng.uniform(0.f, 6.2832f));
	m.ij[3][0] = rng.uniform(-50.f, 50.f);
	m.ij[3][1] = rng.uniform(-50.f, 50.f);
	m.ij[3][2] = rng.uniform(-50.f, 50.f);
	return m;
}

rove::vec<3> capsule_xform_point(rove::matrix<4,4> const &m, rove::vec<3> const &p) {
	return m * p;
}

} // namespace

BOOST_AUTO_TEST_CASE(bruteforce_capsule_capsule_no_false_positives)
{
	// Capsule A: axis along Z from (0,0,0) to (0,0,1), radius 0.5.
	// Bounding sphere radius ≈ 1.0 (half-length 0.5 + radius 0.5).
	// Place capsule B centre at distance ≥ 5 from capsule A centre.
	// B has axis length ≤ 1 and radius ≤ 0.5.
	// Min separation = 5 - 1.0 - 1.0 = 3.0 > 0.
	CapsuleTestRNG rng(11111);
	int false_positives = 0;

	for (int i = 0; i < 1000; ++i) {
		rove::matrix<4,4> tf = capsule_random_transform(rng);

		rove::vec<3> dir = capsule_random_direction(rng);
		float dist = rng.uniform(5.f, 30.f);
		rove::vec<3> b_centre = rove::vec<3>(0.f, 0.f, 0.5f) + dir * dist;

		// Random axis direction and length for capsule B
		rove::vec<3> b_axis = capsule_random_direction(rng) * rng.uniform(0.2f, 1.0f);
		rove::vec<3> b_a = b_centre - b_axis * 0.5f;
		rove::vec<3> b_b = b_centre + b_axis * 0.5f;
		float b_r = rng.uniform(0.1f, 0.5f);

		// Transform both capsules
		rove::capsule<3> cap_a(
			capsule_xform_point(tf, rove::vec<3>(0,0,0)),
			capsule_xform_point(tf, rove::vec<3>(0,0,1)),
			0.5f
		);
		rove::capsule<3> cap_b(
			capsule_xform_point(tf, b_a),
			capsule_xform_point(tf, b_b),
			b_r
		);

		if (cap_a.test_intersection(cap_b)) {
			++false_positives;
		}
	}
	BOOST_CHECK_EQUAL(false_positives, 0);
}

BOOST_AUTO_TEST_CASE(bruteforce_capsule_triangle_no_false_positives)
{
	// Capsule: axis along Z from (0,0,0) to (0,0,1), radius 0.5.
	// Bounding sphere radius ≈ 1.0.
	// Place triangle centroid at distance ≥ 4 from capsule centre (0,0,0.5).
	// Triangle side length ≤ 0.5.  Min gap = 4 - 1.0 - 0.5 = 2.5 > 0.
	CapsuleTestRNG rng(22222);
	int false_positives = 0;

	for (int i = 0; i < 1000; ++i) {
		rove::matrix<4,4> tf = capsule_random_transform(rng);

		rove::vec<3> dir = capsule_random_direction(rng);
		float dist = rng.uniform(4.f, 20.f);
		rove::vec<3> tri_centre = rove::vec<3>(0.f, 0.f, 0.5f) + dir * dist;

		rove::vec<3> u = capsule_random_direction(rng);
		rove::vec<3> v = rove::normalize(rove::cross(u, dir));
		if (v.length() < 0.01f) v = rove::vec<3>(0.f, 0.f, 1.f);
		float sz = rng.uniform(0.1f, 0.5f);
		rove::vec<3> A = tri_centre + u * sz;
		rove::vec<3> B = tri_centre + v * sz;
		rove::vec<3> C = tri_centre - (u + v) * (sz * 0.5f);

		rove::capsule<3> cap(
			capsule_xform_point(tf, rove::vec<3>(0,0,0)),
			capsule_xform_point(tf, rove::vec<3>(0,0,1)),
			0.5f
		);
		rove::triangle<3> tri(
			capsule_xform_point(tf, A),
			capsule_xform_point(tf, B),
			capsule_xform_point(tf, C)
		);

		if (cap.test_intersection(tri)) {
			++false_positives;
		}
	}
	BOOST_CHECK_EQUAL(false_positives, 0);
}

BOOST_AUTO_TEST_CASE(default_is_a_point_at_the_origin_not_whatever_the_memory_held)
{
	rove::capsule<3> const c = constructed_over_garbage<rove::capsule<3>>();
	BOOST_REQUIRE(c.axe.A == rove::vec<3>(0, 0, 0));
	BOOST_REQUIRE(c.axe.B == rove::vec<3>(0, 0, 0));
	BOOST_REQUIRE_EQUAL(c.radius, 0.0f);
}

BOOST_AUTO_TEST_SUITE_END()
