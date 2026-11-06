// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file noise.h
 * @brief Value noise and its octaves: smooth random fields for terrain, wind and wear, the same on every machine
 *
 * A random value in [-1, 1) at every lattice point (hash.h of the point and a seed), blended between them with the quintic fade
 * 6t^5 - 15t^4 + 10t^3, so the field and its slope are continuous.  On dfloat the same bits everywhere.  The lattice is indexed by
 * 64-bit integers: a coordinate must stay within their range.
 */

#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include "hash.h"
#include "scalar.h"

namespace rove
{

namespace detail
{

template<class T> inline T
noise_fade(T t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

template<class T> inline T
lattice(std::uint64_t seed, std::int64_t x, std::int64_t y = 0, std::int64_t z = 0)
{
	std::uint64_t const h = hash_combine(hash_combine(hash_combine(seed, std::uint64_t(x)), std::uint64_t(y)), std::uint64_t(z));
	return 2 * hash_to_unit<T>(h) - 1;
}

template<class T> inline std::int64_t
cell(T x)
{
	using std::floor;
	return static_cast<std::int64_t>(floor(x));
}

}

/// Value noise along a line, in [-1, 1]
template<class T> inline T
value_noise(T x, std::uint64_t seed)
{
	std::int64_t const i = detail::cell(x);
	T const t = detail::noise_fade(x - T(i));
	return lerp(detail::lattice<T>(seed, i), detail::lattice<T>(seed, i + 1), t);
}

/// Value noise over a plane, in [-1, 1]
template<class T> inline T
value_noise(T x, T y, std::uint64_t seed)
{
	std::int64_t const i = detail::cell(x), j = detail::cell(y);
	T const u = detail::noise_fade(x - T(i)), v = detail::noise_fade(y - T(j));
	T const bottom = lerp(detail::lattice<T>(seed, i, j), detail::lattice<T>(seed, i + 1, j), u);
	T const top = lerp(detail::lattice<T>(seed, i, j + 1), detail::lattice<T>(seed, i + 1, j + 1), u);
	return lerp(bottom, top, v);
}

/// Value noise through space, in [-1, 1]
template<class T> inline T
value_noise(T x, T y, T z, std::uint64_t seed)
{
	std::int64_t const i = detail::cell(x), j = detail::cell(y), k = detail::cell(z);
	T const u = detail::noise_fade(x - T(i)), v = detail::noise_fade(y - T(j)), w = detail::noise_fade(z - T(k));
	T layer[2];
	for (int dz = 0; dz < 2; ++dz) {
		T const bottom = lerp(detail::lattice<T>(seed, i, j, k + dz), detail::lattice<T>(seed, i + 1, j, k + dz), u);
		T const top = lerp(detail::lattice<T>(seed, i, j + 1, k + dz), detail::lattice<T>(seed, i + 1, j + 1, k + dz), u);
		layer[dz] = lerp(bottom, top, v);
	}
	return lerp(layer[0], layer[1], w);
}

/**
 * @brief Fractal noise over a plane: `octaves` layers of value noise, each `lacunarity` times finer and `gain` times as strong as
 * the last, each from a seed of its own
 *
 * Its size is at most the sum of the amplitudes, 1 + gain + gain^2 ...  No octaves is std::invalid_argument.
 */
template<class T> inline T
fbm(T x, T y, int octaves, T lacunarity, T gain, std::uint64_t seed)
{
	if (octaves < 1)
		throw std::invalid_argument("rove: fbm of no octaves");
	T sum = 0, amplitude = 1, frequency = 1;
	for (int o = 0; o < octaves; ++o) {
		sum += amplitude * value_noise(x * frequency, y * frequency, hash_combine(seed, std::uint64_t(o)));
		amplitude *= gain;
		frequency *= lacunarity;
	}
	return sum;
}

}
