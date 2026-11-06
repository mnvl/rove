// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <stdlib.h>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <boost/test/unit_test.hpp>
#include <rove/matrix.h>
#include <rove/vec.h>
#include <rove/quaternion.h>
#include "test_tolerance.h"
#include "test_garbage.h"

#include <iostream>

namespace
{

void test_matrix_quaternion_matrix(rove::matrix<3, 3> const& tf) {
	BOOST_REQUIRE (abs(tf.determinant() - 1) < TOLERANCE);

	rove::quaternion<> q;
	q.set_unit(tf);

	rove::matrix<3, 3> m;
	m.rotation(q.normalized());

	BOOST_REQUIRE (rove::equal(m, tf, rove::scalar(0.001)));
}

void test_slerp_i(rove::vec<3> const& p, rove::vec<3> const& q) {
	rove::matrix<3, 3> tf1;
	tf1.rotation(p);
	rove::quaternion<> q1;
	q1.set_unit(tf1);

	rove::matrix<3, 3> tf2;
	tf2.rotation(q);
	rove::quaternion<> q2;
	q2.set_unit(tf2);

	rove::quaternion_slerper<> slerper;
	slerper.setup(q1, q2);

	for (int i = 0; i <= 100; ++i) {
		rove::scalar x = rove::scalar(i) / 100;

		rove::quaternion<> q3 = slerper.interpolate(x);
		rove::matrix<3, 3> tf3;
		tf3.rotation(q3);

		rove::vec<3> angles = p * (1 - x) + q * x;
		rove::matrix<3, 3> tf4;
		tf4.rotation(angles);

		BOOST_REQUIRE(equal(tf3, tf4, rove::scalar(0.001)));
	}
}

void test_slerp(rove::vec<3> const& p, rove::vec<3> const& q) {
	test_slerp_i(p, q);
	test_slerp_i(q, p);
}

}

BOOST_AUTO_TEST_SUITE (test_quaternion)

BOOST_AUTO_TEST_CASE (identity_matrix)
{
	rove::matrix<3, 3> tf;
	tf.identity();

	test_matrix_quaternion_matrix(tf);
}

BOOST_AUTO_TEST_CASE (rotate_90_degrees)
{
	rove::matrix<3, 3> tf;

	tf.rotation(rove::PI / 2, 0, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(-rove::PI / 2, 0, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, rove::PI / 2, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, -rove::PI / 2, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, 0, rove::PI / 2);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, 0, -rove::PI / 2);
	test_matrix_quaternion_matrix(tf);
}

BOOST_AUTO_TEST_CASE (rotate_60_degrees)
{
	rove::matrix<3, 3> tf;

	tf.rotation(rove::PI / 3, 0, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(-rove::PI / 3, 0, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, rove::PI / 3, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, -rove::PI / 3, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, 0, rove::PI / 3);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, 0, -rove::PI / 3);
	test_matrix_quaternion_matrix(tf);
}

BOOST_AUTO_TEST_CASE (rotate_30_degrees)
{
	rove::matrix<3, 3> tf;

	tf.rotation(rove::PI / 6, 0, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(-rove::PI / 6, 0, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, rove::PI / 6, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, -rove::PI / 6, 0);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, 0, rove::PI / 6);
	test_matrix_quaternion_matrix(tf);

	tf.rotation(0, 0, -rove::PI / 6);
	test_matrix_quaternion_matrix(tf);
}

BOOST_AUTO_TEST_CASE (rotate_random_degrees)
{
	rove::matrix<3, 3> tf;

	for (int i = 0; i < 1000; ++i) {
		rove::vec<3> angles{
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) * rove::PI,
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) * rove::PI,
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) * rove::PI};
		tf.rotation(angles);
		test_matrix_quaternion_matrix(tf);
	}
}

BOOST_AUTO_TEST_CASE (slerp_identity)
{
	test_slerp(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 0, 0));
}

BOOST_AUTO_TEST_CASE (slerp_90_degrees)
{
	test_slerp(rove::vec<3>(0, 0, 0), rove::vec<3>(rove::PI / 2, 0, 0));
	test_slerp(rove::vec<3>(0, 0, 0), rove::vec<3>(-rove::PI / 2, 0, 0));
	test_slerp(rove::vec<3>(0, 0, 0), rove::vec<3>(0, rove::PI / 2, 0));
	test_slerp(rove::vec<3>(0, 0, 0), rove::vec<3>(0, -rove::PI / 2, 0));
	test_slerp(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 0, rove::PI / 2));
	test_slerp(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 0, -rove::PI / 2));

	test_slerp(rove::vec<3>(-rove::PI/2, 0, 0), rove::vec<3>(rove::PI / 2, 0, 0));
	test_slerp(rove::vec<3>(0, -rove::PI/2, 0), rove::vec<3>(0, rove::PI / 2, 0));
	test_slerp(rove::vec<3>(0, 0, -rove::PI/2), rove::vec<3>(0, 0, rove::PI / 2));
}

BOOST_AUTO_TEST_CASE (slerp_random_degrees)
{
	for (int i = 0; i < 1000; ++i) {
		rove::vec<3> axis{
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) - 1,
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) - 1,
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) - 1};

		if (axis.length_sq() < 0.1) continue;

		rove::scalar angle1 = rove::scalar(rand()) / rove::scalar(RAND_MAX) * rove::PI * 2;
		rove::scalar angle2 = angle1 + rove::scalar(rand() - 1) / rove::scalar(RAND_MAX) * rove::PI;

		rove::matrix<3,3> tf1;
		tf1.rotation(axis, angle1);

		rove::matrix<3,3> tf2;
		tf2.rotation(axis, angle2);

		rove::quaternion<> q1;
		q1.set_unit(tf1);

		rove::quaternion<> q2;
		q2.set_unit(tf2);

		rove::quaternion_slerper<> slerper;
		slerper.setup(q1, q2);

		for (int j = 0; j <= 100; ++j) {
			rove::scalar x = rove::scalar(j) / 100;

			rove::quaternion<> q = slerper.interpolate(x);

			rove::matrix<3,3> m1;
			m1.rotation(q);

			rove::matrix<3,3> m2;
			m2.rotation(axis, angle1 * (1 - x) + angle2 * x);

			BOOST_REQUIRE(equal(m1, m2, rove::scalar(0.001)));
		}
	}
}

// Test that set_unit produces a unit quaternion without needing normalization
// This catches the bug where w component was missing /2 divisor
BOOST_AUTO_TEST_CASE (set_unit_produces_unit_quaternion)
{
	// Identity matrix should produce quaternion (0, 0, 0, 1)
	rove::matrix<3, 3> identity;
	identity.identity();
	rove::quaternion<> q_identity;
	q_identity.set_unit(identity);

	BOOST_REQUIRE(abs(q_identity.norm() - 1) < TOLERANCE);
	BOOST_REQUIRE(abs(q_identity.w - 1) < TOLERANCE);
	BOOST_REQUIRE(abs(q_identity.x) < TOLERANCE);
	BOOST_REQUIRE(abs(q_identity.y) < TOLERANCE);
	BOOST_REQUIRE(abs(q_identity.z) < TOLERANCE);

	// 90-degree rotation around X axis should produce unit quaternion
	rove::matrix<3, 3> rot_x;
	rot_x.rotation(rove::PI / 2, 0, 0);
	rove::quaternion<> q_rot_x;
	q_rot_x.set_unit(rot_x);

	BOOST_REQUIRE(abs(q_rot_x.norm() - 1) < TOLERANCE);

	// Random rotations should produce unit quaternions
	for (int i = 0; i < 100; ++i) {
		rove::vec<3> angles{
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) * rove::PI,
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) * rove::PI,
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) * rove::PI};

		rove::matrix<3, 3> tf;
		tf.rotation(angles);

		rove::quaternion<> q;
		q.set_unit(tf);

		BOOST_REQUIRE(abs(q.norm() - 1) < TOLERANCE);
	}
}

// Test SLERP with quaternions that have negative dot product
// This catches the bug where SLERP would take the long path instead of short path
BOOST_AUTO_TEST_CASE (slerp_negative_dot_product)
{
	// Create a quaternion and its negation (same rotation, opposite quaternion)
	// Then test SLERP from a third quaternion to both - should give same result
	rove::vec<3> axis(0, 0, 1);

	// q1: small rotation around Z
	rove::matrix<3, 3> tf1;
	tf1.rotation(axis, rove::scalar(0.2));
	rove::quaternion<> q1;
	q1.set_unit(tf1);

	// q2: another rotation around Z
	rove::matrix<3, 3> tf2;
	tf2.rotation(axis, rove::scalar(0.5));
	rove::quaternion<> q2;
	q2.set_unit(tf2);

	// q2_neg: negation of q2 (same rotation, but negative dot product with q1)
	rove::quaternion<> q2_neg = -q2;

	// Verify q2_neg has negative dot product with q1
	rove::scalar dot_pos = rove::dot(q1.normalized(), q2.normalized());
	rove::scalar dot_neg = rove::dot(q1.normalized(), q2_neg.normalized());
	BOOST_REQUIRE(dot_pos > 0);
	BOOST_REQUIRE(dot_neg < 0);

	// SLERP to q2 and to q2_neg should produce the same rotation matrices
	rove::quaternion_slerper<> slerper_pos, slerper_neg;
	slerper_pos.setup(q1, q2);
	slerper_neg.setup(q1, q2_neg);

	for (int i = 0; i <= 10; ++i) {
		rove::scalar t = rove::scalar(i) / 10;

		rove::quaternion<> r_pos = slerper_pos.interpolate(t);
		rove::quaternion<> r_neg = slerper_neg.interpolate(t);

		// Both should be unit quaternions
		BOOST_REQUIRE(abs(r_pos.norm() - 1) < TOLERANCE);
		BOOST_REQUIRE(abs(r_neg.norm() - 1) < TOLERANCE);

		// Convert to matrices - should be equal since SLERP should handle negative dot
		rove::matrix<3, 3> m_pos, m_neg;
		m_pos.rotation(r_pos);
		m_neg.rotation(r_neg);

		BOOST_REQUIRE(rove::equal(m_pos, m_neg, rove::scalar(0.001)));
	}
}

// Test SLERP specifically with opposite quaternions (dot product near -1)
BOOST_AUTO_TEST_CASE (slerp_opposite_quaternions)
{
	rove::quaternion<> q1;
	q1.identity();  // (0, 0, 0, 1)

	// Create quaternion for 180-degree rotation around Z axis
	// This gives (0, 0, 1, 0) which has dot product 0 with identity
	// But we want to test near -1, so use a small rotation and its opposite
	rove::matrix<3, 3> tf;
	tf.rotation(0, 0, rove::scalar(0.1));
	rove::quaternion<> q2;
	q2.set_unit(tf);

	// Negate q2 - represents same rotation but opposite quaternion
	rove::quaternion<> q2_neg = -q2;

	// dot(q1, q2_neg) should be negative
	rove::scalar dot = rove::dot(q1, q2_neg);
	BOOST_REQUIRE(dot < 0);

	// SLERP between q1 and q2_neg should give same result as q1 to q2
	rove::quaternion_slerper<> slerper1, slerper2;
	slerper1.setup(q1, q2);
	slerper2.setup(q1, q2_neg);

	for (int i = 0; i <= 10; ++i) {
		rove::scalar t = rove::scalar(i) / 10;

		rove::quaternion<> r1 = slerper1.interpolate(t);
		rove::quaternion<> r2 = slerper2.interpolate(t);

		// Convert to matrices - should be equal since q and -q represent same rotation
		rove::matrix<3, 3> m1, m2;
		m1.rotation(r1);
		m2.rotation(r2);

		BOOST_REQUIRE(rove::equal(m1, m2, rove::scalar(0.001)));
	}
}

// A half turn has w = 0, so every off-diagonal difference -- 4 w x, 4 w y,
// 4 w z -- is zero: the signs of x, y and z cannot be read from them.
// Axes with mixed signs are the ones a single guessed sign gets wrong.
namespace
{
rove::vec<3> const HALF_TURN_AXES[] = {
	{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {1, -1, 0}, {1, 0, -1},
	{0, 1, -1}, {1, 1, 1}, {-1, 1, 1}, {1, -1, 1}, {1, 1, -1}, {1, -2, 3},
};

void test_axis_angle_round_trip(rove::vec<3> axis, rove::scalar angle) {
	axis.normalize();
	rove::matrix<3, 3> tf;
	tf.rotation(axis, angle);

	rove::quaternion<> q;
	q.set_unit(tf);
	BOOST_TEST_INFO("axis " << axis.x << " " << axis.y << " " << axis.z << ", angle " << angle);
	BOOST_REQUIRE(abs(q.norm() - 1) < 1e-5f);

	rove::matrix<3, 3> m;
	m.rotation(q);
	BOOST_TEST_INFO("axis " << axis.x << " " << axis.y << " " << axis.z << ", angle " << angle);
	BOOST_REQUIRE(rove::equal(m, tf, rove::scalar(1e-4)));
}
}

BOOST_AUTO_TEST_CASE (set_unit_of_a_half_turn_is_that_half_turn)
{
	for (rove::vec<3> const &axis : HALF_TURN_AXES) {
		test_axis_angle_round_trip(axis, rove::PI);
	}
}

BOOST_AUTO_TEST_CASE (set_unit_near_a_half_turn_is_that_turn)
{
	// w is a few ulp from zero and the differences are rounding noise.
	for (rove::vec<3> const &axis : HALF_TURN_AXES) {
		for (rove::scalar angle : {rove::PI - 1e-3f, rove::PI - 1e-5f, rove::PI + 1e-5f, rove::PI + 1e-3f}) {
			test_axis_angle_round_trip(axis, angle);
		}
	}
}

BOOST_AUTO_TEST_CASE (set_unit_small_and_quarter_turns)
{
	for (rove::vec<3> const &axis : HALF_TURN_AXES) {
		for (rove::scalar angle : {0.0f, 1e-4f, -1e-4f, rove::PI / 2, -rove::PI / 2, 2.0f, -2.5f}) {
			test_axis_angle_round_trip(axis, angle);
		}
	}
}

namespace
{

rove::matrix<3, 3> matrix_of(rove::quaternion<> const &q)
{
	rove::matrix<3, 3> m;
	m.rotation(q);
	return m;
}

rove::quaternion<> quaternion_of(rove::vec<3> const &axis, rove::scalar angle)
{
	rove::matrix<3, 3> m;
	m.rotation(axis, angle);
	rove::quaternion<> q;
	q.set_unit(m);
	return q;
}

std::vector<rove::quaternion<>> some_rotations()
{
	std::vector<rove::quaternion<>> out;
	for (rove::vec<3> const &axis : {rove::vec<3>(1, 0, 0), rove::vec<3>(0, 1, 0), rove::direction(rove::vec<3>(0.3f, -0.8f, 0.52f)), rove::direction(rove::vec<3>(-1, 2, 0.5f))})
		for (rove::scalar angle : {0.0f, 0.4f, -1.3f, 2.9f})
			out.push_back(quaternion_of(axis, angle));
	return out;
}

}

BOOST_AUTO_TEST_CASE (product_is_the_rotations_one_after_the_other_as_matrices_compose)
{
	for (rove::quaternion<> const &p : some_rotations())
		for (rove::quaternion<> const &q : some_rotations())
		{
			rove::matrix<3, 3> const pq = matrix_of(p) * matrix_of(q);
			BOOST_REQUIRE(rove::equal(matrix_of(p * q), pq, rove::scalar(1e-4)));
		}
}

BOOST_AUTO_TEST_CASE (conjugate_undoes_a_unit_rotation)
{
	for (rove::quaternion<> const &q : some_rotations())
	{
		rove::matrix<3, 3> identity;
		identity.identity();
		BOOST_REQUIRE(rove::equal(matrix_of(q * conjugate(q)), identity, rove::scalar(1e-5)));
		rove::matrix<3, 3> back;
		matrix_of(q).transpose(back);
		BOOST_REQUIRE(rove::equal(matrix_of(conjugate(q)), back, rove::scalar(1e-5)));
	}
}

BOOST_AUTO_TEST_CASE (axis_angle_is_the_matrix_rotation_about_that_axis)
{
	for (rove::vec<3> const &axis : {rove::vec<3>(0, 0, 1), rove::vec<3>(0.6f, 0, -0.8f), rove::direction(rove::vec<3>(1, 1, 1))})
		for (rove::scalar angle : {0.0f, 0.3f, -2.0f, 3.1f})
		{
			rove::matrix<3, 3> m;
			m.rotation(axis, angle);
			rove::quaternion<> const q = rove::axis_angle(axis, angle);
			BOOST_REQUIRE(abs(q.norm() - 1) < 1e-5f);
			BOOST_REQUIRE(rove::equal(matrix_of(q), m, rove::scalar(1e-5)));
		}
}

BOOST_AUTO_TEST_CASE (angle_between_is_the_turn_from_one_to_the_other_whichever_sign)
{
	for (rove::quaternion<> const &p : some_rotations())
		for (rove::scalar angle : {0.0f, 0.2f, 1.5f, 3.0f})
		{
			rove::quaternion<> const q = rove::axis_angle(rove::vec<3>(0.36f, 0.48f, 0.8f), angle) * p;
			BOOST_REQUIRE(abs(rove::angle_between(p, q) - angle) < 1e-3f);
			BOOST_REQUIRE(abs(rove::angle_between(p, -q) - angle) < 1e-3f);          // the same rotation
			BOOST_REQUIRE(abs(rove::angle_between(q, p) - angle) < 1e-3f);
		}
}

BOOST_AUTO_TEST_CASE (angle_between_a_rotation_and_itself_is_nothing_to_the_float)
{
	// (2 acos of the dot product is a thousandth of a radian off for a dot product a rounding short of one)
	for (rove::quaternion<> const &q : some_rotations())
	{
		rove::quaternion<> const again = (q * rove::scalar(1.0000003)).normalized();
		BOOST_REQUIRE(rove::angle_between(q, again) < 1e-5f);
		BOOST_REQUIRE(rove::angle_between(q, q) < 1e-6f);
		rove::quaternion<> const tiny = rove::axis_angle(rove::vec<3>(0, 0, 1), rove::scalar(2e-4)) * q;
		BOOST_REQUIRE(abs(rove::angle_between(q, tiny) - 2e-4f) < 2e-5f);
	}
}

BOOST_AUTO_TEST_CASE (power_takes_that_share_of_the_turn_about_the_same_axis)
{
	rove::vec<3> const axis = rove::vec<3>(0.36f, 0.48f, 0.8f);
	for (rove::scalar angle : {0.5f, 2.0f, -2.5f})
		for (rove::scalar share : {0.0f, 0.25f, 0.5f, 1.0f})
		{
			rove::quaternion<> const q = rove::axis_angle(axis, angle);
			BOOST_REQUIRE(rove::equal(matrix_of(rove::power(q, share)), matrix_of(rove::axis_angle(axis, angle * share)), rove::scalar(1e-4)));
			BOOST_REQUIRE(rove::equal(matrix_of(rove::power(-q, share)), matrix_of(rove::axis_angle(axis, angle * share)), rove::scalar(1e-4)));          // the shorter arc
		}
}

BOOST_AUTO_TEST_CASE (default_is_the_identity_not_whatever_the_memory_held)
{
	rove::quaternion<> const q = constructed_over_garbage<rove::quaternion<>>();
	BOOST_REQUIRE_EQUAL(q.x, 0.0f);
	BOOST_REQUIRE_EQUAL(q.y, 0.0f);
	BOOST_REQUIRE_EQUAL(q.z, 0.0f);
	BOOST_REQUIRE_EQUAL(q.w, 1.0f);
	rove::quaternion<double> const d = constructed_over_garbage<rove::quaternion<double>>();
	BOOST_REQUIRE_EQUAL(d.w, 1.0);
	BOOST_REQUIRE_EQUAL(d.x, 0.0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(quaternion_operations)

namespace
{

rove::quaternion<> random_unit(std::mt19937 &rng)
{
	std::normal_distribution<float> n;
	rove::quaternion<> q(n(rng), n(rng), n(rng), n(rng));
	q.normalize();
	return q;
}

rove::vec<3> random_vec(std::mt19937 &rng)
{
	std::uniform_real_distribution<float> u(-5, 5);
	return rove::vec<3>(u(rng), u(rng), u(rng));
}

bool near(rove::vec<3> const &a, rove::vec<3> const &b, float tolerance)
{
	return (a - b).length() <= tolerance;
}

bool same_rotation(rove::quaternion<> const &p, rove::quaternion<> const &q, float tolerance)
{
	return rove::angle_between(p.normalized(), q.normalized()) <= tolerance;
}

}

BOOST_AUTO_TEST_CASE(normalizing_no_quaternion_is_an_error_not_the_identity)
{
	// it became the identity: a zero quaternion (a lerp of opposite ones, a damaged key) turned into no rotation at all
	rove::quaternion<> zero(0, 0, 0, 0);
	BOOST_REQUIRE_THROW(zero.normalize(), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)zero.normalized(), std::invalid_argument);
	rove::quaternion<> nan(std::numeric_limits<float>::quiet_NaN(), 0, 0, 1);
	BOOST_REQUIRE_THROW(nan.normalize(), std::invalid_argument);
	rove::quaternion<> huge(std::numeric_limits<float>::infinity(), 0, 0, 1);
	BOOST_REQUIRE_THROW((void)huge.normalized(), std::invalid_argument);
	rove::quaternion_slerper<> slerper;
	BOOST_REQUIRE_THROW(slerper.setup(zero, rove::quaternion<>()), std::invalid_argument);
	// a quaternion of any other length normalizes, the bits as they were: q / |q|
	rove::quaternion<> q(1, 2, 3, 4);
	rove::quaternion<> const n = q.normalized();
	float const len = std::sqrt(30.0f);
	BOOST_REQUIRE_EQUAL(n.x, 1 * (1 / len));
	BOOST_REQUIRE_EQUAL(n.w, 4 * (1 / len));
}

BOOST_AUTO_TEST_CASE(a_quaternion_turns_a_vector_as_its_matrix_does)
{
	std::mt19937 rng(11);
	for (int k = 0; k < 1000; ++k) {
		rove::quaternion<> const q = random_unit(rng);
		rove::vec<3> const v = random_vec(rng);
		rove::matrix<3, 3> m;
		m.rotation(q);
		BOOST_REQUIRE(near(q * v, m * v, 1e-5f));
		BOOST_REQUIRE(near(rove::rotate(q, v), m * v, 1e-5f));
		// a quaternion of another length is the same rotation
		BOOST_REQUIRE(near(rove::rotate(q * 3.5f, v), m * v, 1e-4f));
		BOOST_REQUIRE(near(rove::rotate(-q, v), m * v, 1e-5f));
	}
	// a quarter turn about +z takes +x to +y
	rove::quaternion<> const quarter = rove::axis_angle(rove::vec<3>(0, 0, 1), rove::PI / 2);
	BOOST_REQUIRE(near(quarter * rove::vec<3>(1, 0, 0), rove::vec<3>(0, 1, 0), 1e-6f));
	BOOST_REQUIRE_THROW((void)rove::rotate(rove::quaternion<>(0, 0, 0, 0), rove::vec<3>(1, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(slerp_is_the_slerpers_and_nlerp_takes_the_short_way)
{
	std::mt19937 rng(12);
	for (int k = 0; k < 300; ++k) {
		rove::quaternion<> const p = random_unit(rng), q = random_unit(rng);
		rove::quaternion_slerper<> slerper;
		slerper.setup(p, q);
		for (float t : {0.0f, 0.25f, 0.5f, 0.9f, 1.0f}) {
			rove::quaternion<> const s = rove::slerp(p, q, t), expected = slerper.interpolate(t);
			BOOST_REQUIRE_EQUAL(s.x, expected.x);
			BOOST_REQUIRE_EQUAL(s.w, expected.w);
			rove::quaternion<> const n = rove::nlerp(p, q, t);
			BOOST_REQUIRE(std::abs(n.norm() - 1) < 1e-6f);
			// nlerp is no constant speed, but the same arc: at most about 0.071 behind or ahead along it (inputs half a turn apart, t a
			// quarter), 0.142 of rotation
			BOOST_REQUIRE(same_rotation(n, s, 0.15f));
		}
		BOOST_REQUIRE(same_rotation(rove::nlerp(p, q, 0.0f), p, 1e-3f));
		BOOST_REQUIRE(same_rotation(rove::nlerp(p, q, 1.0f), q, 1e-3f));
		// opposite signs are the same rotation: nlerp halfway is no zero and so no error
		BOOST_REQUIRE(same_rotation(rove::nlerp(p, -p, 0.5f), p, 1e-3f));
	}
}

BOOST_AUTO_TEST_CASE(inverse_undoes_a_rotation_of_any_length)
{
	rove::quaternion<> const q(1, 2, 3, 4);
	rove::quaternion<> const i = q * rove::inverse(q);
	BOOST_REQUIRE(std::abs(i.x) < 1e-6f && std::abs(i.y) < 1e-6f && std::abs(i.z) < 1e-6f && std::abs(i.w - 1) < 1e-6f);
	rove::vec<3> const v(1, -2, 0.5f);
	BOOST_REQUIRE(near(rove::rotate(rove::inverse(q), rove::rotate(q, v)), v, 1e-5f));
	BOOST_REQUIRE_THROW((void)rove::inverse(rove::quaternion<>(0, 0, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(to_quaternion_is_set_unit_by_value)
{
	std::mt19937 rng(13);
	for (int k = 0; k < 200; ++k) {
		rove::matrix<4, 4> m;
		m.rotation(random_unit(rng));
		rove::quaternion<> expected;
		expected.set_unit(m);
		rove::quaternion<> const q = rove::to_quaternion(m);
		BOOST_REQUIRE(q.x == expected.x && q.y == expected.y && q.z == expected.z && q.w == expected.w);
	}
}

BOOST_AUTO_TEST_CASE(rotation_between_turns_one_direction_onto_the_other_the_short_way)
{
	std::mt19937 rng(14);
	for (int k = 0; k < 1000; ++k) {
		rove::vec<3> const a = random_vec(rng), b = random_vec(rng);
		rove::quaternion<> const q = rove::rotation_between(a, b);
		BOOST_REQUIRE(std::abs(q.norm() - 1) < 1e-6f);
		BOOST_REQUIRE(near(rove::rotate(q, rove::normalize(a)), rove::normalize(b), 1e-5f));
		// the short way: by the angle between them, about their common perpendicular
		BOOST_REQUIRE(std::abs(rove::angle_between(rove::quaternion<>(), q) - rove::angle_between(a, b)) < 1e-4f);
	}
	// the same direction: no turn; opposite ones: a half turn about some perpendicular
	rove::quaternion<> const none = rove::rotation_between(rove::vec<3>(1, 2, 3), rove::vec<3>(2, 4, 6));
	BOOST_REQUIRE(same_rotation(none, rove::quaternion<>(), 1e-6f));
	for (rove::vec<3> const d : {rove::vec<3>(1, 0, 0), rove::vec<3>(0, 1, 0), rove::vec<3>(0, 0, -2), rove::vec<3>(1, 1, 1)}) {
		rove::quaternion<> const half = rove::rotation_between(d, -d);
		BOOST_REQUIRE(near(rove::rotate(half, rove::normalize(d)), -rove::normalize(d), 1e-5f));
		BOOST_REQUIRE(std::abs(half.norm() - 1) < 1e-6f);
	}
	BOOST_REQUIRE_THROW((void)rove::rotation_between(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0)), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)rove::rotation_between(rove::vec<3>(1, 0, 0), rove::vec<3>(0, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(axis_angle_and_rotation_vectors_go_there_and_back)
{
	std::mt19937 rng(15);
	for (int k = 0; k < 1000; ++k) {
		rove::quaternion<> const q = random_unit(rng);
		rove::vec<3> axis;
		float angle;
		rove::to_axis_angle(q, axis, angle);
		BOOST_REQUIRE(angle >= 0 && angle <= rove::PI);
		BOOST_REQUIRE(std::abs(axis.length() - 1) < 1e-5f);
		BOOST_REQUIRE(same_rotation(rove::axis_angle(axis, angle), q, 1e-4f));
		rove::vec<3> const r = rove::rotation_vector(q);
		BOOST_REQUIRE(std::abs(r.length() - angle) < 1e-5f);
		BOOST_REQUIRE(same_rotation(rove::from_rotation_vector(r), q, 1e-4f));
	}
	// no rotation: the zero vector, and back
	BOOST_REQUIRE(rove::rotation_vector(rove::quaternion<>()) == rove::vec<3>(0, 0, 0));
	BOOST_REQUIRE(same_rotation(rove::from_rotation_vector(rove::vec<3>(0, 0, 0)), rove::quaternion<>(), 0));
	// a tiny turn keeps its size (the exp and log maps near zero)
	double const tiny = 1e-10;
	rove::quaternion<double> const small = rove::axis_angle(rove::vec<3, double>(0, 1, 0), tiny);
	BOOST_REQUIRE(std::abs(rove::rotation_vector(small).y - tiny) < 1e-20);
	BOOST_REQUIRE(std::abs(rove::from_rotation_vector(rove::vec<3, double>(0, tiny, 0)).y - tiny / 2) < 1e-20);
	BOOST_REQUIRE_THROW((void)rove::rotation_vector(rove::quaternion<>(0, 0, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(the_matrix_of_a_quaternion_of_any_length_is_its_rotation)
{
	// rotation(q) scaled by 2 / |q| where 2 / |q|^2 belongs: right for a unit quaternion only
	std::mt19937 rng(16);
	for (int k = 0; k < 200; ++k) {
		rove::quaternion<> const q = random_unit(rng);
		rove::matrix<3, 3> unit, scaled;
		unit.rotation(q);
		scaled.rotation(q * 3.0f);
		BOOST_REQUIRE(rove::equal(unit, scaled, 1e-5f));
		rove::matrix<4, 4> unit4, scaled4;
		unit4.rotation(q);
		scaled4.rotation(q * 0.25f);
		BOOST_REQUIRE(rove::equal(unit4, scaled4, 1e-5f));
		BOOST_REQUIRE(std::abs(scaled4.determinant() - 1) < 1e-5f);
	}
	rove::matrix<3, 3> m;
	BOOST_REQUIRE_THROW(m.rotation(rove::quaternion<>(0, 0, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(the_matrix_of_a_unit_quaternion_has_the_bits_it_had)
{
	// for a quaternion within a rounding of unit length the arithmetic is the one it always was: a lockstep simulation's recordings
	// still play back
	std::mt19937 rng(17);
	for (int k = 0; k < 1000; ++k) {
		rove::quaternion<> const q = random_unit(rng);
		float const s = 2 / q.norm();
		float const x2 = q.x * s, y2 = q.y * s, z2 = q.z * s;
		float const xx = q.x * x2, xy = q.x * y2, xz = q.x * z2, yy = q.y * y2, yz = q.y * z2, zz = q.z * z2;
		float const wx = q.w * x2, wy = q.w * y2, wz = q.w * z2;
		rove::matrix<4, 4> m;
		m.rotation(q);
		BOOST_REQUIRE_EQUAL(m.ij[0][0], 1 - (yy + zz));
		BOOST_REQUIRE_EQUAL(m.ij[1][0], xy - wz);
		BOOST_REQUIRE_EQUAL(m.ij[2][1], yz - wx);
		BOOST_REQUIRE_EQUAL(m.ij[0][2], xz - wy);
		BOOST_REQUIRE_EQUAL(m.ij[2][2], 1 - (xx + yy));
		BOOST_REQUIRE_EQUAL(m.ij[0][1], xy + wz);
		BOOST_REQUIRE_EQUAL(m.ij[2][0], xz + wy);
	}
}

// As rotation_matrix takes it
BOOST_AUTO_TEST_CASE(axis_angle_takes_an_axis_of_any_length)
{
	rove::quaternion<float> const q = rove::axis_angle(rove::vec<3>(0, 0, 10), 0.5f);
	rove::quaternion<float> const unit = rove::axis_angle(rove::vec<3>(0, 0, 1), 0.5f);
	BOOST_TEST(q.z == unit.z);
	BOOST_TEST(q.w == unit.w);
	BOOST_CHECK_THROW((void)rove::axis_angle(rove::vec<3>(0, 0, 0), 0.5f), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

