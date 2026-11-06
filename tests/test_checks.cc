// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <boost/test/unit_test.hpp>
#include <rove/aabb.h>
#include <rove/collision.h>
#include <rove/heightfield.h>
#include <rove/matrix.h>
#include <rove/ray.h>
#include <rove/triangle.h>
#include <rove/vec.h>

namespace
{

float const not_a_number = std::numeric_limits<float>::quiet_NaN();
float const infinity = std::numeric_limits<float>::infinity();

/// What f throws as std::invalid_argument, or "" when it throws nothing
template<class F> std::string invalid_argument_of(F f)
{
	try {
		f();
	} catch (std::invalid_argument const &e) {
		return e.what();
	}
	return "";
}

template<int N, class T> bool same_bits(rove::vec<N, T> const &a, rove::vec<N, T> const &b)
{
	return std::memcmp(a.i, b.i, sizeof(a.i)) == 0;
}

}

BOOST_AUTO_TEST_SUITE(vec_checks)

BOOST_AUTO_TEST_CASE(check_finite_names_what_it_checked)
{
	rove::vec<3> const v(1, 2, 3);
	BOOST_TEST(&v.check_finite("a point") == &v);
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<3>(1, not_a_number, 3).check_finite("a point"); }) == "a point that is not finite");
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<2>(-infinity, 0).check_finite("a move"); }) == "a move that is not finite");
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<5>{}.check_finite("five"); }) == "");
}

BOOST_AUTO_TEST_CASE(check_nonzero_refuses_what_has_no_direction)
{
	BOOST_TEST(rove::vec<3>(3, 4, 0).check_nonzero("an axis").length() == 5);       // it chains
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<3>().check_nonzero("an axis"); }).starts_with("an axis of length 0"));
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<3>(1e-7f, 0, 0).check_nonzero("an axis"); }).starts_with("an axis of length"));
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<3>(not_a_number, 1, 0).check_nonzero("an axis"); }) == "an axis that is not finite");
	// a tolerance of one's own, and none at all
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<2>(0.5f, 0).check_nonzero("a step", 1.0f); }) != "");
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<2>(1e-20f, 0).check_nonzero("a step", 0.0f); }) == "");
	// a vector so long that its square overflows is not short
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<3>(3e38f, 3e38f, 0).check_nonzero("far"); }) == "");
	// integers and doubles
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<2, int>(0, 0).check_nonzero("a cell step"); }) != "");
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<2, int>(0, 1).check_nonzero("a cell step"); }) == "");
	BOOST_TEST(invalid_argument_of([] { (void)rove::vec<3, double>(1e-13, 0, 0).check_nonzero("d"); }) != "");
}

BOOST_AUTO_TEST_CASE(direction_keeps_its_messages)
{
	BOOST_TEST(invalid_argument_of([] { (void)rove::direction(rove::vec<3>()); }).starts_with("rove: direction of a vector of length 0"));
	BOOST_TEST(invalid_argument_of([] { (void)rove::direction(rove::vec<3>(infinity, 0, 0)); }) == "rove: direction of a vector that is not finite");
}

BOOST_AUTO_TEST_CASE(direction_keeps_its_bits)
{
	// 1.0's direction(): scaled down by the largest component where the square overflows, then times one over the length
	auto old = [](rove::vec<3> const &v) {
		rove::vec<3> t(v);
		float l = v.length();
		if (!std::isfinite(l)) {
			float const largest = std::max({std::abs(v.x), std::abs(v.y), std::abs(v.z)});
			for (int n = 0; n < 3; ++n)
				t.i[n] = v.i[n] / largest;
			l = t.length();
		}
		t *= 1 / l;
		return t;
	};
	for (rove::vec<3> const v : {rove::vec<3>(3, 4, 0), rove::vec<3>(-0.1f, 0.2f, 0.3f), rove::vec<3>(3e38f, -1e38f, 2e37f),
		rove::vec<3>(1e30f, 1e30f, 1e30f), rove::vec<3>(2e-6f, 0, 0)})
		BOOST_TEST(same_bits(rove::direction(v), old(v)));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(vec_normalize)

BOOST_AUTO_TEST_CASE(a_vector_of_no_length_or_not_a_number_throws)
{
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<2>()), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<3>()), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<4>()), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<6>{}), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<3, double>()), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<3>(not_a_number, 1, 0)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<3>(infinity, 1, 0)), std::invalid_argument);
	BOOST_TEST(invalid_argument_of([] { (void)rove::normalize(rove::vec<3>()); }).starts_with("rove: normalize a vector of length 0"));

	// the member too, leaving the vector as it was
	rove::vec<3> v;
	BOOST_CHECK_THROW(v.normalize(), std::invalid_argument);
	BOOST_TEST((v == rove::vec<3>()));
}

BOOST_AUTO_TEST_CASE(any_other_vector_gives_the_bits_it_always_did)
{
	// the old normalize: divided by length() when it was not zero
	auto old = [](auto v) { auto const l = v.length(); if (std::abs(l) > 0) v /= l; return v; };
	for (rove::vec<3> const v : {rove::vec<3>(3, 4, 0), rove::vec<3>(1e-7f, 0, 0), rove::vec<3>(-0.1f, 0.2f, 0.3f), rove::vec<3>(1e-20f, 2e-20f, 0)}) {
		BOOST_TEST(same_bits(rove::normalize(v), old(v)));
		BOOST_TEST(same_bits(rove::normalize_or_zero(v), old(v)));
		rove::vec<3> m = v;
		m.normalize();
		BOOST_TEST(same_bits(m, old(v)));
	}
	rove::vec<2, double> const d(1e-13, -2e-13);
	BOOST_TEST(same_bits(rove::normalize(d), old(d)));       // shorter than direction() takes, but a direction all the same
	BOOST_CHECK_THROW((void)rove::direction(d), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(a_vector_whose_square_overflows_comes_out_unit_length)
{
	// it came out zero: divided by a length of infinity
	rove::vec<3> const n = rove::normalize(rove::vec<3>(3e38f, 3e38f, 0));
	BOOST_TEST(std::abs(n.length() - 1) < 1e-6f);
	BOOST_TEST(std::abs(n.x - n.y) == 0);
}

BOOST_AUTO_TEST_CASE(normalize_or_zero_is_the_old_normalize)
{
	BOOST_TEST((rove::normalize_or_zero(rove::vec<3>()) == rove::vec<3>()));
	BOOST_TEST((rove::normalize_or_zero(rove::vec<5>{}) == rove::vec<5>{}));
	BOOST_TEST(std::isnan(rove::normalize_or_zero(rove::vec<2>(not_a_number, 1)).x));       // not a number, as it was
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(no_length_inside_rove)

// Where a zero vector comes of valid input, rove gives what it always gave instead of throwing
BOOST_AUTO_TEST_CASE(a_triangle_of_no_area_has_a_zero_normal)
{
	rove::triangle<3> const flat(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0), rove::vec<3>(2, 0, 0));
	BOOST_TEST((flat.normal() == rove::vec<3>()));
}

BOOST_AUTO_TEST_CASE(spheres_of_one_centre_touch_with_a_zero_normal)
{
	rove::sphere<3> const a(rove::vec<3>(1, 2, 3), 1), b(rove::vec<3>(1, 2, 3), 0.5f);
	auto const hit = rove::first_contact(a, b, rove::vec<3>(0, 0, 1));
	BOOST_REQUIRE(hit);
	BOOST_TEST(hit->penetrated);
	BOOST_TEST((hit->normal == rove::vec<3>()));
}

BOOST_AUTO_TEST_CASE(a_view_straight_up_keeps_its_zero_axes)
{
	rove::matrix<4, 4> m;
	BOOST_CHECK_NO_THROW(m.direction(rove::vec<3>(0, 1, 0)));
	BOOST_CHECK_NO_THROW(m.lookat(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0), rove::vec<3>(0, 1, 0)));
}

// Where it is the caller's own input that has no length, there is no answer
BOOST_AUTO_TEST_CASE(a_view_of_nothing_throws)
{
	rove::matrix<4, 4> m;
	BOOST_TEST(invalid_argument_of([&] { m.direction(rove::vec<3>()); }).starts_with("rove: matrix::direction along a vector of length 0"));
	BOOST_TEST(invalid_argument_of([&] { m.lookat(rove::vec<3>(1, 1, 1), rove::vec<3>(1, 1, 1), rove::vec<3>(0, 1, 0)); })
		.starts_with("rove: lookat from an eye at the target of length 0"));
	rove::matrix<3, 3> m3;
	BOOST_CHECK_THROW(m3.direction(rove::vec<3>()), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(matrix_checks)

BOOST_AUTO_TEST_CASE(check_finite)
{
	rove::matrix<4, 4> m;
	BOOST_TEST(&m.check_finite("a transform") == &m);
	m[2][1] = not_a_number;
	BOOST_TEST(invalid_argument_of([&] { (void)m.check_finite("a transform"); }) == "a transform that is not finite");
	rove::matrix<3, 3, double> d;
	d[0][0] = std::numeric_limits<double>::infinity();
	BOOST_CHECK_THROW((void)d.check_finite("d"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(check_invertible)
{
	rove::matrix<4, 4> m;
	BOOST_TEST(invalid_argument_of([&] { (void)m.check_invertible("a transform"); }) == "");
	m.scaling(1, 0, 1);
	BOOST_TEST(invalid_argument_of([&] { (void)m.check_invertible("a transform"); }).starts_with("a transform of determinant 0"));
	rove::matrix<3, 3> n;
	n[1][1] = not_a_number;
	BOOST_CHECK_THROW((void)n.check_invertible("n"), std::invalid_argument);
	// inverse() keeps its message
	BOOST_TEST(invalid_argument_of([&] { (void)rove::inverse(m); }).starts_with("rove: inverse of a matrix of determinant 0"));
}

BOOST_AUTO_TEST_CASE(check_affine)
{
	rove::matrix<4, 4> m;
	m.translation(1, 2, 3);
	BOOST_TEST(invalid_argument_of([&] { (void)m.check_affine("a pose"); }) == "");
	rove::matrix<4, 4> const p = rove::perspective_matrix(1.0f, 1.0f, 1.0f, 10.0f);
	BOOST_TEST(invalid_argument_of([&] { (void)p.check_affine("a pose"); }) == "a pose whose last row is not (0, 0, 0, 1)");
	m[3][3] = not_a_number;           // it let NaN through
	BOOST_CHECK_THROW((void)m.check_affine("a pose"), std::invalid_argument);
	BOOST_TEST(invalid_argument_of([&] { (void)rove::transform_aabb(rove::aabb<3>(), p); })
		== "rove: transform_aabb by a matrix whose last row is not (0, 0, 0, 1)");
}

BOOST_AUTO_TEST_CASE(a_heightfield_placed_by_a_singular_matrix_throws)
{
	rove::matrix<4, 4> tf;
	rove::heightfield<std::uint8_t> hf(tf, 2, 2);
	tf.scaling(1, 0, 1);
	BOOST_TEST(invalid_argument_of([&] { hf.set_local_to_world(tf); })
		.starts_with("heightfield::set_local_to_world: a matrix of determinant 0"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(shape_checks)

BOOST_AUTO_TEST_CASE(aabb_and_ray_check_finite)
{
	rove::aabb<3> const box(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1));
	BOOST_TEST(&box.check_finite("a box") == &box);
	BOOST_TEST(invalid_argument_of([] { (void)rove::aabb<2>(rove::vec<2>(0, 0), rove::vec<2>(infinity, 1)).check_finite("a box"); })
		== "a box that is not finite");
	rove::ray<3> const r(rove::vec<3>(0, 0, 0), rove::vec<3>(0, 0, 1));
	BOOST_TEST(&r.check_finite("a ray") == &r);
	BOOST_TEST(invalid_argument_of([] { (void)rove::ray<3>(rove::vec<3>(0, not_a_number, 0), rove::vec<3>(0, 0, 1)).check_finite("a ray"); })
		== "a ray that is not finite");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(manhattan)

BOOST_AUTO_TEST_CASE(every_vec_has_manhattan_length)
{
	BOOST_TEST(rove::vec<2>(3, -4).manhattan_length() == 7);
	BOOST_TEST(rove::vec<3>(1, -2, 3).manhattan_length() == 6);
	BOOST_TEST(rove::vec<4>(-1, -1, 1, 1).manhattan_length() == 4);
	BOOST_TEST((rove::vec<3, int>(1, -2, 3).manhattan_length() == 6));
}

BOOST_AUTO_TEST_SUITE_END()
