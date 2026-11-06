// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <set>
#include <stdexcept>
#include <vector>
#include <boost/test/unit_test.hpp>
#include <rove/random.h>
#include <rove/hash.h>
#include <rove/noise.h>

BOOST_AUTO_TEST_SUITE(random_numbers)

BOOST_AUTO_TEST_CASE(pcg32_is_the_reference_generator)
{
	// pcg32-demo, seeded 42 and stream 54: its first six numbers
	rove::pcg32 rng(42, 54);
	std::uint32_t const expected[] = {0xa15c02b7u, 0x7b47f409u, 0xba1d3330u, 0x83d2f293u, 0xbfa4784bu, 0xcbed606eu};
	for (std::uint32_t e : expected)
		BOOST_REQUIRE_EQUAL(rng.next(), e);
}

BOOST_AUTO_TEST_CASE(a_copy_goes_on_as_the_original_and_seeds_differ)
{
	rove::pcg32 a(7), b(8);
	rove::pcg32 const saved = a;
	std::uint32_t const first = a.next();
	BOOST_REQUIRE(a != saved);
	rove::pcg32 again = saved;            // a rollback: the state is the value
	BOOST_REQUIRE_EQUAL(again.next(), first);
	BOOST_REQUIRE(again == a);
	BOOST_REQUIRE(b.next() != first);
}

BOOST_AUTO_TEST_CASE(a_double_is_the_next_number_high_and_the_one_after_low)
{
	// The two numbers a double is made of are drawn in a stated order, the first the high bits: in one expression the order of the
	// two draws is the compiler's (unspecified), and another compiler would make another double of the same seed.
	rove::pcg32 a(5), b(5);
	for (int k = 0; k < 1000; ++k) {
		std::uint64_t const high = b.next();
		std::uint64_t const low = b.next();
		BOOST_REQUIRE_EQUAL(a.unit<double>(), double((high << 21) ^ (low >> 11)) * 0x1p-53);
	}
}

BOOST_AUTO_TEST_CASE(below_is_in_range_and_unbiased)
{
	rove::pcg32 rng(1);
	// n = 3 * 2^30 is where a modulo is most biased: the low third twice as likely as the rest
	std::uint32_t const n = 3u << 30;
	int low = 0;
	int const draws = 300000;
	for (int k = 0; k < draws; ++k) {
		std::uint32_t const x = rng.below(n);
		BOOST_REQUIRE(x < n);
		low += x < (1u << 30);
	}
	BOOST_REQUIRE(std::abs(double(low) / draws - 1.0 / 3) < 0.01);       // a modulo gives one half
	std::vector<int> count(6, 0);
	for (int k = 0; k < 60000; ++k) ++count[rng.below(6)];
	for (int c : count) BOOST_REQUIRE(std::abs(c - 10000) < 500);
	BOOST_REQUIRE_EQUAL(rng.below(1), 0u);
	BOOST_REQUIRE_THROW(rng.below(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(uniform_reals_are_portable_bits_in_range)
{
	rove::pcg32 a(5), b(5);
	for (int k = 0; k < 10000; ++k) {
		float const f = a.uniform(-2.0f, 3.0f);
		BOOST_REQUIRE(f >= -2.0f && f <= 3.0f);
		// from 24 bits: exactly a multiple of 2^-24 of the way, no library distribution in between
		std::uint32_t const bits = b.next() >> 8;
		BOOST_REQUIRE_EQUAL(f, -2.0f + 5.0f * (float(bits) * 0x1p-24f));
	}
	double total = 0;
	for (int k = 0; k < 100000; ++k) total += a.uniform(0.0, 1.0);
	BOOST_REQUIRE(std::abs(total / 100000 - 0.5) < 0.005);
	BOOST_REQUIRE_THROW(a.uniform(1.0f, 0.0f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(normal_has_its_mean_and_spread)
{
	rove::pcg32 rng(9);
	double sum = 0, sum_sq = 0;
	int const draws = 200000;
	for (int k = 0; k < draws; ++k) {
		double const x = rng.normal(3.0, 2.0);
		BOOST_REQUIRE(std::isfinite(x));
		sum += x;
		sum_sq += x * x;
	}
	double const mean = sum / draws, sd = std::sqrt(sum_sq / draws - mean * mean);
	BOOST_REQUIRE(std::abs(mean - 3) < 0.03);
	BOOST_REQUIRE(std::abs(sd - 2) < 0.03);
	BOOST_REQUIRE_THROW(rng.normal(0.0f, -1.0f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(chance_unit_vectors_shuffle_and_weighted)
{
	rove::pcg32 rng(11);
	int yes = 0;
	for (int k = 0; k < 100000; ++k) yes += rng.chance(0.25f);
	BOOST_REQUIRE(std::abs(yes - 25000) < 600);
	BOOST_REQUIRE(!rng.chance(0.0f));
	BOOST_REQUIRE(rng.chance(1.0f));
	// unit vectors: of length one, and spread over the sphere (each octant about an eighth)
	std::vector<int> octant(8, 0);
	for (int k = 0; k < 80000; ++k) {
		rove::vec<3> const v = rng.unit_vector<float>();
		BOOST_REQUIRE(std::abs(v.length() - 1) < 1e-5f);
		++octant[(v.x > 0) + 2 * (v.y > 0) + 4 * (v.z > 0)];
	}
	for (int c : octant) BOOST_REQUIRE(std::abs(c - 10000) < 500);
	// a shuffle keeps every element and, from the same seed, the same order
	std::vector<int> v(50), w;
	std::iota(v.begin(), v.end(), 0);
	w = v;
	rove::pcg32 s1(3), s2(3);
	s1.shuffle(v.begin(), v.end());
	s2.shuffle(w.begin(), w.end());
	BOOST_REQUIRE(v == w);
	BOOST_REQUIRE(std::set<int>(v.begin(), v.end()).size() == 50u);
	BOOST_REQUIRE(!std::is_sorted(v.begin(), v.end()));
	// weighted: each index as often as its weight says, none for a weight of zero
	std::vector<float> const weights{1, 0, 3};
	std::vector<int> picked(3, 0);
	for (int k = 0; k < 40000; ++k) ++picked[rng.weighted(weights.begin(), weights.end())];
	BOOST_REQUIRE_EQUAL(picked[1], 0);
	BOOST_REQUIRE(std::abs(picked[0] - 10000) < 500);
	std::vector<float> const nothing{0, 0}, negative{1, -1};
	BOOST_REQUIRE_THROW(rng.weighted(nothing.begin(), nothing.end()), std::invalid_argument);
	BOOST_REQUIRE_THROW(rng.weighted(negative.begin(), negative.end()), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(hashes_and_noise)

BOOST_AUTO_TEST_CASE(splitmix64_is_the_reference_mix)
{
	// the splitmix64 generator from state 0 gives 0xe220a8397b1dcdaf first: its mix of 0 + the golden gamma
	BOOST_REQUIRE_EQUAL(rove::splitmix64(0), 0xe220a8397b1dcdafull);
	BOOST_REQUIRE(rove::splitmix64(1) != rove::splitmix64(2));
	BOOST_REQUIRE(rove::hash_combine(1, 2) != rove::hash_combine(2, 1));
}

BOOST_AUTO_TEST_CASE(hash_to_unit_is_in_0_1_and_from_the_top_bits)
{
	BOOST_REQUIRE_EQUAL(rove::hash_to_unit<float>(0), 0.0f);
	BOOST_REQUIRE(rove::hash_to_unit<float>(~0ull) < 1.0f);
	BOOST_REQUIRE(rove::hash_to_unit<double>(~0ull) < 1.0);
	BOOST_REQUIRE_EQUAL(rove::hash_to_unit<float>(1ull << 63), 0.5f);
	double total = 0;
	for (std::uint64_t k = 0; k < 100000; ++k) total += rove::hash_to_unit<double>(rove::splitmix64(k));
	BOOST_REQUIRE(std::abs(total / 100000 - 0.5) < 0.005);
}

BOOST_AUTO_TEST_CASE(value_noise_is_smooth_bounded_and_the_seeds_own)
{
	for (int k = 0; k < 2000; ++k) {
		float const x = k * 0.037f - 30;
		float const n = rove::value_noise(x, 7);
		BOOST_REQUIRE(n >= -1 && n <= 1);
		// continuous: a tiny step moves it a tiny way
		BOOST_REQUIRE(std::abs(rove::value_noise(x + 1e-3f, 7) - n) < 0.02f);
		float const n2 = rove::value_noise(x, x * 0.5f, 7);
		BOOST_REQUIRE(n2 >= -1 && n2 <= 1);
		BOOST_REQUIRE(std::abs(rove::value_noise(x, x * 0.5f + 1e-3f, 7) - n2) < 0.02f);
		float const n3 = rove::value_noise(x, 1.5f, -x, 7);
		BOOST_REQUIRE(n3 >= -1 && n3 <= 1);
	}
	// at the lattice points it is the lattice's value, and the same each time
	BOOST_REQUIRE_EQUAL(rove::value_noise(3.0f, 7), rove::value_noise(3.0f, 7));
	BOOST_REQUIRE(rove::value_noise(3.3f, 7) != rove::value_noise(3.3f, 8));
	// fbm: octaves of halving size, still within the sum of their amplitudes
	float const total_amplitude = 1 + 0.5f + 0.25f + 0.125f;
	for (int k = 0; k < 500; ++k) {
		float const f = rove::fbm(k * 0.11f, k * 0.07f, 4, 2.0f, 0.5f, 1);
		BOOST_REQUIRE(std::abs(f) <= total_amplitude);
	}
	BOOST_REQUIRE_THROW(rove::fbm(0.0f, 0.0f, 0, 2.0f, 0.5f, 1), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
