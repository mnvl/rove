// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <format>
#include <string>
#include <boost/test/unit_test.hpp>
#include <rove/format.h>

BOOST_AUTO_TEST_SUITE(format)

BOOST_AUTO_TEST_CASE(a_vector_is_its_components_as_operator_shift_writes_them)
{
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::vec<2>(1, -2)), "1 -2");
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::vec<3>(1, 2.5f, 3)), "1 2.5 3");
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::vec<4, double>(0.25, 0, 0, 1)), "0.25 0 0 1");
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::vec<2, int>(3, -4)), "3 -4");
}

BOOST_AUTO_TEST_CASE(the_spec_applies_to_each_component)
{
	BOOST_REQUIRE_EQUAL(std::format("{:.2f}", rove::vec<3>(1, 2, 3)), "1.00 2.00 3.00");
	BOOST_REQUIRE_EQUAL(std::format("{:>4}", rove::vec<2, int>(1, 22)), "   1   22");
	BOOST_REQUIRE_EQUAL(std::format("[{:+}]", rove::vec<2>(1, -1)), "[+1 -1]");
}

BOOST_AUTO_TEST_CASE(every_storage_and_any_arity)
{
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::rgb<float>(1, 0.5f, 0)), "1 0.5 0");
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::bgra<int>(1, 2, 3, 4)), "1 2 3 4");     // in memory order, as the type names it
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::xz<float>(1, 2)), "1 2");
	rove::vec<5> v;
	for (int n = 0; n < 5; ++n)
		v.i[n] = float(n);
	BOOST_REQUIRE_EQUAL(std::format("{}", v), "0 1 2 3 4");
}

BOOST_AUTO_TEST_CASE(a_quaternion_is_x_y_z_w)
{
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::quaternion<>()), "0 0 0 1");
	BOOST_REQUIRE_EQUAL(std::format("{:.1f}", rove::quaternion<double>(0.5, 0, 0, 0.5)), "0.5 0.0 0.0 0.5");
}

BOOST_AUTO_TEST_CASE(a_matrix_is_its_rows)
{
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::matrix<3, 3>()), "1 0 0; 0 1 0; 0 0 1");
	rove::matrix<4, 4> m;
	m.translation(1, 2, 3);
	// m[3] is the translation column: the last entry of the first three rows
	BOOST_REQUIRE_EQUAL(m[3][0], 1.0f);
	BOOST_REQUIRE_EQUAL(std::format("{}", m), "1 0 0 1; 0 1 0 2; 0 0 1 3; 0 0 0 1");
}

BOOST_AUTO_TEST_CASE(an_aabb_is_lo_x_hi)
{
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::aabb<3>(rove::vec<3>(-1, -2, -3), rove::vec<3>(1, 2, 3))), "-1 -2 -3 x 1 2 3");
	BOOST_REQUIRE_EQUAL(std::format("{:.1f}", rove::aabb<2>(rove::vec<2>(0, 0), rove::vec<2>(1, 1))), "0.0 0.0 x 1.0 1.0");
}

BOOST_AUTO_TEST_CASE(wide_strings)
{
	BOOST_REQUIRE(std::format(L"{}", rove::vec<3>(1, 2, 3)) == L"1 2 3");
	BOOST_REQUIRE(std::format(L"{}", rove::matrix<3, 3>()) == L"1 0 0; 0 1 0; 0 0 1");
}

#ifdef ROVE_DFLOAT
BOOST_AUTO_TEST_CASE(a_dfloat_is_the_float_it_holds)
{
	BOOST_REQUIRE_EQUAL(std::format("{:.3f}", rove::dfloat(0.5f)), "0.500");
	BOOST_REQUIRE_EQUAL(std::format("{}", rove::vec<3, rove::dfloat>(1, 2, 3)), "1 2 3");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
