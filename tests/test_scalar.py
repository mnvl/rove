# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's scalar helpers (scalar.h): angles, wraps, remaps and the smoothing steps, on Python's floats (doubles), and the steps on
vectors too.  The oracles are Python's math, not rove."""
import math
import unittest

import pyrove


class TestAngles(unittest.TestCase):
    def test_radians_and_degrees_are_exact_at_the_round_angles(self):
        self.assertEqual(pyrove.radians(180.0), math.pi)
        self.assertEqual(pyrove.radians(90.0), math.pi / 2)
        self.assertEqual(pyrove.degrees(math.pi), 180.0)
        self.assertAlmostEqual(pyrove.degrees(pyrove.radians(37.5)), 37.5, places=12)

    def test_wrap_to_pi_is_the_half_open_turn(self):
        for a in [-7.0, -math.pi, -1.0, 0.0, 1.0, math.pi, 4.0, 100.0]:
            w = pyrove.wrap_to_pi(a)
            self.assertTrue(-math.pi < w <= math.pi, (a, w))
            self.assertAlmostEqual(math.cos(w), math.cos(a), places=9)
            self.assertAlmostEqual(math.sin(w), math.sin(a), places=9)
        self.assertEqual(pyrove.wrap_to_pi(math.pi), math.pi)       # (-pi, pi]: pi stays, -pi is pi
        self.assertEqual(pyrove.wrap_to_pi(-math.pi), math.pi)
        self.assertEqual(pyrove.wrap_to_pi(0.25), 0.25)              # in range: bit for bit

    def test_wrap_to_two_pi_and_wrap(self):
        self.assertAlmostEqual(pyrove.wrap_to_two_pi(-0.5), 2 * math.pi - 0.5, places=12)
        self.assertEqual(pyrove.wrap_to_two_pi(0.0), 0.0)
        self.assertAlmostEqual(pyrove.wrap(370.0, -180.0, 180.0), 10.0, places=9)
        self.assertEqual(pyrove.wrap(180.0, -180.0, 180.0), -180.0)
        with self.assertRaises(ValueError):
            pyrove.wrap(1.0, 1.0, 5.0)                              # [lo, hi) must hold zero

    def test_delta_angle_goes_the_short_way_across_the_seam(self):
        self.assertAlmostEqual(pyrove.delta_angle(3.0, -3.0), 2 * math.pi - 6.0, places=12)
        self.assertAlmostEqual(pyrove.delta_angle(-3.0, 3.0), 6.0 - 2 * math.pi, places=12)
        self.assertAlmostEqual(pyrove.delta_angle(0.5, 1.5), 1.0, places=12)


class TestRemaps(unittest.TestCase):
    def test_fract_is_what_floor_leaves(self):
        self.assertEqual(pyrove.fract(-0.25), 0.75)
        self.assertEqual(pyrove.fract(2.5), 0.5)
        self.assertEqual(pyrove.fract(3.0), 0.0)

    def test_lerp_inverse_lerp_and_remap_are_unclamped(self):
        self.assertEqual(pyrove.lerp(2.0, 4.0, 1.5), 5.0)
        self.assertEqual(pyrove.inverse_lerp(2.0, 4.0, 5.0), 1.5)
        self.assertEqual(pyrove.inverse_lerp(4.0, 2.0, 3.0), 0.5)
        self.assertEqual(pyrove.remap(15.0, 10.0, 20.0, 0.0, -100.0), -50.0)
        self.assertEqual(pyrove.remap(30.0, 10.0, 20.0, 0.0, 1.0), 2.0)
        with self.assertRaises(ValueError):
            pyrove.inverse_lerp(1.0, 1.0, 1.0)
        with self.assertRaises(ValueError):
            pyrove.remap(1.0, 3.0, 3.0, 0.0, 1.0)

    def test_smoothstep_is_glsls(self):
        for x in [-1.0, 0.0, 0.1, 0.5, 0.8, 1.0, 2.0]:
            t = min(max(x, 0.0), 1.0)
            self.assertAlmostEqual(pyrove.smoothstep(0.0, 1.0, x), t * t * (3 - 2 * t), places=12)
        self.assertAlmostEqual(pyrove.smoothstep(1.0, 0.0, 0.2), 1 - pyrove.smoothstep(0.0, 1.0, 0.2), places=12)
        with self.assertRaises(ValueError):
            pyrove.smoothstep(2.0, 2.0, 2.0)


class TestSteps(unittest.TestCase):
    def test_damp_is_exponential_and_does_not_care_how_time_is_cut(self):
        self.assertAlmostEqual(pyrove.damp(0.0, 10.0, 2.0, 0.5), 10.0 * (1 - math.exp(-1.0)), places=12)
        once = pyrove.damp(1.0, 5.0, 3.0, 0.2)
        twice = pyrove.damp(pyrove.damp(1.0, 5.0, 3.0, 0.1), 5.0, 3.0, 0.1)
        self.assertAlmostEqual(once, twice, places=12)
        self.assertLessEqual(pyrove.damp(1.0, 5.0, 1e6, 10.0), 5.0)     # never past the target
        for bad in [(-1.0, 0.1), (1.0, -0.1), (float("nan"), 0.1)]:
            with self.assertRaises(ValueError):
                pyrove.damp(0.0, 1.0, *bad)

    def test_damp_of_vectors_is_damp_of_each_component(self):
        v = pyrove.damp(pyrove.vec3d(0, 10, -4), pyrove.vec3d(2, 0, 4), 1.5, 0.3)
        k = math.exp(-0.45)
        for got, a, b in zip(v, (0, 10, -4), (2, 0, 4)):
            self.assertAlmostEqual(got, b + (a - b) * k, places=12)
        v2 = pyrove.damp(pyrove.vec2(0, 1), pyrove.vec2(1, 1), 1.0, 1.0)
        self.assertIsInstance(v2, pyrove.vec2)
        self.assertAlmostEqual(v2.x, 1 - math.exp(-1.0), places=6)

    def test_move_towards_lands_exactly_and_never_passes(self):
        self.assertEqual(pyrove.move_towards(0.0, 1.0, 0.25), 0.25)
        self.assertEqual(pyrove.move_towards(0.0, -1.0, 0.25), -0.25)
        self.assertEqual(pyrove.move_towards(0.9, 1.0, 0.25), 1.0)
        with self.assertRaises(ValueError):
            pyrove.move_towards(0.0, 1.0, -0.1)
        v = pyrove.move_towards(pyrove.vec3d(0, 0, 0), pyrove.vec3d(3, 4, 0), 1.0)
        self.assertAlmostEqual(v.x, 0.6, places=12)
        self.assertAlmostEqual(v.y, 0.8, places=12)
        self.assertEqual(pyrove.move_towards(pyrove.vec3d(0, 0, 0), pyrove.vec3d(3, 4, 0), 5.0), pyrove.vec3d(3, 4, 0))
        with self.assertRaises(ValueError):
            pyrove.move_towards(pyrove.vec2(0, 0), pyrove.vec2(1, 0), -1.0)

    def test_critically_damped_is_the_exact_solution(self):
        omega, x0, v0, target = 3.0, 2.0, -1.0, 0.5
        for t in [0.0, 0.1, 0.7, 2.0]:
            x, v = pyrove.critically_damped(x0, v0, target, omega, t)
            d = x0 - target
            want = target + (d + (v0 + omega * d) * t) * math.exp(-omega * t)
            self.assertAlmostEqual(x, want, places=12)
            # the velocity is the solution's derivative
            h = 1e-6
            ahead, _ = pyrove.critically_damped(x0, v0, target, omega, t + h)
            self.assertAlmostEqual(v, (ahead - x) / h, places=4)
        # any cut of the time ends in the same place
        x, v = x0, v0
        for _ in range(10):
            x, v = pyrove.critically_damped(x, v, target, omega, 0.05)
        once, _ = pyrove.critically_damped(x0, v0, target, omega, 0.5)
        self.assertAlmostEqual(x, once, places=12)
        with self.assertRaises(ValueError):
            pyrove.critically_damped(0.0, 0.0, 1.0, -1.0, 0.1)
        with self.assertRaises(ValueError):
            pyrove.critically_damped(0.0, 0.0, 1.0, 1.0, -0.1)

    def test_critically_damped_vectors_from_rest_never_overshoot(self):
        x, v = pyrove.vec3d(0, 0, 0), pyrove.vec3d(0, 0, 0)
        target = pyrove.vec3d(1, -2, 3)
        for _ in range(200):
            x, v = pyrove.critically_damped(x, v, target, 4.0, 0.02)
            self.assertLessEqual(x.x, 1.0)
            self.assertGreaterEqual(x.y, -2.0)
        self.assertAlmostEqual(x.z, 3.0, places=4)
        xf, vf = pyrove.critically_damped(pyrove.vec2(0, 0), pyrove.vec2(0, 0), pyrove.vec2(1, 1), 2.0, 0.1)
        self.assertIsInstance(xf, pyrove.vec2)
        self.assertIsInstance(vf, pyrove.vec2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
