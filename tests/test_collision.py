# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

import unittest
import sys
import os

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestFirstContact(unittest.TestCase):
    def setUp(self):
        self.bar = pyrove.capsule3(pyrove.vec3(-1.0, 0.0, 0.0), pyrove.vec3(1.0, 0.0, 0.0), 0.1)

    def test_a_falling_ball_meets_a_bar(self):
        hit = pyrove.first_contact(self.bar, self.bar, pyrove.sphere3(pyrove.vec3(0.3, 2.0, 0.0), 0.1),
                                   pyrove.vec3(0.0, -3.0, 0.0), 1e-4)
        self.assertIsNotNone(hit)
        self.assertLessEqual(hit["time"], 0.6 + 1e-6)
        self.assertGreater(hit["time"], 0.6 - 1e-3)
        self.assertAlmostEqual(hit["normal"].y, 1.0, places=5)
        self.assertAlmostEqual(hit["position"].y, 0.1, places=3)
        self.assertFalse(hit["penetrated"])

    def test_a_miss_is_none(self):
        hit = pyrove.first_contact(self.bar, self.bar, pyrove.sphere3(pyrove.vec3(-3.0, 0.21, 0.0), 0.1),
                                   pyrove.vec3(6.0, 0.0, 0.0), 1e-4)
        self.assertIsNone(hit)

    def test_overlap_at_the_start(self):
        hit = pyrove.first_contact(self.bar, self.bar, pyrove.sphere3(pyrove.vec3(0.5, 0.15, 0.0), 0.1),
                                   pyrove.vec3(0.0, 0.0, 0.0), 1e-4)
        self.assertEqual(hit["time"], 0.0)
        self.assertTrue(hit["penetrated"])
        self.assertAlmostEqual(hit["penetration_depth"], 0.05, places=5)

    def test_a_tolerance_that_is_not_positive_is_refused(self):
        with self.assertRaises(ValueError):
            pyrove.first_contact(self.bar, self.bar, pyrove.sphere3(pyrove.vec3(0.0, 1.0, 0.0), 0.1),
                                 pyrove.vec3(0.0, -1.0, 0.0), 0.0)

    def test_in_the_plane(self):
        c = pyrove.capsule2(pyrove.vec2(-1.0, 0.0), pyrove.vec2(1.0, 0.0), 0.1)
        hit = pyrove.first_contact(c, c, pyrove.sphere2(pyrove.vec2(0.0, 1.0), 0.1), pyrove.vec2(0.0, -1.0), 1e-4)
        self.assertAlmostEqual(hit["time"], 0.8, places=3)


if __name__ == '__main__':
    unittest.main()
