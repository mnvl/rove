// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <concepts>
#include <numbers>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace rove
{

using scalar = float;

inline constexpr scalar PI = std::numbers::pi_v<scalar>;
inline constexpr scalar TWO_PI = 2 * PI;

/**
 * @brief The tolerances rove works to in precision T
 *
 * default_epsilon is what a function taking an `epsilon` argument uses when
 * given none, and nothing else reads it: a caller who wants another
 * tolerance passes one.  The rest are the fixed thresholds below which the
 * library takes a quantity for zero, where dividing by it or taking a
 * direction from it would give nothing meaningful; each is compared with
 * a quantity of its own kind, a length with zero_length and a squared
 * length with zero_length_sq.  contact_skin is not a precision but a length
 * in world units, the same in either: an overlap first_contact() takes for a
 * touch rather than a penetration.
 */
template<class T, class = void>
struct scalar_traits;

/// Integers are exact: every tolerance is zero
template<std::integral T>
struct scalar_traits<T> {
	static constexpr T default_epsilon = 0;
	static constexpr T zero_length = 0;
	static constexpr T zero_length_sq = 0;
	static constexpr T zero_angle = 0;
	static constexpr T contact_skin = 0;
};

template<>
struct scalar_traits<float, void> {
	/// The `epsilon` argument a function takes when given none
	static constexpr float default_epsilon = 1.0e-6f;
	/// A length, a vector's component or a plane's coefficient at or below which it is zero
	static constexpr float zero_length = 1.0e-6f;
	/// A squared length (a length_sq(), a discriminant) below which it is zero: zero_length squared
	static constexpr float zero_length_sq = zero_length * zero_length;
	/// A sine, or one less a cosine, below which an angle is zero
	static constexpr float zero_angle = 1.0e-6f;
	/// How deep two shapes overlap before first_contact() calls it a penetration, not a touch: a length in world units
	static constexpr float contact_skin = 1.0e-3f;
};

template<>
struct scalar_traits<double, void> {
	static constexpr double default_epsilon = 1.0e-12;
	static constexpr double zero_length = 1.0e-12;
	static constexpr double zero_length_sq = zero_length * zero_length;
	static constexpr double zero_angle = 1.0e-12;
	static constexpr double contact_skin = 1.0e-3;
};

// These helpers are templates on the argument type rather than plain
// scalar (float) functions, and must stay that way.  Everything in this
// library lives in namespace rove, so an unqualified sin()/abs()/square()
// inside matrix<N,N,T>, vec<N,T> or quaternion<T> finds them by ordinary
// unqualified lookup in preference to std::.  Taking `scalar` would make
// every double-precision instantiation round its intermediates to float:
// mat4d::rotation() would build a rotation matrix accurate to ~1e-8, and
// abs() would flush any double below FLT_MIN to zero.  Returning `auto`
// keeps the exact result type std:: gives (float in, float out).

// 1, -1, or 0 within epsilon of zero; NaN has no sign and stays NaN.
template<class T> inline T
sign(T x, T epsilon = scalar_traits<T>::default_epsilon) {
	if (x > epsilon) return 1;
	if (x < -epsilon) return -1;
	using std::isnan;
	if (isnan(x)) return x;
	return 0;
}

namespace detail
{

// For code templated on the scalar (a T of one's own, like dfloat, has its own functions, and its constructor refuses a double).
template<class T> inline std::string to_string_of(T v) { using std::to_string; return to_string(v); }
template<class T> constexpr T constant_of(double v)
{
	if constexpr (std::is_floating_point_v<T>) return T(v);
	else return T(float(v));
}

}

// (by cases: the absolute value of a zero is +0, on every compiler, where MSVC folds 0 - (-0.0f) to -0; and a NaN stays one)
template<class T> inline T
abs(T s) {
	if constexpr (std::is_unsigned_v<T>) {
		return s;
	} else {
		if (s > T(0)) return s;
		if (s < T(0)) return -s;
		return s == T(0) ? T(0) : s;
	}
}

// (Each through a using-declaration: for float, double and the integers the call is std::'s, as it always was; for a T of one's own,
// like dfloat, argument lookup finds its own, so rove::sin(x) of one is its sin too.)
template<class T> inline bool
isnan(T s) {
	using std::isnan;
	return isnan(s);
}

template<class T> inline bool
isfinite(T s) {
	using std::isfinite;
	return isfinite(s);
}

template<class T> inline auto
floor(T s) {
	using std::floor;
	return floor(s);
}

template<class T> inline auto
sin(T s) {
	using std::sin;
	return sin(s);
}

template<class T> inline auto
cos(T s) {
	using std::cos;
	return cos(s);
}

template<class T> inline auto
tan(T s) {
	using std::tan;
	return tan(s);
}

template<class T> inline T
square(T s) {
	return s * s;
}

template<class T> inline T
cube(T s) {
	return s * s * s;
}

template<class T> inline auto
square_root(T s) {
	using std::sqrt;
	return sqrt(s);
}

template<class T> inline T
clamp(T x, T low = 0, T high = 1) {
	return std::clamp(x, low, high);
}

// a + (b - a) * t: a at 0, b at 1, and on along the line outside them.  Not
// std::lerp, which rounds otherwise to be exact at both ends.
template<class T> inline T
lerp(T a, T b, T t) {
	return a + (b - a) * t;
}

// x moved by whole periods (hi - lo) into [lo, hi).  The range must hold
// zero, lo <= 0 < hi, as an angle range does: fmod is exact and keeps x's
// sign, so an x already in range comes back bit for bit and a single
// period's shift brings any other in.  NaN and infinities come out NaN.
template<class T> inline T
wrap(T x, T lo, T hi) {
	static_assert(!std::is_integral_v<T>, "wrap: a real number, not an integer");
	using std::fmod, std::to_string;   // (for a T of one's own, like dfloat, its own, by argument lookup)
	if (!(lo <= 0 && 0 < hi)) {
		throw std::invalid_argument("wrap: [lo, hi) must hold zero, got [" +
			to_string(lo) + ", " + to_string(hi) + ")");
	}
	T const period = hi - lo;
	T r = fmod(x, period);
	if (r < lo) {
		r += period;
		// A hair below lo plus a period can round onto hi, which is lo.
		if (r >= hi) r = lo;
	} else if (r >= hi) {
		r -= period;
	}
	return r;
}

// Into (-pi, pi]: a yaw difference across the atan2 seam at west is a
// small turn, not a full one.  wrap() is half open at the top, so it runs
// on -a.
template<class T> inline T
wrap_to_pi(T a) {
	T const pi = std::numbers::pi_v<T>;
	return -wrap(-a, -pi, pi);
}

// Into [0, 2 pi): the bearing of a beam, for sorting a sweep.
template<class T> inline T
wrap_to_two_pi(T a) {
	return wrap(a, T(0), 2 * std::numbers::pi_v<T>);
}

// The turn (-pi, pi] from angle `from` to angle `to`: the short way round, across the seam at west if that is shorter.
template<class T> inline T
delta_angle(T from, T to) {
	return wrap_to_pi(to - from);
}

// Degrees to radians and back; 180 is exactly pi, 90 exactly pi / 2.
template<class T> inline T
radians(T degrees) {
	static_assert(!std::is_integral_v<T>, "radians: a real number, not an integer");
	return degrees / T(180) * std::numbers::pi_v<T>;
}

template<class T> inline T
degrees(T radians) {
	static_assert(!std::is_integral_v<T>, "degrees: a real number, not an integer");
	return radians / std::numbers::pi_v<T> * T(180);
}

// What floor leaves: in [0, 1), so fract(-0.25) is 0.75.
template<class T> inline T
fract(T x) {
	using std::floor;
	return x - floor(x);
}

// Where x is from a (0) to b (1), unclamped: 2 past b by as much again.  a == b has no such place: std::invalid_argument.
template<class T> inline T
inverse_lerp(T a, T b, T x) {
	if (a == b) {
		using std::to_string;
		throw std::invalid_argument("inverse_lerp: a and b are the same, " + to_string(a));
	}
	return (x - a) / (b - a);
}

// x moved from [a0, a1] to [b0, b1], unclamped: lerp(b0, b1, inverse_lerp(a0, a1, x)).
template<class T> inline T
remap(T x, T a0, T a1, T b0, T b1) {
	return lerp(b0, b1, inverse_lerp(a0, a1, x));
}

// 0 at edge0, 1 at edge1, flat outside, 3t^2 - 2t^3 between (GLSL's smoothstep); edges the other way round run the other way.  Equal
// edges have no between: std::invalid_argument.
template<class T> inline T
smoothstep(T edge0, T edge1, T x) {
	T const t = std::clamp(inverse_lerp(edge0, edge1, x), T(0), T(1));
	return t * t * (3 - 2 * t);
}

namespace detail
{

// std::invalid_argument naming `what` unless v is zero or more (a NaN is not)
template<class T> inline void
require_not_negative(T v, char const *what) {
	if (!(v >= 0)) {
		using std::to_string;
		throw std::invalid_argument(std::string(what) + " must be zero or more, got " + to_string(v));
	}
}

}

// current moved towards target by the share 1 - e^{-rate dt} of the way: exponential smoothing that ends in the same place however dt
// is cut up (two steps of dt/2 are one step of dt), so a frame rate does not change it, as current + (target - current) * k dt does.
// It never passes the target.  For scalars and vectors alike (V times T).  A negative rate or dt is std::invalid_argument.
template<class V, class T> inline V
damp(V const &current, V const &target, T rate, T dt) {
	detail::require_not_negative(rate, "damp: the rate");
	detail::require_not_negative(dt, "damp: dt");
	using std::exp;
	return target + (current - target) * exp(-rate * dt);
}

// current moved towards target by max_delta at most, landing on it exactly.  (For vectors: vec.h's.)  A negative max_delta is
// std::invalid_argument.
template<class T> inline T
move_towards(T current, T target, T max_delta) {
	detail::require_not_negative(max_delta, "move_towards: max_delta");
	T const d = target - current;
	if (abs(d) <= max_delta) return target;
	return d > 0 ? current + max_delta : current - max_delta;
}

// One step of dt of a critically damped spring pulling value towards target, with value's velocity: its exact solution,
// x(t) = target + (d + (v + omega d) t) e^{-omega t}, d = x - target, so any cut of the time into steps ends in the same place and
// it never overshoots a target it starts at rest from.  omega (1/s) is how stiff: about 4.7 / omega seconds to come within 5 %.  For
// scalars and vectors alike.  A negative omega or dt is std::invalid_argument.
template<class V, class T> inline void
critically_damped(V &value, V &velocity, V const &target, T omega, T dt) {
	detail::require_not_negative(omega, "critically_damped: omega");
	detail::require_not_negative(dt, "critically_damped: dt");
	using std::exp;
	V const d = value - target;
	V const a = velocity + d * omega;
	T const e = exp(-omega * dt);
	value = target + (d + a * dt) * e;
	velocity = (velocity - a * (omega * dt)) * e;
}

}

