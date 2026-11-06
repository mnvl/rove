// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

// dfloat's functions (dfloat.h): each computed in double from + - * / and sqrt alone, which round alike on
// every platform, and rounded once to float at the end.  The coefficients are written as the exact doubles (hex), not computed.
//
// sin, cos, tan: the argument reduced to r in [-pi/4, pi/4] and the quarter turn n, then Taylor series of sin and cos in r.  A float
// above pi/4 is reduced exactly (Payne-Hanek): its 24-bit mantissa times 256 bits of 2/pi, as integers, which keeps 64 bits of the
// fraction however large it is.
// atan: |t| > 1 by pi/2 - atan(1/|t|), then above tan(pi/12) by pi/6 + atan((t sqrt 3 - 1) / (t + sqrt 3)), then the series.
// asin, acos: atan of x / sqrt((1 - x)(1 + x)) and its inverse.  exp: 2^k e^r, r within ln 2 / 2, the series.  log: k ln 2 + 2 atanh s,
// s = (m - 1) / (m + 1) for the mantissa m within [sqrt 1/2, sqrt 2).  pow: exp(y log |x|) in double, the special cases C's Annex F's.

#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

#include "dfloat.h"

namespace rove::detail
{

namespace
{

constexpr double PI = 0x1.921fb54442d18p+1;
constexpr double PI_2 = 0x1.921fb54442d18p+0;
constexpr double PI_4 = 0x1.921fb54442d18p-1;
constexpr double PI_6 = 0x1.0c152382d7365p-1;
constexpr double TAN_PI_12 = 0x1.126145e9ecd58p-2;
constexpr double SQRT3 = 0x1.bb67ae8584caap+0;
constexpr double SQRT1_2 = 0x1.6a09e667f3bcdp-1;
constexpr double INV_LN2 = 0x1.71547652b82fep+0;
constexpr double LN2_HI = 0x1.62e42fee00000p-1;          // the upper bits of ln 2: k * LN2_HI is exact for |k| < 2^20
constexpr double LN2_LO = 0x1.a39ef35793c76p-33;         // ... and the rest
constexpr float INF = std::numeric_limits<float>::infinity();
constexpr float NaN = std::numeric_limits<float>::quiet_NaN();

/// floor(2^256 * 2 / pi), most significant word first.
constexpr std::uint32_t TWO_OVER_PI[8] = {0xa2f9836e, 0x4e441529, 0xfc2757d1, 0xf534ddc0, 0xdb629599, 0x3c439041, 0xfe5163ab, 0xdebbc561};

bool is_nan(double v) { return v != v; }

/// 2^k for k in [-1022, 1023], built from its bits.
double two_to(int k) { return std::bit_cast<double>(std::uint64_t(k + 1023) << 52); }

/// sin r for |r| <= pi/4 (to x^17: the rest is below 1e-19 there).
double sin_kernel(double r)
{
	double const r2 = r * r;
	double const p = -0x1.5555555555555p-3 + r2 * (0x1.1111111111111p-7 + r2 * (-0x1.a01a01a01a01ap-13 + r2 * (0x1.71de3a556c734p-19 +
		r2 * (-0x1.ae64567f544e4p-26 + r2 * (0x1.6124613a86d09p-33 + r2 * (-0x1.ae7f3e733b81fp-41 + r2 * 0x1.952c77030ad4ap-49))))));
	return r + r * r2 * p;
}

/// cos r for |r| <= pi/4 (to x^18).
double cos_kernel(double r)
{
	double const r2 = r * r;
	double const p = -0x1.0000000000000p-1 + r2 * (0x1.5555555555555p-5 + r2 * (-0x1.6c16c16c16c17p-10 + r2 * (0x1.a01a01a01a01ap-16 +
		r2 * (-0x1.27e4fb7789f5cp-22 + r2 * (0x1.1eed8eff8d898p-29 + r2 * (-0x1.93974a8c07c9dp-37 + r2 * (0x1.ae7f3e733b81fp-45 +
		r2 * -0x1.6827863b97d97p-53)))))));
	return 1 + r2 * p;
}

/// |x| (a finite float above pi/4) as (n + f) pi/2, f within [-1/2, 1/2): n mod 4 and r = f pi/2.
struct Reduced
{
	int n;
	double r;
};
Reduced reduce(float ax)
{
	std::uint32_t const b = std::bit_cast<std::uint32_t>(ax);
	int const e = int(b >> 23) - 150;                         // ax = m 2^e (a normal float: it is above pi/4)
	std::uint64_t const m = (b & 0x7fffffu) | 0x800000u;
	// P = m * TWO_OVER_PI, 288 bits in nine words, the least significant first: ax 2/pi = P 2^(e - 256)
	std::uint32_t p[9];
	std::uint64_t carry = 0;
	for (int i = 0; i < 8; ++i)
	{
		std::uint64_t const t = m * TWO_OVER_PI[7 - i] + carry;
		p[i] = std::uint32_t(t);
		carry = t >> 32;
	}
	p[8] = std::uint32_t(carry);
	auto word = [&](int k) -> std::uint64_t { return k >= 0 && k <= 8 ? p[k] : 0; };
	auto bits64 = [&](int lo) {          // the 64 bits of P from bit `lo` up (lo >= 0)
		int const w = lo >> 5, s = lo & 31;
		std::uint64_t v = word(w) >> s | word(w + 1) << (32 - s);
		if (s)
			v |= word(w + 2) << (64 - s);
		return v;
	};
	int const point = 256 - e;                                 // P's bit for 1 (ax < 2^128: point > 128)
	int n = int(bits64(point) & 3);
	std::int64_t const f = std::bit_cast<std::int64_t>(bits64(point - 64));          // the fraction, read as signed: f >= 1/2 is f - 1 ...
	if (f < 0)
		n += 1;                                                                        // ... of the next quarter turn
	return {n & 3, double(f) * 0x1p-64 * PI_2};
}

/// atan t for any double t (but NaN), its sign kept for a zero.
double atan_d(double t)
{
	if (t == 0)
		return t;
	double a = t < 0 ? -t : t, base = 0;
	bool const inverted = a > 1;
	if (inverted)
		a = 1 / a;
	if (a > TAN_PI_12)
	{
		a = (a * SQRT3 - 1) / (a + SQRT3);
		base = PI_6;
	}
	double const a2 = a * a;          // |a| <= tan(pi/12): to a^31 (the rest below 1e-18)
	double const p = -0x1.5555555555555p-2 + a2 * (0x1.999999999999ap-3 + a2 * (-0x1.2492492492492p-3 + a2 * (0x1.c71c71c71c71cp-4 +
		a2 * (-0x1.745d1745d1746p-4 + a2 * (0x1.3b13b13b13b14p-4 + a2 * (-0x1.1111111111111p-4 + a2 * (0x1.e1e1e1e1e1e1ep-5 +
		a2 * (-0x1.af286bca1af28p-5 + a2 * (0x1.8618618618618p-5 + a2 * (-0x1.642c8590b2164p-5 + a2 * (0x1.47ae147ae147bp-5 +
		a2 * (-0x1.2f684bda12f68p-5 + a2 * (0x1.1a7b9611a7b96p-5 + a2 * -0x1.0842108421084p-5)))))))))))));
	double r = base + (a + a * a2 * p);
	if (inverted)
		r = PI_2 - r;
	return t < 0 ? -r : r;
}

/// e^y for |y| <= 200.
double exp_d(double y)
{
	double const k = std::floor(y * INV_LN2 + 0.5);
	double const r = (y - k * LN2_HI) - k * LN2_LO;          // |r| <= ln 2 / 2 (and a hair): to r^13
	double const p = 1 + r * (1 + r * (0x1.0000000000000p-1 + r * (0x1.5555555555555p-3 + r * (0x1.5555555555555p-5 + r * (0x1.1111111111111p-7 +
		r * (0x1.6c16c16c16c17p-10 + r * (0x1.a01a01a01a01ap-13 + r * (0x1.a01a01a01a01ap-16 + r * (0x1.71de3a556c734p-19 +
		r * (0x1.27e4fb7789f5cp-22 + r * (0x1.ae64567f544e4p-26 + r * (0x1.1eed8eff8d898p-29 + r * 0x1.6124613a86d09p-33))))))))))));
	return p * two_to(int(k));
}

/// ln v for a positive finite double that is normal.
double log_d(double v)
{
	std::uint64_t const b = std::bit_cast<std::uint64_t>(v);
	int k = int((b >> 52) & 0x7ff) - 1022;
	double m = std::bit_cast<double>((b & 0x000fffffffffffffull) | 0x3fe0000000000000ull);          // in [1/2, 1): v = m 2^k
	if (m < SQRT1_2)
		m *= 2, k -= 1;
	double const f = m - 1;                      // exact
	double const s = f / (2 + f), s2 = s * s;    // |s| <= 0.1716: to s^23
	double const p = 0x1.5555555555555p-2 + s2 * (0x1.999999999999ap-3 + s2 * (0x1.2492492492492p-3 + s2 * (0x1.c71c71c71c71cp-4 +
		s2 * (0x1.745d1745d1746p-4 + s2 * (0x1.3b13b13b13b14p-4 + s2 * (0x1.1111111111111p-4 + s2 * (0x1.e1e1e1e1e1e1ep-5 +
		s2 * (0x1.af286bca1af28p-5 + s2 * (0x1.8618618618618p-5 + s2 * 0x1.642c8590b2164p-5)))))))));
	double const log_m = 2 * (s + s * s2 * p);
	return k * LN2_HI + (k * LN2_LO + log_m);
}

/// y (finite) is a whole number, and an odd one.
bool is_integer(float y) { return std::floor(y) == y; }
bool is_odd_integer(float y)
{
	float const a = y < 0 ? -y : y;
	return is_integer(y) && a < 0x1p24f && (std::int64_t(a) & 1) != 0;          // (from 2^24 up every float is even)
}

}

float dfloat_sin(float x)
{
	if (x == 0 || x != x)
		return x;
	float const ax = x < 0 ? -x : x;
	if (ax == INF)
		return NaN;
	double s;
	if (ax <= PI_4)
		s = sin_kernel(ax);
	else
	{
		Reduced const q = reduce(ax);
		s = q.n == 0 ? sin_kernel(q.r) : q.n == 1 ? cos_kernel(q.r) : q.n == 2 ? -sin_kernel(q.r) : -cos_kernel(q.r);
	}
	return float(x < 0 ? -s : s);
}

float dfloat_cos(float x)
{
	if (x != x)
		return x;
	float const ax = x < 0 ? -x : x;
	if (ax == INF)
		return NaN;
	if (ax <= PI_4)
		return float(cos_kernel(ax));
	Reduced const q = reduce(ax);
	return float(q.n == 0 ? cos_kernel(q.r) : q.n == 1 ? -sin_kernel(q.r) : q.n == 2 ? -cos_kernel(q.r) : sin_kernel(q.r));
}

float dfloat_tan(float x)
{
	if (x == 0 || x != x)
		return x;
	float const ax = x < 0 ? -x : x;
	if (ax == INF)
		return NaN;
	double t;
	if (ax <= PI_4)
		t = sin_kernel(ax) / cos_kernel(ax);
	else
	{
		Reduced const q = reduce(ax);
		t = q.n % 2 == 0 ? sin_kernel(q.r) / cos_kernel(q.r) : -cos_kernel(q.r) / sin_kernel(q.r);
	}
	return float(x < 0 ? -t : t);
}

float dfloat_atan(float x)
{
	if (x != x)
		return x;
	return float(atan_d(x));          // (an infinite x: 1/x is 0, and pi/2 is left)
}

float dfloat_atan2(float y, float x)
{
	if (x != x || y != y)
		return x + y;
	bool const y_negative = std::signbit(y);
	auto signed_y = [&](double v) { return float(y_negative ? -v : v); };
	if (y == 0)
		return std::signbit(x) ? signed_y(PI) : y;
	bool const y_inf = y == INF || y == -INF, x_inf = x == INF || x == -INF;
	if (y_inf)
		return signed_y(x_inf ? (x > 0 ? PI_4 : 3 * PI_4) : PI_2);
	if (x == 0)
		return signed_y(PI_2);
	if (x_inf)
		return x > 0 ? (y_negative ? -0.0f : 0.0f) : signed_y(PI);
	double const a = atan_d(double(y) / double(x));
	if (x > 0)
		return float(a);
	return float(y_negative ? a - PI : a + PI);
}

float dfloat_asin(float x)
{
	if (x == 0 || x != x)
		return x;
	if (x > 1 || x < -1)
		return NaN;
	double const d = x, s = std::sqrt((1 - d) * (1 + d));
	return float(atan_d(d / s));          // (x = +-1: s is 0, the quotient infinite, and +-pi/2 is left)
}

float dfloat_acos(float x)
{
	if (x != x)
		return x;
	if (x > 1 || x < -1)
		return NaN;
	double const d = x, s = std::sqrt((1 - d) * (1 + d));
	if (d == 0)
		return float(PI_2);
	double const a = atan_d(s / d);
	return float(d > 0 ? a : PI + a);
}

float dfloat_exp(float x)
{
	if (x != x)
		return x;
	if (x > 89)
		return INF;
	if (x < -104)
		return 0;          // (below half the least float)
	return float(exp_d(x));
}

float dfloat_log(float x)
{
	if (x != x)
		return x;
	if (x < 0)
		return NaN;
	if (x == 0)
		return -INF;
	if (x == INF)
		return INF;
	return float(log_d(x));          // (a float is a normal double)
}

float dfloat_pow(float x, float y)
{
	if (y == 0 || x == 1)
		return 1;
	if (x != x || y != y)
		return x + y;
	bool const y_inf = y == INF || y == -INF;
	float const ax = x < 0 ? -x : x;
	if (y_inf)
	{
		if (ax == 1)
			return 1;
		return (ax < 1) == (y < 0) ? INF : 0.0f;
	}
	bool const odd = is_odd_integer(y);
	if (x == 0)
	{
		bool const negative = odd && std::signbit(x);
		return y < 0 ? (negative ? -INF : INF) : (negative ? -0.0f : 0.0f);
	}
	if (ax == INF)
	{
		bool const negative = odd && x < 0;
		return y < 0 ? (negative ? -0.0f : 0.0f) : (negative ? -INF : INF);
	}
	if (x < 0 && !is_integer(y))
		return NaN;
	bool const negative = x < 0 && odd;
	double const t = double(y) * log_d(ax);
	double r;
	if (t > 89)
		r = INF;
	else if (t < -104)
		r = 0;
	else
		r = exp_d(t);
	return float(negative ? -r : r);
}

float dfloat_fmod(float x, float y)
{
	return std::fmod(x, y);          // exact: the remainder is a float, and every library gives it
}

}
