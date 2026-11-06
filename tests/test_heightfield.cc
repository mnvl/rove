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

namespace
{
// 3 columns and 2 rows, the height of column c in row r being 10 * r + c
rove::heightfield<rove::scalar> wide_field(rove::matrix<4,4> const &tf)
{
	rove::heightfield<rove::scalar> hf(tf, 3, 2);
	static rove::scalar const heights[] = { 0, 1, 2, 10, 11, 12 };
	hf.load_from_raw_buffer(heights);
	return hf;
}

rove::matrix<4,4> identity_matrix()
{
	rove::matrix<4,4> tf;
	tf.identity();
	return tf;
}
}

BOOST_AUTO_TEST_CASE(the_constructor_takes_columns_then_rows)
{
	rove::heightfield<rove::scalar> hf(identity_matrix(), 3, 2);
	BOOST_TEST(hf.ncols() == 3u);
	BOOST_TEST(hf.nrows() == 2u);
	BOOST_TEST(hf.get_local_aabb().hi.x == 2.0f);
	BOOST_TEST(hf.get_local_aabb().hi.z == 1.0f);
}

BOOST_AUTO_TEST_CASE(y_under_interpolates_along_z_with_the_row)
{
	auto hf = wide_field(identity_matrix());
	// in the cell of column 1: k1 = 0.5, k2 = 0.25 -> 1.5 + 0.25 * 10
	BOOST_TEST(hf.y_under(rove::vec<3>(1.5f, 0, 0.25f)) == 4.0f, boost::test_tools::tolerance(1e-5f));
	rove::heightfield<rove::scalar> tall(identity_matrix(), 2, 3);
	static rove::scalar const heights[] = { 0, 1, 10, 11, 20, 21 };
	tall.load_from_raw_buffer(heights);
	// column 0, row 1: k1 = 0.5, k2 = 0.5 -> (10.5 + 20.5) / 2
	BOOST_TEST(tall.y_under(rove::vec<3>(0.5f, 0, 1.5f)) == 15.5f, boost::test_tools::tolerance(1e-5f));
}

BOOST_AUTO_TEST_CASE(y_under_off_the_near_edges_is_the_default)
{
	auto hf = wide_field(identity_matrix());
	float const none = -123;
	BOOST_TEST(hf.y_under(rove::vec<3>(-0.5f, 0, 0.5f), none) == none);
	BOOST_TEST(hf.y_under(rove::vec<3>(0.5f, 0, -0.5f), none) == none);
	BOOST_TEST(hf.max_y_in_cell(rove::vec<3>(-0.5f, 0, 0.5f), none) == none);
	BOOST_TEST(hf.world_position_to_cell(rove::vec<3>(-0.5f, 0, -0.5f)).x == -1);
	BOOST_TEST(hf.world_position_to_cell(rove::vec<3>(-0.5f, 0, -0.5f)).y == -1);
}

BOOST_AUTO_TEST_CASE(trace_hits_on_a_field_that_is_not_square_at_the_right_height)
{
	auto hf = wide_field(identity_matrix());
	// over column 1 (x = 1.5), rows 0..1 (z = 0.25)
	auto ci = hf.trace(rove::ray<3>(rove::vec<3>(1.5f, 100, 0.25f), rove::vec<3>(0, -1, 0)));
	BOOST_REQUIRE(ci.happened);
	BOOST_TEST(ci.position.y == hf.y_under(rove::vec<3>(1.5f, 0, 0.25f)), boost::test_tools::tolerance(0.01f));
}

BOOST_AUTO_TEST_CASE(trace_hits_from_a_start_inside_the_box_of_the_field)
{
	auto hf = wide_field(identity_matrix());
	// the field's box goes up to y = 13: the ray starts inside it
	auto ci = hf.trace(rove::ray<3>(rove::vec<3>(0.25f, 12.5f, 0.25f), rove::vec<3>(0, -1, 0)));
	BOOST_REQUIRE(ci.happened);
	BOOST_TEST(ci.position.y == hf.y_under(rove::vec<3>(0.25f, 0, 0.25f)), boost::test_tools::tolerance(0.01f));
}

BOOST_AUTO_TEST_CASE(trace_answers_in_the_terms_of_the_ray_it_was_given)
{
	// a field turned a quarter around x, moved and scaled: local y is world -z, scaled by 2
	rove::matrix<4,4> rotate, scale, move;
	rotate.identity();
	rotate.ij[1][1] = 0; rotate.ij[1][2] = -1;
	rotate.ij[2][1] = 1; rotate.ij[2][2] = 0;
	scale.identity();
	scale.ij[0][0] = scale.ij[1][1] = scale.ij[2][2] = 2;
	move.identity();
	move.ij[3][0] = 5;
	rove::matrix<4,4> const tf = rotate * scale * move;
	rove::heightfield<rove::scalar> flat(tf, 3, 3);
	static rove::scalar const zeros[9] = {};
	flat.load_from_raw_buffer(zeros);

	rove::vec<3> const local_target(1.25f, 0, 0.75f);
	rove::vec<3> const target = tf * local_target;
	rove::vec<3> const up = tf * rove::vec<3>(1.25f, 0.5f, 0.75f) - target;
	for (float const speed : {1.0f, 4.0f}) {
		rove::ray<3> const r(target + up * 3.0f, normalize(up) * -speed);
		auto ci = flat.trace(r);
		BOOST_REQUIRE(ci.happened);
		BOOST_TEST((ci.position - target).length() < 1e-3f);
		// a world-space normal of unit length, facing the ray
		BOOST_TEST(ci.normal.length() == 1.0f, boost::test_tools::tolerance(1e-4f));
		BOOST_TEST(dot(ci.normal, normalize(up)) == 1.0f, boost::test_tools::tolerance(1e-4f));
		// the time of the ray it was given: r0 + a * time is the position
		BOOST_TEST((r.apply(ci.time) - ci.position).length() < 1e-3f);
	}
}

BOOST_AUTO_TEST_CASE(trace_of_a_field_with_no_cells_misses)
{
	rove::heightfield<rove::scalar> hf(identity_matrix(), 1, 3);
	auto ci = hf.trace(rove::ray<3>(rove::vec<3>(0, 10, 1), rove::vec<3>(0, -1, 0)));
	BOOST_TEST(!ci.happened);
}

BOOST_AUTO_TEST_CASE(build_path_walks_columns_and_rows_of_a_field_that_is_not_square)
{
	// 6 columns, 2 rows: x runs to 4 as a cell, y only to 0
	rove::heightfield<rove::scalar> hf(identity_matrix(), 6, 2);
	static rove::scalar const heights[12] = {};
	hf.load_from_raw_buffer(heights);
	auto const path = hf.build_path(rove::vec<3>(0.5f, 0, 0.5f), rove::vec<3>(4.5f, 0, 0.5f), 1);
	BOOST_REQUIRE(!path.empty());
	BOOST_TEST(path.back().x == 4);
	for (auto const &c : path) {
		BOOST_TEST(c.x >= 0);
		BOOST_TEST(c.x <= 4);
		BOOST_TEST(c.y == 0);
	}
	// a goal off the field in rows has no path, though it is inside by columns
	BOOST_TEST(hf.build_path(rove::vec<3>(0.5f, 0, 0.5f), rove::vec<3>(0.5f, 0, 3.5f), 1).empty());
	// the last line of samples is the edge of the field, not a cell to walk on
	BOOST_TEST(hf.build_path(rove::vec<3>(0.5f, 0, 0.5f), rove::vec<3>(5.5f, 0, 0.5f), 1).empty());
}

BOOST_AUTO_TEST_SUITE_END()
