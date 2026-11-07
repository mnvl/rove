// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <type_traits>
#include <rove/scalar.h>
#include <rove/vec.h>
#include "test_tolerance.h"

BOOST_AUTO_TEST_SUITE(scalar_helpers)

// rove's math helpers live in namespace rove alongside every templated type
// in the library, so an unqualified sin()/cos()/abs() inside a
// matrix<N,N,double> or vec<N,double> resolves to them by ordinary lookup
// rather than to std::.  If they only accept `scalar` (float), every
// double-precision instantiation silently rounds its intermediate results
// to float.  These tests pin the helpers to "preserve the argument type".

namespace
{
double const ANGLE = 0.7123456789012345;
}

BOOST_AUTO_TEST_CASE(sin_preserves_double_precision)
{
	double const result = rove::sin(ANGLE);
	BOOST_REQUIRE_EQUAL(result, std::sin(ANGLE));
}

BOOST_AUTO_TEST_CASE(cos_preserves_double_precision)
{
	double const result = rove::cos(ANGLE);
	BOOST_REQUIRE_EQUAL(result, std::cos(ANGLE));
}

BOOST_AUTO_TEST_CASE(tan_preserves_double_precision)
{
	double const result = rove::tan(ANGLE);
	BOOST_REQUIRE_EQUAL(result, std::tan(ANGLE));
}

BOOST_AUTO_TEST_CASE(square_root_preserves_double_precision)
{
	double const result = rove::square_root(2.0);
	BOOST_REQUIRE_EQUAL(result, std::sqrt(2.0));
}

BOOST_AUTO_TEST_CASE(float_overloads_still_return_float)
{
	// The float path must not silently widen either: rove::scalar code
	// depends on these being exactly the float results.
	BOOST_REQUIRE_EQUAL(rove::sin(0.5f), std::sin(0.5f));
	BOOST_REQUIRE_EQUAL(rove::cos(0.5f), std::cos(0.5f));
	BOOST_REQUIRE_EQUAL(rove::square_root(2.0f), std::sqrt(2.0f));
	BOOST_REQUIRE(sizeof(rove::sin(0.5f)) == sizeof(float));
}

BOOST_AUTO_TEST_CASE(abs_does_not_flush_subnormal_doubles_to_zero)
{
	// 1e-300 is a perfectly ordinary double and zero as a float.
	BOOST_REQUIRE_EQUAL(rove::abs(-1.0e-300), 1.0e-300);
	BOOST_REQUIRE_EQUAL(rove::abs(1.0e-300), 1.0e-300);
}

BOOST_AUTO_TEST_CASE(square_and_cube_do_not_underflow_doubles)
{
	// float(1e-30)^2 underflows to zero; in double it is ~1e-60.
	// Compare against the double product itself, not the literal 1e-60 --
	// they are neighbouring doubles, and the point here is the exponent
	// range, not which of the two the multiply lands on.
	double const tiny = 1.0e-30;
	BOOST_REQUIRE_EQUAL(rove::square(tiny), tiny * tiny);
	BOOST_REQUIRE_EQUAL(rove::cube(tiny), tiny * tiny * tiny);
	BOOST_REQUIRE(rove::square(tiny) > 0.0);
	BOOST_REQUIRE(rove::cube(tiny) > 0.0);
	// ...and overflow at the top end: float(1e30)^2 is inf.
	double const huge = 1.0e30;
	BOOST_REQUIRE(std::isfinite(rove::square(huge)));
	BOOST_REQUIRE_EQUAL(rove::square(huge), huge * huge);
}

BOOST_AUTO_TEST_CASE(abs_and_square_still_exact_for_float)
{
	BOOST_REQUIRE_EQUAL(rove::abs(-2.5f), 2.5f);
	BOOST_REQUIRE_EQUAL(rove::square(1.5f), 2.25f);
	BOOST_REQUIRE_EQUAL(rove::cube(1.5f), 3.375f);
	BOOST_REQUIRE(sizeof(rove::square(1.5f)) == sizeof(float));
}

BOOST_AUTO_TEST_CASE(sign_of_zero_is_zero)
{
	BOOST_REQUIRE_EQUAL(rove::sign(0.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-0.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(0.0), 0.0);
}

BOOST_AUTO_TEST_CASE(sign_is_zero_within_epsilon_either_side)
{
	// The default dead zone is scalar_traits<T>::default_epsilon; a hair above zero is not negative.
	BOOST_REQUIRE_EQUAL(rove::sign(5e-7f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-5e-7f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(rove::scalar_traits<float>::default_epsilon), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-rove::scalar_traits<float>::default_epsilon), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(2e-6f), 1.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-2e-6f), -1.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-3.0f), -1.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(3.0f), 1.0f);
}

BOOST_AUTO_TEST_CASE(sign_honours_the_epsilon_it_is_given)
{
	BOOST_REQUIRE_EQUAL(rove::sign(0.5f, 1.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-0.5f, 1.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(1.0f, 1.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(1.5f, 1.0f), 1.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-1.5f, 1.0f), -1.0f);
	// A zero epsilon is the plain sign, down to the smallest double.
	BOOST_REQUIRE_EQUAL(rove::sign(1e-300, 0.0), 1.0);
	BOOST_REQUIRE_EQUAL(rove::sign(-1e-300, 0.0), -1.0);
	BOOST_REQUIRE_EQUAL(rove::sign(0.0, 0.0), 0.0);
}

BOOST_AUTO_TEST_CASE(scalar_traits_are_per_precision)
{
	// double works to a finer tolerance than float, integers to none
	static_assert(rove::scalar_traits<double>::default_epsilon < rove::scalar_traits<float>::default_epsilon);
	static_assert(rove::scalar_traits<float>::zero_length_sq == rove::scalar_traits<float>::zero_length * rove::scalar_traits<float>::zero_length);
	static_assert(rove::scalar_traits<double>::zero_length_sq == rove::scalar_traits<double>::zero_length * rove::scalar_traits<double>::zero_length);
	static_assert(rove::scalar_traits<int>::default_epsilon == 0);
	static_assert(std::is_same_v<decltype(rove::scalar_traits<double>::default_epsilon), double const>);
	BOOST_REQUIRE_EQUAL(rove::sign(1e-9), 1.0);        // a float's dead zone would call it zero
}

BOOST_AUTO_TEST_CASE(sign_of_infinity_and_nan)
{
	float const inf = std::numeric_limits<float>::infinity();
	BOOST_REQUIRE_EQUAL(rove::sign(inf), 1.0f);
	BOOST_REQUIRE_EQUAL(rove::sign(-inf), -1.0f);
	// NaN has no sign; it must not come out as one.
	BOOST_REQUIRE(std::isnan(rove::sign(std::numeric_limits<float>::quiet_NaN())));
	BOOST_REQUIRE(std::isnan(rove::sign(std::numeric_limits<double>::quiet_NaN())));
	static_assert(std::is_same_v<decltype(rove::sign(1.0)), double>);
}

BOOST_AUTO_TEST_CASE(pi_is_the_correctly_rounded_constant)
{
	BOOST_REQUIRE_EQUAL(rove::PI, std::numbers::pi_v<float>);
	BOOST_REQUIRE_EQUAL(rove::TWO_PI, 2 * std::numbers::pi_v<float>);
	// Within half a float ulp of pi: no hand-typed digits short of it.
	double const half_ulp = (std::nextafter(rove::PI, 4.0f) - rove::PI) / 2.0;
	BOOST_REQUIRE(std::abs(double(rove::PI) - std::numbers::pi) <= half_ulp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(angle_wrap)

// The contract: wrap(x, lo, hi) is x moved by whole periods (hi - lo) into
// [lo, hi); wrap_to_pi() into (-pi, pi], wrap_to_two_pi() into [0, 2 pi).
// An angle already in range is not touched, a NaN or an infinity comes out
// NaN, and a double is wrapped with a double pi.

namespace
{

// x - r, in whole periods; fmod is exact, so for the wraps that only ever
// add or take off a period exactly this is an integer to the last bit.
double turns(double x, double r, double period)
{
	return (x - r) / period;
}

template<class T> void
require_in_pi_range(T r)
{
	T const pi = std::numbers::pi_v<T>;
	BOOST_REQUIRE_MESSAGE(r > -pi && r <= pi, "wrap_to_pi gave " << r << ", outside (-pi, pi]");
}

template<class T> void
require_in_two_pi_range(T r)
{
	T const two_pi = 2 * std::numbers::pi_v<T>;
	BOOST_REQUIRE_MESSAGE(r >= 0 && r < two_pi, "wrap_to_two_pi gave " << r << ", outside [0, 2 pi)");
}

}

BOOST_AUTO_TEST_CASE(wrap_to_pi_leaves_an_angle_in_range_bit_for_bit)
{
	// fmod(x + pi, 2 pi) - pi would round 1e-9 away to zero.
	for (float a : {0.0f, 1e-30f, -1e-30f, 1e-9f, -1e-9f, 0.5f, -0.5f, 3.0f, -3.0f,
			std::nextafter(-rove::PI, 0.0f)}) {
		BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(a), a);
		BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(double(a)), double(a));
	}
	BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(rove::PI), rove::PI);
	BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(std::numbers::pi), std::numbers::pi);
	// The float pi rounds up past the double one: as a double it is out of
	// range, just past -pi.
	BOOST_REQUIRE(rove::wrap_to_pi(double(rove::PI)) < -3.14159);
}

BOOST_AUTO_TEST_CASE(wrap_to_pi_is_half_open_at_minus_pi)
{
	BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(-rove::PI), rove::PI);
	BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(rove::PI), rove::PI);
	BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(-std::numbers::pi), std::numbers::pi);
	// Just past pi is just past -pi.
	float const past = std::nextafter(rove::PI, 4.0f);
	float const r = rove::wrap_to_pi(past);
	require_in_pi_range(r);
	BOOST_REQUIRE_EQUAL(turns(past, r, rove::TWO_PI), 1.0);
}

BOOST_AUTO_TEST_CASE(wrap_to_pi_takes_off_exactly_whole_turns)
{
	for (int k = -1000; k <= 1000; ++k) {
		for (float a : {-3.0f, -1.0f, -1e-6f, 0.0f, 1e-6f, 0.25f, 3.0f}) {
			float const x = a + float(k) * rove::TWO_PI;
			float const r = rove::wrap_to_pi(x);
			require_in_pi_range(r);
			double const n = turns(x, r, rove::TWO_PI);
			BOOST_REQUIRE_MESSAGE(n == std::round(n), "wrap_to_pi(" << x << ") = " << r
				<< " is " << n << " turns off, not a whole number");
			// ...and it is a's own residue, up to the rounding of x.
			float const ulp = std::nextafter(std::abs(x), INFINITY) - std::abs(x);
			BOOST_REQUIRE_MESSAGE(std::abs(r - a) <= ulp, "wrap_to_pi(" << x << ") = " << r << ", expected ~" << a);
		}
	}
}

BOOST_AUTO_TEST_CASE(wrap_to_pi_of_a_double_uses_a_double_pi)
{
	static_assert(std::is_same_v<decltype(rove::wrap_to_pi(0.1)), double>);
	static_assert(std::is_same_v<decltype(rove::wrap_to_pi(0.1f)), float>);
	// Ten turns of a float pi is ~1e-6 off ten turns of the real one.
	double const x = 0.1 + 10 * 2 * std::numbers::pi;
	BOOST_REQUIRE(std::abs(rove::wrap_to_pi(x) - 0.1) < 1e-13);
	BOOST_REQUIRE(std::abs(rove::wrap_to_two_pi(x) - 0.1) < 1e-13);
}

BOOST_AUTO_TEST_CASE(huge_angles_wrap_in_one_step)
{
	// A loop of `a -= 2 pi` never moves 1e30 at all.
	for (float a : {1e30f, -1e30f, std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()}) {
		require_in_pi_range(rove::wrap_to_pi(a));
		require_in_two_pi_range(rove::wrap_to_two_pi(a));
	}
	require_in_pi_range(rove::wrap_to_pi(1e300));
	require_in_two_pi_range(rove::wrap_to_two_pi(-1e300));
}

BOOST_AUTO_TEST_CASE(non_finite_angles_come_out_nan)
{
	for (float a : {std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::quiet_NaN()}) {
		BOOST_REQUIRE(std::isnan(rove::wrap_to_pi(a)));
		BOOST_REQUIRE(std::isnan(rove::wrap_to_two_pi(a)));
		BOOST_REQUIRE(std::isnan(rove::wrap(a, -180.0f, 180.0f)));
	}
}

BOOST_AUTO_TEST_CASE(wrap_to_two_pi_leaves_an_angle_in_range_bit_for_bit)
{
	for (float a : {0.0f, 1e-30f, 1e-9f, 0.5f, rove::PI, 6.0f, std::nextafter(rove::TWO_PI, 0.0f)}) {
		BOOST_REQUIRE_EQUAL(rove::wrap_to_two_pi(a), a);
		BOOST_REQUIRE_EQUAL(rove::wrap_to_two_pi(double(a)), double(a));
	}
}

BOOST_AUTO_TEST_CASE(wrap_to_two_pi_never_returns_two_pi)
{
	BOOST_REQUIRE_EQUAL(rove::wrap_to_two_pi(rove::TWO_PI), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::wrap_to_two_pi(-rove::TWO_PI), 0.0f);
	// -1e-9 + 2 pi rounds to 2 pi in float; the wrapped bearing must still
	// sort first in a sweep, not after every other beam.
	for (float a : {-1e-9f, -1e-30f, -std::numeric_limits<float>::denorm_min()}) {
		float const r = rove::wrap_to_two_pi(a);
		require_in_two_pi_range(r);
		BOOST_REQUIRE_MESSAGE(r == 0.0f || r > 6.0f, "wrap_to_two_pi(" << a << ") = " << r);
	}
	// A hair above -2 pi is a hair above zero, exactly.
	float const a = std::nextafter(-rove::TWO_PI, 0.0f);
	BOOST_REQUIRE_EQUAL(rove::wrap_to_two_pi(a), a + rove::TWO_PI);
	BOOST_REQUIRE(rove::wrap_to_two_pi(a) > 0.0f);
}

BOOST_AUTO_TEST_CASE(wrap_to_two_pi_takes_off_whole_turns)
{
	for (int k = -1000; k <= 1000; ++k) {
		for (float a : {0.0f, 1e-6f, 0.25f, 3.0f, 6.28f}) {
			float const x = a + float(k) * rove::TWO_PI;
			float const r = rove::wrap_to_two_pi(x);
			require_in_two_pi_range(r);
			// A negative x is shifted up by a period, which rounds.
			double const n = turns(x, r, rove::TWO_PI);
			BOOST_REQUIRE_MESSAGE(std::abs(n - std::round(n)) < 1e-6, "wrap_to_two_pi(" << x << ") = " << r);
		}
	}
	BOOST_REQUIRE_CLOSE(rove::wrap_to_two_pi(-rove::PI / 2), 3 * rove::PI / 2, 1e-4);
}

BOOST_AUTO_TEST_CASE(wrap_moves_by_whole_periods_into_lo_hi)
{
	BOOST_REQUIRE_EQUAL(rove::wrap(370.0f, -180.0f, 180.0f), 10.0f);
	BOOST_REQUIRE_EQUAL(rove::wrap(-190.0f, -180.0f, 180.0f), 170.0f);
	BOOST_REQUIRE_EQUAL(rove::wrap(-180.0f, -180.0f, 180.0f), -180.0f);
	BOOST_REQUIRE_EQUAL(rove::wrap(180.0f, -180.0f, 180.0f), -180.0f);
	BOOST_REQUIRE_EQUAL(rove::wrap(725.0, 0.0, 360.0), 5.0);
	BOOST_REQUIRE_EQUAL(rove::wrap(-725.0, 0.0, 360.0), 355.0);
	BOOST_REQUIRE_EQUAL(rove::wrap(-1.0f, -1.0f, 3.0f), -1.0f);
	BOOST_REQUIRE_EQUAL(rove::wrap(3.0f, -1.0f, 3.0f), -1.0f);
}

BOOST_AUTO_TEST_CASE(wrap_stays_half_open_when_the_shift_rounds_onto_hi)
{
	// Just below lo = -1, plus the period 4, is 3 - 2^-23: halfway between
	// two floats, and the tie goes to 3 = hi.
	float const x = std::nextafter(-1.0f, -2.0f);
	float const r = rove::wrap(x, -1.0f, 3.0f);
	BOOST_REQUIRE_MESSAGE(r >= -1.0f && r < 3.0f, "wrap(" << x << ", -1, 3) = " << r);
}

BOOST_AUTO_TEST_CASE(wrap_refuses_a_range_without_zero)
{
	// fmod keeps x's sign: one period's shift only reaches a range that
	// holds zero.  [10, 12) would silently hand back 1 for 11.
	BOOST_CHECK_THROW(rove::wrap(11.0f, 10.0f, 12.0f), std::invalid_argument);
	BOOST_CHECK_THROW(rove::wrap(-2.0f, -3.0f, -1.0f), std::invalid_argument);
	BOOST_CHECK_THROW(rove::wrap(0.5f, 0.0f, 0.0f), std::invalid_argument);
	BOOST_CHECK_THROW(rove::wrap(0.5f, 1.0f, -1.0f), std::invalid_argument);
	BOOST_CHECK_THROW(rove::wrap(0.5f, std::numeric_limits<float>::quiet_NaN(), 1.0f), std::invalid_argument);
	BOOST_CHECK_THROW(rove::wrap(0.5f, 0.0f, std::numeric_limits<float>::quiet_NaN()), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(scalar_shaping_and_motion)

BOOST_AUTO_TEST_CASE(smoothstep_is_0_and_1_at_the_edges_and_flat_outside)
{
	BOOST_REQUIRE_EQUAL(rove::smoothstep(2.0f, 4.0f, 2.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::smoothstep(2.0f, 4.0f, 4.0f), 1.0f);
	BOOST_REQUIRE_EQUAL(rove::smoothstep(2.0f, 4.0f, 3.0f), 0.5f);
	BOOST_REQUIRE_EQUAL(rove::smoothstep(2.0f, 4.0f, -100.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::smoothstep(2.0f, 4.0f, 100.0f), 1.0f);
	BOOST_REQUIRE(std::abs(rove::smoothstep(0.0, 1.0, 0.25) - 0.15625) < 1e-15);     // 3t^2 - 2t^3
	// edges the other way round run the other way
	BOOST_REQUIRE_EQUAL(rove::smoothstep(4.0f, 2.0f, 2.0f), 1.0f);
	BOOST_REQUIRE(std::abs(rove::smoothstep(4.0, 2.0, 2.5) - (1 - rove::smoothstep(2.0, 4.0, 2.5))) < 1e-15);
	// rising everywhere between
	for (int k = 1; k < 100; ++k)
		BOOST_REQUIRE(rove::smoothstep(0.0f, 1.0f, k / 100.0f) > rove::smoothstep(0.0f, 1.0f, (k - 1) / 100.0f));
	BOOST_REQUIRE_THROW(rove::smoothstep(1.0f, 1.0f, 1.0f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(inverse_lerp_and_remap_are_unclamped)
{
	BOOST_REQUIRE_EQUAL(rove::inverse_lerp(10.0f, 20.0f, 15.0f), 0.5f);
	BOOST_REQUIRE_EQUAL(rove::inverse_lerp(10.0f, 20.0f, 30.0f), 2.0f);
	BOOST_REQUIRE_EQUAL(rove::inverse_lerp(20.0, 10.0, 15.0), 0.5);
	BOOST_REQUIRE_EQUAL(rove::lerp(3.0, 7.0, rove::inverse_lerp(3.0, 7.0, 6.0)), 6.0);
	BOOST_REQUIRE_EQUAL(rove::remap(5.0f, 0.0f, 10.0f, 100.0f, 200.0f), 150.0f);
	BOOST_REQUIRE_EQUAL(rove::remap(-5.0f, 0.0f, 10.0f, 100.0f, 200.0f), 50.0f);
	BOOST_REQUIRE_EQUAL(rove::remap(5.0f, 0.0f, 10.0f, 1.0f, -1.0f), 0.0f);
	BOOST_REQUIRE_THROW(rove::inverse_lerp(1.0f, 1.0f, 1.0f), std::invalid_argument);
	BOOST_REQUIRE_THROW(rove::remap(1.0f, 2.0f, 2.0f, 0.0f, 1.0f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(radians_and_degrees)
{
	BOOST_REQUIRE_EQUAL(rove::radians(180.0), std::numbers::pi);
	BOOST_REQUIRE_EQUAL(rove::radians(180.0f), std::numbers::pi_v<float>);
	BOOST_REQUIRE_EQUAL(rove::degrees(std::numbers::pi), 180.0);
	BOOST_REQUIRE(std::abs(rove::degrees(rove::radians(37.5)) - 37.5) < 1e-12);
	BOOST_REQUIRE_EQUAL(rove::radians(-90.0f), -std::numbers::pi_v<float> / 2);
}

BOOST_AUTO_TEST_CASE(fract_is_what_floor_leaves)
{
	BOOST_REQUIRE_EQUAL(rove::fract(2.25), 0.25);
	BOOST_REQUIRE_EQUAL(rove::fract(-0.25), 0.75);           // not -0.25: from floor, not trunc
	BOOST_REQUIRE_EQUAL(rove::fract(-3.0f), 0.0f);
	BOOST_REQUIRE_EQUAL(rove::fract(0.0f), 0.0f);
}

BOOST_AUTO_TEST_CASE(damp_goes_the_same_way_whatever_the_steps)
{
	// two steps of dt/2 end where one step of dt does: the frame rate does not change where it goes
	double one = 0, two = 0;
	one = rove::damp(one, 10.0, 3.0, 0.1);
	two = rove::damp(rove::damp(two, 10.0, 3.0, 0.05), 10.0, 3.0, 0.05);
	BOOST_REQUIRE(std::abs(one - two) < 1e-12);
	BOOST_REQUIRE(std::abs(one - 10 * (1 - std::exp(-0.3))) < 1e-12);
	// a hundred small steps against ten large
	double small = 0, large = 0;
	for (int k = 0; k < 100; ++k) small = rove::damp(small, 1.0, 2.0, 0.01);
	for (int k = 0; k < 10; ++k) large = rove::damp(large, 1.0, 2.0, 0.1);
	BOOST_REQUIRE(std::abs(small - large) < 1e-12);
	// no time, no change; it never overshoots
	BOOST_REQUIRE_EQUAL(rove::damp(3.0f, 10.0f, 5.0f, 0.0f), 3.0f);
	BOOST_REQUIRE(rove::damp(3.0f, 10.0f, 1000.0f, 1.0f) <= 10.0f);
	BOOST_REQUIRE_EQUAL(rove::damp(3.0f, 10.0f, 1000.0f, 1.0f), 10.0f);
	// vectors too
	rove::vec<3> const v = rove::damp(rove::vec<3>(0, 0, 0), rove::vec<3>(10, -10, 0), 3.0f, 0.1f);
	BOOST_REQUIRE(std::abs(v.x - 10 * (1 - std::exp(-0.3f))) < 1e-5f);
	BOOST_REQUIRE_EQUAL(v.x, -v.y);
	BOOST_REQUIRE_THROW(rove::damp(0.0f, 1.0f, -1.0f, 0.1f), std::invalid_argument);
	BOOST_REQUIRE_THROW(rove::damp(0.0f, 1.0f, 1.0f, -0.1f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(move_towards_steps_at_most_so_far_and_lands_exactly)
{
	BOOST_REQUIRE_EQUAL(rove::move_towards(0.0f, 10.0f, 3.0f), 3.0f);
	BOOST_REQUIRE_EQUAL(rove::move_towards(0.0f, -10.0f, 3.0f), -3.0f);
	BOOST_REQUIRE_EQUAL(rove::move_towards(9.0f, 10.0f, 3.0f), 10.0f);           // lands, not past
	BOOST_REQUIRE_EQUAL(rove::move_towards(10.0f, 10.0f, 3.0f), 10.0f);
	rove::vec<2> const p = rove::move_towards(rove::vec<2>(0, 0), rove::vec<2>(3, 4), 2.5f);
	BOOST_REQUIRE(rove::abs(p.x - 1.5f) < TOLERANCE && rove::abs(p.y - 2) < TOLERANCE);
	BOOST_REQUIRE(rove::move_towards(rove::vec<2>(0, 0), rove::vec<2>(3, 4), 5.0f) == rove::vec<2>(3, 4));
	BOOST_REQUIRE(rove::move_towards(rove::vec<2>(0, 0), rove::vec<2>(3, 4), 50.0f) == rove::vec<2>(3, 4));
	BOOST_REQUIRE_THROW(rove::move_towards(0.0f, 1.0f, -1.0f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(a_critically_damped_spring_follows_its_exact_curve)
{
	// from rest, a unit away from the target: x(t) = (1 + w t) e^{-w t}, never past the target
	double x = 1, v = 0;
	double const w = 4;
	for (int k = 1; k <= 200; ++k) {
		rove::critically_damped(x, v, 0.0, w, 0.01);
		double const t = k * 0.01;
		BOOST_REQUIRE(std::abs(x - (1 + w * t) * std::exp(-w * t)) < 1e-12);
		BOOST_REQUIRE(x > 0);
	}
	// one step of 0.2 against twenty of 0.01, from a moving start: the same, the curve is exact
	double x1 = 2, v1 = -3, x2 = 2, v2 = -3;
	rove::critically_damped(x1, v1, 0.5, 6.0, 0.2);
	for (int k = 0; k < 20; ++k) rove::critically_damped(x2, v2, 0.5, 6.0, 0.01);
	BOOST_REQUIRE(std::abs(x1 - x2) < 1e-12);
	BOOST_REQUIRE(std::abs(v1 - v2) < 1e-12);
	// vectors, and no time is no change
	rove::vec<3> p(1, 2, 3), u(0, 0, 0);
	rove::critically_damped(p, u, rove::vec<3>(0, 0, 0), 2.0f, 0.0f);
	BOOST_REQUIRE(p == rove::vec<3>(1, 2, 3) && u == rove::vec<3>(0, 0, 0));
	rove::critically_damped(p, u, rove::vec<3>(0, 0, 0), 2.0f, 10.0f);
	BOOST_REQUIRE(p.length() < 1e-5f);
	BOOST_REQUIRE_THROW(rove::critically_damped(x, v, 0.0, -1.0, 0.1), std::invalid_argument);
	BOOST_REQUIRE_THROW(rove::critically_damped(x, v, 0.0, 1.0, -0.1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(delta_angle_is_the_short_turn_across_the_seam)
{
	float const pi = std::numbers::pi_v<float>;
	BOOST_REQUIRE(std::abs(rove::delta_angle(0.1f, 0.4f) - 0.3f) < 1e-6f);
	BOOST_REQUIRE(std::abs(rove::delta_angle(pi - 0.1f, -pi + 0.1f) - 0.2f) < 1e-5f);       // across the seam at west
	BOOST_REQUIRE(std::abs(rove::delta_angle(-pi + 0.1f, pi - 0.1f) + 0.2f) < 1e-5f);
	BOOST_REQUIRE(std::abs(rove::delta_angle(0.0f, 5 * pi / 2) - pi / 2) < 1e-5f);
	BOOST_REQUIRE_EQUAL(rove::delta_angle(1.0, 1.0), 0.0);
}

// For code templated on the scalar, as rove::sin: the standard's for float and double, dfloat's own by argument lookup
BOOST_AUTO_TEST_CASE(isfinite_isnan_and_floor_of_either_precision)
{
	float const inf = std::numeric_limits<float>::infinity(), nan = std::numeric_limits<float>::quiet_NaN();
	BOOST_TEST(rove::isfinite(1.5f));
	BOOST_TEST(!rove::isfinite(inf));
	BOOST_TEST(!rove::isfinite(-inf));
	BOOST_TEST(!rove::isfinite(nan));
	BOOST_TEST(rove::isfinite(1.5));
	BOOST_TEST(!rove::isfinite(std::numeric_limits<double>::infinity()));
	BOOST_TEST(rove::isnan(nan));
	BOOST_TEST(!rove::isnan(inf));
	BOOST_TEST(!rove::isnan(0.0f));
	BOOST_TEST(rove::isnan(std::numeric_limits<double>::quiet_NaN()));
	BOOST_TEST(rove::floor(-0.5f) == -1.0f);
	BOOST_TEST(rove::floor(2.5) == 2.0);
}

BOOST_AUTO_TEST_SUITE_END()
