// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

// rove::dfloat: float maths that gives the same bits on every platform.  The values written below are the contract: the correctly rounded
// results (the double maths library's, rounded to float, taken once and written here), which a build for ARM or WebAssembly must give
// too.  Against the platform's double maths over many arguments, each function is within one ulp and almost always exact.

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <rove/capsule.h>
#include <rove/collision.h>
#include <rove/dfloat.h>
#include <rove/matrix.h>
#include <rove/noise.h>
#include <rove/random.h>
#include <rove/quaternion.h>
#include <rove/scalar.h>
#include <rove/sphere.h>
#include <rove/vec.h>
#include <rove/aabb.h>
#include <rove/obb.h>
#include <rove/frustum.h>
#include <rove/plane.h>
#include <rove/triangle.h>
#include <rove/cone.h>
#include <rove/ray.h>
#include <rove/octree.h>
#include <rove/hash_grid.h>
#include <rove/traits.h>

namespace
{

using rove::dfloat;

// The functions of dfloat (found by argument lookup) on floats.
float d_sin(float x) { return float(sin(dfloat(x))); }
float d_cos(float x) { return float(cos(dfloat(x))); }
float d_tan(float x) { return float(tan(dfloat(x))); }
float d_asin(float x) { return float(asin(dfloat(x))); }
float d_acos(float x) { return float(acos(dfloat(x))); }
float d_atan(float x) { return float(atan(dfloat(x))); }
float d_atan2(float y, float x) { return float(atan2(dfloat(y), dfloat(x))); }
float d_exp(float x) { return float(exp(dfloat(x))); }
float d_log(float x) { return float(log(dfloat(x))); }
float d_pow(float x, float y) { return float(pow(dfloat(x), dfloat(y))); }

float of_bits(std::uint32_t b) { return std::bit_cast<float>(b); }
template<class T> concept takes_std_sin = requires(T x) { std::sin(x); };
template<class T> concept takes_std_sqrt = requires(T x) { std::sqrt(x); };
template<class T> concept takes_a_double = requires(T x) { x * 0.5; };
template<class T> concept does_its_own = requires(T x) { x * 0.5f; x + 1; sin(x); atan2(x, x); pow(x, 2); exp(x); fmod(x, x); };

std::uint32_t bits(float f) { return std::bit_cast<std::uint32_t>(f); }

// The floats in order as integers: -0 and +0 one apart, the distance between two floats in ulps.
std::int64_t ordered(float f)
{
	std::int32_t const i = std::bit_cast<std::int32_t>(f);
	return i >= 0 ? std::int64_t(i) : -std::int64_t(i & 0x7fffffff) - 1;
}
std::int64_t ulps(float a, float b) { return std::llabs(ordered(a) - ordered(b)); }

float const NaN = std::numeric_limits<float>::quiet_NaN();
float const INF = std::numeric_limits<float>::infinity();

struct Golden
{
	char const *fn;
	std::uint32_t x, result;
};

float call(char const *fn, float x)
{
	std::string const f = fn;
	if (f == "sin") return d_sin(x);
	if (f == "cos") return d_cos(x);
	if (f == "tan") return d_tan(x);
	if (f == "atan") return d_atan(x);
	if (f == "asin") return d_asin(x);
	if (f == "acos") return d_acos(x);
	if (f == "exp") return d_exp(x);
	if (f == "log") return d_log(x);
	throw std::invalid_argument(f);
}

/// The float nearest the true value: the platform's double function, rounded.  (A double result within a double ulp of a float rounding
/// boundary can round the other way: this reference is right but for such rare cases, which the tests allow one ulp for.)
float reference(char const *fn, float x)
{
	std::string const f = fn;
	double const d = x;
	if (f == "sin") return float(std::sin(d));
	if (f == "cos") return float(std::cos(d));
	if (f == "tan") return float(std::tan(d));
	if (f == "atan") return float(std::atan(d));
	if (f == "asin") return float(std::asin(d));
	if (f == "acos") return float(std::acos(d));
	if (f == "exp") return float(std::exp(d));
	if (f == "log") return float(std::log(d));
	throw std::invalid_argument(f);
}

/// Every `stride`th float from `lo` to `hi` (both positive or both negative, by their bits).
std::vector<float> floats(float lo, float hi, std::uint32_t stride)
{
	std::vector<float> v;
	std::uint32_t a = bits(lo), b = bits(hi);
	if (a > b)
		std::swap(a, b);
	for (std::uint64_t i = a; i <= b; i += stride)
		v.push_back(of_bits(std::uint32_t(i)));
	return v;
}

/// Over `xs` and their negatives: the worst error in ulps against the reference, and the share that is not exact.
void against_reference(char const *fn, std::vector<float> const &xs, bool negatives = true)
{
	std::int64_t worst = 0;
	float worst_x = 0;
	std::size_t inexact = 0, n = 0;
	for (float x0 : xs)
		for (float x : {x0, -x0})
		{
			if (!negatives && x < 0)
				continue;
			float const r = reference(fn, x), got = call(fn, x);
			std::int64_t const e = ulps(r, got);
			++n;
			inexact += e != 0;
			if (e > worst)
				worst = e, worst_x = x;
		}
	BOOST_TEST_MESSAGE(fn << ": " << n << " arguments, worst " << worst << " ulp at " << worst_x << ", " << inexact << " not exact");
	BOOST_CHECK_MESSAGE(worst <= 1, fn << ": " << worst << " ulps off at " << worst_x);
	BOOST_CHECK_MESSAGE(double(inexact) <= 1e-4 * double(n), fn << ": " << inexact << " of " << n << " not exact");
}

}

BOOST_AUTO_TEST_SUITE(dfloat_suite)

BOOST_AUTO_TEST_CASE(gives_the_written_bits)
{
	Golden const golden[] = {
		{"sin", 0x3f000000, 0x3ef57744}, {"sin", 0x3f800000, 0x3f576aa4}, {"sin", 0x40000000, 0x3f68c7b7}, {"sin", 0x40400000, 0x3e1081c3},
		{"sin", 0xbfa00000, 0xbf72f0a8}, {"sin", 0x41200000, 0xbf0b44f8}, {"sin", 0x42c80000, 0xbf01a12e}, {"sin", 0x447a2000, 0x3f7eca46},
		{"sin", 0x47f12000, 0xbf3d8337}, {"sin", 0x501502f9, 0xbef99a64}, {"sin", 0x7f61b1e6, 0x3f5ff9c4},
		{"cos", 0x3f000000, 0x3f60a940}, {"cos", 0x3f800000, 0x3f0a5140}, {"cos", 0x40000000, 0xbed51133}, {"cos", 0x40400000, 0xbf7d7026},
		{"cos", 0xbfa00000, 0x3ea171ef}, {"cos", 0x41200000, 0xbf56cd64}, {"cos", 0x42c80000, 0x3f5cc0ee}, {"cos", 0x447a2000, 0x3dc6dffc},
		{"cos", 0x47f12000, 0xbf2c1b84}, {"cos", 0x501502f9, 0x3f5f84c5}, {"cos", 0x7f61b1e6, 0xbef7f57c},
		{"tan", 0x3f000000, 0x3f0bda7b}, {"tan", 0x3f800000, 0x3fc75923}, {"tan", 0x3fc00000, 0x41619f6b}, {"tan", 0xc0000000, 0x400bd7b1},
		{"tan", 0x41200000, 0x3f25fafa}, {"tan", 0x49742400, 0xbebf4bb4},
		{"atan", 0x3dcccccd, 0x3dcc1f14}, {"atan", 0x3e800000, 0x3e7adbb0}, {"atan", 0x3e99999a, 0x3e9539d4}, {"atan", 0x3f000000, 0x3eed6338},
		{"atan", 0x3f800000, 0x3f490fdb}, {"atan", 0x40000000, 0x3f8db70d}, {"atan", 0xc0600000, 0xbfa57088}, {"atan", 0x42c80000, 0x3fc7c82f},
		{"atan", 0x60ad78ec, 0x3fc90fdb},
		{"asin", 0x3dcccccd, 0x3dcd2494}, {"asin", 0x3f000000, 0x3f060a92}, {"asin", 0xbf333333, 0xbf468073}, {"asin", 0x3f7d70a4, 0x3fb6f1e4},
		{"asin", 0x3f800000, 0x3fc90fdb},
		{"acos", 0x3dcccccd, 0x3fbc3d91}, {"acos", 0x3f000000, 0x3f860a92}, {"acos", 0xbf333333, 0x4016280a}, {"acos", 0x3f7d70a4, 0x3e10efb5},
		{"acos", 0xbf800000, 0x40490fdb}, {"acos", 0x3f800000, 0x00000000},
		{"exp", 0x3f000000, 0x3fd3094c}, {"exp", 0x3f800000, 0x402df854}, {"exp", 0xbf800000, 0x3ebc5ab2}, {"exp", 0x40200000, 0x4142eb7f},
		{"exp", 0x41200000, 0x46ac14ee}, {"exp", 0xc1a00000, 0x310da433}, {"exp", 0x42480000, 0x638c881f}, {"exp", 0x42b00000, 0x7ef882b7},
		{"exp", 0xc2c80000, 0x0000001b},
		{"log", 0x3f000000, 0xbf317218}, {"log", 0x40000000, 0x3f317218}, {"log", 0x40400000, 0x3f8c9f54}, {"log", 0x41200000, 0x40135d8e},
		{"log", 0x3a83126f, 0xc0dd0c55}, {"log", 0x7149f2ca, 0x428a27b5}, {"log", 0x000116c2, 0xc2b834f2}, {"log", 0x40f00000, 0x4000f42c},
	};
	for (Golden const &g : golden)
	{
		float const got = call(g.fn, of_bits(g.x));
		BOOST_CHECK_MESSAGE(bits(got) == g.result, g.fn << "(" << of_bits(g.x) << ") = " << got << ", not " << of_bits(g.result));
	}
	struct Two
	{
		std::uint32_t a, b, result;
	};
	Two const atan2s[] = {{0x3f800000, 0x40000000, 0x3eed6338}, {0xbf800000, 0xc0000000, 0xc02b6374}, {0x40400000, 0xbf000000, 0x3fde3372},
		{0xbe800000, 0x40800000, 0xbd7faade}, {0x0da24260, 0xbf800000, 0x40490fdb}};
	for (Two const &t : atan2s)
		BOOST_CHECK_MESSAGE(bits(d_atan2(of_bits(t.a), of_bits(t.b))) == t.result, "atan2(" << of_bits(t.a) << ", " << of_bits(t.b) << ")");
	Two const pows[] = {{0x40000000, 0x3f000000, 0x3fb504f3}, {0x3e99999a, 0x402ccccd, 0x3d1eb42d}, {0x41200000, 0xc0400000, 0x3a83126f},
		{0xc0000000, 0x40400000, 0xc1000000}, {0xc0000000, 0x40800000, 0x41800000}, {0x3f666666, 0x42c80000, 0x37ded005},
		{0x3fc00000, 0x3f333333, 0x3faa027f}};
	for (Two const &t : pows)
		BOOST_CHECK_MESSAGE(bits(d_pow(of_bits(t.a), of_bits(t.b))) == t.result, "pow(" << of_bits(t.a) << ", " << of_bits(t.b) << ")");
}

BOOST_AUTO_TEST_CASE(sin_cos_tan_are_within_an_ulp_small_and_huge)
{
	for (char const *fn : {"sin", "cos", "tan"})
	{
		against_reference(fn, floats(1e-30f, 1e-3f, 4099));
		against_reference(fn, floats(1e-3f, 1e5f, 509));          // where angles are: a dense sweep
		against_reference(fn, floats(1e5f, 3.4e38f, 6007));       // the reduction of huge arguments
	}
}

BOOST_AUTO_TEST_CASE(atan_asin_acos_are_within_an_ulp)
{
	against_reference("atan", floats(1e-30f, 1e30f, 1499));
	against_reference("asin", floats(1e-30f, 1.0f, 997));
	against_reference("acos", floats(1e-30f, 1.0f, 997));
}

BOOST_AUTO_TEST_CASE(exp_log_are_within_an_ulp_over_their_whole_range)
{
	against_reference("exp", floats(1e-30f, 89.0f, 499));
	against_reference("exp", floats(-1e-30f, -104.0f, 499));
	against_reference("log", floats(std::numeric_limits<float>::denorm_min(), std::numeric_limits<float>::max(), 2003), false);
}

BOOST_AUTO_TEST_CASE(atan2_and_pow_are_within_an_ulp)
{
	std::mt19937 g(5);
	auto any = [&](float scale) {          // a float of any exponent within the scale, either sign
		float const m = float(g() >> 8) / 16777216.0f;
		int const e = int(g() % 60) - 30;
		return (g() & 1 ? 1.0f : -1.0f) * std::ldexp(m, e) * scale;
	};
	std::int64_t worst2 = 0, worst_pow = 0;
	std::size_t inexact2 = 0, inexact_pow = 0, n = 200000;
	for (std::size_t i = 0; i < n; ++i)
	{
		float const y = any(1), x = any(1);
		std::int64_t const e = ulps(float(std::atan2(double(y), double(x))), d_atan2(y, x));
		worst2 = std::max(worst2, e);
		inexact2 += e != 0;
		float const b = std::abs(any(1)) * 4, p = any(1) * 8;          // a positive base, an exponent of either sign
		float const want = float(std::pow(double(b), double(p)));
		if (std::isfinite(want) && want != 0)
		{
			std::int64_t const f = ulps(want, d_pow(b, p));
			worst_pow = std::max(worst_pow, f);
			inexact_pow += f != 0;
		}
	}
	BOOST_CHECK(worst2 <= 1);
	BOOST_CHECK(double(inexact2) <= 1e-4 * double(n));
	BOOST_CHECK(worst_pow <= 1);
	BOOST_CHECK(double(inexact_pow) <= 1e-4 * double(n));
	// whole powers of a negative base, as exactly as a positive one
	for (int k = -10; k <= 10; ++k)
		BOOST_CHECK(d_pow(-1.5f, float(k)) == float(std::pow(-1.5, double(k))));
}

BOOST_AUTO_TEST_CASE(odd_functions_are_odd_to_the_bit_and_keep_the_sign_of_zero)
{
	for (float x : floats(1e-20f, 1e6f, 100003))
	{
		BOOST_CHECK(bits(d_sin(-x)) == bits(-d_sin(x)));
		BOOST_CHECK(bits(d_cos(-x)) == bits(d_cos(x)));
		BOOST_CHECK(bits(d_tan(-x)) == bits(-d_tan(x)));
		BOOST_CHECK(bits(d_atan(-x)) == bits(-d_atan(x)));
	}
	for (float z : {0.0f, -0.0f})
	{
		BOOST_CHECK(bits(d_sin(z)) == bits(z));
		BOOST_CHECK(bits(d_tan(z)) == bits(z));
		BOOST_CHECK(bits(d_atan(z)) == bits(z));
		BOOST_CHECK(bits(d_asin(z)) == bits(z));
		BOOST_CHECK(d_cos(z) == 1.0f);
		BOOST_CHECK(d_exp(z) == 1.0f);
	}
}

BOOST_AUTO_TEST_CASE(monotone_across_their_seams)
{
	// where the reductions change branch: atan at tan(pi/12) and 1, exp at the halves of ln 2, log at the square root of a half
	struct Seam
	{
		char const *fn;
		float at;
	};
	for (Seam s : {Seam{"atan", 0.26794919f}, Seam{"atan", 1.0f}, Seam{"atan", 3.7320508f}, Seam{"exp", 0.34657359f}, Seam{"exp", -0.34657359f},
			 Seam{"exp", 1.0397208f}, Seam{"log", 0.70710678f}, Seam{"log", 1.4142135f}, Seam{"log", 1.0f}, Seam{"asin", 0.5f}, Seam{"acos", 0.5f}})
	{
		bool const rising = std::string(s.fn) != "acos";
		float x = s.at;
		for (int i = 0; i < 20000; ++i)
			x = std::nextafter(x, -INF);
		float last = call(s.fn, x);
		for (int i = 0; i < 40000; ++i)
		{
			x = std::nextafter(x, INF);
			float const v = call(s.fn, x);
			BOOST_CHECK_MESSAGE(rising ? v >= last : v <= last, s.fn << " turns at " << x);
			if (rising ? v < last : v > last)
				break;
			last = v;
		}
	}
}

BOOST_AUTO_TEST_CASE(special_values_are_the_standards)
{
	auto sin = d_sin; auto cos = d_cos; auto tan = d_tan; auto asin = d_asin; auto acos = d_acos; auto atan = d_atan;
	auto atan2 = d_atan2; auto exp = d_exp; auto log = d_log; auto pow = d_pow;
	for (float x : {NaN, INF, -INF})
	{
		BOOST_CHECK(std::isnan(sin(x)) && std::isnan(cos(x)) && std::isnan(tan(x)));
	}
	BOOST_CHECK(std::isnan(sin(NaN)) && std::isnan(atan(NaN)) && std::isnan(exp(NaN)) && std::isnan(log(NaN)));
	BOOST_CHECK(atan(INF) == float(M_PI / 2) && atan(-INF) == -float(M_PI / 2));
	BOOST_CHECK(std::isnan(asin(1.0000001f)) && std::isnan(acos(-1.0000001f)) && std::isnan(asin(INF)));
	BOOST_CHECK(exp(INF) == INF && exp(-INF) == 0 && exp(100.0f) == INF && exp(-110.0f) == 0);
	BOOST_CHECK(log(INF) == INF && log(0.0f) == -INF && log(-0.0f) == -INF && std::isnan(log(-1.0f)) && log(1.0f) == 0);
	// atan2: every pair of signed zeros, infinities and a finite value, as std::atan2 has them (which is C's Annex F)
	for (float y : {0.0f, -0.0f, 1.0f, -1.0f, INF, -INF, NaN})
		for (float x : {0.0f, -0.0f, 1.0f, -1.0f, INF, -INF, NaN})
		{
			float const want = std::atan2(y, x), got = atan2(y, x);
			BOOST_CHECK_MESSAGE((std::isnan(want) && std::isnan(got)) || bits(want) == bits(got), "atan2(" << y << ", " << x << ") = " << got << ", not " << want);
		}
	// pow: the special cases of Annex F, against std::pow
	for (float x : {0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 2.0f, -2.0f, INF, -INF, NaN})
		for (float y : {0.0f, -0.0f, 1.0f, -1.0f, 2.0f, -2.0f, 3.0f, -3.0f, 0.5f, -0.5f, 1e10f, -1e10f, INF, -INF, NaN})
		{
			float const want = std::pow(x, y), got = pow(x, y);
			BOOST_CHECK_MESSAGE((std::isnan(want) && std::isnan(got)) || bits(want) == bits(got), "pow(" << x << ", " << y << ") = " << got << ", not " << want);
		}
}

BOOST_AUTO_TEST_CASE(rove_functions_of_dfloat_are_its_own_and_of_float_and_double_the_libraries)
{
	for (float x : {0.3f, -2.0f, 1234.5f})
	{
		BOOST_CHECK(bits(float(rove::sin(dfloat(x)))) == bits(d_sin(x)));
		BOOST_CHECK(bits(float(rove::cos(dfloat(x)))) == bits(d_cos(x)));
		BOOST_CHECK(bits(float(rove::tan(dfloat(x)))) == bits(d_tan(x)));
		BOOST_CHECK(bits(float(rove::square_root(dfloat(std::abs(x))))) == bits(std::sqrt(std::abs(x))));
		// float and double: what they always were, std::'s
		BOOST_CHECK(bits(rove::sin(x)) == bits(std::sin(x)) && bits(rove::cos(x)) == bits(std::cos(x)) && bits(rove::tan(x)) == bits(std::tan(x)));
		BOOST_CHECK(rove::sin(double(x)) == std::sin(double(x)) && rove::square_root(std::abs(double(x))) == std::sqrt(std::abs(double(x))));
	}
	BOOST_CHECK(rove::sign(dfloat(-3.0f)) == -1 && rove::sign(dfloat(0.0f)) == 0 && isnan(rove::sign(std::numeric_limits<dfloat>::quiet_NaN())));
}

BOOST_AUTO_TEST_CASE(nothing_slips_through_to_float_or_double)
{
	static_assert(std::is_convertible_v<float, dfloat> && std::is_convertible_v<int, dfloat> && std::is_convertible_v<std::size_t, dfloat>);
	static_assert(!std::is_convertible_v<dfloat, float> && std::is_constructible_v<float, dfloat>);          // explicit only
	static_assert(!std::is_constructible_v<dfloat, double> && !std::is_constructible_v<dfloat, long double>);
	static_assert(!std::is_constructible_v<dfloat, bool>);
	static_assert(!std::is_convertible_v<dfloat, int> && std::is_constructible_v<int, dfloat>);          // explicit, truncated
	static_assert(int(dfloat(-2.7f)) == -2 && std::size_t(dfloat(3.9f)) == 3);
	static_assert(!takes_std_sin<dfloat> && takes_std_sin<float>);          // the library's maths does not take it
	static_assert(!takes_std_sqrt<dfloat> && takes_std_sqrt<float>);
	static_assert(!takes_a_double<dfloat> && takes_a_double<float>);        // nor a double
	static_assert(does_its_own<dfloat>);
	BOOST_CHECK(dfloat(7) == 7.0f && dfloat{} == 0.0f);
	for (float x : {7.5f, -7.5f, 1e6f, -0.3f})
		for (float y : {2.0f, 6.2831855f, 0.7f})
		{
			BOOST_CHECK(bits(float(remainder(dfloat(x), dfloat(y)))) == bits(std::remainder(x, y)));
			BOOST_CHECK(bits(float(fmod(dfloat(x), dfloat(y)))) == bits(std::fmod(x, y)));
		}
}

BOOST_AUTO_TEST_CASE(rove_on_dfloat_is_rove_on_float_bit_for_bit_but_for_the_transcendentals)
{
	using V = rove::vec<3, dfloat>;
	using F = rove::vec<3, float>;
	std::mt19937 g(11);
	auto any = [&] { return float(g() >> 8) / 16777216.0f * 20 - 10; };
	auto same = [](V const &a, F const &b) { return bits(float(a.x)) == bits(b.x) && bits(float(a.y)) == bits(b.y) && bits(float(a.z)) == bits(b.z); };
	for (int i = 0; i < 2000; ++i)
	{
		float const ax = any(), ay = any(), az = any(), bx = any(), by = any(), bz = any(), t = any();
		V const a(ax, ay, az), b(bx, by, bz);
		F const af(ax, ay, az), bf(bx, by, bz);
		BOOST_CHECK(same(a + b * dfloat(t), af + bf * t));
		BOOST_CHECK(same(cross(a, b), cross(af, bf)));
		BOOST_CHECK(bits(float(dot(a, b))) == bits(dot(af, bf)));
		BOOST_CHECK(bits(float(a.length())) == bits(af.length()));
		BOOST_CHECK(same(rove::direction(a), rove::direction(af)));
		// a rotation from its sine and cosine is products and sums only
		rove::matrix<3, 3, dfloat> m;
		rove::matrix<3, 3, float> mf;
		V const axis = rove::direction(b);
		m.rotation(axis, dfloat(0.6f), dfloat(0.8f));
		mf.rotation(rove::direction(bf), 0.6f, 0.8f);
		BOOST_CHECK(same(m * a, mf * af));
		// the quaternion of a rotation matrix: square roots and divisions
		rove::quaternion<dfloat> q;
		rove::quaternion<float> qf;
		q.set_unit(m);
		qf.set_unit(mf);
		BOOST_CHECK(bits(float(q.x)) == bits(qf.x) && bits(float(q.w)) == bits(qf.w));
		// a ball meeting a sphere: a quadratic, its square root
		auto const hit = rove::first_contact(rove::sphere<3, dfloat>(V(0, 0, 0), 1), rove::sphere<3, dfloat>(a * dfloat(3), dfloat(0.2f)), -a * dfloat(4));
		auto const hitf = rove::first_contact(rove::sphere<3, float>(F(0, 0, 0), 1), rove::sphere<3, float>(af * 3.0f, 0.2f), -af * 4.0f);
		BOOST_CHECK(hit.has_value() == hitf.has_value());
		if (hit && hitf)
			BOOST_CHECK(bits(float(hit->time)) == bits(hitf->time));
	}
	// two capsules touching or not: segment distances
	for (int i = 0; i < 500; ++i)
	{
		float const v[13] = {any(), any(), any(), any(), any(), any(), any(), any(), any(), any(), any(), any(), std::abs(any()) / 2};
		rove::capsule<3, dfloat> const c1(V(v[0], v[1], v[2]), V(v[3], v[4], v[5]), dfloat(v[12])), c2(V(v[6], v[7], v[8]), V(v[9], v[10], v[11]), dfloat(v[12]));
		rove::capsule<3, float> const f1(F(v[0], v[1], v[2]), F(v[3], v[4], v[5]), v[12]), f2(F(v[6], v[7], v[8]), F(v[9], v[10], v[11]), v[12]);
		BOOST_CHECK(c1.test_intersection(c2) == f1.test_intersection(f2));
	}
	// with a sine: within an ulp of float's, for it is the same formula over a sine within an ulp
	rove::matrix<3, 3, dfloat> m;
	rove::matrix<3, 3, float> mf;
	m.rotation(V(0, 1, 0), dfloat(1.1f));
	mf.rotation(F(0, 1, 0), 1.1f);
	F const r = mf * F(1, 0, 0);
	V const rd = m * V(1, 0, 0);
	BOOST_CHECK(ulps(float(rd.x), r.x) <= 1 && ulps(float(rd.z), r.z) <= 1);
	rove::quaternion<dfloat> const turn = rove::axis_angle(V(0, 0, 1), dfloat(0.9f));
	BOOST_CHECK(std::abs(float(rove::angle_between(rove::quaternion<dfloat>(0, 0, 0, 1), turn)) - 0.9f) < 1e-6f);
}

BOOST_AUTO_TEST_CASE(the_vector_helpers_work_on_dfloat_as_on_float)
{
	using rove::dfloat;
	using V = rove::vec<3, dfloat>;
	V const a(1.0f, -5.0f, 3.0f), b(-2.0f, 4.0f, 3.0f);
	BOOST_REQUIRE(rove::min(a, b) == V(-2.0f, -5.0f, 3.0f));
	BOOST_REQUIRE(rove::max(a, b) == V(1.0f, 4.0f, 3.0f));
	BOOST_REQUIRE(rove::abs(a) == V(1.0f, 5.0f, 3.0f));
	BOOST_REQUIRE(rove::clamp(a, V(0.0f, 0.0f, 0.0f), V(2.0f, 2.0f, 2.0f)) == V(1.0f, 0.0f, 2.0f));
	BOOST_REQUIRE(rove::all_less(V(0.0f, 0.0f, 0.0f), V(1.0f, 1.0f, 1.0f)));
	BOOST_REQUIRE(rove::is_finite(a));
	// the angle through dfloat's own atan2, within a rounding of float's
	rove::vec<3> const fa(1, -5, 3), fb(-2, 4, 3);
	BOOST_REQUIRE(std::abs(float(rove::angle_between(a, b)) - rove::angle_between(fa, fb)) < 1e-6f);
	BOOST_REQUIRE(rove::to_xz(a) == rove::xz<dfloat>(1.0f, 3.0f));
	BOOST_REQUIRE(rove::horizontal_length(V(3.0f, 9.0f, 4.0f)) == dfloat(5.0f));
	// keys of hashed and ordered containers
	std::unordered_set<rove::xz<dfloat>> ground{rove::xz<dfloat>(1.0f, 2.0f), rove::xz<dfloat>(2.0f, 1.0f), rove::xz<dfloat>(1.0f, 2.0f)};
	BOOST_REQUIRE_EQUAL(ground.size(), 2u);
	BOOST_REQUIRE_EQUAL(std::hash<dfloat>()(dfloat(0.0f)), std::hash<dfloat>()(dfloat(-0.0f)));
	BOOST_REQUIRE_EQUAL(std::hash<dfloat>()(dfloat(1.5f)), std::hash<float>()(1.5f));
	std::set<V> ordered{V(0.0f, 1.0f, 0.0f), V(1.0f, 0.0f, 0.0f)};
	BOOST_REQUIRE_EQUAL(ordered.size(), 2u);
}

BOOST_AUTO_TEST_CASE(wrap_and_the_scalar_helpers_work_on_dfloat)
{
	using rove::dfloat;
	// wrap was floating point only; on dfloat it is dfloat's own fmod, and float's bits where float's are exact
	for (float a : {0.0f, 1.0f, -1.0f, 3.5f, -3.5f, 7.0f, -7.0f, 100.0f, -1000.25f, 3.14159265f, -3.14159265f})
		BOOST_REQUIRE_EQUAL(rove::wrap_to_pi(dfloat(a)).value(), rove::wrap_to_pi(a));
	BOOST_REQUIRE_EQUAL(rove::wrap_to_two_pi(dfloat(-1.0f)).value(), rove::wrap_to_two_pi(-1.0f));
	BOOST_REQUIRE_EQUAL(rove::delta_angle(dfloat(3.0f), dfloat(-3.0f)).value(), rove::delta_angle(3.0f, -3.0f));
	BOOST_REQUIRE_EQUAL(rove::smoothstep(dfloat(0.0f), dfloat(2.0f), dfloat(1.0f)), dfloat(0.5f));
	BOOST_REQUIRE_EQUAL(rove::remap(dfloat(5.0f), dfloat(0.0f), dfloat(10.0f), dfloat(0.0f), dfloat(1.0f)), dfloat(0.5f));
	BOOST_REQUIRE_EQUAL(rove::fract(dfloat(-0.25f)), dfloat(0.75f));
	BOOST_REQUIRE_EQUAL(rove::radians(dfloat(180.0f)), std::numbers::pi_v<dfloat>);
	dfloat x = 1.0f, v = 0.0f;
	rove::critically_damped(x, v, dfloat(0.0f), dfloat(4.0f), dfloat(0.25f));
	BOOST_REQUIRE(std::abs(x.value() - 2.0f * std::exp(-1.0f)) < 1e-6f);
	BOOST_REQUIRE(std::abs(rove::damp(dfloat(0.0f), dfloat(1.0f), dfloat(1.0f), dfloat(1.0f)).value() - (1 - std::exp(-1.0f))) < 1e-6f);
	BOOST_REQUIRE_EQUAL(rove::move_towards(dfloat(0.0f), dfloat(10.0f), dfloat(3.0f)), dfloat(3.0f));
}

BOOST_AUTO_TEST_CASE(random_numbers_and_noise_on_dfloat)
{
	using rove::dfloat;
	rove::pcg32 a(77), b(77);
	for (int k = 0; k < 1000; ++k)
		BOOST_REQUIRE_EQUAL(a.uniform(dfloat(-1.0f), dfloat(4.0f)).value(), b.uniform(-1.0f, 4.0f));       // the same 24 bits
	// normal and unit vectors through dfloat's own log, sin and cos: what lockstep peers agree on
	rove::pcg32 c(5);
	double sum = 0;
	for (int k = 0; k < 20000; ++k) sum += float(c.normal(dfloat(0.0f), dfloat(1.0f)));
	BOOST_REQUIRE(std::abs(sum / 20000) < 0.03);
	rove::vec<3, dfloat> const v = c.unit_vector<dfloat>();
	BOOST_REQUIRE(std::abs(float(v.length()) - 1) < 1e-5f);
	// value noise: at a lattice point the lattice's value, which is float's
	BOOST_REQUIRE_EQUAL(rove::value_noise(dfloat(3.0f), 9).value(), rove::value_noise(3.0f, 9));
	BOOST_REQUIRE(std::abs(float(rove::value_noise(dfloat(3.3f), dfloat(-1.25f), 9)) - rove::value_noise(3.3f, -1.25f, 9)) < 1e-6f);
}

// What the README says: every template takes dfloat.  Each answered through dfloat's own arithmetic, compared with the float's.
BOOST_AUTO_TEST_CASE(the_shapes_on_dfloat_answer_as_the_shapes_on_float_do)
{
	using D = rove::dfloat;
	using V = rove::vec<3, D>;
	V const zero(D(0.0f), D(0.0f), D(0.0f)), one(D(1.0f), D(1.0f), D(1.0f)), x(D(1.0f), D(0.0f), D(0.0f));

	rove::aabb<3, D> const box(zero, one);
	D t0, t1;
	BOOST_REQUIRE(box.trace(rove::ray<3, D>(V(D(-1.0f), D(0.5f), D(0.5f)), x), &t0, &t1, D(0.0f), D(10.0f)));
	BOOST_TEST(t0.value() == 1.0f);
	BOOST_TEST(t1.value() == 2.0f);
	rove::aabb<2, D> const rect(rove::vec<2, D>(D(0.0f), D(0.0f)), rove::vec<2, D>(D(1.0f), D(1.0f)));
	BOOST_TEST(rect.trace(rove::ray<2, D>(rove::vec<2, D>(D(-1.0f), D(0.5f)), rove::vec<2, D>(D(1.0f), D(0.0f))), D(0.0f), D(10.0f)));

	rove::obb<3, D> const o(box);
	BOOST_TEST(o.test_intersection(o));
	BOOST_TEST(float(o.radius()) == std::sqrt(0.75f), boost::test_tools::tolerance(1e-5f));
	rove::obb<3, D> const far_box(V(D(5.0f), D(5.0f), D(5.0f)), x, V(D(0.0f), D(1.0f), D(0.0f)), V(D(0.0f), D(0.0f), D(1.0f)));
	BOOST_TEST(!o.test_intersection(far_box));
	BOOST_TEST(!o.test_collision(far_box, V(D(0.0f), D(0.0f), D(0.0f))));

	rove::plane<D> const p(zero, x);
	BOOST_TEST(p.contains(V(D(0.0f), D(2.0f), D(3.0f))));
	BOOST_TEST((p.classify(V(D(1.0f), D(0.0f), D(0.0f))) != p.classify(V(D(-1.0f), D(0.0f), D(0.0f)))));

	rove::matrix<4, 4, D> view_projection;
	view_projection.identity();
	rove::frustum<D> const f(view_projection);
	BOOST_TEST(f.contains(V(D(0.0f), D(0.0f), D(0.5f))));
	BOOST_TEST(!f.contains(V(D(5.0f), D(0.0f), D(0.5f))));

	rove::triangle<3, D> const tri(zero, x, V(D(0.0f), D(1.0f), D(0.0f)));
	BOOST_TEST(float(tri.area()) == 0.5f);
	BOOST_TEST(tri.contains(V(D(0.25f), D(0.25f), D(0.0f))));
	BOOST_TEST(float(tri.closest_point(V(D(0.25f), D(0.25f), D(5.0f))).z) == 0.0f);

	rove::cone<3, D> const c(zero, x, D(0.5f), D(1.0f));
	BOOST_TEST(c.bounds().contains(x));

	rove::capsule<3, D> const cap(zero, one, D(1.0f));
	BOOST_TEST(float(cap.bounds().lo.x) == -1.0f);
	BOOST_TEST(cap.test_intersection(cap));
	BOOST_TEST(float(cap.distance(V(D(5.0f), D(0.0f), D(0.0f)))) > 0.0f);

	rove::sphere<3, D> const s(zero, D(1.0f));
	BOOST_TEST(s.test_intersection(rove::ray<3, D>(V(D(3.0f), D(0.0f), D(0.0f)), V(D(-1.0f), D(0.0f), D(0.0f)))));

	rove::octree<rove::aabb<3, D>, int> tree(box);
	tree.insert(box, 7);
	tree.build();
	BOOST_TEST(tree.query_intersection(box).size() == 1u);

	rove::hash_grid<rove::aabb<3, D>, int> grid(D(1.0f));
	grid.insert(box, 9);
	BOOST_TEST(grid.query_intersection(box).size() == 1u);
}

BOOST_AUTO_TEST_SUITE_END()
