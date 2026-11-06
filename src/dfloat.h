// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

// dfloat: a float that gives the same bits on every platform: x86-64, ARM64 and WebAssembly, any standard library, any of the
// compilers that build for them.  For a simulation that two machines run in lockstep from the same inputs.
//
// Only with ROVE_DFLOAT defined, which rove's CMake option ROVE_DFLOAT does for rove and whoever links it, together with
// -ffp-contract=off (below); without it there is no dfloat and rove is as it was.
//
// It holds one float and nothing else (the same size, layout and copying), and is what rove's templates take for T: vec<3, dfloat>,
// matrix<3, 3, dfloat>, quaternion<dfloat>, sphere<3, dfloat>...  What it does and why it is the same everywhere:
//
//   + - * / and sqrt  are the float operations, which IEEE 754 rounds correctly and so alike everywhere; each is an operator call of
//                     its own, so no compiler fuses a * b + c into one rounding (a fused multiply-add) unless told to contract across
//                     statements: hence -ffp-contract=off, which the option ROVE_DFLOAT adds.
//   sin cos tan asin acos atan atan2 exp log pow
//                     rove's own (dfloat.cc), computed in double with + - * / and sqrt only and rounded to float: within
//                     an ulp of the true value and almost always the float nearest it.  The platform's maths library differs from one
//                     to the next, so it is never called.
//   floor ceil trunc round fmod remainder abs copysign, the comparisons, isnan isfinite isinf signbit
//                     exact operations, the same everywhere.
//
// What it will not do, so that nothing slips through to a float or a double: no conversion to float (nor to an integer) but an explicit one (a
// std::sin(x) does not compile; sin(x), found by argument lookup, is rove's), none from double or long double at all (0.5 does not
// compile, 0.5f does), and none from bool.  From float and the integers it converts implicitly (an integer as float(i) does).  The
// functions are hidden friends: they are found only for an argument that is a dfloat, never for a float.

#include <bit>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numbers>
#include <ostream>
#include <string>

#include "scalar.h"

#if defined(__FAST_MATH__)
#error "rove::dfloat: -ffast-math reorders and fuses float operations; build without it"
#endif
#if !defined(ROVE_DFLOAT)
#error "rove::dfloat is built only with rove's CMake option ROVE_DFLOAT (which also adds -ffp-contract=off)"
#endif

namespace rove
{

namespace detail
{
// dfloat.cc
float dfloat_sin(float x);
float dfloat_cos(float x);
float dfloat_tan(float x);
float dfloat_asin(float x);
float dfloat_acos(float x);
float dfloat_atan(float x);
float dfloat_atan2(float y, float x);
float dfloat_exp(float x);
float dfloat_log(float x);
float dfloat_pow(float x, float y);
float dfloat_fmod(float x, float y);
}

class dfloat
{
public:
	dfloat() = default;          // as a float: uninitialised, and dfloat{} is 0
	constexpr dfloat(float v): v_(v) {}
	template<std::integral I>
		requires(!std::same_as<I, bool>)
	constexpr dfloat(I i): v_(static_cast<float>(i)) {}
	template<std::floating_point F>
		requires(!std::same_as<F, float>)
	dfloat(F) = delete;          // a double (a literal without f) would have been computed in double first
	dfloat(bool) = delete;

	constexpr explicit operator float() const { return v_; }
	template<std::integral I>
	constexpr explicit operator I() const { return static_cast<I>(v_); }          // truncated, as a float is
	constexpr float value() const { return v_; }

	constexpr dfloat operator-() const { return -v_; }
	constexpr dfloat operator+() const { return v_; }
	constexpr dfloat &operator+=(dfloat b) { v_ = v_ + b.v_; return *this; }
	constexpr dfloat &operator-=(dfloat b) { v_ = v_ - b.v_; return *this; }
	constexpr dfloat &operator*=(dfloat b) { v_ = v_ * b.v_; return *this; }
	constexpr dfloat &operator/=(dfloat b) { v_ = v_ / b.v_; return *this; }

	friend constexpr dfloat operator+(dfloat a, dfloat b) { return a.v_ + b.v_; }
	friend constexpr dfloat operator-(dfloat a, dfloat b) { return a.v_ - b.v_; }
	friend constexpr dfloat operator*(dfloat a, dfloat b) { return a.v_ * b.v_; }
	friend constexpr dfloat operator/(dfloat a, dfloat b) { return a.v_ / b.v_; }

	friend constexpr bool operator==(dfloat a, dfloat b) { return a.v_ == b.v_; }
	friend constexpr bool operator!=(dfloat a, dfloat b) { return a.v_ != b.v_; }
	friend constexpr bool operator<(dfloat a, dfloat b) { return a.v_ < b.v_; }
	friend constexpr bool operator<=(dfloat a, dfloat b) { return a.v_ <= b.v_; }
	friend constexpr bool operator>(dfloat a, dfloat b) { return a.v_ > b.v_; }
	friend constexpr bool operator>=(dfloat a, dfloat b) { return a.v_ >= b.v_; }

	friend std::ostream &operator<<(std::ostream &out, dfloat a) { return out << a.v_; }
	friend std::string to_string(dfloat a) { return std::to_string(a.v_); }

	// exact everywhere
	friend dfloat sqrt(dfloat a) { return std::sqrt(a.v_); }
	friend constexpr dfloat abs(dfloat a) { return std::bit_cast<float>(std::bit_cast<std::uint32_t>(a.v_) & 0x7fffffffu); }
	friend constexpr dfloat fabs(dfloat a) { return abs(a); }
	friend constexpr dfloat copysign(dfloat a, dfloat b)
	{
		return std::bit_cast<float>((std::bit_cast<std::uint32_t>(a.v_) & 0x7fffffffu) | (std::bit_cast<std::uint32_t>(b.v_) & 0x80000000u));
	}
	friend constexpr bool signbit(dfloat a) { return (std::bit_cast<std::uint32_t>(a.v_) >> 31) != 0; }
	friend constexpr bool isnan(dfloat a) { return a.v_ != a.v_; }
	friend constexpr bool isinf(dfloat a) { return abs(a).v_ == std::numeric_limits<float>::infinity(); }
	friend constexpr bool isfinite(dfloat a) { return abs(a).v_ <= std::numeric_limits<float>::max(); }
	friend dfloat floor(dfloat a) { return std::floor(a.v_); }
	friend dfloat ceil(dfloat a) { return std::ceil(a.v_); }
	friend dfloat trunc(dfloat a) { return std::trunc(a.v_); }
	friend dfloat round(dfloat a) { return std::round(a.v_); }
	friend dfloat fmod(dfloat a, dfloat b) { return detail::dfloat_fmod(a.v_, b.v_); }
	friend dfloat remainder(dfloat a, dfloat b) { return std::remainder(a.v_, b.v_); }          // IEEE's remainder: exact

	// rove's own
	friend dfloat sin(dfloat a) { return detail::dfloat_sin(a.v_); }
	friend dfloat cos(dfloat a) { return detail::dfloat_cos(a.v_); }
	friend dfloat tan(dfloat a) { return detail::dfloat_tan(a.v_); }
	friend dfloat asin(dfloat a) { return detail::dfloat_asin(a.v_); }
	friend dfloat acos(dfloat a) { return detail::dfloat_acos(a.v_); }
	friend dfloat atan(dfloat a) { return detail::dfloat_atan(a.v_); }
	friend dfloat atan2(dfloat y, dfloat x) { return detail::dfloat_atan2(y.v_, x.v_); }
	friend dfloat exp(dfloat a) { return detail::dfloat_exp(a.v_); }
	friend dfloat log(dfloat a) { return detail::dfloat_log(a.v_); }
	friend dfloat pow(dfloat x, dfloat y) { return detail::dfloat_pow(x.v_, y.v_); }

private:
	float v_;
};


static_assert(sizeof(dfloat) == sizeof(float) && alignof(dfloat) == alignof(float));
static_assert(std::is_trivially_copyable_v<dfloat> && std::is_standard_layout_v<dfloat>);
static_assert(std::is_trivially_default_constructible_v<dfloat>);

/// The tolerances are a float's.
template<>
struct scalar_traits<dfloat, void> {
	static constexpr dfloat default_epsilon = scalar_traits<float>::default_epsilon;
	static constexpr dfloat zero_length = scalar_traits<float>::zero_length;
	static constexpr dfloat zero_length_sq = scalar_traits<float>::zero_length_sq;
	static constexpr dfloat zero_angle = scalar_traits<float>::zero_angle;
	static constexpr dfloat contact_skin = scalar_traits<float>::contact_skin;
};

}

/// A float's limits (the standard lets a program specialise numeric_limits for its own types).
template<>
class std::numeric_limits<rove::dfloat> : public std::numeric_limits<float>
{
	using F = std::numeric_limits<float>;

public:
	static constexpr rove::dfloat min() noexcept { return F::min(); }
	static constexpr rove::dfloat max() noexcept { return F::max(); }
	static constexpr rove::dfloat lowest() noexcept { return F::lowest(); }
	static constexpr rove::dfloat epsilon() noexcept { return F::epsilon(); }
	static constexpr rove::dfloat round_error() noexcept { return F::round_error(); }
	static constexpr rove::dfloat infinity() noexcept { return F::infinity(); }
	static constexpr rove::dfloat quiet_NaN() noexcept { return F::quiet_NaN(); }
	static constexpr rove::dfloat signaling_NaN() noexcept { return F::signaling_NaN(); }
	static constexpr rove::dfloat denorm_min() noexcept { return F::denorm_min(); }
};

/// A dfloat hashes as its float, +0 and -0 alike (they are equal): a key of std::unordered_map, a vector of dfloats too.
template<>
struct std::hash<rove::dfloat>
{
	std::size_t operator()(rove::dfloat a) const { return std::hash<float>()(a.value() == 0.0f ? 0.0f : a.value()); }
};

// The constants as floats (the standard lets a program specialise them for its own types).
template<> inline constexpr rove::dfloat std::numbers::pi_v<rove::dfloat> = std::numbers::pi_v<float>;
template<> inline constexpr rove::dfloat std::numbers::inv_pi_v<rove::dfloat> = std::numbers::inv_pi_v<float>;
template<> inline constexpr rove::dfloat std::numbers::e_v<rove::dfloat> = std::numbers::e_v<float>;
template<> inline constexpr rove::dfloat std::numbers::sqrt2_v<rove::dfloat> = std::numbers::sqrt2_v<float>;
template<> inline constexpr rove::dfloat std::numbers::ln2_v<rove::dfloat> = std::numbers::ln2_v<float>;
