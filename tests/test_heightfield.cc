// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <rove/heightfield.h>

BOOST_AUTO_TEST_SUITE(test_triangle)

BOOST_AUTO_TEST_CASE(test_raytrace_simple_1)
{
	rove::matrix<4,4> tf;
	tf.identity();

	rove::heightfield<std::uint8_t> hf(tf, 2, 2);

	static std::uint8_t const heights[] = {
		0, 1,
		3, 7
	};

	hf.load_from_raw_buffer(heights);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(rove::vec<3>(0, 10.0f, 0), rove::vec<3>(0, -1.0f, 0)));
	BOOST_REQUIRE(!ci.index);

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE ((ci.position - rove::vec<3>(0, 0, 0)).length_sq() < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_simple_2)
{
	rove::matrix<4,4> tf;
	tf.identity();

	rove::heightfield<std::uint8_t> hf(tf, 2, 2);

	static std::uint8_t const heights[] = {
		0, 1,
		3, 7
	};

	hf.load_from_raw_buffer(heights);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(rove::vec<3>(0.5f, 10.0f, 0), rove::vec<3>(0, -1.0f, 0)));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE ((ci.position - rove::vec<3>(0.5f, 0.5f, 0)).length_sq() < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_simple_3)
{
	rove::matrix<4,4> tf;
	tf.identity();

	rove::heightfield<std::uint8_t> hf(tf, 2, 2);

	static std::uint8_t const heights[] = {
		0, 1,
		3, 7
	};

	hf.load_from_raw_buffer(heights);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(rove::vec<3>(1, 10.0f, 0), rove::vec<3>(0, -1.0f, 0)));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE ((ci.position - rove::vec<3>(1, 1, 0)).length_sq() < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_simple_4)
{
	rove::matrix<4,4> tf;
	tf.identity();

	rove::heightfield<std::uint8_t> hf(tf, 2, 2);

	static std::uint8_t const heights[] = {
		0, 1,
		3, 7
	};

	hf.load_from_raw_buffer(heights);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(rove::vec<3>(0, 10.0f, 0.5f), rove::vec<3>(0, -1.0f, 0)));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE ((ci.position - rove::vec<3>(0, 1.5f, 0.5f)).length_sq() < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_simple_5)
{
	rove::matrix<4,4> tf;
	tf.identity();

	rove::heightfield<std::uint8_t> hf(tf, 2, 2);

	static std::uint8_t const heights[] = {
		0, 1,
		3, 7
	};

	hf.load_from_raw_buffer(heights);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(rove::vec<3>(0, 10.0f,1), rove::vec<3>(0, -1.0f, 0)));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE ((ci.position - rove::vec<3>(0, 3, 1)).length_sq() < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_simple_6)
{
	rove::matrix<4,4> tf;
	tf.identity();

	rove::heightfield<std::uint8_t> hf(tf, 2, 2);

	static std::uint8_t const heights[] = {
		0, 1,
		3, 7
	};

	hf.load_from_raw_buffer(heights);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(rove::vec<3>(1, 10.0f,1), rove::vec<3>(0, -1.0f, 0)));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE ((ci.position - rove::vec<3>(1, 7, 1)).length_sq() < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_complex_1)
{
	rove::matrix<4,4> tf;
	tf.scaling(10, 10, 10);

	rove::heightfield<std::uint8_t> hf(tf, 11, 11);

	static std::uint8_t const heights[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	};

	hf.load_from_raw_buffer(heights);

	rove::vec<3> p1(0, 10, 0);
	rove::vec<3> p2(6.6737876f, 0, 4.9803157f);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(p1, p2 - p1));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE (rove::abs(ci.position.y / ci.position.x - 1) < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_complex_2)
{
	rove::matrix<4,4> tf;
	tf.scaling(10, 10, 10);

	rove::heightfield<std::uint8_t> hf(tf, 11, 11);

	static std::uint8_t const heights[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	};

	hf.load_from_raw_buffer(heights);

	rove::vec<3> p1(0, 10, 0);
	rove::vec<3> p2(2.0899076f, 0, 5.1918087f);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(p1, p2 - p1));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE (rove::abs(ci.position.y / ci.position.x - 1) < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_complex_3)
{
	rove::matrix<4,4> tf;
	tf.scaling(10, 10, 10);

	rove::heightfield<std::uint8_t> hf(tf, 11, 11);

	static std::uint8_t const heights[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	};
	hf.load_from_raw_buffer(heights);

	rove::vec<3> p1(0, 10, 0);
	rove::vec<3> p2(4.0656757f, 0, 7.5460067f);

	rove::contact_info<3> ci = hf.trace(rove::ray<3>(p1, p2 - p1));

	BOOST_REQUIRE (ci.happened == true);
	BOOST_REQUIRE (ci.penetrated == false);
	BOOST_REQUIRE (rove::abs(ci.position.y / ci.position.x - 1) < 0.01f);
	BOOST_REQUIRE (ci.normal.y > 0);
}

BOOST_AUTO_TEST_CASE(test_raytrace_complex_4)
{
	rove::matrix<4,4> tf;
	tf.scaling(10, 10, 10);

	rove::heightfield<std::uint8_t> hf(tf, 11, 11);

	static std::uint8_t const heights[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	};

	hf.load_from_raw_buffer(heights);

	for (int i = 0; i < 100; ++i)
	{
		rove::vec<3> p1(0, 10, 0);
		rove::vec<3> p2{rove::scalar(rand()) / rove::scalar(RAND_MAX) * 10, 0, rove::scalar(rand()) / rove::scalar(RAND_MAX) * 10};

		rove::contact_info<3> ci = hf.trace(rove::ray<3>(p1, p2 - p1));

		BOOST_REQUIRE (ci.happened == true);
		BOOST_REQUIRE (ci.penetrated == false);
		BOOST_REQUIRE (rove::abs(ci.position.y / ci.position.x - 1) < 0.01f);
		BOOST_REQUIRE (ci.normal.y > 0);
	}
}

BOOST_AUTO_TEST_CASE(test_y_under)
{
	rove::matrix<4,4> tf;
	tf.scaling(1, 1, 1);

	rove::heightfield<std::uint8_t> hf(tf, 2, 2);

	static std::uint8_t heights[2 * 2] = {
		0, 1,
		4, 9,
	};

	hf.load_from_raw_buffer(heights);

	for (size_t i = 0; i < 100; ++i)
	{
		rove::scalar height = hf.y_under(rove::vec<3>(i / 100.0f, 100, 0));
		BOOST_REQUIRE (rove::abs(height - i / 100.0) < 1e-3);
	}

	for (size_t i = 0; i < 100; ++i)
	{
		rove::scalar height = hf.y_under(rove::vec<3>(0, 100, i / 100.0f));
		BOOST_REQUIRE (rove::abs(height - 4 * i / 100.0f) < 1e-3);
	}

	for (size_t i = 0; i < 100; ++i)
	{
		rove::scalar height1 = hf.y_under(rove::vec<3>(i / 100.0f, 100, 1 - 1e-6f));
		rove::scalar height2 = 4 + (9 - 4) * i / 100.0f;
		BOOST_REQUIRE (rove::abs(height1 - height2) < 0.01f);
	}

	for (size_t i = 0; i < 100; ++i)
	{
		rove::scalar height1 = hf.y_under(rove::vec<3>(1 - 1e-6f, 100, i / 100.0f));
		rove::scalar height2 = 1 + (9 - 1) * i / 100.0f;
		BOOST_REQUIRE (rove::abs(height1 - height2) < 0.01f);
	}
}

BOOST_AUTO_TEST_SUITE_END()
