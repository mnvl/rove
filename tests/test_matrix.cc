// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <numbers>
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <rove/matrix.h>
#include <rove/quaternion.h>
#include <rove/vec.h>
#include "test_tolerance.h"

namespace
{

template<int N> void
test_axis_angle_i(rove::vec<3> const &src, rove::vec<3> const &axis, rove::scalar angle, rove::vec<3> const &res)
{
	rove::matrix<N,N> m;
	m.rotation(axis, angle);

	rove::vec<3> result = m * src;

	BOOST_REQUIRE ((result.length() - src.length()) < TOLERANCE);
	BOOST_REQUIRE ((res - result).length() < TOLERANCE);
}

void test_axis_angle(rove::vec<3> const &src, rove::vec<3> const &axis, rove::scalar angle, rove::vec<3> const &res)
{
	test_axis_angle_i<3>(src, axis, angle, res);
	test_axis_angle_i<4>(src, axis, angle, res);

	test_axis_angle_i<3>(res, axis, -angle, src);
	test_axis_angle_i<4>(res, axis, -angle, src);
}

template<int N> void
test_rotation_i(rove::vec<3> const &src, rove::vec<3> const &angles, rove::vec<3> const &res)
{
	rove::matrix<N,N> m;
	m.rotation(angles.x, angles.y, angles.z);

	rove::vec<3> result = m * src;

	BOOST_REQUIRE ((result.length() - src.length()) < TOLERANCE);
	BOOST_REQUIRE ((res - result).length() < TOLERANCE);
}

void test_rotation(rove::vec<3> const &src, rove::vec<3> const &angles, rove::vec<3> const &res)
{
	test_rotation_i<3>(src, angles, res);
	test_rotation_i<4>(src, angles, res);

	test_rotation_i<3>(res, -angles, src);
	test_rotation_i<4>(res, -angles, src);
}

}

template<class M, class... A>
concept rotation_takes = requires(M m, A... a) { m.rotation(a...); };

template<class M, class... A>
concept rotate_takes = requires(M m, A... a) { m.rotate(a...); };

BOOST_AUTO_TEST_SUITE (test_matrix)

BOOST_AUTO_TEST_CASE (test_ijk)
{
	rove::vec<3> const i(1, 0, 0), j(0, 1, 0), k(0, 0, 1);

	test_axis_angle(i, j, -rove::PI / 2, k);
	test_rotation(i, -rove::PI / 2 * j, k);

	test_axis_angle(j, k, -rove::PI / 2, i);
	test_rotation(j, -rove::PI / 2 * k, i);

	test_axis_angle(k, i, -rove::PI / 2, j);
	test_rotation(k, -rove::PI / 2 * i, j);
}

BOOST_AUTO_TEST_CASE (assign_puts_each_vector_in_its_own_column_whole)
{
	// ij[c][r]: column c, row r; l1 is column 0, l2 column 1, l3 column 2, every component where it belongs
	rove::vec<3> const l1(1, 2, 3), l2(4, 5, 6), l3(7, 8, 9);
	rove::matrix<3,3> m;
	m.assign(l1, l2, l3);
	rove::vec<3> const l[3] = {l1, l2, l3};
	for (int c = 0; c < 3; ++c)
		for (int r = 0; r < 3; ++r)
			BOOST_CHECK_MESSAGE(m.ij[c][r] == l[c][r], "ij[" << c << "][" << r << "] = " << m.ij[c][r] << ", not " << l[c][r]);
	rove::matrix<3,3,double> d;
	d.assign(rove::vec<3,double>(1, 2, 3), rove::vec<3,double>(4, 5, 6), rove::vec<3,double>(7, 8, 9));
	BOOST_CHECK(d.ij[2][0] == 7 && d.ij[2][1] == 8 && d.ij[2][2] == 9);
}

BOOST_AUTO_TEST_CASE (test_pi4)
{
	{
		rove::vec<3> src(1, 0, 0);
		rove::vec<3> res = rove::normalize(rove::vec<3>(1, -1, 0));

		test_rotation(src, rove::vec<3>(0, 0, -rove::PI / 4), res);
		test_axis_angle(src, rove::vec<3>(0, 0, 1), -rove::PI / 4, res);
	}

	{
		rove::vec<3> src(0, 1, 0);
		rove::vec<3> res = rove::normalize(rove::vec<3>(1, 1, 0));

		test_rotation(src, rove::vec<3>(0, 0, -rove::PI / 4), res);
		test_axis_angle(src, rove::vec<3>(0, 0, 1), -rove::PI / 4, res);
	}
}

BOOST_AUTO_TEST_CASE (test_pi4_number_2)
{
	{
		rove::vec<3> src(1, 0, 0);
		rove::vec<3> res = rove::normalize(rove::vec<3>(1, 1, 0));

		test_rotation(src, rove::vec<3>(0, 0, rove::PI / 4), res);
		test_axis_angle(src, rove::vec<3>(0, 0, 1), rove::PI / 4, res);
	}

	{
		rove::vec<3> src(0, 1, 0);
		rove::vec<3> res = rove::normalize(rove::vec<3>(-1, 1, 0));

		test_rotation(src, rove::vec<3>(0, 0, rove::PI / 4), res);
		test_axis_angle(src, rove::vec<3>(0, 0, 1), rove::PI / 4, res);
	}
}

BOOST_AUTO_TEST_CASE (test_matrix_inverse_4x4_1)
{
	rove::matrix<4,4> M, I, R, U;

	// one draw at a time: the arguments of a call are evaluated in an order the compiler picks
	rove::scalar const sx = rove::scalar(rand()) + 1, sy = rove::scalar(rand()) + 1, sz = rove::scalar(rand()) + 1;
	M.scaling(sx, sy, sz);

	BOOST_REQUIRE(M.inverse(I));

	R = M * I;
	U.identity();

	BOOST_REQUIRE(rove::equal(R, U));
}

BOOST_AUTO_TEST_CASE (test_matrix_inverse_4x4_2)
{
	rove::matrix<4,4> M, I, R, U;

	rove::scalar const ax = rand() * rove::PI / rove::scalar(RAND_MAX), ay = rand() * rove::PI / rove::scalar(RAND_MAX),
		az = rand() * rove::PI / rove::scalar(RAND_MAX);
	M.rotation(ax, ay, az);

	BOOST_REQUIRE(M.inverse(I));

	R = M * I;
	U.identity();

	BOOST_REQUIRE(rove::equal(R, U));
}


BOOST_AUTO_TEST_CASE (test_matrix_inverse_4x4_3)
{
	rove::matrix<4,4> M, I, R, U;

	rove::scalar const dx = rand() * 10.0f / rove::scalar(RAND_MAX), dy = rand() * 10.0f / rove::scalar(RAND_MAX),
		dz = rand() * 10.0f / rove::scalar(RAND_MAX);
	M.translation(dx, dy, dz);

	BOOST_REQUIRE(M.inverse(I));

	R = M * I;
	U.identity();

	BOOST_REQUIRE(rove::equal(R, U));
}

BOOST_AUTO_TEST_CASE (test_matrix_inverse_4x4_4)
{
	std::mt19937 gen(4);
	std::uniform_real_distribution<rove::scalar> offset(0, 10), angle(0, rove::PI);

	for (int k = 0; k < 1000; ++k)
	{
		rove::matrix<4,4> M, I, R, U;

		// one draw at a time: the arguments of a call are evaluated in an order the compiler picks
		rove::scalar const dx = offset(gen), dy = offset(gen), dz = offset(gen);
		M.translation(dx, dy, dz);
		rove::scalar const ax = angle(gen), ay = angle(gen), az = angle(gen);
		M.rotate(ax, ay, az);

		BOOST_REQUIRE(M.inverse(I));

		R = M * I;
		U.identity();

		// M * I is the identity up to rounding at M's largest element: with
		// translations up to 10 one float ulp there is ~1e-6, which a fixed
		// EPSILON cannot hold (it failed ~5% of random draws).
		rove::scalar magnitude = 0;
		for (int c = 0; c < 4; ++c)
			for (int r = 0; r < 4; ++r)
				magnitude = std::max(magnitude, std::abs(M.ij[c][r]));

		BOOST_REQUIRE(rove::equal(R, U, 8 * magnitude * std::numeric_limits<rove::scalar>::epsilon()));
	}
}

// ── Double-precision instantiations ───────────────────────────────────────
// matrix<N,N,double> exists so callers can keep float64 data in float64.
// These pin that promise: a double rotation must be built from double
// trigonometry, and the named element accessors must alias the same storage
// the ij[][] array does.

BOOST_AUTO_TEST_CASE(matrix_3x3_double_rotation_is_exact_in_double)
{
	double const angle = 0.7123456789012345;
	rove::matrix<3,3,double> m;
	m.rotation(rove::vec<3,double>(0, 0, 1), angle);

	BOOST_REQUIRE_EQUAL(m.ij[0][0], std::cos(angle));
	BOOST_REQUIRE_EQUAL(m.ij[0][1], std::sin(angle));
	BOOST_REQUIRE_EQUAL(m.ij[1][0], -std::sin(angle));
	BOOST_REQUIRE_EQUAL(m.ij[1][1], std::cos(angle));
}

BOOST_AUTO_TEST_CASE(matrix_4x4_double_rotation_is_exact_in_double)
{
	double const angle = 0.7123456789012345;
	rove::matrix<4,4,double> m;
	m.rotation(rove::vec<3,double>(0, 0, 1), angle);

	BOOST_REQUIRE_EQUAL(m.ij[0][0], std::cos(angle));
	BOOST_REQUIRE_EQUAL(m.ij[0][1], std::sin(angle));
	BOOST_REQUIRE_EQUAL(m.ij[1][0], -std::sin(angle));
	BOOST_REQUIRE_EQUAL(m.ij[1][1], std::cos(angle));
}

BOOST_AUTO_TEST_CASE(matrix_3x3_double_euler_rotation_is_exact_in_double)
{
	double const a = 0.31234567890123456;
	rove::matrix<3,3,double> m;
	m.rotation(0.0, 0.0, a);

	// Rotation about Z only: the top-left 2x2 block is the plain 2-D rotation.
	BOOST_REQUIRE_EQUAL(m.ij[0][0], std::cos(a));
	BOOST_REQUIRE_EQUAL(m.ij[1][1], std::cos(a));
}

BOOST_AUTO_TEST_CASE(matrix_4x4_double_perspective_is_exact_in_double)
{
	// perspective() divides by tan(fovy/2); a float tangent shows up here.
	double const fovy = 0.9123456789012345;
	rove::matrix<4,4,double> m;
	m.perspective(fovy, 1.0, 1.0, 100.0);

	BOOST_REQUIRE_EQUAL(m.ij[1][1], 1.0 / std::tan(fovy / 2));
}

BOOST_AUTO_TEST_CASE(matrix_3x3_named_members_alias_the_element_array)
{
	// The union's named members must have the matrix's own scalar type;
	// declaring them `float` in a double matrix reinterprets half of
	// ij[0][0] as a whole element.
	rove::matrix<3,3,double> m;
	m.zero();
	m.ij[0][0] = 1.5;
	m.ij[0][1] = 2.5;
	m.ij[1][0] = 3.5;
	m.ij[2][2] = 4.5;

	BOOST_REQUIRE_EQUAL(m._11, 1.5);
	BOOST_REQUIRE_EQUAL(m._12, 2.5);
	BOOST_REQUIRE_EQUAL(m._21, 3.5);
	BOOST_REQUIRE_EQUAL(m._33, 4.5);
}

BOOST_AUTO_TEST_CASE(matrix_3x3_float_named_members_alias_the_element_array)
{
	rove::matrix<3,3,float> m;
	m.zero();
	m.ij[0][0] = 1.5f;
	m.ij[0][2] = 2.5f;
	m.ij[2][1] = 3.5f;

	BOOST_REQUIRE_EQUAL(m._11, 1.5f);
	BOOST_REQUIRE_EQUAL(m._13, 2.5f);
	BOOST_REQUIRE_EQUAL(m._32, 3.5f);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_matrix_conversion)

// Every element distinct, so a transposed or shifted copy cannot pass.
template<int N> rove::matrix<N, N, double> distinct()
{
	rove::matrix<N, N, double> m;
	for (int c = 0; c < N; ++c)
		for (int r = 0; r < N; ++r)
			m.ij[c][r] = (c * N + r + 1) * 1.1 - 0.3;
	return m;
}

template<int N> void check_conversion()
{
	rove::matrix<N, N, double> const d = distinct<N>();
	rove::matrix<N, N, float> const f(d);
	rove::matrix<N, N, double> const back(f);
	for (int c = 0; c < N; ++c)
		for (int r = 0; r < N; ++r)
		{
			BOOST_TEST(f.ij[c][r] == float(d.ij[c][r]));
			BOOST_TEST(back.ij[c][r] == double(f.ij[c][r]));
		}
}

BOOST_AUTO_TEST_CASE(matrix_3x3_converts_element_by_element) { check_conversion<3>(); }
BOOST_AUTO_TEST_CASE(matrix_4x4_converts_element_by_element) { check_conversion<4>(); }

BOOST_AUTO_TEST_CASE(matrix_conversion_is_explicit)
{
	static_assert(!std::is_convertible_v<rove::matrix<4, 4, double>, rove::matrix<4, 4, float>>);
	static_assert(!std::is_convertible_v<rove::matrix<3, 3, float>, rove::matrix<3, 3, double>>);
	static_assert(std::is_constructible_v<rove::matrix<4, 4, float>, rove::matrix<4, 4, double>>);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_matrix_from_column_major)

// 1..N*N: every number distinct, so a transposed or shifted read cannot pass.
template<int N> void check_from_column_major()
{
	double v[N * N];
	for (int k = 0; k < N * N; ++k)
		v[k] = k + 1;
	rove::matrix<N, N, double> const m = rove::matrix<N, N, double>::from_column_major(v);
	for (int c = 0; c < N; ++c)
		for (int r = 0; r < N; ++r)
			BOOST_TEST(m.ij[c][r] == v[c * N + r]);
}

BOOST_AUTO_TEST_CASE(matrix_3x3_from_column_major_reads_columns) { check_from_column_major<3>(); }
BOOST_AUTO_TEST_CASE(matrix_4x4_from_column_major_reads_columns) { check_from_column_major<4>(); }

BOOST_AUTO_TEST_CASE(matrix_4x4_from_column_major_takes_gltf_translation_last)
{
	// glTF's node.matrix: translation (5, 6, 7) is numbers 12-14, a quarter
	// turn about +Y turns +X to -Z.
	float const gltf[16] = {0, 0, -1, 0,  0, 1, 0, 0,  1, 0, 0, 0,  5, 6, 7, 1};
	rove::matrix<4, 4> const m = rove::matrix<4, 4>::from_column_major(gltf);
	BOOST_TEST((m * rove::vec<3>(1, 0, 0) - rove::vec<3>(5, 6, 6)).length() < TOLERANCE);
	BOOST_TEST((m * rove::vec<3>(0, 0, 1) - rove::vec<3>(6, 6, 7)).length() < TOLERANCE);
	BOOST_TEST(m._41 == 5);  // named members are _<column><row>
	BOOST_TEST(m._13 == -1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_matrix_transform_vector)

rove::matrix<4, 4> affine()
{
	rove::matrix<4, 4> t, r, s;
	t.translation(10, -20, 30);
	r.rotation(rove::vec<3>(0, 1, 0), rove::PI / 2);
	s.scaling(2, 3, 4);
	return t * r * s;
}

BOOST_AUTO_TEST_CASE(transform_vector_ignores_translation)
{
	rove::matrix<4, 4> t;
	t.translation(10, -20, 30);
	rove::vec<3> const d(1, 2, 3);
	BOOST_TEST((t.transform_vector(d) - d).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(transform_vector_rotates_and_scales_without_normalising)
{
	// scale (2, 3, 4) first, then +X turns to -Z
	rove::matrix<4, 4> const m = affine();
	BOOST_TEST((m.transform_vector(rove::vec<3>(1, 0, 0)) - rove::vec<3>(0, 0, -2)).length() < TOLERANCE);
	BOOST_TEST((m.transform_vector(rove::vec<3>(0, 1, 0)) - rove::vec<3>(0, 3, 0)).length() < TOLERANCE);
	BOOST_TEST((m.transform_vector(rove::vec<3>(0, 0, 1)) - rove::vec<3>(4, 0, 0)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(transform_vector_is_the_difference_of_two_transformed_points)
{
	rove::matrix<4, 4> const m = affine();
	rove::vec<3> const p(0.5f, -1.5f, 2.25f), d(-3, 0.75f, 1.25f);
	rove::vec<3> const expected = m * (p + d) - m * p;
	BOOST_TEST((m.transform_vector(d) - expected).length() < 1e-4f);
}

BOOST_AUTO_TEST_CASE(transform_vector_keeps_double_precision)
{
	rove::matrix<4, 4, double> m;
	m.rotation(rove::vec<3, double>(0, 0, 1), std::numbers::pi / 6);
	m.translate(1e9, 0, 0);
	rove::vec<3, double> const d = m.transform_vector(rove::vec<3, double>(1, 0, 0));
	BOOST_TEST(std::abs(d.x - std::cos(std::numbers::pi / 6)) < 1e-12);
	BOOST_TEST(std::abs(d.y - std::sin(std::numbers::pi / 6)) < 1e-12);
	BOOST_TEST(d.z == 0);
}

BOOST_AUTO_TEST_SUITE_END()

// m[c] is column c and m[c][r] the element in its row r, as in GLSL: the
// storage order, ij[c][r].  A column past the last one throws.
BOOST_AUTO_TEST_SUITE(test_matrix_index)

BOOST_AUTO_TEST_CASE(an_index_is_a_column_then_a_row)
{
	rove::matrix<3,3> m3;
	rove::matrix<4,4> m4;
	for (int c = 0; c < 4; ++c)
		for (int r = 0; r < 4; ++r) {
			m4.ij[c][r] = rove::scalar(10 * c + r);
			if (c < 3 && r < 3)
				m3.ij[c][r] = rove::scalar(10 * c + r);
		}
	for (int c = 0; c < 4; ++c)
		for (int r = 0; r < 4; ++r) {
			BOOST_TEST(m4[c][r] == rove::scalar(10 * c + r));
			if (c < 3 && r < 3)
				BOOST_TEST(m3[c][r] == rove::scalar(10 * c + r));
		}
	BOOST_TEST(m4[1][2] == m4._23);
	BOOST_TEST(m3[2][0] == m3._31);
}

// What a translation is, by index: the last column, as a shader reads it.
BOOST_AUTO_TEST_CASE(a_translation_is_the_last_column)
{
	rove::matrix<4,4> m;
	m.translation(1, 2, 3);
	BOOST_TEST((m[3][0] == 1 && m[3][1] == 2 && m[3][2] == 3 && m[3][3] == 1));
	BOOST_TEST((m[0][0] == 1 && m[0][3] == 0));
	rove::vec<3> const moved = m * rove::vec<3>(10, 20, 30);
	BOOST_TEST((moved == rove::vec<3>(m[3][0] + 10, m[3][1] + 20, m[3][2] + 30)));

	rove::matrix<3,3> m3;
	m3.translation(4, 5);
	BOOST_TEST((m3[2][0] == 4 && m3[2][1] == 5 && m3[2][2] == 1));
}

BOOST_AUTO_TEST_CASE(an_index_writes_the_element)
{
	rove::matrix<4,4> m;
	m[3][1] = 7;
	BOOST_TEST(m.ij[3][1] == 7);
	BOOST_TEST(m._42 == 7);
	m[0][0] *= 3;
	BOOST_TEST(m.ij[0][0] == 3);

	rove::matrix<3,3, double> d;
	d[1][2] = -2;
	BOOST_TEST(d._23 == -2);

	static_assert(std::is_same_v<decltype(m[0][0]), rove::scalar &>);
	rove::matrix<4,4> const c;
	static_assert(std::is_same_v<decltype(c[0][0]), rove::scalar const &>);
	rove::matrix<3,3> const c3;
	static_assert(std::is_same_v<decltype(c3[0][0]), rove::scalar const &>);
}

BOOST_AUTO_TEST_CASE(a_column_past_the_last_one_throws)
{
	rove::matrix<3,3> m3;
	rove::matrix<4,4> const m4;
	BOOST_CHECK_THROW(m3[3], std::out_of_range);
	BOOST_CHECK_THROW(m4[4], std::out_of_range);
	BOOST_CHECK_THROW(m4[size_t(-1)], std::out_of_range);
	BOOST_CHECK_NO_THROW(m4[3]);
	BOOST_CHECK_NO_THROW(m3[2]);
}

BOOST_AUTO_TEST_SUITE_END()

// matrix's operators are found by argument-dependent lookup, from a
// namespace that declares an operator * of its own.
namespace elsewhere_matrix
{

struct thing {};
inline thing operator *(thing, thing) { return {}; }

static rove::vec<3> moved(rove::matrix<4,4> const &m, rove::vec<3> const &v) { return m * v; }
static rove::vec<4> moved(rove::matrix<4,4> const &m, rove::vec<4> const &v) { return m * v; }
static rove::matrix<4,4> composed(rove::matrix<4,4> const &a, rove::matrix<4,4> const &b) { return a * b; }

}

BOOST_AUTO_TEST_SUITE(test_matrix_lookup)

BOOST_AUTO_TEST_CASE(matrix_operators_are_found_from_another_namespace)
{
	rove::matrix<4,4> a, b;
	a.translation(1, 0, 0);
	b.translation(0, 2, 0);
	BOOST_TEST((elsewhere_matrix::moved(a, rove::vec<3>(0, 0, 0)) == rove::vec<3>(1, 0, 0)));
	BOOST_TEST((elsewhere_matrix::moved(a, rove::vec<4>(0, 0, 0, 1)) == rove::vec<4>(1, 0, 0, 1)));
	BOOST_TEST((elsewhere_matrix::moved(elsewhere_matrix::composed(a, b), rove::vec<3>()) == rove::vec<3>(1, 2, 0)));
}

BOOST_AUTO_TEST_SUITE_END()

// An M x N matrix times an N x K one is M x K, and times an N vector is an M
// one (N - 1 and M - 1 for a point in homogeneous coordinates).  Only the
// square matrices are defined, so the shapes are checked where nothing need
// be complete: the declared type of the product.
BOOST_AUTO_TEST_SUITE(test_matrix_shapes)

template<int M, int N> using mat = rove::matrix<M, N>;
template<class L, class R> using product = decltype(std::declval<L const &>() * std::declval<R const &>());
template<class L, class R> concept multipliable = requires(L const &l, R const &r) { l * r; };

BOOST_AUTO_TEST_CASE(a_product_has_the_rows_of_the_left_and_the_columns_of_the_right)
{
	static_assert(std::is_same_v<product<mat<4, 4>, mat<4, 4>>, mat<4, 4>>);
	static_assert(std::is_same_v<product<mat<3, 3>, mat<3, 3>>, mat<3, 3>>);
	static_assert(std::is_same_v<product<mat<2, 3>, mat<3, 5>>, mat<2, 5>>);
	static_assert(std::is_same_v<product<mat<5, 4>, mat<4, 4>>, mat<5, 4>>);

	static_assert(std::is_same_v<product<mat<4, 4>, rove::vec<4>>, rove::vec<4>>);
	static_assert(std::is_same_v<product<mat<4, 4>, rove::vec<3>>, rove::vec<3>>);
	// a 3x3 times a 2D point is no product: transform_point (no divide) or project_point (divide by z) says which
	static_assert(!multipliable<mat<3, 3>, rove::vec<2>>);
	static_assert(std::is_same_v<product<mat<2, 4>, rove::vec<4>>, rove::vec<2>>);
	static_assert(std::is_same_v<product<mat<3, 4>, rove::vec<3>>, rove::vec<2>>);

	rove::matrix<4,4> a, b;
	a.translation(1, 0, 0);
	b.scaling(2, 2, 2);
	BOOST_TEST((a * b * rove::vec<3>(1, 1, 1) == rove::vec<3>(3, 2, 2)));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(matrix_by_value)

namespace
{

rove::matrix<4, 4> random_affine(std::mt19937 &rng)
{
	std::uniform_real_distribution<float> u(-2, 2), s(0.5f, 2);
	rove::matrix<4, 4> r, sc, t;
	r.rotation(rove::vec<3>(u(rng), u(rng), u(rng)), u(rng));
	sc.scaling(s(rng), s(rng), s(rng));
	t.translation(u(rng), u(rng), u(rng));
	return t * r * sc;
}

bool near(rove::vec<3> const &a, rove::vec<3> const &b, float tolerance)
{
	return (a - b).length() <= tolerance;
}

}

BOOST_AUTO_TEST_CASE(transform_point_is_affine_and_project_point_divides_by_w)
{
	rove::matrix<4, 4> p;
	p.perspective(1.0f, 1.5f, 1.0f, 100.0f);
	rove::vec<3> const v(0.5f, -0.25f, 50);
	rove::vec<4> const clip = p * rove::vec<4>(v, 1);
	// the operator is the affine one, w taken as 1 and never divided by: no projection
	BOOST_REQUIRE(rove::transform_point(p, v) == p * v);
	BOOST_REQUIRE(rove::project_point(p, v) == rove::vec<3>(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w));
	// on the near plane depth 0, on the far 1, as rove's projections put them
	BOOST_REQUIRE(std::abs(rove::project_point(p, rove::vec<3>(0, 0, 1)).z) < 1e-6f);
	BOOST_REQUIRE(std::abs(rove::project_point(p, rove::vec<3>(0, 0, 100)).z - 1) < 1e-6f);
	// for an affine matrix the two agree
	std::mt19937 rng(21);
	rove::matrix<4, 4> const a = random_affine(rng);
	BOOST_REQUIRE(near(rove::project_point(a, v), rove::transform_point(a, v), 1e-4f));
	// a vector ignores the translation
	rove::matrix<4, 4> t;
	t.translation(5, 6, 7);
	BOOST_REQUIRE(rove::transform_vector(t, rove::vec<3>(1, 2, 3)) == rove::vec<3>(1, 2, 3));
	// a point on the plane through the eye has no projection: w is 0
	BOOST_REQUIRE_THROW((void)rove::project_point(p, rove::vec<3>(1, 1, 0)), std::invalid_argument);
	// 2D: affine, and projective with the divide
	rove::matrix<3, 3> m;
	m.translation(1, 2);
	BOOST_REQUIRE(rove::transform_point(m, rove::vec<2>(3, 4)) == rove::vec<2>(4, 6));
	BOOST_REQUIRE(rove::project_point(m, rove::vec<2>(3, 4)) == rove::vec<2>(4, 6));
}

BOOST_AUTO_TEST_CASE(inverse_and_transpose_by_value)
{
	std::mt19937 rng(22);
	for (int k = 0; k < 200; ++k) {
		rove::matrix<4, 4> const m = random_affine(rng);
		rove::matrix<4, 4> expected;
		BOOST_REQUIRE(m.inverse(expected));
		rove::matrix<4, 4> const i = rove::inverse(m);
		BOOST_REQUIRE(rove::equal(i, expected, 0.0f));          // the member's, bit for bit
		BOOST_REQUIRE(rove::equal(m * i, rove::matrix<4, 4>(), 1e-4f));
		rove::matrix<4, 4> t;
		m.transpose(t);
		BOOST_REQUIRE(rove::equal(rove::transpose(m), t, 0.0f));
	}
	rove::matrix<4, 4> singular;
	singular.scaling(1, 0, 1);
	BOOST_REQUIRE_THROW((void)rove::inverse(singular), std::invalid_argument);
	rove::matrix<3, 3> flat;
	flat.zero();
	BOOST_REQUIRE_THROW((void)rove::inverse(flat), std::invalid_argument);
	rove::matrix<3, 3> r;
	r.rotation(rove::vec<3>(0, 0, 1), 0.5f);
	BOOST_REQUIRE(rove::equal(rove::inverse(r), rove::transpose(r), 1e-6f));
}

BOOST_AUTO_TEST_CASE(builders_are_the_setters_by_value)
{
	rove::matrix<4, 4> m;
	m.translation(1, 2, 3);
	BOOST_REQUIRE(rove::equal(rove::translation_matrix(rove::vec<3>(1, 2, 3)), m, 0.0f));
	m.scaling(2, 3, 4);
	BOOST_REQUIRE(rove::equal(rove::scaling_matrix(rove::vec<3>(2, 3, 4)), m, 0.0f));
	m.rotation(rove::vec<3>(0, 1, 0), 0.7f);
	BOOST_REQUIRE(rove::equal(rove::rotation_matrix(rove::vec<3>(0, 1, 0), 0.7f), m, 0.0f));
	m.lookat(rove::vec<3>(1, 2, 3), rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0));
	BOOST_REQUIRE(rove::equal(rove::look_at_matrix(rove::vec<3>(1, 2, 3), rove::vec<3>(0, 0, 0), rove::vec<3>(0, 1, 0)), m, 0.0f));
	m.perspective(1.0f, 1.5f, 0.1f, 100.0f);
	BOOST_REQUIRE(rove::equal(rove::perspective_matrix(1.0f, 1.5f, 0.1f, 100.0f), m, 0.0f));
	rove::quaternion<> const q = rove::axis_angle(rove::vec<3>(1, 0, 0), 0.3f);
	m.rotation(q);
	BOOST_REQUIRE(rove::equal(rove::rotation_matrix(q), m, 0.0f));
}

/// A turn about an axis of no length (or not a number, or infinite) has no axis: std::invalid_argument, from the setters, the
/// members that turn a matrix and the builder alike.  It was a matrix of NaN.  An axis of any other length is its direction.
BOOST_AUTO_TEST_CASE(a_turn_about_no_axis_is_refused)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	for (rove::vec<3> const &axis : {rove::vec<3>(0, 0, 0), rove::vec<3>(nan, 0, 1), rove::vec<3>(0, inf, 0)}) {
		rove::matrix<4, 4> m4;
		rove::matrix<3, 3> m3;
		BOOST_CHECK_THROW(m4.rotation(axis, 0.5f), std::invalid_argument);
		BOOST_CHECK_THROW(m4.rotation(axis, 0.5f, 0.8f), std::invalid_argument);
		BOOST_CHECK_THROW(m4.rotate(axis, 0.5f), std::invalid_argument);
		BOOST_CHECK_THROW(m3.rotation(axis, 0.5f), std::invalid_argument);
		BOOST_CHECK_THROW(m3.rotate(axis, 0.5f), std::invalid_argument);
		BOOST_CHECK_THROW((void)rove::rotation_matrix(axis, 0.5f), std::invalid_argument);
	}
	rove::matrix<4, 4> unit, long_;
	unit.rotation(rove::vec<3>(0, 0.6f, 0.8f), 0.9f);
	long_.rotation(rove::vec<3>(0, 6, 8), 0.9f);
	BOOST_REQUIRE(rove::equal(unit, long_, 1e-6f));
}

BOOST_AUTO_TEST_CASE(an_off_centre_ortho_puts_its_box_on_the_unit_cube_depth_0_to_1)
{
	rove::matrix<4, 4> const o = rove::ortho_matrix(-1.0f, 3.0f, 2.0f, 10.0f, 5.0f, 25.0f);
	BOOST_REQUIRE(near(rove::transform_point(o, rove::vec<3>(-1, 2, 5)), rove::vec<3>(-1, -1, 0), 1e-6f));
	BOOST_REQUIRE(near(rove::transform_point(o, rove::vec<3>(3, 10, 25)), rove::vec<3>(1, 1, 1), 1e-6f));
	BOOST_REQUIRE(near(rove::transform_point(o, rove::vec<3>(1, 6, 15)), rove::vec<3>(0, 0, 0.5f), 1e-6f));
	// centred, it is the member's
	rove::matrix<4, 4> centred;
	centred.ortho(4, 8, 5, 25);
	BOOST_REQUIRE(rove::equal(rove::ortho_matrix(-2.0f, 2.0f, -4.0f, 4.0f, 5.0f, 25.0f), centred, 1e-6f));
	BOOST_REQUIRE_THROW((void)rove::ortho_matrix(1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f), std::invalid_argument);
	BOOST_REQUIRE_THROW((void)rove::ortho_matrix(0.0f, 1.0f, 0.0f, 1.0f, 2.0f, 2.0f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(the_normal_matrix_keeps_normals_square_to_a_stretched_surface)
{
	rove::matrix<4, 4> m = rove::scaling_matrix(rove::vec<3>(4, 1, 1));
	m = rove::rotation_matrix(rove::vec<3>(0, 0, 1), 0.4f) * m;
	// a surface along (1, 1, 0) with normal (1, -1, 0): after the stretch the transformed normal is still square to it
	rove::vec<3> const along = rove::transform_vector(m, rove::vec<3>(1, 1, 0));
	rove::vec<3> const normal = rove::normal_matrix(m) * rove::vec<3>(1, -1, 0);
	BOOST_REQUIRE(std::abs(rove::dot(along, normal)) < 1e-5f);
	BOOST_REQUIRE(std::abs(rove::dot(along, rove::linear_part(m) * rove::vec<3>(1, -1, 0))) > 0.1f);    // the plain one is not
	BOOST_REQUIRE(rove::linear_part(m) * rove::vec<3>(1, 2, 3) == rove::transform_vector(m, rove::vec<3>(1, 2, 3)));
}

BOOST_AUTO_TEST_CASE(euler_rotation_turns_about_x_then_y_then_z)
{
	// rotation(a, b, g) is Rz(g) Ry(b) Rx(a): x first, then y, then z, about the fixed axes
	float const a = 0.3f, b = -0.7f, g = 1.1f;
	rove::matrix<3, 3> e, rx, ry, rz;
	e.rotation(a, b, g);
	rx.rotation(rove::vec<3>(1, 0, 0), a);
	ry.rotation(rove::vec<3>(0, 1, 0), b);
	rz.rotation(rove::vec<3>(0, 0, 1), g);
	BOOST_REQUIRE(rove::equal(e, rz * ry * rx, 1e-6f));
}

// m *= m read m's elements as it wrote them, for the left operand had been copied and the right had not
template<int N, class T>
static void check_squaring_in_place()
{
	rove::matrix<N, N, T> m;
	for (int c = 0; c < N; c++) {
		for (int r = 0; r < N; r++) {
			m.ij[c][r] = T(1 + c * N + r) / T(7);
		}
	}
	rove::matrix<N, N, T> const want = m * m;

	m *= m;

	for (int c = 0; c < N; c++) {
		for (int r = 0; r < N; r++) {
			BOOST_REQUIRE_EQUAL(m.ij[c][r], want.ij[c][r]);
		}
	}
}

BOOST_AUTO_TEST_CASE (a_matrix_squared_in_place_is_its_square)
{
	check_squaring_in_place<3, float>();
	check_squaring_in_place<3, double>();
	check_squaring_in_place<4, float>();
	check_squaring_in_place<4, double>();
}

template<int N, class T>
static void check_composing_in_place()
{
	rove::matrix<N, N, T> a, b;
	for (int c = 0; c < N; c++) {
		for (int r = 0; r < N; r++) {
			a.ij[c][r] = T(1 + c * N + r) / T(7);
			b.ij[c][r] = T(3 + r * N - c) / T(5);
		}
	}
	rove::matrix<N, N, T> const want = a * b, b_before = b;
	BOOST_REQUIRE(!rove::equal(want, b * a, T(1e-3)));

	a *= b;

	for (int c = 0; c < N; c++) {
		for (int r = 0; r < N; r++) {
			BOOST_REQUIRE_EQUAL(a.ij[c][r], want.ij[c][r]);
			BOOST_REQUIRE_EQUAL(b.ij[c][r], b_before.ij[c][r]);
		}
	}
}

BOOST_AUTO_TEST_CASE (composing_in_place_is_the_product_in_its_order_bit_for_bit)
{
	check_composing_in_place<3, float>();
	check_composing_in_place<3, double>();
	check_composing_in_place<4, float>();
	check_composing_in_place<4, double>();
}

BOOST_AUTO_TEST_CASE(inverse_into_itself_is_the_inverse)
{
	rove::matrix<4,4> m = rove::translation_matrix(rove::vec<3>(1, 2, 3)) * rove::rotation_matrix(rove::vec<3>(0, 0, 1), 0.7f);
	rove::matrix<4,4> expected;
	BOOST_REQUIRE(m.inverse(expected));
	rove::matrix<4,4> aliased = m;
	BOOST_REQUIRE(aliased.inverse(aliased));
	for (int c = 0; c < 4; ++c) for (int r = 0; r < 4; ++r) BOOST_TEST(aliased.ij[c][r] == expected.ij[c][r]);

	rove::matrix<3,3> n;
	n.rotation(rove::vec<3>(0, 1, 0), 0.4f);
	n.ij[2][0] = 1.5f;
	rove::matrix<3,3> expected3;
	BOOST_REQUIRE(n.inverse(expected3));
	rove::matrix<3,3> aliased3 = n;
	BOOST_REQUIRE(aliased3.inverse(aliased3));
	for (int c = 0; c < 3; ++c) for (int r = 0; r < 3; ++r) BOOST_TEST(aliased3.ij[c][r] == expected3.ij[c][r]);
}

BOOST_AUTO_TEST_CASE(transpose_into_itself_is_the_transpose)
{
	rove::matrix<4,4> m = rove::translation_matrix(rove::vec<3>(1, 2, 3));
	rove::matrix<4,4> aliased = m;
	aliased.transpose(aliased);
	for (int c = 0; c < 4; ++c) for (int r = 0; r < 4; ++r) BOOST_TEST(aliased.ij[c][r] == m.ij[r][c]);
	rove::matrix<3,3> n;
	n.rotation(rove::vec<3>(0, 1, 0), 0.4f);
	rove::matrix<3,3> aliased3 = n;
	aliased3.transpose(aliased3);
	for (int c = 0; c < 3; ++c) for (int r = 0; r < 3; ++r) BOOST_TEST(aliased3.ij[c][r] == n.ij[r][c]);
}

BOOST_AUTO_TEST_CASE(axes_to_world_of_axes_with_no_inverse_says_so)
{
	rove::matrix<3,3> m;
	m.identity();
	rove::vec<3> const x(1, 0, 0), y(0, 1, 0);
	BOOST_TEST(!m.axes_to_world(x, x, y));
	BOOST_TEST(m.axes_to_world(x, y, rove::vec<3>(0, 0, 1)));
}

// Euler angles are three numbers: a vector of them was never a thing to keep, and read as a rotation vector it was wrong without a word
BOOST_AUTO_TEST_CASE(a_rotation_is_not_set_from_a_vector_alone)
{
	static_assert(!rotation_takes<rove::matrix<4,4>, rove::vec<3>>);
	static_assert(!rotation_takes<rove::matrix<3,3>, rove::vec<3>>);
	static_assert(rotation_takes<rove::matrix<4,4>, rove::quaternion<float>>);
	static_assert(rotation_takes<rove::matrix<4,4>, float, float, float>);
	static_assert(rotation_takes<rove::matrix<4,4>, rove::vec<3>, float>);
}

BOOST_AUTO_TEST_CASE(a_matrix_is_not_turned_by_a_vector_alone)
{
	static_assert(!rotate_takes<rove::matrix<4,4>, rove::vec<3>>);
	static_assert(!rotate_takes<rove::matrix<3,3>, rove::vec<3>>);
	static_assert(rotate_takes<rove::matrix<4,4>, rove::quaternion<float>>);
	static_assert(rotate_takes<rove::matrix<4,4>, float, float, float>);
	static_assert(rotate_takes<rove::matrix<4,4>, rove::vec<3>, float>);
}

BOOST_AUTO_TEST_CASE(a_scale_too_small_for_its_determinant_has_no_inverse)
{
	BOOST_CHECK_THROW((void)rove::inverse(rove::scaling_matrix(rove::vec<3>(1e-13f, 1e-13f, 1e-13f))), std::invalid_argument);
	rove::matrix<4,4> out;
	BOOST_TEST(!rove::scaling_matrix(rove::vec<3>(1e-13f, 1e-13f, 1e-13f)).inverse(out));
	// the small scales that do have one keep working
	rove::matrix<4,4> const small = rove::inverse(rove::scaling_matrix(rove::vec<3>(1e-3f, 1e-3f, 1e-3f)));
	BOOST_TEST(small.ij[0][0] == 1000.0f, boost::test_tools::tolerance(1e-4f));
}

BOOST_AUTO_TEST_CASE(projections_with_nothing_between_their_planes_throw)
{
	rove::matrix<4,4> m;
	BOOST_CHECK_THROW(m.perspective(1, 1, 5, 5), std::invalid_argument);
	BOOST_CHECK_THROW(m.ortho(1, 1, 5, 5), std::invalid_argument);
	BOOST_CHECK_THROW(m.ortho(0, 1, 1, 5), std::invalid_argument);
	BOOST_CHECK_THROW(m.perspective(1, 0, 1, 5), std::invalid_argument);
	m.perspective(1, 1, 1, 5);
	BOOST_TEST(m.ij[2][3] == 1.0f);
}

BOOST_AUTO_TEST_SUITE_END()
