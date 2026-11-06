// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file hash.h
 * @brief Integer hashes for procedural content: the same numbers on every machine
 *
 * splitmix64 is the finaliser of Steele, Lea and Flood's SplitMix generator: every bit of the input moves every bit of the output.
 * Hash a lattice point, a seed and an index with it, and take a number in [0, 1) of the result with hash_to_unit: the same tree in
 * the same place on every platform, whatever order things were made in.
 */

#pragma once

#include <cstdint>
#include <type_traits>

namespace rove
{

/// x + the golden gamma, mixed: splitmix64's output for state x
constexpr std::uint64_t splitmix64(std::uint64_t x)
{
	x += 0x9e3779b97f4a7c15ull;
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
	return x ^ (x >> 31);
}

/// seed and value hashed together; the order matters (hash_combine(a, b) is not hash_combine(b, a))
constexpr std::uint64_t hash_combine(std::uint64_t seed, std::uint64_t value)
{
	return splitmix64(splitmix64(seed) ^ value);
}

/// A number in [0, 1) from a hash's top bits: 24 of them for a float (and a float's kind, like dfloat), 53 for a double, so it is
/// exact and the same everywhere
template<class T>
T hash_to_unit(std::uint64_t h)
{
	if constexpr (std::is_same_v<T, double> || std::is_same_v<T, long double>) {
		return T(double(h >> 11) * 0x1p-53);
	} else {
		return T(float(h >> 40) * 0x1p-24f);
	}
}

}
