// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file random.h
 * @brief pcg32: a small, fast random number generator whose numbers are the same on every machine
 *
 * O'Neill's PCG32 (XSH-RR): 64 bits of state, 32-bit outputs, independent streams.  Its state is its value, so a copy is a saved
 * game or a rollback, and two machines seeded alike draw alike.  The distributions are its own, never the standard library's (whose
 * uniform_int_distribution, normal_distribution and shuffle differ between libraries): below(n) and uniform() are the same bits
 * everywhere; normal() and unit_vector() go through log, sin and cos, the same everywhere on dfloat (rove's own), within a rounding of
 * each other on float and double (the platform's).
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include "scalar.h"
#include "vec.h"

namespace rove
{

class pcg32
{
public:
	using result_type = std::uint32_t;

	/// The generator of `seed` on stream `stream`: different streams give unrelated numbers from the same seed.  (pcg32_srandom_r.)
	explicit pcg32(std::uint64_t seed, std::uint64_t stream = 0xda3e39cb94b95bdbull):
		state_(0),
		increment_((stream << 1) | 1)
	{
		next();
		state_ += seed;
		next();
	}

	/// The next 32 random bits
	std::uint32_t next()
	{
		std::uint64_t const old = state_;
		state_ = old * 6364136223846793005ull + increment_;
		std::uint32_t const shifted = std::uint32_t(((old >> 18) ^ old) >> 27);
		std::uint32_t const rotation = std::uint32_t(old >> 59);
		return (shifted >> rotation) | (shifted << ((32 - rotation) & 31));
	}

	/// A uniform random bit generator, for whatever wants one (the standard library's distributions differ between libraries, though:
	/// the ones below do not)
	result_type operator()() { return next(); }
	static constexpr result_type (min)() { return 0; }
	static constexpr result_type (max)() { return (std::numeric_limits<result_type>::max)(); }

	/// A whole number in [0, n), each as likely as the others (Lemire's method: no modulo's bias).  n of 0 is std::invalid_argument.
	std::uint32_t below(std::uint32_t n)
	{
		if (n == 0)
			throw std::invalid_argument("rove: pcg32::below(0): there is no number below 0");
		std::uint64_t m = std::uint64_t(next()) * n;
		std::uint32_t low = std::uint32_t(m);
		if (low < n) {
			std::uint32_t const threshold = std::uint32_t(-n) % n;
			while (low < threshold) {
				m = std::uint64_t(next()) * n;
				low = std::uint32_t(m);
			}
		}
		return std::uint32_t(m >> 32);
	}

	/// A number in [0, 1): 24 random bits for a float (and dfloat), 53 for a double, exactly.  A whole-number type holds none of
	/// these: it does not compile.
	template<class T>
	T unit()
	{
		static_assert(!std::is_integral_v<T>, "rove: pcg32::unit of a whole-number type is always 0: use below() or uniform()");
		if constexpr (std::is_same_v<T, double> || std::is_same_v<T, long double>) {
			// two statements: in one expression the order of the draws is the compiler's, and the double with it
			std::uint64_t const high = next();
			std::uint64_t const low = next();
			std::uint64_t const bits = (high << 21) ^ (low >> 11);
			return T(double(bits) * 0x1p-53);
		} else {
			return T(float(next() >> 8) * 0x1p-24f);
		}
	}

	/// lo + (hi - lo) unit(): in [lo, hi] (hi only where the sum rounds up to it); of a whole-number type, each of lo to hi as likely
	/// as the others, hi included.  lo above hi is std::invalid_argument.
	template<class T>
	T uniform(T lo, T hi)
	{
		if (!(lo <= hi)) {
			using std::to_string;
			throw std::invalid_argument("rove: pcg32::uniform from " + to_string(lo) + " to " + to_string(hi));
		}
		if constexpr (std::is_integral_v<T>) {
			using wide = std::make_unsigned_t<T>;
			std::uint64_t const span = std::uint64_t(wide(hi) - wide(lo));       // how many above lo; modulo the width, so never overflows
			return T(wide(lo) + wide(whole_up_to(span)));
		} else {
			return lo + (hi - lo) * unit<T>();
		}
	}

	/// True with probability p (unit() < p): never for 0, always for 1
	template<class T>
	bool chance(T p)
	{
		return unit<T>() < p;
	}

	/// Normally distributed, of this mean and standard deviation (Box-Muller, one of its pair).  A negative deviation is
	/// std::invalid_argument.
	template<class T>
	T normal(T mean, T deviation)
	{
		if (!(deviation >= 0)) {
			using std::to_string;
			throw std::invalid_argument("rove: pcg32::normal of deviation " + to_string(deviation));
		}
		using std::sqrt, std::log, std::cos;
		T const u = 1 - unit<T>();          // (0, 1]: no log of 0
		T const v = unit<T>();
		return mean + deviation * (sqrt(-2 * log(u)) * cos(2 * std::numbers::pi_v<T> * v));
	}

	/// A direction uniformly over the sphere
	template<class T>
	vec<3, T> unit_vector()
	{
		using std::sqrt, std::sin, std::cos;
		T const z = uniform(T(-1), T(1));
		T const a = 2 * std::numbers::pi_v<T> * unit<T>();
		T const r = sqrt((std::max)(T(0), 1 - z * z));
		return vec<3, T>(r * cos(a), r * sin(a), z);
	}

	/// The range in a random order (Fisher-Yates through below(): the same order everywhere, as std::shuffle is not)
	template<class It>
	void shuffle(It first, It last)
	{
		auto const count = std::distance(first, last);
		for (auto i = count - 1; i > 0; --i) {
			using std::swap;
			swap(first[i], first[below(std::uint32_t(i + 1))]);
		}
	}

	/// An index into the weights, each as likely as its weight's share of their sum.  A negative weight, or a sum that is not more
	/// than zero, is std::invalid_argument.
	template<class It>
	std::size_t weighted(It first, It last)
	{
		using weight_type = std::remove_cvref_t<decltype(*first)>;
		weight_type total = 0;
		for (It w = first; w != last; ++w) {
			if (!(*w >= 0))
				throw std::invalid_argument("rove: pcg32::weighted: a negative weight");
			total += *w;
		}
		if (!(total > 0))
			throw std::invalid_argument("rove: pcg32::weighted: the weights add up to nothing");
		// (a whole-number total is drawn whole: unit() of one would be 0 every time)
		weight_type pick;
		if constexpr (std::is_integral_v<weight_type>) pick = uniform(weight_type(0), weight_type(total - 1));
		else pick = unit<weight_type>() * total;
		weight_type sum = 0;
		std::size_t index = 0, last_positive = 0;
		for (It w = first; w != last; ++w, ++index) {
			if (!(*w > 0)) continue;
			last_positive = index;
			sum += *w;
			if (pick < sum) return index;
		}
		return last_positive;          // pick rounded up to the total: the last that has any weight
	}

	bool operator==(pcg32 const &other) const = default;

private:
	/// A whole number in [0, span], each as likely as the others
	std::uint64_t whole_up_to(std::uint64_t span)
	{
		if (span < (std::numeric_limits<std::uint32_t>::max)())
			return below(std::uint32_t(span) + 1);
		// by rejection, of 64 bits under the smallest mask that covers the span
		std::uint64_t mask = span;
		for (int shift = 1; shift < 64; shift *= 2)
			mask |= mask >> shift;
		while (true) {
			std::uint64_t const high = next();
			std::uint64_t const low = next();
			std::uint64_t const candidate = ((high << 32) | low) & mask;
			if (candidate <= span) return candidate;
		}
	}

	std::uint64_t state_, increment_;
};

}
