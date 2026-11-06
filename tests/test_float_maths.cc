// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

// rove's templates of float compute in float: their maths called as rove calls it for a T of its own (sqrt unqualified, `using std::sqrt`
// beside it), so that a float finds std::sqrt(float) and a dfloat its own -- never C's double sqrt(double), which takes a float promoted
// and rounds the result back.  The value is the same (sqrt is correctly rounded either way); the work is not.  So this file is compiled
// with -Werror=float-conversion (CMakeLists.txt): what it instantiates does not build if a double is rounded back to a float in it.

#include <optional>

#include <boost/test/unit_test.hpp>
#include <rove/collision.h>
#include <rove/quaternion.h>
#include <rove/ray.h>
#include <rove/segment.h>
#include <rove/sphere.h>
#include <rove/vec.h>

BOOST_AUTO_TEST_SUITE (float_maths)

BOOST_AUTO_TEST_CASE (lengths_and_distances_of_floats_are_computed_in_float)
{
	BOOST_TEST(rove::vec<2>(3.0f, 4.0f).length() == 5.0f);
	BOOST_TEST(rove::vec<3>(2.0f, 3.0f, 6.0f).length() == 7.0f);
	BOOST_TEST(rove::quaternion<float>(1.0f, 2.0f, 2.0f, 4.0f).norm() == 5.0f);
	rove::segment<3> const s(rove::vec<3>(0.0f, 0.0f, 0.0f), rove::vec<3>(10.0f, 0.0f, 0.0f));
	BOOST_TEST(s.distance(rove::vec<3>(4.0f, 3.0f, 4.0f)) == 5.0f);
	rove::ray<3> const r(rove::vec<3>(0.0f, 0.0f, 0.0f), rove::vec<3>(1.0f, 0.0f, 0.0f));
	BOOST_TEST(r.distance(rove::vec<3>(7.0f, 0.0f, 2.0f)) == 2.0f);
	// a ball of 1 from x = 5 moved 6 back onto a still one of 1 at the origin: it touches at x = 2, half the way
	std::optional<rove::contact_info<3, float>> const hit = rove::first_contact(rove::sphere<3>(rove::vec<3>(0.0f, 0.0f, 0.0f), 1.0f),
		rove::sphere<3>(rove::vec<3>(5.0f, 0.0f, 0.0f), 1.0f), rove::vec<3>(-6.0f, 0.0f, 0.0f));
	BOOST_REQUIRE(hit.has_value());
	BOOST_TEST(hit->time > 0.49f);
	BOOST_TEST(hit->time < 0.51f);
}

BOOST_AUTO_TEST_SUITE_END()
