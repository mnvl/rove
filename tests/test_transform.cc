// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cmath>
#include <random>
#include <stdexcept>
#include <boost/test/unit_test.hpp>
#include <rove/transform.h>
#include "test_garbage.h"

BOOST_AUTO_TEST_SUITE(transform_trs)

namespace
{

rove::trs<float> random_trs(std::mt19937 &rng, bool uniform_scale)
{
	std::uniform_real_distribution<float> u(-3, 3), s(0.25f, 3);
	std::normal_distribution<float> n;
	rove::quaternion<float> q(n(rng), n(rng), n(rng), n(rng));
	q.normalize();
	float const k = s(rng);
	return rove::trs<float>(rove::vec<3>(u(rng), u(rng), u(rng)), q,
		uniform_scale ? rove::vec<3>(k, k, k) : rove::vec<3>(s(rng), s(rng), s(rng)));
}

bool near(rove::vec<3> const &a, rove::vec<3> const &b, float tolerance)
{
	return (a - b).length() <= tolerance * std::max(1.0f, b.length());
}

bool near(rove::matrix<4, 4> const &a, rove::matrix<4, 4> const &b, float tolerance)
{
	return rove::equal(a, b, tolerance);
}

}

BOOST_AUTO_TEST_CASE(the_default_is_no_transform_whatever_the_memory_held)
{
	rove::trs<float> const t = constructed_over_garbage<rove::trs<float>>();
	BOOST_REQUIRE(t.translation == rove::vec<3>(0, 0, 0));
	BOOST_REQUIRE(t.scale == rove::vec<3>(1, 1, 1));
	BOOST_REQUIRE(t.rotation.x == 0 && t.rotation.y == 0 && t.rotation.z == 0 && t.rotation.w == 1);
	BOOST_REQUIRE(rove::equal(t.to_matrix(), rove::matrix<4, 4>(), 0.0f));
}

BOOST_AUTO_TEST_CASE(its_matrix_scales_then_turns_then_moves)
{
	std::mt19937 rng(31);
	for (int k = 0; k < 200; ++k) {
		rove::trs<float> const t = random_trs(rng, false);
		rove::matrix<4, 4> const expected = rove::translation_matrix(t.translation) * rove::rotation_matrix(t.rotation) *
			rove::scaling_matrix(t.scale);
		BOOST_REQUIRE(near(t.to_matrix(), expected, 1e-5f));
		rove::vec<3> const p(0.5f, -1, 2);
		BOOST_REQUIRE(near(t.apply_point(p), rove::transform_point(expected, p), 1e-5f));
		BOOST_REQUIRE(near(t.apply_vector(p), rove::transform_vector(expected, p), 1e-5f));
	}
}

BOOST_AUTO_TEST_CASE(a_matrix_comes_apart_into_its_translation_rotation_and_scale)
{
	std::mt19937 rng(32);
	for (int k = 0; k < 500; ++k) {
		rove::trs<float> const t = random_trs(rng, false);
		rove::trs<float> const back = rove::to_trs(t.to_matrix());
		BOOST_REQUIRE(near(back.translation, t.translation, 1e-5f));
		BOOST_REQUIRE(near(back.scale, t.scale, 1e-4f));
		BOOST_REQUIRE(rove::angle_between(back.rotation, t.rotation) < 1e-3f);
		BOOST_REQUIRE(std::abs(back.rotation.norm() - 1) < 1e-5f);
		BOOST_REQUIRE(near(back.to_matrix(), t.to_matrix(), 1e-4f));
	}
	// a mirror is a negative scale on x, with a proper rotation
	rove::matrix<4, 4> const mirror = rove::scaling_matrix(rove::vec<3>(-2, 3, 4));
	rove::trs<float> const m = rove::to_trs(mirror);
	BOOST_REQUIRE(near(m.scale, rove::vec<3>(-2, 3, 4), 1e-6f));
	BOOST_REQUIRE(near(m.to_matrix(), mirror, 1e-6f));
}

BOOST_AUTO_TEST_CASE(a_matrix_that_is_no_trs_does_not_come_apart)
{
	// sheared: its columns are not square to each other
	rove::matrix<4, 4> shear;
	shear.ij[1][0] = 0.5f;
	BOOST_REQUIRE_THROW((void)rove::to_trs(shear), std::invalid_argument);
	// a projection
	BOOST_REQUIRE_THROW((void)rove::to_trs(rove::perspective_matrix(1.0f, 1.0f, 1.0f, 10.0f)), std::invalid_argument);
	// flattened: a scale of zero has no rotation to read
	BOOST_REQUIRE_THROW((void)rove::to_trs(rove::scaling_matrix(rove::vec<3>(1, 0, 1))), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(composing_is_the_matrices_product_where_it_is_a_trs)
{
	std::mt19937 rng(33);
	for (int k = 0; k < 300; ++k) {
		// a parent of uniform scale: any child
		rove::trs<float> const a = random_trs(rng, true), b = random_trs(rng, false);
		BOOST_REQUIRE(near((a * b).to_matrix(), a.to_matrix() * b.to_matrix(), 1e-4f));
		// a parent of any scale over a child that does not turn
		rove::trs<float> const c = random_trs(rng, false);
		rove::trs<float> d = random_trs(rng, false);
		d.rotation = rove::quaternion<float>();
		BOOST_REQUIRE(near((c * d).to_matrix(), c.to_matrix() * d.to_matrix(), 1e-4f));
	}
	// a stretched parent over a turned child is a shear, no trs: an error, not a wrong answer
	rove::trs<float> stretched;
	stretched.scale = rove::vec<3>(1, 3, 1);
	rove::trs<float> turned;
	turned.rotation = rove::axis_angle(rove::vec<3>(0, 0, 1), 0.5f);
	BOOST_REQUIRE_THROW((void)(stretched * turned), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(the_inverse_undoes_it_where_it_is_a_trs)
{
	std::mt19937 rng(34);
	for (int k = 0; k < 300; ++k) {
		rove::trs<float> const t = random_trs(rng, true);
		rove::vec<3> const p(1, -2, 3);
		BOOST_REQUIRE(near(rove::inverse(t).apply_point(t.apply_point(p)), p, 1e-4f));
		BOOST_REQUIRE(near(rove::inverse(t).to_matrix(), rove::inverse(t.to_matrix()), 1e-4f));
	}
	// a stretch without a turn inverts; with one it is no trs
	rove::trs<float> stretch;
	stretch.scale = rove::vec<3>(2, 4, 8);
	stretch.translation = rove::vec<3>(1, 1, 1);
	BOOST_REQUIRE(near(rove::inverse(stretch).to_matrix(), rove::inverse(stretch.to_matrix()), 1e-6f));
	stretch.rotation = rove::axis_angle(rove::vec<3>(1, 0, 0), 0.3f);
	BOOST_REQUIRE_THROW((void)rove::inverse(stretch), std::invalid_argument);
	rove::trs<float> flat;
	flat.scale = rove::vec<3>(1, 0, 1);
	BOOST_REQUIRE_THROW((void)rove::inverse(flat), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(lerp_moves_scales_and_slerps)
{
	rove::trs<float> a, b;
	b.translation = rove::vec<3>(10, 0, 0);
	b.scale = rove::vec<3>(3, 3, 3);
	b.rotation = rove::axis_angle(rove::vec<3>(0, 1, 0), 1.0f);
	rove::trs<float> const half = rove::lerp(a, b, 0.5f);
	BOOST_REQUIRE(half.translation == rove::vec<3>(5, 0, 0));
	BOOST_REQUIRE(half.scale == rove::vec<3>(2, 2, 2));
	BOOST_REQUIRE(std::abs(rove::angle_between(half.rotation, rove::axis_angle(rove::vec<3>(0, 1, 0), 0.5f))) < 1e-6f);
	BOOST_REQUIRE(rove::lerp(a, b, 0.0f).translation == a.translation);
	BOOST_REQUIRE(rove::lerp(a, b, 1.0f).translation == b.translation);
}

BOOST_AUTO_TEST_CASE(the_ray_under_a_screen_point_projects_back_onto_it)
{
	rove::matrix<4, 4> const view = rove::look_at_matrix(rove::vec<3>(1, 2, -5), rove::vec<3>(0, 0, 3), rove::vec<3>(0, 1, 0));
	rove::matrix<4, 4> const projection = rove::perspective_matrix(1.0f, 1.5f, 0.5f, 50.0f);
	rove::matrix<4, 4> const vp = projection * view;
	rove::matrix<4, 4> const inverse_vp = rove::inverse(vp);
	for (float x : {-0.9f, 0.0f, 0.3f})
		for (float y : {-0.5f, 0.0f, 0.8f}) {
			rove::ray<3> const r = rove::unproject_ray(inverse_vp, x, y);
			for (float t : {0.0f, 0.3f, 1.0f}) {
				rove::vec<3> const back = rove::project_point(vp, r.apply(t));
				BOOST_REQUIRE(std::abs(back.x - x) < 1e-3f && std::abs(back.y - y) < 1e-3f);
			}
			BOOST_REQUIRE(std::abs(rove::project_point(vp, r.apply(0)).z) < 1e-3f);          // starts on the near plane
			BOOST_REQUIRE(std::abs(rove::project_point(vp, r.apply(1)).z - 1) < 1e-3f);      // ends on the far
		}
	// the centre of the screen looks where the camera does
	rove::ray<3> const centre = rove::unproject_ray(inverse_vp, 0.0f, 0.0f);
	BOOST_REQUIRE(rove::angle_between(centre.a, rove::vec<3>(0, 0, 3) - rove::vec<3>(1, 2, -5)) < 1e-3f);
}

BOOST_AUTO_TEST_SUITE_END()
