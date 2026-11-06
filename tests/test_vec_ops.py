# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's vector operations (vec.h): comparisons on every component by name, component-wise min, max, abs and clamp, projections,
reflections and angles, and vec4 built from a vec3.  For float and double vectors alike; each answer is of its arguments' type."""
import math
import unittest

import pyrove


class TestEveryComponent(unittest.TestCase):
    def test_all_less_and_friends_ask_every_component(self):
        a, b = pyrove.vec3(0, 1, 2), pyrove.vec3(1, 2, 3)
        self.assertTrue(pyrove.all_less(a, b))
        self.assertFalse(pyrove.all_less(b, a))
        mixed = pyrove.vec3(0, 5, 0)
        self.assertFalse(pyrove.all_less(mixed, b))
        self.assertFalse(pyrove.all_greater(mixed, b))
        self.assertTrue(pyrove.all_less_equal(a, a))
        self.assertFalse(pyrove.all_less(a, a))
        self.assertTrue(pyrove.all_greater_equal(b, a))
        self.assertTrue(pyrove.all_greater(pyrove.vec2d(2, 2), pyrove.vec2d(1, 1)))

    def test_min_max_abs_clamp_go_component_by_component(self):
        a, b = pyrove.vec3(1, -5, 3), pyrove.vec3(2, -6, 3)
        self.assertEqual(pyrove.min(a, b), pyrove.vec3(1, -6, 3))
        self.assertEqual(pyrove.max(a, b), pyrove.vec3(2, -5, 3))
        self.assertEqual(pyrove.abs(pyrove.vec3d(-1, 2, -0.5)), pyrove.vec3d(1, 2, 0.5))
        # clamped component by component: each bound's own component, not the vector "all below"
        v = pyrove.clamp(pyrove.vec3(-2, 0.5, 9), pyrove.vec3(0, 0, 0), pyrove.vec3(1, 1, 1))
        self.assertEqual(v, pyrove.vec3(0, 0.5, 1))
        self.assertEqual(pyrove.clamp(pyrove.vec2d(-3, 3), -1.0, 1.0), pyrove.vec2d(-1, 1))
        self.assertIsInstance(pyrove.min(pyrove.vec4d(0, 0, 0, 0), pyrove.vec4d(1, 1, 1, 1)), pyrove.vec4d)

    def test_a_star_import_leaves_pythons_abs_min_and_max(self):
        namespace = {}
        exec("from pyrove import *\nresult = (min(3, 1), max(3, 1), abs(-2))", namespace)
        self.assertEqual(namespace["result"], (1, 3, 2))
        self.assertIn("clamp", namespace)
        self.assertIs(pyrove.min, pyrove.pyrove_bind.min)

    def test_is_finite(self):
        self.assertTrue(pyrove.is_finite(pyrove.vec3(1, 2, 3)))
        self.assertFalse(pyrove.is_finite(pyrove.vec3(1, float("nan"), 3)))
        self.assertFalse(pyrove.is_finite(pyrove.vec2d(float("inf"), 0)))


class TestProjections(unittest.TestCase):
    def test_project_and_reject_split_a_vector(self):
        v, onto = pyrove.vec3d(3, 4, 5), pyrove.vec3d(0, 0, 2)          # onto of any length
        p, r = pyrove.project(v, onto), pyrove.reject(v, onto)
        self.assertEqual(p, pyrove.vec3d(0, 0, 5))
        self.assertEqual(r, pyrove.vec3d(3, 4, 0))
        self.assertEqual(p + r, v)
        with self.assertRaises(ValueError):
            pyrove.project(v, pyrove.vec3d(0, 0, 0))
        with self.assertRaises(ValueError):
            pyrove.reject(pyrove.vec2(1, 1), pyrove.vec2(0, 0))

    def test_reflect_turns_round_the_part_along_the_normal(self):
        self.assertEqual(pyrove.reflect(pyrove.vec3d(1, -2, 3), pyrove.vec3d(0, 1, 0)), pyrove.vec3d(1, 2, 3))
        self.assertEqual(pyrove.reflect(pyrove.vec2(1, -1), pyrove.vec2(0, 1)), pyrove.vec2(1, 1))


class TestAngles(unittest.TestCase):
    def test_angle_between_is_unsigned_and_exact_for_tiny_ones(self):
        self.assertAlmostEqual(pyrove.angle_between(pyrove.vec3d(1, 0, 0), pyrove.vec3d(0, 5, 0)), math.pi / 2, places=12)
        self.assertAlmostEqual(pyrove.angle_between(pyrove.vec3d(1, 0, 0), pyrove.vec3d(-2, 0, 0)), math.pi, places=12)
        tiny = 1e-9
        self.assertAlmostEqual(pyrove.angle_between(pyrove.vec3d(1, 0, 0), pyrove.vec3d(1, tiny, 0)), tiny, delta=1e-15)
        self.assertAlmostEqual(pyrove.angle_between(pyrove.vec2d(0, 1), pyrove.vec2d(1, 0)), math.pi / 2, places=12)
        with self.assertRaises(ValueError):
            pyrove.angle_between(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0))

    def test_signed_angle_about_an_axis_is_right_handed(self):
        z = pyrove.vec3d(0, 0, 1)
        self.assertAlmostEqual(pyrove.signed_angle(pyrove.vec3d(1, 0, 0), pyrove.vec3d(0, 1, 0), z), math.pi / 2, places=12)
        self.assertAlmostEqual(pyrove.signed_angle(pyrove.vec3d(0, 1, 0), pyrove.vec3d(1, 0, 0), z), -math.pi / 2, places=12)
        # the parts along the axis do not count
        self.assertAlmostEqual(pyrove.signed_angle(pyrove.vec3d(1, 0, 7), pyrove.vec3d(0, 1, -3), z), math.pi / 2, places=12)
        with self.assertRaises(ValueError):
            pyrove.signed_angle(pyrove.vec3d(0, 0, 2), pyrove.vec3d(1, 0, 0), z)     # along the axis: no angle across it
        with self.assertRaises(ValueError):
            pyrove.signed_angle(pyrove.vec3d(1, 0, 0), pyrove.vec3d(0, 1, 0), pyrove.vec3d(0, 0, 0))

    def test_signed_angle_in_2d(self):
        self.assertAlmostEqual(pyrove.signed_angle(pyrove.vec2d(1, 0), pyrove.vec2d(0, 1)), math.pi / 2, places=12)
        self.assertAlmostEqual(pyrove.signed_angle(pyrove.vec2d(1, 0), pyrove.vec2d(0, -1)), -math.pi / 2, places=12)
        self.assertAlmostEqual(pyrove.signed_angle(pyrove.vec2(1, 0), pyrove.vec2(-1, -1e-3)), -math.pi + 1e-3, places=5)


class TestVec4(unittest.TestCase):
    def test_a_vec4_of_a_vec3_and_w(self):
        v = pyrove.vec4(pyrove.vec3(1, 2, 3), 4)
        self.assertEqual((v.x, v.y, v.z, v.w), (1, 2, 3, 4))
        self.assertEqual(pyrove.vec4d(pyrove.vec3d(1, 2, 3), 0.5).w, 0.5)

    def test_component_wise_product_and_quotient(self):
        a, b = pyrove.vec4(1, 2, 3, 4), pyrove.vec4(2, 4, 6, 8)
        self.assertEqual(a * b, pyrove.vec4(2, 8, 18, 32))
        self.assertEqual(b / a, pyrove.vec4(2, 2, 2, 2))


if __name__ == "__main__":
    unittest.main(verbosity=2)
