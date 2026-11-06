# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's portable randomness (random.h, hash.h, noise.h): the same numbers as rove's C++ from the same
seed, wherever they are drawn."""
import copy
import unittest

import numpy as np

import pyrove


class TestPcg32(unittest.TestCase):
    def test_it_is_the_reference_generator(self):
        rng = pyrove.pcg32(42, 54)                      # pcg32-demo's first six, as rove's C++ test has them
        self.assertEqual([rng.next() for _ in range(6)],
                         [0xa15c02b7, 0x7b47f409, 0xba1d3330, 0x83d2f293, 0xbfa4784b, 0xcbed606e])

    def test_seeds_and_streams_differ_and_a_copy_is_a_rollback(self):
        a = pyrove.pcg32(7)
        saved = copy.copy(a)
        first = a.next()
        self.assertNotEqual(a, saved)
        again = copy.deepcopy(saved)
        self.assertEqual(again.next(), first)
        self.assertEqual(again, a)
        self.assertNotEqual(pyrove.pcg32(8).next(), first)
        self.assertNotEqual(pyrove.pcg32(7, 1).next(), pyrove.pcg32(7, 2).next())

    def test_seeds_are_64_bits_and_never_negative(self):
        self.assertNotEqual(pyrove.pcg32(2**64 - 1).next(), pyrove.pcg32(2**63 - 1).next())
        with self.assertRaises(TypeError):
            pyrove.pcg32(-1)
        with self.assertRaises(TypeError):
            pyrove.pcg32(2**64)

    def test_below_is_in_range_unbiased_and_refuses_zero(self):
        rng = pyrove.pcg32(1)
        n = 3 << 30
        low = sum(rng.below(n) < (1 << 30) for _ in range(30000))
        self.assertAlmostEqual(low / 30000, 1 / 3, delta=0.02)
        self.assertEqual(rng.below(1), 0)
        with self.assertRaises(ValueError):
            rng.below(0)

    def test_unit_is_53_bits_of_the_next_two_numbers(self):
        a, b = pyrove.pcg32(5), pyrove.pcg32(5)
        for _ in range(1000):
            u = a.unit()
            hi, lo = b.next(), b.next()
            bits = ((hi << 21) ^ (lo >> 11)) & ((1 << 53) - 1)
            self.assertEqual(u, bits * 2.0 ** -53)
            self.assertTrue(0 <= u < 1)

    def test_uniform_normal_and_chance(self):
        rng = pyrove.pcg32(9)
        xs = [rng.uniform(-2.0, 3.0) for _ in range(20000)]
        self.assertTrue(all(-2 <= x <= 3 for x in xs))
        self.assertAlmostEqual(sum(xs) / len(xs), 0.5, delta=0.05)
        with self.assertRaises(ValueError):
            rng.uniform(1.0, 0.0)
        ns = np.array([rng.normal(3.0, 2.0) for _ in range(20000)])
        self.assertAlmostEqual(ns.mean(), 3.0, delta=0.06)
        self.assertAlmostEqual(ns.std(), 2.0, delta=0.06)
        with self.assertRaises(ValueError):
            rng.normal(0.0, -1.0)
        self.assertFalse(any(rng.chance(0.0) for _ in range(1000)))
        self.assertTrue(all(rng.chance(1.0) for _ in range(1000)))

    def test_unit_vectors_are_unit_and_cover_the_sphere(self):
        rng = pyrove.pcg32(3)
        vs = [rng.unit_vector() for _ in range(5000)]
        self.assertIsInstance(vs[0], pyrove.vec3)
        self.assertTrue(all(abs(v.length() - 1) < 1e-5 for v in vs))
        mean = np.mean([tuple(v) for v in vs], axis=0)
        self.assertTrue(np.all(np.abs(mean) < 0.05))

    def test_shuffle_is_in_place_a_permutation_and_the_same_from_the_same_seed(self):
        items = list(range(20))
        pyrove.pcg32(11).shuffle(items)
        self.assertEqual(sorted(items), list(range(20)))
        self.assertNotEqual(items, list(range(20)))
        other = [str(k) for k in range(20)]
        pyrove.pcg32(11).shuffle(other)
        self.assertEqual(other, [str(k) for k in items])      # the order depends on the seed, not the items
        empty = []
        pyrove.pcg32(1).shuffle(empty)
        self.assertEqual(empty, [])

    def test_shuffle_is_rove_s_fisher_yates(self):
        # from the last item down, each swapped with one at or below it, picked by below(): what C++'s shuffle of the list does
        twin = pyrove.pcg32(23)
        want = list("abcdefghijklmnop")
        for i in range(len(want) - 1, 0, -1):
            j = twin.below(i + 1)
            want[i], want[j] = want[j], want[i]
        got = list("abcdefghijklmnop")
        rng = pyrove.pcg32(23)
        rng.shuffle(got)
        self.assertEqual(got, want)
        self.assertEqual(rng, twin)                 # and it drew as many numbers

    def test_weighted_picks_by_share_and_refuses_bad_weights(self):
        rng = pyrove.pcg32(4)
        counts = [0, 0, 0]
        for _ in range(30000):
            counts[rng.weighted([1.0, 0.0, 3.0])] += 1
        self.assertEqual(counts[1], 0)
        self.assertAlmostEqual(counts[2] / 30000, 0.75, delta=0.02)
        for bad in ([1.0, -0.5], [0.0, 0.0], []):
            with self.assertRaises(ValueError):
                rng.weighted(bad)


class TestHash(unittest.TestCase):
    def test_splitmix64_is_the_reference_mix(self):
        self.assertEqual(pyrove.splitmix64(0), 0xe220a8397b1dcdaf)
        self.assertNotEqual(pyrove.hash_combine(1, 2), pyrove.hash_combine(2, 1))
        self.assertLess(pyrove.splitmix64(2**64 - 1), 2**64)

    def test_hash_to_unit_takes_the_top_bits(self):
        self.assertEqual(pyrove.hash_to_unit(0), 0.0)
        self.assertEqual(pyrove.hash_to_unit(1 << 63), 0.5)
        self.assertLess(pyrove.hash_to_unit(2**64 - 1), 1.0)


class TestNoise(unittest.TestCase):
    def test_value_noise_is_bounded_continuous_and_the_seeds_own(self):
        for k in range(500):
            x = k * 0.037 - 9
            for args in ((x,), (x, x * 0.5), (x, 1.5, -x)):
                n = pyrove.value_noise(*args, seed=7)
                self.assertTrue(-1 <= n <= 1)
                nudged = pyrove.value_noise(*(a + 1e-4 for a in args), seed=7)
                self.assertLess(abs(nudged - n), 0.01)
        self.assertNotEqual(pyrove.value_noise(3.3, seed=7), pyrove.value_noise(3.3, seed=8))

    def test_fbm_is_within_its_amplitudes_and_needs_an_octave(self):
        total = 1 + 0.5 + 0.25 + 0.125
        for k in range(200):
            self.assertLessEqual(abs(pyrove.fbm(k * 0.11, k * 0.07, 4, 2.0, 0.5, 1)), total)
        with self.assertRaises(ValueError):
            pyrove.fbm(0.0, 0.0, 0, 2.0, 0.5, 1)


if __name__ == "__main__":
    unittest.main(verbosity=2)
