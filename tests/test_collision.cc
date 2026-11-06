// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <limits>
#include <random>
#include <stdexcept>
#include <boost/test/unit_test.hpp>
#include <rove/collision.h>
#include "test_tolerance.h"

BOOST_AUTO_TEST_SUITE (test_collision)

BOOST_AUTO_TEST_CASE (test_sphere_line_collision_1)
{
	rove::sphere<2> sph;
	sph.centre.set(10, 10);
	sph.radius = 1;

	rove::segment<2> l;
	l.A.set(-100, 0);
	l.B.set(100, 0);

	auto const ci = rove::first_contact(l, sph, rove::vec<2>(-2, -1));

	BOOST_REQUIRE (ci);
	BOOST_REQUIRE (ci->happened);
	BOOST_REQUIRE (rove::abs(ci->time - 9.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (test_sphere_line_collision_2)
{
	rove::sphere<2> sph;
	sph.centre.set(10, 0.5f);
	sph.radius = 1;

	rove::segment<2> l;
	l.A.set(0, 0);
	l.B.set(0, 1);

	auto const ci = rove::first_contact(l, sph, rove::vec<2>(-1, 0));

	BOOST_REQUIRE(ci);
	BOOST_REQUIRE(rove::abs(ci->time - 9.0f) < TOLERANCE);
	BOOST_REQUIRE((ci->position - rove::vec<2>(0, 0.5f)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((ci->normal - rove::vec<2>(1, 0)).length_sq() < TOLERANCE);
	BOOST_REQUIRE(!ci->index);          // one shape against another: no item of a collection
}

BOOST_AUTO_TEST_CASE (test_sphere_line_collision_3)
{
	rove::segment<2> l;
	l.A.set(1, -10);
	l.B.set(1, 10);

	rove::sphere<2> s;
	s.centre.set(10, 10);
	s.radius = 1;

	auto const ci = rove::first_contact(l, s, rove::vec<2>(-1, -1));

	BOOST_REQUIRE (ci);
	BOOST_REQUIRE ((ci->position - rove::vec<2>(1, 2)).length() < TOLERANCE);
	BOOST_REQUIRE ((ci->time - 8) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (test_sphere_line_parallel_motion_never_touches)
{
	// Moving along the line, clear of it: no normal speed, no contact --
	// whichever way sign() rounds a normal speed of zero.
	rove::segment<2> l;
	l.A.set(-100, 0);
	l.B.set(100, 0);

	rove::sphere<2> s;
	s.centre.set(0, 5);
	s.radius = 1;

	for (rove::scalar vy : {0.0f, 5e-7f, -5e-7f})
		BOOST_REQUIRE(!rove::first_contact(l, s, rove::vec<2>(-1, vy)));
}

BOOST_AUTO_TEST_CASE (test_sphere_obb_collision_1)
{
	rove::obb<2> obb;
	obb.origin.set(0, 0);
	obb.tangent.set(1, 0);
	obb.normal.set(0, 1);

	rove::sphere<2> sph;
	sph.centre.set(0.5, 10);
	sph.radius = 1;

	auto const ci = rove::first_contact(obb, sph, rove::vec<2>(0, -1));

	BOOST_REQUIRE (ci);
	BOOST_REQUIRE ((ci->position - rove::vec<2>(0.5, 1)).length() < TOLERANCE);
	BOOST_REQUIRE ((ci->normal - rove::vec<2>(0, 1)).length() < TOLERANCE);
	BOOST_REQUIRE (rove::abs(ci->time - 8) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (test_sphere_obb_collision_2)
{
	rove::obb<2> obb;
	obb.origin.set(0, 0);
	obb.tangent.set(1, 0);
	obb.normal.set(0, 1);

	rove::sphere<2> sph;
	sph.centre.set(10, 0.5f);
	sph.radius = 1;

	auto const ci = rove::first_contact(obb, sph, rove::vec<2>(-1.0f, -0.01f));

	BOOST_REQUIRE (ci);
	BOOST_REQUIRE ((ci->position - rove::vec<2>(1, 0.42f)).length() < TOLERANCE);
	BOOST_REQUIRE ((ci->normal - rove::vec<2>(1, 0)).length() < TOLERANCE);
	BOOST_REQUIRE ((ci->time - 8) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (an_overlap_within_the_skin_is_a_touch_not_a_penetration)
{
	// overlapping by 0.0005: under the default skin of 0.001, over a skin of 0.0001
	rove::sphere<2> a(rove::vec<2>(0, 0), 1), b(rove::vec<2>(1.9995f, 0), 1);
	rove::vec<2> const still(0, 0);

	auto touch = rove::first_contact(a, b, still);
	BOOST_REQUIRE(touch);
	BOOST_TEST(!touch->penetrated);
	BOOST_TEST(touch->time == 0);

	touch = rove::first_contact(a, b, still, 0.0001f);
	BOOST_REQUIRE(touch);
	BOOST_TEST(touch->penetrated);

	// the line test takes it too: a sphere sunk 0.0005 into a line
	rove::segment<2> l(rove::vec<2>(-10, 0), rove::vec<2>(10, 0));
	rove::sphere<2> s(rove::vec<2>(0, 0.9995f), 1);
	auto const sunk = rove::first_contact(l, s, rove::vec<2>(0, -1));
	BOOST_TEST(!(sunk && sunk->penetrated));
	auto const deeper = rove::first_contact(l, s, rove::vec<2>(0, -1), 0.0001f);
	BOOST_REQUIRE(deeper);
	BOOST_TEST(deeper->penetrated);
}

BOOST_AUTO_TEST_CASE (spheres_that_touch_at_the_start_touch_at_time_0_whichever_way_they_go)
{
	// within the skin and moving apart: the touch is now, not when they part
	rove::sphere<3> a(rove::vec<3>(0, 0, 0), 1), b(rove::vec<3>(1.9995f, 0, 0), 1);
	auto const away = rove::first_contact(a, b, rove::vec<3>(1, 0, 0));
	BOOST_REQUIRE(away);
	BOOST_TEST(away->time == 0);
	BOOST_TEST((away->normal - rove::vec<3>(1, 0, 0)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE (spheres_apart_and_still_never_touch)
{
	rove::sphere<3> a(rove::vec<3>(0, 0, 0), 1), b(rove::vec<3>(5, 0, 0), 1);
	BOOST_TEST(!rove::first_contact(a, b, rove::vec<3>(0, 0, 0)));
}

BOOST_AUTO_TEST_CASE (spheres_in_3d)
{
	rove::sphere<3> a(rove::vec<3>(0, 0, 0), 1), b(rove::vec<3>(0, 0, 10), 1);
	auto const ci = rove::first_contact(a, b, rove::vec<3>(0, 0, -1));
	BOOST_REQUIRE(ci);
	BOOST_TEST(rove::abs(ci->time - 8) < TOLERANCE);
	BOOST_TEST((ci->normal - rove::vec<3>(0, 0, 1)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_SUITE_END()

namespace
{

using v3 = rove::vec<3>;
using capsule3 = rove::capsule<3>;
using sphere3 = rove::sphere<3>;

/// The gap between the surfaces at time t, the motion taken as first_contact defines it: everything linear in t.
float gap_at(capsule3 const &from, capsule3 const &to, sphere3 const &ball, v3 const &move, float t)
{
	rove::segment<3> const axis(rove::lerp(from.axe.A, to.axe.A, t), rove::lerp(from.axe.B, to.axe.B, t));
	return axis.distance(ball.centre + move * t) - rove::lerp(from.radius, to.radius, t) - ball.radius;
}

float const SWEEP_TOLERANCE = 1e-4f;

}

BOOST_AUTO_TEST_SUITE (first_contact)

BOOST_AUTO_TEST_CASE (apart_and_still_never_touch)
{
	capsule3 const c(v3(-1, 0, 0), v3(1, 0, 0), 0.1f);
	auto const hit = rove::first_contact(c, c, sphere3(v3(0, 1, 0), 0.1f), v3(0, 0, 0), SWEEP_TOLERANCE);
	BOOST_REQUIRE(!hit);
}

BOOST_AUTO_TEST_CASE (overlapping_at_the_start_touch_at_time_zero_and_say_how_deep)
{
	capsule3 const c(v3(-1, 0, 0), v3(1, 0, 0), 0.1f);
	auto const hit = rove::first_contact(c, c, sphere3(v3(0.5f, 0.15f, 0), 0.1f), v3(0, 5, 0), SWEEP_TOLERANCE);
	BOOST_REQUIRE(hit);
	BOOST_REQUIRE(hit->time == 0);
	BOOST_REQUIRE(hit->penetrated);
	BOOST_REQUIRE(rove::abs(hit->penetration_depth - 0.05f) < 1e-5f);
	BOOST_REQUIRE((hit->normal - v3(0, 1, 0)).length() < 1e-5f);
}

// A ball falling on a bar: the centre is at 0.2 above the axis at t = 0.6.
BOOST_AUTO_TEST_CASE (a_ball_meets_a_still_bar_when_the_gap_closes)
{
	capsule3 const c(v3(-1, 0, 0), v3(1, 0, 0), 0.1f);
	auto const hit = rove::first_contact(c, c, sphere3(v3(0.3f, 2, 0), 0.1f), v3(0, -3, 0), SWEEP_TOLERANCE);
	BOOST_REQUIRE(hit);
	BOOST_REQUIRE(!hit->penetrated);
	// never later than the touch, and early by no more than the tolerance takes at the closing speed
	BOOST_REQUIRE(hit->time <= 0.6f + 1e-6f);
	BOOST_REQUIRE(hit->time >= 0.6f - 2 * SWEEP_TOLERANCE / 3);
	BOOST_REQUIRE((hit->normal - v3(0, 1, 0)).length() < 1e-5f);
	// the point of contact is on the bar's surface, under the ball
	BOOST_REQUIRE((hit->position - v3(0.3f, 0.1f, 0)).length() < 1e-3f);
}

// Sampling the ends of a step would see the ball above the bar, then below it, and never touching.
BOOST_AUTO_TEST_CASE (a_fast_ball_does_not_tunnel_through_a_thin_bar)
{
	capsule3 const c(v3(-1, 0, 0), v3(1, 0, 0), 0.01f);
	auto const hit = rove::first_contact(c, c, sphere3(v3(0, 5, 0), 0.01f), v3(0, -10, 0), SWEEP_TOLERANCE);
	BOOST_REQUIRE(hit);
	BOOST_REQUIRE(rove::abs(hit->time - (5 - 0.02f) / 10) < 1e-4f);
}

BOOST_AUTO_TEST_CASE (a_near_miss_is_a_miss)
{
	capsule3 const c(v3(-1, 0, 0), v3(1, 0, 0), 0.1f);
	// passes over the bar with 0.01 between the surfaces
	auto const hit = rove::first_contact(c, c, sphere3(v3(-3, 0.21f, 0), 0.1f), v3(6, 0, 0), SWEEP_TOLERANCE);
	BOOST_REQUIRE(!hit);
	// and past its end, 0.01 clear of the rounded cap
	auto const past = rove::first_contact(c, c, sphere3(v3(1.21f, 3, 0), 0.1f), v3(0, -6, 0), SWEEP_TOLERANCE);
	BOOST_REQUIRE(!past);
}

BOOST_AUTO_TEST_CASE (a_contact_after_the_step_is_not_in_it)
{
	capsule3 const c(v3(-1, 0, 0), v3(1, 0, 0), 0.1f);
	auto const hit = rove::first_contact(c, c, sphere3(v3(0, 2, 0), 0.1f), v3(0, -1.7f, 0), SWEEP_TOLERANCE);          // reaches 0.3 above
	BOOST_REQUIRE(!hit);
}

// A swinging leg: one end still, the other sweeping a quarter turn (along the chord), into a ball at rest.  The axis points at the
// ball's centre at t = 0.5, so the touch comes before then.
BOOST_AUTO_TEST_CASE (a_swinging_capsule_hits_a_ball_at_rest)
{
	capsule3 const from(v3(0, 0, 0), v3(1, 0, 0), 0.05f), to(v3(0, 0, 0), v3(0, 0, 1), 0.05f);
	sphere3 const ball(v3(0.4f, 0, 0.4f), 0.1f);
	auto const hit = rove::first_contact(from, to, ball, v3(0, 0, 0), SWEEP_TOLERANCE);
	BOOST_REQUIRE(hit);
	BOOST_REQUIRE(hit->time > 0 && hit->time < 0.5f);
	BOOST_REQUIRE(gap_at(from, to, ball, v3(0, 0, 0), hit->time) <= SWEEP_TOLERANCE);
	BOOST_REQUIRE(gap_at(from, to, ball, v3(0, 0, 0), hit->time - 1e-3f) > 0);
	// the normal points from the leg to the ball: away from the side the leg comes from (+X)
	BOOST_REQUIRE(hit->normal.z > 0.5f);
	BOOST_REQUIRE(rove::abs(hit->normal.length() - 1) < 1e-5f);
}

// Both move: a ball rolling towards a capsule that comes to meet it.
BOOST_AUTO_TEST_CASE (both_moving_meet_where_their_paths_cross)
{
	capsule3 const from(v3(-0.5f, 0, 2), v3(0.5f, 0, 2), 0.1f), to(v3(-0.5f, 0, 0), v3(0.5f, 0, 0), 0.1f);
	sphere3 const ball(v3(0, 0, -2), 0.1f);
	auto const hit = rove::first_contact(from, to, ball, v3(0, 0, 2), SWEEP_TOLERANCE);
	// gap 4 - 0.2 closing at 4 a unit of time
	BOOST_REQUIRE(hit);
	BOOST_REQUIRE(rove::abs(hit->time - 3.8f / 4) < 1e-4f);
}

BOOST_AUTO_TEST_CASE (a_capsule_that_grows_into_the_ball_touches_it)
{
	capsule3 const from(v3(-1, 0, 0), v3(1, 0, 0), 0.1f), to(v3(-1, 0, 0), v3(1, 0, 0), 0.5f);
	auto const hit = rove::first_contact(from, to, sphere3(v3(0, 0.5f, 0), 0.1f), v3(0, 0, 0), SWEEP_TOLERANCE);
	// 0.5 - 0.1 = 0.1 + 0.4 t at t = 0.75
	BOOST_REQUIRE(hit);
	BOOST_REQUIRE(rove::abs(hit->time - 0.75f) < 1e-3f);
}

BOOST_AUTO_TEST_CASE (works_in_the_plane)
{
	rove::capsule<2> const c(rove::vec<2>(-1, 0), rove::vec<2>(1, 0), 0.1f);
	auto const hit = rove::first_contact(c, c, rove::sphere<2>(rove::vec<2>(0, 1), 0.1f), rove::vec<2>(0, -1), SWEEP_TOLERANCE);
	BOOST_REQUIRE(hit);
	BOOST_REQUIRE(rove::abs(hit->time - 0.8f) < 1e-3f);
}

BOOST_AUTO_TEST_CASE (nonsense_is_refused)
{
	capsule3 const c(v3(-1, 0, 0), v3(1, 0, 0), 0.1f);
	sphere3 const ball(v3(0, 1, 0), 0.1f);
	BOOST_REQUIRE_THROW((void)rove::first_contact(c, c, ball, v3(0, -1, 0), 0.0f), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)rove::first_contact(c, c, ball, v3(0, -1, 0), -1.0f), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)rove::first_contact(c, capsule3(v3(-1, 0, 0), v3(1, 0, 0), -0.1f), ball, v3(0, -1, 0), SWEEP_TOLERANCE), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)rove::first_contact(c, c, sphere3(v3(0, 1, 0), -0.1f), v3(0, -1, 0), SWEEP_TOLERANCE), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)rove::first_contact(c, c, ball, v3(0, std::nanf(""), 0), SWEEP_TOLERANCE), std::invalid_argument);
	// every point, and the move, NaN or infinite in each component
	for (float const bad : {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()})
		for (int k = 0; k < 3; ++k) {
			v3 off(0, 0, 0);
			off.i[k] = bad;
			BOOST_CHECK_THROW((void)rove::first_contact(capsule3(off, c.axe.B, 0.1f), c, ball, v3(0, -1, 0), SWEEP_TOLERANCE), std::invalid_argument);
			BOOST_CHECK_THROW((void)rove::first_contact(capsule3(c.axe.A, off, 0.1f), c, ball, v3(0, -1, 0), SWEEP_TOLERANCE), std::invalid_argument);
			BOOST_CHECK_THROW((void)rove::first_contact(c, capsule3(off, c.axe.B, 0.1f), ball, v3(0, -1, 0), SWEEP_TOLERANCE), std::invalid_argument);
			BOOST_CHECK_THROW((void)rove::first_contact(c, capsule3(c.axe.A, off, 0.1f), ball, v3(0, -1, 0), SWEEP_TOLERANCE), std::invalid_argument);
			BOOST_CHECK_THROW((void)rove::first_contact(c, c, sphere3(off, 0.1f), v3(0, -1, 0), SWEEP_TOLERANCE), std::invalid_argument);
			BOOST_CHECK_THROW((void)rove::first_contact(c, c, ball, off, SWEEP_TOLERANCE), std::invalid_argument);
		}
}

// Against brute force: random swinging capsules and moving balls, the gap sampled densely.  A reported contact has the surfaces within
// the tolerance and no sample before it overlapping; no contact means no sample overlaps.
BOOST_AUTO_TEST_CASE (agrees_with_dense_sampling)
{
	std::mt19937 rng(7);
	std::uniform_real_distribution<float> coord(-1, 1), radius(0.02f, 0.3f);
	auto point = [&] { float const x = coord(rng), y = coord(rng), z = coord(rng); return v3(x, y, z); };
	int hits = 0, misses = 0;
	for (int k = 0; k < 2000; ++k)
	{
		v3 const a0 = point(), b0 = point(), a1 = point(), b1 = point(), c = point() * 1.5f, wander = point() * 2.0f, aside = point() * 0.4f;
		v3 const move = k % 2 ? wander : (a1 + b1) * 0.5f + aside - c;          // half of them aimed near where the capsule ends up
		float const cr = radius(rng), br = radius(rng);
		capsule3 const from(a0, b0, cr), to(a1, b1, cr);
		sphere3 const ball(c, br);
		auto const hit = rove::first_contact(from, to, ball, move, SWEEP_TOLERANCE);
		int const SAMPLES = 4000;
		float first_overlap = 2;
		for (int i = 0; i <= SAMPLES; ++i)
		{
			float const t = float(i) / SAMPLES;
			if (gap_at(from, to, ball, move, t) < 0)
			{
				first_overlap = t;
				break;
			}
		}
		if (hit)
		{
			++hits;
			BOOST_REQUIRE(hit->time >= 0 && hit->time <= 1);
			BOOST_REQUIRE(gap_at(from, to, ball, move, hit->time) <= SWEEP_TOLERANCE * 1.01f);
			BOOST_REQUIRE(hit->time <= first_overlap + 1e-6f);
		}
		else
		{
			++misses;
			BOOST_REQUIRE(first_overlap == 2);
		}
	}
	BOOST_TEST_MESSAGE("hits " << hits << ", misses " << misses);
	// the draw has to test both answers
	BOOST_REQUIRE(hits > 200);
	BOOST_REQUIRE(misses > 200);
}

// t + gap / rate that does not change t: the distance left is below what time can tell, so this is the contact
BOOST_AUTO_TEST_CASE(first_contact_returns_when_the_step_is_below_the_resolution_of_time)
{
	using v3 = rove::vec<3>;
	rove::capsule<3> const c(v3(0, 0, 0), v3(0, 1, 0), 0.5f);
	rove::sphere<3> const ball(v3(2, 0.5f, 0), 0.5f);
	auto const hit = rove::first_contact(c, c, ball, v3(-1.5f, 0, 0), 1e-8f);
	BOOST_REQUIRE(hit.has_value());
	BOOST_TEST(hit->time == 2.0f / 3.0f, boost::test_tools::tolerance(1e-3f));
	// near the contact the gap comes in steps of 1.5e-8 and t in steps of 6e-8: a gap above the tolerance that no step of t can close
	rove::capsule<3> const line(v3(0, 0, 0), v3(0, 1, 0), 0);
	rove::sphere<3> const small(v3(1, 0.5f, 0), 0.25f);
	auto const touch = rove::first_contact(line, line, small, v3(-1, 0, 0), 1e-8f);
	BOOST_REQUIRE(touch.has_value());
	BOOST_TEST(touch->time == 0.75f, boost::test_tools::tolerance(1e-4f));
}

// Whether two lines cross is a question of the angle between them, not of how long their directions are
BOOST_AUTO_TEST_CASE(a_short_direction_is_not_parallel_to_everything)
{
	rove::ray<3> const a(rove::vec<3>(0, 0, 0), rove::vec<3>(5e-4f, 0, 0));
	rove::ray<3> const b(rove::vec<3>(0, 1, 0), rove::vec<3>(0, 0, 5e-4f));
	BOOST_TEST(a.distance(b) == 1.0f, boost::test_tools::tolerance(1e-5f));
	rove::ray<3> const crossing(rove::vec<3>(1, -1, 0), rove::vec<3>(0, 5e-4f, 0));
	float t;
	BOOST_TEST(a.query_intersection(crossing, t));
	BOOST_TEST(a.apply(t).x == 1.0f, boost::test_tools::tolerance(1e-3f));
	rove::ray<2> const a2(rove::vec<2>(0, 0), rove::vec<2>(5e-4f, 0));
	rove::ray<2> const b2(rove::vec<2>(1, -1), rove::vec<2>(0, 5e-4f));
	BOOST_TEST(a2.distance(b2) == 0.0f);
	BOOST_TEST(a2.query_intersection(b2, t));
	// parallel stays parallel
	rove::ray<3> const parallel(rove::vec<3>(0, 2, 0), rove::vec<3>(1, 0, 0));
	BOOST_TEST(a.distance(parallel) == 2.0f, boost::test_tools::tolerance(1e-5f));
	BOOST_TEST(!a.query_intersection(parallel, t));
}

BOOST_AUTO_TEST_CASE(a_circle_above_a_short_segment_touches_it_when_it_should)
{
	using v2 = rove::vec<2>;
	rove::sphere<2> const ball(v2(0.005f, 2e-5f + 1e-5f), 1e-5f);
	rove::segment<2> const short_one(v2(0, 0), v2(0.01f, 0));
	auto const hit = rove::first_contact(short_one, ball, v2(0, -5e-5f));
	BOOST_REQUIRE(hit.has_value());
	BOOST_TEST(hit->time == 0.4f, boost::test_tools::tolerance(1e-2f));
}

BOOST_AUTO_TEST_SUITE_END()
