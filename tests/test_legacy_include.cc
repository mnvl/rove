// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
// Spelled the old way, on purpose: it has to keep compiling whatever the checkout's directory is called
#include <rove/src/vec.h>
#include <rove/src/matrix.h>
#include <rove/src/quaternion.h>
#include <rove/src/scalar.h>
#include <rove/src/segment.h>

BOOST_AUTO_TEST_SUITE(legacy_include)

BOOST_AUTO_TEST_CASE(rove_src_headers_are_the_same_headers)
{
	rove::vec<3> const v(1, 2, 3);
	BOOST_TEST(dot(v, v) == 14.0f);
}

BOOST_AUTO_TEST_SUITE_END()
