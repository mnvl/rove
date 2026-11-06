// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/test/unit_test.hpp>
#include <rove/vec.h>
#include "test_tolerance.h"
#include "test_garbage.h"

BOOST_AUTO_TEST_SUITE(vec)

// 2D Vector Construction Tests
BOOST_AUTO_TEST_CASE(vec2_default_constructor)
{
	rove::vec<2> v;
	BOOST_REQUIRE(rove::abs(v.x) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec2_component_constructor)
{
	rove::vec<2> v(3.0f, 4.0f);
	BOOST_REQUIRE(rove::abs(v.x - 3.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 4.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec2_copy_constructor)
{
	rove::vec<2> v1(5.0f, 6.0f);
	rove::vec<2> v2(v1);
	BOOST_REQUIRE(rove::abs(v2.x - 5.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.y - 6.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec2_component_access_aliases)
{
	rove::vec<2> v;
	v.x = 1.0f;
	v.y = 2.0f;

	// Test all naming conventions access the same memory
	BOOST_REQUIRE(rove::abs(v.u - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.v - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.s - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.t - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.A - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.B - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.i[0] - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.i[1] - 2.0f) < TOLERANCE);
}

// 3D Vector Construction Tests
BOOST_AUTO_TEST_CASE(vec3_default_constructor)
{
	rove::vec<3> v;
	BOOST_REQUIRE(rove::abs(v.x) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.z) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec3_component_constructor)
{
	rove::vec<3> v(1.0f, 2.0f, 3.0f);
	BOOST_REQUIRE(rove::abs(v.x - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.z - 3.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec3_component_access_aliases)
{
	rove::vec<3> v(1.0f, 2.0f, 3.0f);

	// Test all naming conventions
	BOOST_REQUIRE(rove::abs(v.A - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.B - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.C - 3.0f) < TOLERANCE);
}

// 4D Vector Construction Tests
BOOST_AUTO_TEST_CASE(vec4_component_constructor)
{
	rove::vec<4> v(1.0f, 2.0f, 3.0f, 4.0f);
	BOOST_REQUIRE(rove::abs(v.x - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.z - 3.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.w - 4.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec4_component_access_aliases)
{
	rove::vec<4> v(1.0f, 2.0f, 3.0f, 4.0f);

	BOOST_REQUIRE(rove::abs(v.A - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.B - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.C - 3.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.D - 4.0f) < TOLERANCE);
}

// Set methods tests
BOOST_AUTO_TEST_CASE(vec2_set)
{
	rove::vec<2> v;
	v.set(10.0f, 20.0f);
	BOOST_REQUIRE(rove::abs(v.x - 10.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 20.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec3_set)
{
	rove::vec<3> v;
	v.set(10.0f, 20.0f, 30.0f);
	BOOST_REQUIRE(rove::abs(v.x - 10.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 20.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.z - 30.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_set_all)
{
	rove::vec<3> v;
	v.set_all(5.0f);
	BOOST_REQUIRE(rove::abs(v.x - 5.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 5.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.z - 5.0f) < TOLERANCE);
}

// Arithmetic operations
BOOST_AUTO_TEST_CASE(vec_addition)
{
	rove::vec<3> v1(1.0f, 2.0f, 3.0f);
	rove::vec<3> v2(4.0f, 5.0f, 6.0f);
	rove::vec<3> v3 = v1 + v2;

	BOOST_REQUIRE(rove::abs(v3.x - 5.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.y - 7.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.z - 9.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_subtraction)
{
	rove::vec<3> v1(10.0f, 20.0f, 30.0f);
	rove::vec<3> v2(1.0f, 2.0f, 3.0f);
	rove::vec<3> v3 = v1 - v2;

	BOOST_REQUIRE(rove::abs(v3.x - 9.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.y - 18.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.z - 27.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_scalar_multiplication)
{
	rove::vec<3> v1(2.0f, 3.0f, 4.0f);
	rove::vec<3> v2 = v1 * 3.0f;

	BOOST_REQUIRE(rove::abs(v2.x - 6.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.y - 9.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.z - 12.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_scalar_division)
{
	rove::vec<3> v1(10.0f, 20.0f, 30.0f);
	rove::vec<3> v2 = v1 / 2.0f;

	BOOST_REQUIRE(rove::abs(v2.x - 5.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.y - 10.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.z - 15.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_negation)
{
	rove::vec<3> v1(1.0f, -2.0f, 3.0f);
	rove::vec<3> v2 = -v1;

	BOOST_REQUIRE(rove::abs(v2.x + 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.y - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.z + 3.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_is_zero_only_when_every_component_is)
{
	// Skin weights: a vertex with two influences has zero in the other slots
	// and is not an unweighted vertex.
	rove::vec<8> weights;
	weights.set_all(0.0f);
	BOOST_CHECK(weights.is_zero());
	weights.i[0] = 0.7f;
	weights.i[5] = 0.3f;
	BOOST_CHECK(!weights.is_zero());

	rove::vec<8, unsigned char> joints;
	joints.set_all(0);
	BOOST_CHECK(joints.is_zero());
	joints.i[7] = 12;
	BOOST_CHECK(!joints.is_zero());
}

BOOST_AUTO_TEST_CASE(vec_component_wise_multiplication)
{
	rove::vec<3> v1(2.0f, 3.0f, 4.0f);
	rove::vec<3> v2(5.0f, 6.0f, 7.0f);
	rove::vec<3> v3 = v1 * v2;

	BOOST_REQUIRE(rove::abs(v3.x - 10.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.y - 18.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.z - 28.0f) < TOLERANCE);
}

// Compound assignment operators
BOOST_AUTO_TEST_CASE(vec_addition_assignment)
{
	rove::vec<3> v1(1.0f, 2.0f, 3.0f);
	v1 += rove::vec<3>(4.0f, 5.0f, 6.0f);

	BOOST_REQUIRE(rove::abs(v1.x - 5.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v1.y - 7.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v1.z - 9.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_scalar_multiplication_assignment)
{
	rove::vec<3> v(2.0f, 3.0f, 4.0f);
	v *= 3.0f;

	BOOST_REQUIRE(rove::abs(v.x - 6.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 9.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.z - 12.0f) < TOLERANCE);
}

// Comparison operators
BOOST_AUTO_TEST_CASE(vec_equality)
{
	rove::vec<3> v1(1.0f, 2.0f, 3.0f);
	rove::vec<3> v2(1.0f, 2.0f, 3.0f);
	rove::vec<3> v3(1.0f, 2.0f, 4.0f);

	BOOST_REQUIRE(v1 == v2);
	BOOST_REQUIRE(!(v1 == v3));
}

BOOST_AUTO_TEST_CASE(vec_inequality)
{
	rove::vec<3> v1(1.0f, 2.0f, 3.0f);
	rove::vec<3> v2(1.0f, 2.0f, 4.0f);

	BOOST_REQUIRE(v1 != v2);
}

// Length and normalization tests

BOOST_AUTO_TEST_CASE(vec_length_sq)
{
	rove::vec<3> v(3.0f, 4.0f, 0.0f);
	rove::scalar len_sq = v.length_sq();

	BOOST_REQUIRE(rove::abs(len_sq - 25.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_length)
{
	rove::vec<3> v(3.0f, 4.0f, 0.0f);
	rove::scalar len = v.length();

	BOOST_REQUIRE(rove::abs(len - 5.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_normalize)
{
	rove::vec<3> v(3.0f, 4.0f, 0.0f);
	v.normalize();

	rove::scalar len = v.length();
	BOOST_REQUIRE(rove::abs(len - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.x - 0.6f) < 0.01f);
	BOOST_REQUIRE(rove::abs(v.y - 0.8f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(vec_normalize_function)
{
	rove::vec<3> v1(3.0f, 4.0f, 0.0f);
	rove::vec<3> v2 = rove::normalize(v1);

	BOOST_REQUIRE(rove::abs(v2.length() - 1.0f) < TOLERANCE);
	// Original should be unchanged
	BOOST_REQUIRE(rove::abs(v1.length() - 5.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_negate_method)
{
	rove::vec<3> v(1.0f, -2.0f, 3.0f);
	v.negate();

	BOOST_REQUIRE(rove::abs(v.x + 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.y - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v.z + 3.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec2_manhattan_length)
{
	rove::vec<2> v(3.0f, 4.0f);
	rove::scalar len = v.manhattan_length();

	BOOST_REQUIRE(rove::abs(len - 7.0f) < TOLERANCE);
}

// Dot product tests
BOOST_AUTO_TEST_CASE(vec_dot_product_function)
{
	rove::vec<3> v1(1.0f, 0.0f, 0.0f);
	rove::vec<3> v2(0.0f, 1.0f, 0.0f);
	rove::scalar dot = rove::dot(v1, v2);

	// Orthogonal vectors
	BOOST_REQUIRE(rove::abs(dot) < TOLERANCE);
}

// Cross product tests
BOOST_AUTO_TEST_CASE(vec_cross_product_function)
{
	rove::vec<3> v1(1.0f, 0.0f, 0.0f);
	rove::vec<3> v2(0.0f, 1.0f, 0.0f);
	rove::vec<3> v3;
	v3 = rove::cross(v1, v2);

	BOOST_REQUIRE(rove::abs(v3.x) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.y) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.z - 1.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(vec_cross_product_anti_commutative)
{
	rove::vec<3> v1(1.0f, 2.0f, 3.0f);
	rove::vec<3> v2(4.0f, 5.0f, 6.0f);
	rove::vec<3> v3 = rove::cross(v1, v2);
	rove::vec<3> v4 = rove::cross(v2, v1);

	BOOST_REQUIRE(rove::abs(v3.x + v4.x) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.y + v4.y) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v3.z + v4.z) < TOLERANCE);
}

// Collinearity tests
BOOST_AUTO_TEST_CASE(vec2_collinear)
{
	rove::vec<2> v1(1.0f, 2.0f);
	rove::vec<2> v2(2.0f, 4.0f);  // Parallel to v1
	rove::vec<2> v3(1.0f, 1.0f);  // Not parallel

	BOOST_REQUIRE(v1.is_collinear(v2));
	BOOST_REQUIRE(!v1.is_collinear(v3));
}

BOOST_AUTO_TEST_CASE(vec3_collinear)
{
	rove::vec<3> v1(1.0f, 2.0f, 3.0f);
	rove::vec<3> v2(2.0f, 4.0f, 6.0f);  // Parallel to v1
	rove::vec<3> v3(1.0f, 0.0f, 0.0f);  // Not parallel

	BOOST_REQUIRE(v1.is_collinear(v2));
	BOOST_REQUIRE(!v1.is_collinear(v3));
}

// Perpendicular tests
BOOST_AUTO_TEST_CASE(vec2_perpendicular)
{
	rove::vec<2> v(3.0f, 4.0f);
	rove::vec<2> perp = v.perpendicular();

	// Perpendicular vectors have zero dot product
	rove::scalar dot = rove::dot(v, perp);
	BOOST_REQUIRE(rove::abs(dot) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(perpendiculars)
{
	for (int i = 0; i < 1000; i++)
	{
		rove::vec<3> v{2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) - 1, 2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) - 1,
			2 * rove::scalar(rand()) / rove::scalar(RAND_MAX) - 1};

		rove::vec<3> xy = v.perpendicular_xy();
		rove::vec<3> yz = v.perpendicular_yz();
		rove::vec<3> xz = v.perpendicular_xz();

		BOOST_CHECK (rove::abs(rove::dot(xy, v)) < TOLERANCE);
		BOOST_CHECK (rove::abs(rove::dot(yz, v)) < TOLERANCE);
		BOOST_CHECK (rove::abs(rove::dot(xz, v)) < TOLERANCE);
	}
}

// MinMax tests
BOOST_AUTO_TEST_CASE(vec_minmax)
{
	rove::vec<3> v1(5.0f, 2.0f, 8.0f);
	rove::vec<3> v2(3.0f, 7.0f, 1.0f);

	rove::minmax(v1, v2);

	BOOST_REQUIRE(rove::abs(v1.x - 3.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v1.y - 2.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v1.z - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.x - 5.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.y - 7.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.z - 8.0f) < TOLERANCE);
}

// Lexicographical comparison
BOOST_AUTO_TEST_CASE(vec_lexicographical_less)
{
	rove::vec<3> v1(1.0f, 2.0f, 3.0f);
	rove::vec<3> v2(1.0f, 2.0f, 4.0f);
	rove::vec<3> v3(1.0f, 3.0f, 2.0f);

	BOOST_REQUIRE(rove::lexicographical_less(v1, v2));
	BOOST_REQUIRE(rove::lexicographical_less(v1, v3));
	BOOST_REQUIRE(!rove::lexicographical_less(v2, v1));
}

// Scalar on left-hand side
BOOST_AUTO_TEST_CASE(vec_scalar_lhs_multiplication)
{
	rove::vec<3> v1(2.0f, 3.0f, 4.0f);
	rove::vec<3> v2 = 3.0f * v1;

	BOOST_REQUIRE(rove::abs(v2.x - 6.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.y - 9.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(v2.z - 12.0f) < TOLERANCE);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(vec_free_functions)

BOOST_AUTO_TEST_CASE(cross_is_right_handed)
{
	rove::vec<3, double> const x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
	BOOST_TEST((rove::cross(x, y) == z));
	BOOST_TEST((rove::cross(y, z) == x));
	BOOST_TEST((rove::cross(z, x) == y));
	BOOST_TEST((rove::cross(y, x) == -z));
}

BOOST_AUTO_TEST_CASE(cross_is_anti_commutative_and_perpendicular)
{
	rove::vec<3, double> const a(1.5, -2, 0.25), b(-3, 0.5, 4);
	rove::vec<3, double> const c = rove::cross(a, b);
	BOOST_TEST((rove::cross(b, a) == -c));
	BOOST_TEST(rove::dot(c, a) == 0.0, boost::test_tools::tolerance(1e-12));
	BOOST_TEST(rove::dot(c, b) == 0.0, boost::test_tools::tolerance(1e-12));
	BOOST_TEST((rove::cross(a, a) == rove::vec<3, double>()));
}

BOOST_AUTO_TEST_CASE(dot_in_every_arity)
{
	rove::vec<2, double> const a2(3, -4), b2(0.5, 2);
	rove::vec<3, float> const a3(1, 2, 3), b3(-4, 5, -6);
	rove::vec<4, double> const a4(1, 2, 3, 4), b4(4, -3, 2, -1);
	BOOST_TEST(rove::dot(a2, b2) == -6.5);
	BOOST_TEST(rove::dot(a3, b3) == -12.0f);
	BOOST_TEST(rove::dot(a4, b4) == 0.0);
	BOOST_TEST(rove::dot(a3, a3) == a3.length_sq());
}

BOOST_AUTO_TEST_CASE(length_agrees_with_the_member)
{
	BOOST_TEST(rove::length(rove::vec<2, double>(3, 4)) == 5.0);
	BOOST_TEST(rove::length(rove::vec<3, double>(2, -3, 6)) == 7.0);
	BOOST_TEST(rove::length(rove::vec<3, float>(0, 0, 0)) == 0.0f);
	rove::vec<3, double> const v(0.1, -7.3, 2.9);
	BOOST_TEST(rove::length(v) == v.length());
}

BOOST_AUTO_TEST_CASE(length_sq_agrees_with_the_member)
{
	BOOST_TEST(rove::length_sq(rove::vec<2, double>(3, 4)) == 25.0);
	BOOST_TEST(rove::length_sq(rove::vec<3, double>(2, -3, 6)) == 49.0);
	BOOST_TEST(rove::length_sq(rove::vec<4, float>(1, -1, 1, -1)) == 4.0f);
	BOOST_TEST(rove::length_sq(rove::vec<3, float>(0, 0, 0)) == 0.0f);
	rove::vec<3, double> const v(0.1, -7.3, 2.9);
	BOOST_TEST(rove::length_sq(v) == v.length_sq());
}

BOOST_AUTO_TEST_SUITE_END()

// What a vector's components are called is its storage, vec's third
// parameter: positions by default, a colour order for the colour vectors.
// The order is part of the type, so that bytes read as one order cannot be
// taken for another.
BOOST_AUTO_TEST_SUITE(vec_storage)

namespace
{

template<class V> concept has_r = requires(V v) { v.r; };
template<class V> concept has_a = requires(V v) { v.a; };
template<class V> concept has_x = requires(V v) { v.x; };
template<class V> concept has_w = requires(V v) { v.w; };
template<class V> concept has_A = requires(V v) { v.A; };
template<class L, class R> concept addable = requires(L l, R r) { l + r; };
template<class L, class R> concept assignable = requires(L l, R r) { l = r; };

// What the bytes of a texel are in memory, whatever they are called.
template<class V, class T, size_t N> V from_memory(T const (&components)[N])
{
	static_assert(sizeof(V) == sizeof(components));
	V v;
	std::memcpy(&v, components, sizeof(components));
	return v;
}

}

BOOST_AUTO_TEST_CASE(the_default_is_what_vec_has_always_been)
{
	static_assert(std::is_same_v<rove::vec<2>, rove::vec<2, rove::scalar, rove::storage::xy<rove::scalar>>>);
	static_assert(std::is_same_v<rove::vec<3, double>, rove::vec<3, double, rove::storage::xyz<double>>>);
	static_assert(std::is_same_v<rove::vec<4, int>, rove::vec<4, int, rove::storage::xyzw<int>>>);
	static_assert(std::is_same_v<rove::vec<8, unsigned>, rove::vec<8, unsigned, rove::storage::array<8, unsigned>>>);
	static_assert(has_x<rove::vec<3>> && has_A<rove::vec<3>> && has_w<rove::vec<4>> && has_A<rove::vec<4>>);
}

BOOST_AUTO_TEST_CASE(a_position_has_no_colours)
{
	static_assert(!has_r<rove::vec<3>>);
	static_assert(!has_r<rove::vec<4>> && !has_a<rove::vec<4>>);
	static_assert(!has_r<rove::vec<2>>);
}

BOOST_AUTO_TEST_CASE(a_colour_is_named_in_the_order_of_its_memory)
{
	uint8_t const three[] = {10, 20, 30}, four[] = {10, 20, 30, 40};

	auto const rgb = from_memory<rove::rgb<uint8_t>>(three);
	BOOST_TEST((rgb.r == 10 && rgb.g == 20 && rgb.b == 30));
	auto const bgr = from_memory<rove::bgr<uint8_t>>(three);
	BOOST_TEST((bgr.b == 10 && bgr.g == 20 && bgr.r == 30));
	auto const rgba = from_memory<rove::rgba<uint8_t>>(four);
	BOOST_TEST((rgba.r == 10 && rgba.g == 20 && rgba.b == 30 && rgba.a == 40));
	auto const bgra = from_memory<rove::bgra<uint8_t>>(four);
	BOOST_TEST((bgra.b == 10 && bgra.g == 20 && bgra.r == 30 && bgra.a == 40));

	float const floats[] = {0.25f, 0.5f, 0.75f, 1.0f};
	auto const f = from_memory<rove::bgra<float>>(floats);
	BOOST_TEST((f.b == 0.25f && f.g == 0.5f && f.r == 0.75f && f.a == 1.0f));
}

// The constructor and i[] go by position in memory, as for any vec: it is
// only the names that differ.
BOOST_AUTO_TEST_CASE(a_colour_is_built_and_indexed_by_position)
{
	rove::bgr<uint8_t> const bgr(1, 2, 3);
	BOOST_TEST((bgr.b == 1 && bgr.g == 2 && bgr.r == 3));
	BOOST_TEST((bgr.i[0] == 1 && bgr.i[1] == 2 && bgr.i[2] == 3));
	BOOST_TEST((bgr.x == 1 && bgr.y == 2 && bgr.z == 3));

	rove::rgba<float> const rgba(1, 2, 3, 4);
	BOOST_TEST((rgba.r == 1 && rgba.g == 2 && rgba.b == 3 && rgba.a == 4));
	BOOST_TEST((rgba.x == 1 && rgba.y == 2 && rgba.z == 3 && rgba.w == 4));
	rove::bgra<float> const bgra(1, 2, 3, 4);
	BOOST_TEST((bgra.b == 1 && bgra.g == 2 && bgra.r == 3 && bgra.a == 4));
	rove::rgb<double> const rgb(1, 2, 3);
	BOOST_TEST((rgb.r == 1 && rgb.g == 2 && rgb.b == 3));

	static_assert(!has_A<rove::rgb<float>> && !has_A<rove::bgra<float>>);
	static_assert(!has_a<rove::rgb<float>> && !has_a<rove::bgr<float>>);
}

BOOST_AUTO_TEST_CASE(a_colour_is_zero_by_default)
{
	rove::bgra<uint8_t> const bgra;
	BOOST_TEST((bgra.b == 0 && bgra.g == 0 && bgra.r == 0 && bgra.a == 0));
	rove::rgb<float> const rgb;
	BOOST_TEST((rgb.r == 0 && rgb.g == 0 && rgb.b == 0));
}

// An array of them is the bytes of an image or a texture: nothing between
// the components or after them, and memcpy is a copy.
BOOST_AUTO_TEST_CASE(a_vec_is_its_components_and_nothing_else)
{
	static_assert(sizeof(rove::rgb<uint8_t>) == 3 && sizeof(rove::bgr<uint8_t>) == 3);
	static_assert(sizeof(rove::rgba<uint8_t>) == 4 && sizeof(rove::bgra<uint8_t>) == 4);
	static_assert(sizeof(rove::rgba<uint16_t>) == 8 && sizeof(rove::rgba<float>) == 16);
	static_assert(sizeof(rove::vec<2, float>) == 8 && sizeof(rove::vec<3, double>) == 24 && sizeof(rove::vec<4, float>) == 16);
	static_assert(sizeof(rove::vec<8, unsigned>) == 8 * sizeof(unsigned));
	static_assert(alignof(rove::bgra<float>) == alignof(float) && alignof(rove::vec<3, double>) == alignof(double));   // aligned as the scalar: no packing

	static_assert(std::is_trivially_copyable_v<rove::vec<2, float>>);
	static_assert(std::is_trivially_copyable_v<rove::vec<3, double>>);
	static_assert(std::is_trivially_copyable_v<rove::vec<4, float>>);
	static_assert(std::is_trivially_copyable_v<rove::vec<8, unsigned>>);
	static_assert(std::is_trivially_copyable_v<rove::rgb<uint8_t>>);
	static_assert(std::is_trivially_copyable_v<rove::bgra<uint8_t>>);
	static_assert(std::is_standard_layout_v<rove::vec<3, double>> && std::is_standard_layout_v<rove::bgra<uint8_t>>);

	rove::bgr<uint8_t> const pixels[] = {{1, 2, 3}, {4, 5, 6}};
	uint8_t bytes[6];
	static_assert(sizeof(pixels) == sizeof(bytes));
	std::memcpy(bytes, pixels, sizeof(bytes));
	uint8_t const expected[] = {1, 2, 3, 4, 5, 6};
	BOOST_TEST(bytes == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(a_colour_has_vecs_arithmetic)
{
	rove::bgr<float> const a(1, 2, 3), b(3, 6, 11);
	rove::bgr<float> const mix = a + (b - a) * 0.5f;
	BOOST_TEST((mix.b == 2 && mix.g == 4 && mix.r == 7));
	BOOST_TEST((a * b == rove::bgr<float>(3, 12, 33)));
	BOOST_TEST((b / 2.0f == rove::bgr<float>(1.5f, 3, 5.5f)));
	BOOST_TEST((-a == rove::bgr<float>(-1, -2, -3)));
	BOOST_TEST(a.length_sq() == 14.0f);
	BOOST_TEST((a != b));

	rove::bgra<float> c(1, 2, 3, 4);
	c += rove::bgra<float>(1, 1, 1, 1);
	c *= 2;
	BOOST_TEST((c == rove::bgra<float>(4, 6, 8, 10)));
	BOOST_TEST((c - c == rove::bgra<float>()));

	static_assert(std::is_same_v<decltype(a + b), rove::bgr<float>>);
	static_assert(std::is_same_v<decltype(c * 2.0f), rove::bgra<float>>);
}

BOOST_AUTO_TEST_CASE(orders_do_not_mix)
{
	static_assert(addable<rove::bgr<float>, rove::bgr<float>>);
	static_assert(!addable<rove::bgr<float>, rove::rgb<float>>);
	static_assert(!addable<rove::bgra<float>, rove::rgba<float>>);
	static_assert(!addable<rove::rgb<float>, rove::vec<3, float>>);
	static_assert(!addable<rove::vec<4, float>, rove::rgba<float>>);

	static_assert(!std::is_constructible_v<rove::rgb<float>, rove::bgr<float>>);
	static_assert(!std::is_constructible_v<rove::bgra<uint8_t>, rove::rgba<uint8_t>>);
	static_assert(!std::is_constructible_v<rove::rgb<float>, rove::vec<3, float>>);
	static_assert(!std::is_constructible_v<rove::vec<4, float>, rove::rgba<float>>);
	static_assert(!assignable<rove::rgb<float> &, rove::bgr<float>>);
	static_assert(!assignable<rove::vec<3, float> &, rove::rgb<float>>);
}

// Eight bits wrap (200 + 100 is 44), so what blends them widens first.
BOOST_AUTO_TEST_CASE(a_vec_widens_to_another_scalar_when_asked)
{
	rove::bgr<uint8_t> const a(200, 10, 0), b(100, 250, 255);
	rove::bgr<double> const wide(a);
	BOOST_TEST((wide == rove::bgr<double>(200, 10, 0)));
	rove::bgr<double> const mix = wide + (rove::bgr<double>(b) - wide) * 0.5;
	BOOST_TEST((mix == rove::bgr<double>(150, 130, 127.5)));

	BOOST_TEST((rove::bgra<float>(rove::bgra<uint8_t>(1, 2, 3, 4)) == rove::bgra<float>(1, 2, 3, 4)));
	BOOST_TEST((rove::vec<3, double>(rove::vec<3, float>(1.5f, -2, 3)) == rove::vec<3, double>(1.5, -2, 3)));
	BOOST_TEST((rove::vec<4, float>(rove::vec<4, int>(1, -2, 3, 4)) == rove::vec<4, float>(1, -2, 3, 4)));
	// back down: as a cast would, truncating
	BOOST_TEST((rove::rgb<uint8_t>(rove::rgb<double>(1.9, 254.2, 0)) == rove::rgb<uint8_t>(1, 254, 0)));

	// asked: a float where a double was meant is not quietly made one
	static_assert(std::is_constructible_v<rove::vec<3, double>, rove::vec<3, float>>);
	static_assert(!std::is_convertible_v<rove::vec<3, float>, rove::vec<3, double>>);
	static_assert(!std::is_convertible_v<rove::vec<4, float>, rove::vec<4, double>>);
	static_assert(!std::is_convertible_v<rove::bgr<uint8_t>, rove::bgr<double>>);
	static_assert(std::is_constructible_v<rove::vec<2, int>, rove::vec<2, float>>);
	static_assert(!std::is_convertible_v<rove::vec<2, float>, rove::vec<2, int>>);
	static_assert(!std::is_convertible_v<rove::vec<2, float>, rove::vec<2, double>>);

	// the scalar, not the order
	static_assert(!std::is_constructible_v<rove::rgb<double>, rove::bgr<uint8_t>>);
	static_assert(!std::is_constructible_v<rove::vec<3, double>, rove::rgb<float>>);
	static_assert(!std::is_constructible_v<rove::rgba<float>, rove::vec<4, uint8_t>>);
}

BOOST_AUTO_TEST_SUITE_END()

// v[n] is the component at position n, whatever the storage calls it; a
// position past the last one is a bug in the caller and throws rather than
// reading the bytes after the vector.
BOOST_AUTO_TEST_SUITE(vec_index)

BOOST_AUTO_TEST_CASE(an_index_is_a_position_in_memory)
{
	rove::vec<2> const v2(1, 2);
	BOOST_TEST((v2[0] == 1 && v2[1] == 2));
	rove::vec<3> const v3(1, 2, 3);
	BOOST_TEST((v3[0] == 1 && v3[1] == 2 && v3[2] == 3));
	rove::vec<4, int> const v4(1, 2, 3, 4);
	BOOST_TEST((v4[0] == 1 && v4[1] == 2 && v4[2] == 3 && v4[3] == 4));

	rove::vec<8, unsigned> v8;
	for (unsigned n = 0; n < 8; ++n)
		v8.i[n] = 10 * n + 1;
	for (unsigned n = 0; n < 8; ++n)
		BOOST_TEST(v8[n] == 10 * n + 1);

	// a colour by position, not by name: bgr's first is blue
	rove::bgr<uint8_t> const bgr(1, 2, 3);
	BOOST_TEST((bgr[0] == bgr.b && bgr[2] == bgr.r));
	rove::rgba<float> const rgba(1, 2, 3, 4);
	BOOST_TEST((rgba[0] == rgba.r && rgba[3] == rgba.a));
}

BOOST_AUTO_TEST_CASE(an_index_writes_the_component)
{
	rove::vec<2> v2;
	v2[1] = 5;
	BOOST_TEST((v2.x == 0 && v2.y == 5));
	rove::vec<3, double> v3;
	v3[0] = 1;
	v3[2] = -3;
	BOOST_TEST((v3 == rove::vec<3, double>(1, 0, -3)));
	rove::vec<4> v4;
	v4[3] += 2;
	v4[3] *= 4;
	BOOST_TEST(v4.w == 8);
	rove::vec<6> v6;
	v6[5] = 7;
	BOOST_TEST(v6.i[5] == 7);

	static_assert(std::is_same_v<decltype(v4[0]), float &>);
	rove::vec<4> const c4;
	static_assert(std::is_same_v<decltype(c4[0]), float const &>);
	rove::vec<6> const c6{};
	static_assert(std::is_same_v<decltype(c6[0]), float const &>);
}

BOOST_AUTO_TEST_CASE(an_index_past_the_last_component_throws)
{
	rove::vec<2> v2;
	rove::vec<3> const v3;
	rove::vec<4> v4;
	rove::vec<5> const v5{};
	rove::rgb<uint8_t> rgb;
	BOOST_CHECK_THROW(v2[2], std::out_of_range);
	BOOST_CHECK_THROW(v3[3], std::out_of_range);
	BOOST_CHECK_THROW(v4[4], std::out_of_range);
	BOOST_CHECK_THROW(v5[5], std::out_of_range);
	BOOST_CHECK_THROW(rgb[3], std::out_of_range);
	// a negative index is a huge one
	BOOST_CHECK_THROW(v2[size_t(-1)], std::out_of_range);
	BOOST_CHECK_THROW(v4[-1], std::out_of_range);
	BOOST_CHECK_NO_THROW(v4[3]);
}

BOOST_AUTO_TEST_CASE(any_arity_compares_equal_component_by_component)
{
	rove::vec<5> a{}, b{};
	BOOST_REQUIRE(a == b);
	b.i[4] = 1;
	BOOST_REQUIRE(a != b);
	BOOST_REQUIRE(!(a == b));
	a.i[4] = 1;
	BOOST_REQUIRE(a == b);
	a.i[0] = std::numeric_limits<float>::quiet_NaN();
	b.i[0] = a.i[0];
	BOOST_REQUIRE(a != b);        // NaN equals nothing, as a float
}

BOOST_AUTO_TEST_SUITE_END()

// direction(v) is v made unit length; a vector with no length has no
// direction, and asking for one is a bug in the caller, so it throws where
// normalize(v) would hand the vector back as it is.
BOOST_AUTO_TEST_SUITE(vec_direction)

BOOST_AUTO_TEST_CASE(a_direction_is_unit_length_and_points_the_same_way)
{
	rove::vec<2> const d2 = rove::direction(rove::vec<2>(3, -4));
	BOOST_TEST(rove::abs(d2.x - 0.6f) < TOLERANCE);
	BOOST_TEST(rove::abs(d2.y + 0.8f) < TOLERANCE);

	rove::vec<3> const v3(-2, 10, 11);
	rove::vec<3> const d3 = rove::direction(v3);
	BOOST_TEST(rove::abs(d3.length() - 1) < TOLERANCE);
	BOOST_TEST((d3 - v3 * (1 / 15.0f)).length() < TOLERANCE);

	rove::vec<4, double> const d4 = rove::direction(rove::vec<4, double>(0, 0, 0, -1e-3));
	BOOST_TEST((d4 - rove::vec<4, double>(0, 0, 0, -1)).length() < 1e-12);

	rove::vec<5> v5{};
	v5.i[2] = 7;
	BOOST_TEST(rove::direction(v5).i[2] == 1);

	// a long vector is not a reason to fail
	rove::vec<3> const far = rove::direction(rove::vec<3>(0, 3e15f, 4e15f));
	BOOST_TEST(rove::abs(far.y - 0.6f) < TOLERANCE);

	// nor one whose square overflows: 3e19^2 + 4e19^2 is past float's 3.4e38, and length() is infinite
	BOOST_REQUIRE(!std::isfinite(rove::vec<3>(0, 3e19f, 4e19f).length()));
	rove::vec<3> const farther = rove::direction(rove::vec<3>(0, 3e19f, 4e19f));
	BOOST_TEST(rove::abs(farther.y - 0.6f) < TOLERANCE);
	BOOST_TEST(rove::abs(farther.z - 0.8f) < TOLERANCE);
	float const most = std::numeric_limits<float>::max();
	rove::vec<3> const largest = rove::direction(rove::vec<3>(-most, 0, most));
	BOOST_TEST(rove::abs(largest.x + std::sqrt(0.5f)) < TOLERANCE);
	BOOST_TEST(largest.y == 0);
	BOOST_TEST(rove::abs(largest.z - std::sqrt(0.5f)) < TOLERANCE);
	rove::vec<4, double> const far4 = rove::direction(rove::vec<4, double>(1e300, 0, 0, -1e300));
	BOOST_TEST((far4 - rove::vec<4, double>(std::sqrt(0.5), 0, 0, -std::sqrt(0.5))).length() < 1e-12);
	// a vector whose length is a number keeps its bits: one division by its length, for all the components (README, Bits)
	std::mt19937 rng(11);
	std::uniform_real_distribution<float> coord(-1e6f, 1e6f);
	for (int k = 0; k < 1000; ++k) {
		rove::vec<3> const v(coord(rng), coord(rng), coord(rng));
		rove::vec<3> const d = rove::direction(v);
		rove::vec<3> want(v);
		want *= 1 / v.length();
		BOOST_REQUIRE(std::memcmp(&d, &want, sizeof d) == 0);
	}
	// a vector along an axis is that axis exactly
	BOOST_TEST((rove::direction(rove::vec<3>(0, 0, -5)) == rove::vec<3>(0, 0, -1)));
	BOOST_TEST((rove::direction(rove::vec<3>(0, 3e30f, 0)) == rove::vec<3>(0, 1, 0)));

	static_assert(std::is_same_v<decltype(rove::direction(v3)), rove::vec<3>>);
	static_assert(std::is_same_v<decltype(rove::direction(rove::vec<3, double>())), rove::vec<3, double>>);
}

BOOST_AUTO_TEST_CASE(a_vector_with_no_length_has_no_direction)
{
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<2>()), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3>()), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<4>()), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<6>{}), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3, double>()), std::invalid_argument);
	// too short to tell from rounding noise
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3>(1e-7f, 0, 0)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<2>(-5e-7f, 5e-7f)), std::invalid_argument);

	// normalize throws only for no length at all (test_checks.cc); normalize_or_zero hands it back
	BOOST_TEST((rove::normalize_or_zero(rove::vec<3>()) == rove::vec<3>()));
}

BOOST_AUTO_TEST_CASE(a_vector_that_is_not_a_number_has_no_direction)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3>(nan, 0, 0)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<2>(1, nan)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3>(inf, 0, 0)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<4>(1, 2, -inf, 0)), std::invalid_argument);
	// NaN after a larger component, where a running max would pass it over
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3>(5, nan, 0)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3>(0, 0, nan)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(rove::vec<3>(nan, nan, nan)), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

// The ground plane of a world that is Y up: a 2D vector whose components are
// x and z, so that a position on the ground reads as the world's.  It is its
// own type: a ground vector is not taken for an xy one, nor widened to a 3D
// vector whose y would be its z.
BOOST_AUTO_TEST_SUITE(vec_storage_xz)

namespace
{

template<class V> concept has_y = requires(V v) { v.y; };
template<class V> concept has_z = requires(V v) { v.z; };
template<class L, class R> concept addable = requires(L l, R r) { l + r; };
template<class L, class R> concept dottable = requires(L l, R r) { rove::dot(l, r); };

using ground = rove::xz<float>;

}

BOOST_AUTO_TEST_CASE(a_ground_vector_is_x_and_z)
{
	static_assert(std::is_same_v<ground, rove::vec<2, float, rove::storage::xz<float>>>);
	static_assert(has_z<ground> && !has_y<ground>);

	ground const g(1, 2);
	BOOST_TEST((g.x == 1 && g.z == 2));
	BOOST_TEST((g.i[0] == 1 && g.i[1] == 2));
	BOOST_TEST((g[0] == 1 && g[1] == 2));
	BOOST_CHECK_THROW(g[2], std::out_of_range);

	ground const zero;
	BOOST_TEST((zero.x == 0 && zero.z == 0));
	ground const braced{3, -4};
	BOOST_TEST((braced.x == 3 && braced.z == -4));

	static_assert(sizeof(ground) == 8 && sizeof(rove::xz<double>) == 16);
	static_assert(std::is_trivially_copyable_v<ground> && std::is_standard_layout_v<ground>);
}

BOOST_AUTO_TEST_CASE(a_ground_vector_has_vecs_arithmetic)
{
	ground const a(1, 2), b(4, -2);
	BOOST_TEST((a + b == ground(5, 0)));
	BOOST_TEST((a - b == ground(-3, 4)));
	BOOST_TEST((a * 2 == ground(2, 4)));
	BOOST_TEST((2.0f * a == ground(2, 4)));
	BOOST_TEST((b / 2 == ground(2, -1)));
	BOOST_TEST((-a == ground(-1, -2)));
	BOOST_TEST((a * b == ground(4, -4)));
	BOOST_TEST((a != b));

	ground c = a;
	c += b;
	c *= 0.5f;
	BOOST_TEST((c == ground(2.5f, 0)));

	BOOST_TEST(rove::dot(a, b) == 0);
	BOOST_TEST(rove::dot(a, ground(3, 1)) == 5);
	BOOST_TEST(ground(3, 4).length() == 5);
	BOOST_TEST(rove::length(ground(-3, 4)) == 5);
	BOOST_TEST(ground(3, 4).length_sq() == 25);

	ground const d = rove::direction(ground(0, -2));
	BOOST_TEST((d == ground(0, -1)));
	BOOST_CHECK_THROW((void)rove::direction(ground()), std::invalid_argument);
	BOOST_TEST((rove::normalize(ground(5, 0)) == ground(1, 0)));

	static_assert(std::is_same_v<decltype(a + b), ground>);
	static_assert(std::is_same_v<decltype(2.0f * a), ground>);
	static_assert(std::is_same_v<decltype(rove::direction(a)), ground>);
	static_assert(std::is_same_v<decltype(rove::normalize(a)), ground>);
}

// perpendicular() turns a quarter clockwise, (x, y) to (y, -x): from the first
// axis away from the second, so that v and its perpendicular make a
// clockwise pair, v x p < 0.
BOOST_AUTO_TEST_CASE(perpendicular_turns_a_quarter_clockwise)
{
	BOOST_TEST((rove::vec<2>(3, 4).perpendicular() == rove::vec<2>(4, -3)));
	BOOST_TEST((rove::vec<2>(0, 1).perpendicular() == rove::vec<2>(1, 0)));
	BOOST_TEST((rove::vec<2>(-1, 0).perpendicular() == rove::vec<2>(0, 1)));
	for (float a = 0; a < 6.2f; a += 0.5f) {
		rove::vec<2> const v(std::cos(a), std::sin(a));
		rove::vec<2> const p = v.perpendicular();
		BOOST_TEST(rove::abs(rove::dot(v, p)) < TOLERANCE);
		BOOST_TEST(v.x * p.y - v.y * p.x < 0);
		float m = 0, angle = 0, pa = 0;
		v.to_polar(m, angle);
		p.to_polar(m, pa);
		BOOST_TEST(rove::abs(rove::wrap(angle - pa, -rove::PI, rove::PI) - rove::PI / 2) < 1e-5f);
	}
}

// The 2D members go by position: on the ground, z is the second.
BOOST_AUTO_TEST_CASE(a_ground_vectors_second_component_is_z)
{
	ground const p = ground(1, 0).perpendicular();
	BOOST_TEST((p == ground(0, -1)));
	BOOST_TEST((rove::vec<2>(1, 0).perpendicular() == rove::vec<2>(0, -1)));

	BOOST_TEST(ground(1, 2).is_collinear(ground(-2, -4)));
	BOOST_TEST(!ground(1, 2).is_collinear(ground(2, 1)));

	float modulus = 0, angle = 0;
	ground(0, 2).to_polar(modulus, angle);
	BOOST_TEST(rove::abs(modulus - 2) < TOLERANCE);
	BOOST_TEST(rove::abs(angle - rove::PI / 2) < TOLERANCE);
	rove::vec<2>(-3, 0).to_polar(modulus, angle);
	BOOST_TEST(rove::abs(modulus - 3) < TOLERANCE);
	BOOST_TEST(rove::abs(angle - rove::PI) < TOLERANCE);

	BOOST_TEST(ground(3, -1).manhattan_length() == 4);
	ground n(3, -1);
	n.negate();
	BOOST_TEST((n == ground(-3, 1)));
	n.set(7, 8);
	BOOST_TEST((n.x == 7 && n.z == 8));
	n.set_all(2);
	BOOST_TEST((n == ground(2, 2)));

	rove::xz<double> const wide(ground(1.5f, -2));
	BOOST_TEST((wide == rove::xz<double>(1.5, -2)));
}

BOOST_AUTO_TEST_CASE(the_ground_does_not_mix_with_xy)
{
	static_assert(!addable<ground, rove::vec<2>>);
	static_assert(!addable<rove::vec<2>, ground>);
	static_assert(!dottable<ground, rove::vec<2>>);
	static_assert(!std::is_constructible_v<rove::vec<2>, ground>);
	static_assert(!std::is_constructible_v<ground, rove::vec<2>>);
	// vec<3>(vec<2>, z) would put the ground's z into y
	static_assert(!std::is_constructible_v<rove::vec<3>, ground>);
	static_assert(!std::is_constructible_v<rove::vec<3>, ground, float>);
	static_assert(std::is_constructible_v<rove::vec<3>, rove::vec<2>, float>);
}

BOOST_AUTO_TEST_SUITE_END()

// The operators of vec are found by argument-dependent lookup, so that a
// namespace with operators of its own still has vec's.  Declared at global
// scope, they were hidden by any operator of the same symbol in the
// namespace of the caller.
namespace elsewhere
{

struct thing {};
inline thing operator *(thing, thing) { return {}; }
inline std::ostream &operator <<(std::ostream &os, thing) { return os; }

static rove::vec<3> scaled(float k, rove::vec<3> const &v) { return k * v; }
static std::string printed(rove::vec<3> const &v) { std::ostringstream os; os << v; return os.str(); }
static std::string printed(rove::vec<2> const &v) { std::ostringstream os; os << v; return os.str(); }
static std::string printed(rove::vec<4> const &v) { std::ostringstream os; os << v; return os.str(); }

}

BOOST_AUTO_TEST_SUITE(vec_lookup)

BOOST_AUTO_TEST_CASE(vecs_operators_are_found_from_another_namespace)
{
	rove::vec<3> const a(1, 0, 0);
	BOOST_TEST((elsewhere::scaled(2, a) == rove::vec<3>(2, 0, 0)));
	BOOST_TEST(elsewhere::printed(rove::vec<3>(1, 2, 3)) == "1 2 3");
	BOOST_TEST(elsewhere::printed(rove::vec<2>(1, 2)) == "1 2");
	BOOST_TEST(elsewhere::printed(rove::vec<4>(1, 2, 3, 4)) == "1 2 3 4");
}

BOOST_AUTO_TEST_SUITE_END()

// >> reads what << writes: the components by position, separated by spaces.
BOOST_AUTO_TEST_SUITE(vec_input)

BOOST_AUTO_TEST_CASE(a_vec_reads_back_what_it_wrote)
{
	rove::vec<2> v2;
	rove::vec<3> v3;
	rove::vec<4> v4;
	std::istringstream in("1.5 -2 3 4 5 6 7 8 9");
	in >> v2 >> v3 >> v4;
	BOOST_TEST(bool(in));
	BOOST_TEST((v2 == rove::vec<2>(1.5f, -2)));
	BOOST_TEST((v3 == rove::vec<3>(3, 4, 5)));
	BOOST_TEST((v4 == rove::vec<4>(6, 7, 8, 9)));

	rove::vec<3, double> const original(0.25, -1e10, 7);
	std::stringstream round;
	round << original;
	rove::vec<3, double> read;
	round >> read;
	BOOST_TEST((read == original));
}

BOOST_AUTO_TEST_CASE(a_vec_that_is_not_there_fails_the_stream)
{
	rove::vec<3> v(9, 9, 9);
	std::istringstream in("1 2");
	in >> v;
	BOOST_TEST(!in);
}

BOOST_AUTO_TEST_SUITE_END()

namespace elsewhere_input
{

struct thing {};
inline std::istream &operator >>(std::istream &is, thing &) { return is; }

static rove::vec<3> read3(std::string const &text) { std::istringstream in(text); rove::vec<3> v; in >> v; return v; }

}

BOOST_AUTO_TEST_SUITE(vec_input_lookup)

BOOST_AUTO_TEST_CASE(vecs_input_is_found_from_another_namespace)
{
	BOOST_TEST((elsewhere_input::read3("1 2 3") == rove::vec<3>(1, 2, 3)));
}

BOOST_AUTO_TEST_SUITE_END()

// lerp(a, b, t) is a + (b - a) * t: a at 0, b at 1, and on along the line
// outside them; distance(a, b) is |a - b|.
BOOST_AUTO_TEST_SUITE(vec_lerp_distance)

BOOST_AUTO_TEST_CASE(lerp_goes_from_a_at_0_to_b_at_1_and_on_beyond)
{
	BOOST_TEST(rove::lerp(2.0f, 6.0f, 0.0f) == 2.0f);
	BOOST_TEST(rove::lerp(2.0f, 6.0f, 1.0f) == 6.0f);
	BOOST_TEST(rove::lerp(2.0f, 6.0f, 0.25f) == 3.0f);
	BOOST_TEST(rove::lerp(2.0f, 6.0f, -0.5f) == 0.0f);
	BOOST_TEST(rove::lerp(2.0f, 6.0f, 2.0f) == 10.0f);
	BOOST_TEST(rove::lerp(-1.0, 1.0, 0.5) == 0.0);
	BOOST_TEST(rove::lerp(5.0f, 5.0f, 123.0f) == 5.0f);

	rove::vec<3> const a(0, 2, -4), b(4, 2, 0);
	BOOST_TEST((rove::lerp(a, b, 0.0f) == a));
	BOOST_TEST((rove::lerp(a, b, 1.0f) == b));
	BOOST_TEST((rove::lerp(a, b, 0.5f) == rove::vec<3>(2, 2, -2)));
	BOOST_TEST((rove::lerp(a, b, -1.0f) == rove::vec<3>(-4, 2, -8)));
	BOOST_TEST((rove::lerp(rove::vec<2>(0, 0), rove::vec<2>(2, 4), 0.25f) == rove::vec<2>(0.5f, 1)));
	BOOST_TEST((rove::lerp(rove::vec<4>(0, 0, 0, 1), rove::vec<4>(4, 8, 0, 1), 0.75f) == rove::vec<4>(3, 6, 0, 1)));

	rove::xz<float> const g = rove::lerp(rove::xz<float>(1, 3), rove::xz<float>(3, -1), 0.5f);
	BOOST_TEST((g.x == 2 && g.z == 1));

	rove::vec<5> p{}, q{};
	q.i[4] = 8;
	BOOST_TEST(rove::lerp(p, q, 0.25f).i[4] == 2);

	static_assert(std::is_same_v<decltype(rove::lerp(a, b, 0.5f)), rove::vec<3>>);
	static_assert(std::is_same_v<decltype(rove::lerp(g, g, 0.5f)), rove::xz<float>>);
	static_assert(std::is_same_v<decltype(rove::lerp(1.0, 2.0, 0.5)), double>);
}

// The formula, to the last bit: callers whose results must not move (a
// seeded simulation) depend on it.
BOOST_AUTO_TEST_CASE(lerp_is_a_plus_b_minus_a_times_t)
{
	float const as[] = {0.1f, -3.7f, 1e6f, 2.5e-3f}, bs[] = {0.7f, 11.3f, -1e-6f, 0.3f}, ts[] = {0.3f, 0.77f, 1.0f / 3, -0.2f, 1.7f};
	for (float a : as)
		for (float b : bs)
			for (float t : ts) {
				float const expected = a + (b - a) * t;
				BOOST_TEST(rove::lerp(a, b, t) == expected);
				rove::vec<2> const v = rove::lerp(rove::vec<2>(a, b), rove::vec<2>(b, a), t);
				BOOST_TEST(v.x == expected);
				BOOST_TEST(v.y == b + (a - b) * t);
			}
}

BOOST_AUTO_TEST_CASE(distance_is_the_length_of_the_difference)
{
	BOOST_TEST(rove::distance(rove::vec<2>(1, 1), rove::vec<2>(4, 5)) == 5);
	BOOST_TEST(rove::distance(rove::vec<3>(1, 2, 3), rove::vec<3>(1, 2, 3)) == 0);
	BOOST_TEST(rove::distance(rove::vec<3>(0, 0, 0), rove::vec<3>(2, 3, 6)) == 7);
	BOOST_TEST(rove::distance_sq(rove::vec<3>(0, 0, 0), rove::vec<3>(2, 3, 6)) == 49);
	BOOST_TEST(rove::distance(rove::vec<4>(1, 0, 0, 0), rove::vec<4>(0, 0, 0, 0)) == 1);
	BOOST_TEST(rove::distance(rove::xz<float>(-3, 0), rove::xz<float>(0, 4)) == 5);
	rove::vec<5> p{}, q{};
	q.i[3] = -2;
	BOOST_TEST(rove::distance(p, q) == 2);

	rove::vec<3> const a(0.1f, -7.3f, 2.2f), b(5.5f, 0.01f, -3.3f);
	BOOST_TEST(rove::distance(a, b) == (a - b).length());
	BOOST_TEST(rove::distance(a, b) == rove::distance(b, a));
	BOOST_TEST(rove::distance_sq(a, b) == (a - b).length_sq());
}

BOOST_AUTO_TEST_SUITE_END()

// vec<N> of more than four components keeps the rules of vec<2>, vec<3> and
// vec<4>.
BOOST_AUTO_TEST_SUITE(vec_n_rules)

namespace
{

template<int N> rove::vec<N> filled(std::initializer_list<float> values)
{
	BOOST_REQUIRE(values.size() == size_t(N));
	rove::vec<N> v{};
	int n = 0;
	for (float x : values)
		v.i[n++] = x;
	return v;
}

}

// |x1| + |x2| + ... : vec<2>'s manhattan_length for any number of components.
BOOST_AUTO_TEST_CASE(a_vec_ns_manhattan_length_is_the_sum_of_its_magnitudes)
{
	BOOST_TEST(filled<5>({3, -4, 0, 0, 0}).manhattan_length() == 7);
	BOOST_TEST(filled<5>({0, 0, 0, 0, -2}).manhattan_length() == 2);
	BOOST_TEST(filled<6>({1, 1, 1, 1, 1, 1}).manhattan_length() == 6);
	BOOST_TEST(filled<5>({0, 0, 0, 0, 0}).manhattan_length() == 0);
	BOOST_TEST(filled<5>({-1, 2, -3, 4, -5}).manhattan_length() == 15);

	rove::vec<8, unsigned> joints{};
	for (unsigned n = 0; n < 8; ++n)
		joints.i[n] = n;
	BOOST_TEST(joints.manhattan_length() == 28u);

	rove::vec<2> const v(3, -4);
	BOOST_TEST(v.manhattan_length() == 7);
}

BOOST_AUTO_TEST_CASE(is_collinear_measures_the_cross_product_alike_in_2d_and_3d)
{
	// the cross product of these is 1e-4 long in either arity
	rove::vec<2> const a2(1, 0), b2(1, 1e-4f);
	rove::vec<3> const a3(1, 0, 0), b3(1, 1e-4f, 0);
	BOOST_TEST(!a2.is_collinear(b2));
	BOOST_TEST(!a3.is_collinear(b3));
	BOOST_TEST(a2.is_collinear(b2, 1e-3f));
	BOOST_TEST(a3.is_collinear(b3, 1e-3f));
}

BOOST_AUTO_TEST_CASE(is_zero_takes_its_tolerance)
{
	// is_zero() is on the general vec<N>, beyond the named arities
	rove::vec<5> v;
	for (size_t n = 0; n < 5; ++n) v.i[n] = 0;
	BOOST_TEST(v.is_zero(0));
	v.i[1] = 1e-3f;
	BOOST_TEST(!v.is_zero());
	BOOST_TEST(v.is_zero(1e-2f));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(vec_compare_and_shape)

// The four comparisons that ask about every component.  They are no order (std::set and std::sort need one), so a vector has no
// operator < and friends, and std::less and std::hash of a vector are rove's own.

BOOST_AUTO_TEST_CASE(all_less_and_friends_ask_about_every_component)
{
	rove::vec<3> const a(0, 1, 2), b(1, 2, 3), c(1, 0, 3);
	BOOST_REQUIRE(rove::all_less(a, b));
	BOOST_REQUIRE(!rove::all_less(a, c));               // 1 < 0 is false
	BOOST_REQUIRE(!rove::all_less(a, a));
	BOOST_REQUIRE(rove::all_less_equal(a, a));
	BOOST_REQUIRE(!rove::all_less_equal(a, c));
	BOOST_REQUIRE(rove::all_greater(b, a));
	BOOST_REQUIRE(!rove::all_greater(c, a));
	BOOST_REQUIRE(rove::all_greater_equal(b, b));
	BOOST_REQUIRE(!rove::all_greater_equal(a, c));
	// a NaN is less, greater and equal to nothing
	float const nan = std::numeric_limits<float>::quiet_NaN();
	rove::vec<2> const n(nan, 0);
	BOOST_REQUIRE(!rove::all_less_equal(n, n));
	BOOST_REQUIRE(!rove::all_greater_equal(n, rove::vec<2>(-1, -1)));
	// every size and storage
	BOOST_REQUIRE(rove::all_less(rove::vec<4>(0, 0, 0, 0), rove::vec<4>(1, 1, 1, 1)));
	BOOST_REQUIRE(!rove::all_less(rove::vec<4>(0, 0, 0, 1), rove::vec<4>(1, 1, 1, 1)));
	BOOST_REQUIRE(rove::all_less(rove::xz<float>(0, 0), rove::xz<float>(1, 1)));
	rove::vec<5> lo, hi;
	lo.set_all(0);
	hi.set_all(1);
	BOOST_REQUIRE(rove::all_less(lo, hi));
	hi.i[4] = 0;
	BOOST_REQUIRE(!rove::all_less(lo, hi));
	BOOST_REQUIRE(rove::all_less_equal(lo, hi));
}

BOOST_AUTO_TEST_CASE(a_set_of_vectors_keeps_every_different_one_in_lexicographic_order)
{
	// with every-component < two vectors neither of which is below the other were "equal", and the set kept one of them
	std::set<rove::vec<3>> s{rove::vec<3>(0, 1, 0), rove::vec<3>(1, 0, 0), rove::vec<3>(0, 0, 1), rove::vec<3>(0, 1, 0)};
	BOOST_REQUIRE_EQUAL(s.size(), 3u);
	std::vector<rove::vec<3>> const in_order(s.begin(), s.end());
	BOOST_REQUIRE(in_order[0] == rove::vec<3>(0, 0, 1));
	BOOST_REQUIRE(in_order[1] == rove::vec<3>(0, 1, 0));
	BOOST_REQUIRE(in_order[2] == rove::vec<3>(1, 0, 0));
	std::map<rove::xz<double>, int> m{{rove::xz<double>(1, 0), 1}, {rove::xz<double>(0, 1), 2}};
	BOOST_REQUIRE_EQUAL(m.size(), 2u);
	BOOST_REQUIRE_EQUAL(m.begin()->second, 2);
	// a strict weak order: irreflexive, and of two different vectors one is less
	std::less<rove::vec<4>> const less;
	std::mt19937 rng(3);
	std::uniform_int_distribution<int> small(-1, 1);
	for (int k = 0; k < 2000; ++k) {
		rove::vec<4> const p(small(rng), small(rng), small(rng), small(rng)), q(small(rng), small(rng), small(rng), small(rng));
		BOOST_REQUIRE(!less(p, p));
		BOOST_REQUIRE(!(less(p, q) && less(q, p)));
		BOOST_REQUIRE_EQUAL(less(p, q) || less(q, p), !(p == q));
	}
}

BOOST_AUTO_TEST_CASE(a_hash_of_a_vector_tells_apart_what_equality_does)
{
	std::unordered_set<rove::vec<3>> s;
	for (int x = -5; x <= 5; ++x)
		for (int y = -5; y <= 5; ++y)
			for (int z = -5; z <= 5; ++z)
				s.insert(rove::vec<3>(float(x), float(y), float(z)));
	BOOST_REQUIRE_EQUAL(s.size(), 11u * 11u * 11u);
	BOOST_REQUIRE(s.count(rove::vec<3>(1, 2, 3)) == 1);
	// permutations are different vectors and should not all hash alike
	std::hash<rove::vec<3>> const h;
	BOOST_REQUIRE(h(rove::vec<3>(1, 2, 3)) != h(rove::vec<3>(3, 2, 1)));
	BOOST_REQUIRE(h(rove::vec<3>(1, 2, 3)) != h(rove::vec<3>(2, 1, 3)));
	// +0 and -0 are equal vectors, so they hash alike
	BOOST_REQUIRE_EQUAL(h(rove::vec<3>(0, 0, 0)), h(rove::vec<3>(-0.0f, 0, -0.0f)));
	std::unordered_map<rove::xz<float>, int> ground{{rove::xz<float>(1, 2), 3}};
	BOOST_REQUIRE_EQUAL(ground.at(rove::xz<float>(1, 2)), 3);
}

BOOST_AUTO_TEST_CASE(min_max_abs_and_clamp_go_component_by_component)
{
	rove::vec<3> const a(1, -5, 3), b(-2, 4, 3);
	BOOST_REQUIRE(rove::min(a, b) == rove::vec<3>(-2, -5, 3));
	BOOST_REQUIRE(rove::max(a, b) == rove::vec<3>(1, 4, 3));
	BOOST_REQUIRE(rove::abs(a) == rove::vec<3>(1, 5, 3));
	BOOST_REQUIRE(rove::abs(rove::vec<4>(-1, 2, -3, -0.0f)) == rove::vec<4>(1, 2, 3, 0));
	// clamp of vectors took T for the vector and std::clamp's every-component <: (5, -5, 0.5) clamped to the unit box came back as
	// it was, since it is neither all below lo nor all above hi
	rove::vec<3> const lo(0, 0, 0), hi(1, 1, 1);
	BOOST_REQUIRE(rove::clamp(rove::vec<3>(5, -5, 0.5f), lo, hi) == rove::vec<3>(1, 0, 0.5f));
	BOOST_REQUIRE(rove::clamp(rove::vec<2>(-3, 7), rove::vec<2>(-1, -1), rove::vec<2>(1, 1)) == rove::vec<2>(-1, 1));
	// scalar bounds for every component
	BOOST_REQUIRE(rove::clamp(rove::vec<3>(5, -5, 0.5f), 0.0f, 1.0f) == rove::vec<3>(1, 0, 0.5f));
	// the colours keep their own order
	rove::rgba<float> const c = rove::min(rove::rgba<float>(1, 0.5f, 0, 1), rove::rgba<float>(0.5f, 1, 1, 0));
	BOOST_REQUIRE(c.r == 0.5f && c.g == 0.5f && c.b == 0 && c.a == 0);
	// the scalar ones are still the scalars'
	BOOST_REQUIRE_EQUAL(rove::clamp(2.0f), 1.0f);
	BOOST_REQUIRE_EQUAL(rove::abs(-2.5), 2.5);
}

BOOST_AUTO_TEST_CASE(vec4_has_what_vec2_and_vec3_have)
{
	rove::vec<4> const v(rove::vec<3>(1, 2, 3), 4);
	BOOST_REQUIRE(v == rove::vec<4>(1, 2, 3, 4));
	BOOST_REQUIRE(-v == rove::vec<4>(-1, -2, -3, -4));
	BOOST_REQUIRE(v * rove::vec<4>(2, 3, 4, 5) == rove::vec<4>(2, 6, 12, 20));
	BOOST_REQUIRE(rove::vec<4>(2, 6, 12, 20) / rove::vec<4>(2, 3, 4, 5) == v);
	BOOST_REQUIRE(v.xyz() == rove::vec<3>(1, 2, 3));
	BOOST_REQUIRE(rove::vec<3>(1, 2, 3).xy() == rove::vec<2>(1, 2));
}

BOOST_AUTO_TEST_CASE(is_finite_is_every_component_a_number_and_not_infinite)
{
	BOOST_REQUIRE(rove::is_finite(rove::vec<3>(1, -2, 3)));
	BOOST_REQUIRE(!rove::is_finite(rove::vec<3>(1, std::numeric_limits<float>::quiet_NaN(), 3)));
	BOOST_REQUIRE(!rove::is_finite(rove::vec<2>(std::numeric_limits<float>::infinity(), 0)));
	BOOST_REQUIRE(!rove::is_finite(rove::vec<4>(0, 0, 0, -std::numeric_limits<double>::infinity())));
}

// v.check_finite(what) is is_finite(v) or std::invalid_argument, whose message is `what` and " that is not finite"
BOOST_AUTO_TEST_CASE(check_finite_refuses_each_component_that_is_not_a_number_or_infinite)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	float const most = std::numeric_limits<float>::max(), least = std::numeric_limits<float>::denorm_min();
	BOOST_CHECK_NO_THROW((void)(rove::vec<3>(0, -0.0f, 1).check_finite("a point")));
	BOOST_CHECK_NO_THROW((void)(rove::vec<3>(most, -most, least).check_finite("a point")));
	BOOST_CHECK_NO_THROW((void)(rove::vec<2, double>(1e308, -1e-320).check_finite("a point")));
	for (float const bad : {nan, inf, -inf}) {
		for (int k = 0; k < 2; ++k) {
			rove::vec<2> v2(1, 2);
			v2.i[k] = bad;
			BOOST_CHECK_THROW((void)v2.check_finite("a point"), std::invalid_argument);
		}
		for (int k = 0; k < 3; ++k) {
			rove::vec<3> v3(1, 2, 3);
			v3.i[k] = bad;
			BOOST_CHECK_THROW((void)v3.check_finite("a point"), std::invalid_argument);
		}
		for (int k = 0; k < 4; ++k) {
			rove::vec<4, double> v4(1, 2, 3, 4);
			v4.i[k] = bad;
			BOOST_CHECK_THROW((void)v4.check_finite("a point"), std::invalid_argument);
		}
	}
	try {
		(void)(rove::vec<3>(0, nan, 0).check_finite("octree: a ray's origin"));
		BOOST_FAIL("a NaN passed");
	} catch (std::invalid_argument const &e) {
		BOOST_TEST(std::string(e.what()) == "octree: a ray's origin that is not finite");
	}
}

BOOST_AUTO_TEST_CASE(project_reject_and_reflect)
{
	rove::vec<3> const v(3, 4, 5), onto(0, 2, 0);
	BOOST_REQUIRE(rove::project(v, onto) == rove::vec<3>(0, 4, 0));          // onto need not be a unit vector
	BOOST_REQUIRE(rove::reject(v, onto) == rove::vec<3>(3, 0, 5));
	BOOST_REQUIRE(rove::project(v, onto) + rove::reject(v, onto) == v);
	// reflect off a surface of unit normal n: the part along n turns round
	BOOST_REQUIRE(rove::reflect(rove::vec<3>(1, -1, 0), rove::vec<3>(0, 1, 0)) == rove::vec<3>(1, 1, 0));
	BOOST_REQUIRE(rove::reflect(rove::vec<2>(2, -3), rove::vec<2>(0, 1)) == rove::vec<2>(2, 3));
	// onto nothing has no direction to project on: a bug in the caller
	BOOST_REQUIRE_THROW((void)rove::project(v, rove::vec<3>(0, 0, 0)), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)rove::reject(v, rove::vec<3>(0, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(angle_between_vectors_is_unsigned_and_holds_at_small_angles)
{
	BOOST_REQUIRE(rove::abs(rove::angle_between(rove::vec<3>(1, 0, 0), rove::vec<3>(0, 3, 0)) - rove::PI / 2) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(rove::angle_between(rove::vec<3>(1, 0, 0), rove::vec<3>(-2, 0, 0)) - rove::PI) < TOLERANCE);
	BOOST_REQUIRE_EQUAL(rove::angle_between(rove::vec<3>(1, 2, 3), rove::vec<3>(2, 4, 6)), 0.0f);
	// acos of the dot would give 0 here (the dot rounds to 1); atan2 of the cross gives the angle
	double const tiny = 1e-9;
	BOOST_REQUIRE(std::abs(rove::angle_between(rove::vec<3, double>(1, 0, 0), rove::vec<3, double>(1, tiny, 0)) - tiny) < 1e-18);
	BOOST_REQUIRE(rove::abs(rove::angle_between(rove::vec<2>(1, 0), rove::vec<2>(-1, -1)) - 3 * rove::PI / 4) < TOLERANCE);
	BOOST_REQUIRE_THROW(rove::angle_between(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(signed_angle_turns_from_a_to_b_about_the_axis)
{
	rove::vec<3> const x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
	BOOST_REQUIRE(rove::abs(rove::signed_angle(x, y, z) - rove::PI / 2) < TOLERANCE);     // right-handed about +z
	BOOST_REQUIRE(rove::abs(rove::signed_angle(y, x, z) + rove::PI / 2) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(rove::signed_angle(x, y, -z) + rove::PI / 2) < TOLERANCE);
	// the axis needs no length of one, and the vectors need not be perpendicular to it
	BOOST_REQUIRE(rove::abs(rove::signed_angle(rove::vec<3>(1, 5, 0), rove::vec<3>(0, 5, -1), rove::vec<3>(0, 3, 0)) - rove::PI / 2) < 1e-5f);
	// 2D: from the first axis towards the second, as to_polar measures
	BOOST_REQUIRE(rove::abs(rove::signed_angle(rove::vec<2>(1, 0), rove::vec<2>(0, 1)) - rove::PI / 2) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(rove::signed_angle(rove::vec<2>(1, 0), rove::vec<2>(-1, -0.0001f)) + rove::PI) < 1e-3f);
	BOOST_REQUIRE_THROW(rove::signed_angle(x, y, rove::vec<3>(0, 0, 0)), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(the_ground_of_a_y_up_world)
{
	rove::xz<float> const g = rove::to_xz(rove::vec<3>(1, 2, 3));
	BOOST_REQUIRE(g.x == 1 && g.z == 3);
	BOOST_REQUIRE(rove::from_xz(g, 7.0f) == rove::vec<3>(1, 7, 3));
	BOOST_REQUIRE_EQUAL(rove::horizontal_length(rove::vec<3>(3, 100, 4)), 5.0f);
	rove::xz<double> const d = rove::to_xz(rove::vec<3, double>(0.1, 0.2, 0.3));
	BOOST_REQUIRE(d.x == 0.1 && d.z == 0.3);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(vec_structured_bindings)

BOOST_AUTO_TEST_CASE(a_vector_unpacks_into_its_components)
{
	auto [x, y] = rove::vec<2>(1, 2);
	BOOST_TEST((x == 1 && y == 2));
	auto const [a, b, c] = rove::vec<3, double>(3, 4, 5);
	BOOST_TEST((a == 3 && b == 4 && c == 5));
	rove::vec<4> const q(6, 7, 8, 9);
	auto const &[q0, q1, q2, q3] = q;
	BOOST_TEST((q0 == 6 && q1 == 7 && q2 == 8 && q3 == 9));
	static_assert(std::tuple_size_v<rove::vec<6>> == 6 && std::is_same_v<std::tuple_element_t<5, rove::vec<6>>, float>);
}

BOOST_AUTO_TEST_CASE(by_reference_it_writes_the_components)
{
	rove::vec<3> v(1, 2, 3);
	auto &[x, y, z] = v;
	x = 10;
	z += 5;
	BOOST_TEST((v == rove::vec<3>(10, 2, 8)));
	(void)y;
}

BOOST_AUTO_TEST_CASE(colours_unpack_in_the_order_of_their_memory)
{
	auto [b, g, r, a] = rove::bgra<int>(1, 2, 3, 4);
	BOOST_TEST((b == 1 && g == 2 && r == 3 && a == 4));
	auto [gx, gz] = rove::xz<float>(5, 6);
	BOOST_TEST((gx == 5 && gz == 6));
}

BOOST_AUTO_TEST_CASE(a_vector_of_five_is_a_whole_type)
{
	rove::vec<5> const zero = constructed_over_garbage<rove::vec<5>>();
	for (int n = 0; n < 5; ++n) BOOST_TEST(zero.i[n] == 0.0f);
	rove::vec<5> const a(1, 2, 3, 4, 5), b(5, 4, 3, 2, 1);
	BOOST_TEST(((a + b) == rove::vec<5>(6, 6, 6, 6, 6)));
	BOOST_TEST(((a - b) == rove::vec<5>(-4, -2, 0, 2, 4)));
	BOOST_TEST(((a * 2.0f) == rove::vec<5>(2, 4, 6, 8, 10)));
	BOOST_TEST(((2.0f * a) == rove::vec<5>(2, 4, 6, 8, 10)));
	BOOST_TEST(((a / 2.0f) == rove::vec<5>(0.5f, 1, 1.5f, 2, 2.5f)));
	BOOST_TEST(((a * b) == rove::vec<5>(5, 8, 9, 8, 5)));
	BOOST_TEST(((-a) == rove::vec<5>(-1, -2, -3, -4, -5)));
	rove::vec<5> const unit(1, 0, 0, 0, 0);
	BOOST_TEST((rove::reject(a, unit) == rove::vec<5>(0, 2, 3, 4, 5)));
	BOOST_TEST((rove::move_towards(zero, unit, 0.25f) == rove::vec<5>(0.25f, 0, 0, 0, 0)));
	rove::vec<6, double> const six(1, 2, 3, 4, 5, 6);
	BOOST_TEST((six + six).i[5] == 12.0);
}

BOOST_AUTO_TEST_CASE(to_axis_angle_of_a_rotation_vector)
{
	rove::vec<3> axis;
	float angle;
	rove::vec<3>(0, 0, 1.5f).to_axis_angle(axis, angle);
	BOOST_TEST(angle == 1.5f);
	BOOST_TEST(axis == rove::vec<3>(0, 0, 1));
	rove::vec<3>(0, -2, 0).to_axis_angle(axis, angle);
	BOOST_TEST(angle == 2.0f);
	BOOST_TEST(axis == rove::vec<3>(0, -1, 0));
	// no rotation has no axis of its own: a unit one, and an angle of 0
	rove::vec<3>(0, 0, 0).to_axis_angle(axis, angle);
	BOOST_TEST(angle == 0.0f);
	BOOST_TEST(axis.length() == 1.0f);
}

// These lived in one translation unit, for floats
BOOST_AUTO_TEST_CASE(every_scalar_and_storage_links_what_a_float_vector_has)
{
	rove::vec<3, double> const d(1, 2, 3);
	BOOST_TEST((d.perpendicular_xy() == rove::vec<3, double>(2, -1, 0)));
	BOOST_TEST((d.perpendicular_yz() == rove::vec<3, double>(0, 3, -2)));
	BOOST_TEST((d.perpendicular_xz() == rove::vec<3, double>(3, 0, -1)));
	rove::vec<3, double> axis;
	double angle;
	d.to_axis_angle(axis, angle);
	BOOST_TEST(angle == d.length());
	rove::rgb<float> const c(1, 2, 3);
	BOOST_TEST(c.perpendicular_xy().r == 2.0f);
}

BOOST_AUTO_TEST_CASE(abs_of_zero_is_positive_zero)
{
	BOOST_TEST(!std::signbit(rove::abs(0.0f)));
	BOOST_TEST(!std::signbit(rove::abs(-0.0f)));
	BOOST_TEST(rove::abs(-2.5f) == 2.5f);
	BOOST_TEST(std::isnan(rove::abs(std::nanf(""))));
	BOOST_TEST(!std::signbit(rove::vec<3>(0, 0, 0).manhattan_length()));
	BOOST_TEST(rove::abs(3u) == 3u);
}

BOOST_AUTO_TEST_CASE(normalize_and_normalize_or_zero_agree_at_the_extremes)
{
	rove::vec<3> const tiny(1e-30f, 0, 0), huge(1e20f, 0, 0), both(3e-30f, 4e-30f, 0);
	BOOST_TEST(rove::normalize(tiny) == rove::vec<3>(1, 0, 0));
	BOOST_TEST(rove::normalize_or_zero(tiny) == rove::vec<3>(1, 0, 0));
	BOOST_TEST(rove::normalize(huge) == rove::vec<3>(1, 0, 0));
	BOOST_TEST(rove::normalize_or_zero(huge) == rove::vec<3>(1, 0, 0));
	BOOST_TEST(rove::normalize(both).x == 0.6f, boost::test_tools::tolerance(1e-5f));
	BOOST_TEST(rove::normalize_or_zero(both).y == 0.8f, boost::test_tools::tolerance(1e-5f));
	// what has no direction still has none
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<3>(0, 0, 0)), std::invalid_argument);
	BOOST_TEST(rove::normalize_or_zero(rove::vec<3>(0, 0, 0)) == rove::vec<3>(0, 0, 0));
	BOOST_CHECK_THROW((void)rove::normalize(rove::vec<3>(std::nanf(""), 0, 0)), std::invalid_argument);
	BOOST_CHECK_THROW((void)rove::direction(tiny), std::invalid_argument);
	// bit for bit for an ordinary vector
	rove::vec<3> const v(1, 2, 3);
	BOOST_TEST(rove::normalize_or_zero(v).y == 2.0f / v.length());
	BOOST_TEST(rove::normalize(v).y == 2.0f / v.length());
}

BOOST_AUTO_TEST_SUITE_END()
