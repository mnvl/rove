// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <string>
#include <rove/version.h>

BOOST_AUTO_TEST_SUITE(version)

BOOST_AUTO_TEST_CASE(the_macros_say_what_the_project_says)
{
	std::string const macros = std::to_string(ROVE_VERSION_MAJOR) + "." + std::to_string(ROVE_VERSION_MINOR) + "." + std::to_string(ROVE_VERSION_PATCH);
	BOOST_TEST(macros == ROVE_PROJECT_VERSION);
	BOOST_TEST(ROVE_VERSION == ROVE_VERSION_MAJOR * 10000 + ROVE_VERSION_MINOR * 100 + ROVE_VERSION_PATCH);
}

BOOST_AUTO_TEST_SUITE_END()
