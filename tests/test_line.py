# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestLine2(unittest.TestCase):
    def test_default_constructor(self):
        ln = pyrove.segment2()
        self.assertEqual(ln.A.x, 0.0)
        self.assertEqual(ln.A.y, 0.0)
        self.assertEqual(ln.B.x, 0.0)
        self.assertEqual(ln.B.y, 0.0)

    def test_parameterized_constructor(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(4.0, 6.0)
        ln = pyrove.segment2(a, b)
        self.assertEqual(ln.A.x, 1.0)
        self.assertEqual(ln.A.y, 2.0)
        self.assertEqual(ln.B.x, 4.0)
        self.assertEqual(ln.B.y, 6.0)

    def test_construct(self):
        ln = pyrove.segment2()
        a = pyrove.vec2(2.0, 3.0)
        b = pyrove.vec2(5.0, 7.0)
        ln.construct(a, b)
        self.assertEqual(ln.A.x, 2.0)
        self.assertEqual(ln.A.y, 3.0)
        self.assertEqual(ln.B.x, 5.0)
        self.assertEqual(ln.B.y, 7.0)

    def test_direction(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(4.0, 6.0)
        ln = pyrove.segment2(a, b)
        d = ln.direction()
        self.assertAlmostEqual(d.x, 3.0)
        self.assertAlmostEqual(d.y, 4.0)

    def test_to_ray(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(4.0, 6.0)
        ln = pyrove.segment2(a, b)
        r = ln.to_ray()
        self.assertAlmostEqual(r.r0.x, 1.0)
        self.assertAlmostEqual(r.r0.y, 2.0)
        self.assertAlmostEqual(r.a.x, 3.0)
        self.assertAlmostEqual(r.a.y, 4.0)

    def test_bounds(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(4.0, 6.0)
        ln = pyrove.segment2(a, b)
        bb = ln.bounds()
        self.assertAlmostEqual(bb.lo.x, 1.0)
        self.assertAlmostEqual(bb.lo.y, 2.0)
        self.assertAlmostEqual(bb.hi.x, 4.0)
        self.assertAlmostEqual(bb.hi.y, 6.0)

    def test_length_sq(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(3.0, 4.0)
        ln = pyrove.segment2(a, b)
        self.assertAlmostEqual(ln.length_sq(), 25.0)

    def test_contains(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        ln = pyrove.segment2(a, b)

        # Point on the line segment
        self.assertTrue(ln.contains(pyrove.vec2(2.0, 0.0)))

        # Point on the line but outside the segment
        self.assertFalse(ln.contains(pyrove.vec2(5.0, 0.0)))

        # Point not on the line
        self.assertFalse(ln.contains(pyrove.vec2(2.0, 1.0)))

    def test_distance(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        ln = pyrove.segment2(a, b)

        # Point on the segment has zero distance
        self.assertAlmostEqual(ln.distance(pyrove.vec2(2.0, 0.0)), 0.0, places=5)

        # Point perpendicular to segment
        self.assertAlmostEqual(ln.distance(pyrove.vec2(2.0, 3.0)), 3.0, places=5)

        # Point beyond endpoint B
        d = ln.distance(pyrove.vec2(5.0, 3.0))
        expected = math.sqrt(1.0 + 9.0)  # Distance to point B
        self.assertAlmostEqual(d, expected, places=5)

    def test_distance_sq(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        ln = pyrove.segment2(a, b)

        # Point perpendicular to segment
        self.assertAlmostEqual(ln.distance_sq(pyrove.vec2(2.0, 3.0)), 9.0, places=5)

    def test_repr(self):
        ln = pyrove.segment2(pyrove.vec2(1.0, 2.0), pyrove.vec2(3.0, 4.0))
        s = repr(ln)
        self.assertIn("segment2", s)

    def test_property_modification(self):
        ln = pyrove.segment2()
        ln.A = pyrove.vec2(10.0, 20.0)
        ln.B = pyrove.vec2(30.0, 40.0)
        self.assertEqual(ln.A.x, 10.0)
        self.assertEqual(ln.A.y, 20.0)
        self.assertEqual(ln.B.x, 30.0)
        self.assertEqual(ln.B.y, 40.0)


class TestLine3(unittest.TestCase):
    def test_default_constructor(self):
        ln = pyrove.segment3()
        self.assertEqual(ln.A.x, 0.0)
        self.assertEqual(ln.A.y, 0.0)
        self.assertEqual(ln.A.z, 0.0)
        self.assertEqual(ln.B.x, 0.0)
        self.assertEqual(ln.B.y, 0.0)
        self.assertEqual(ln.B.z, 0.0)

    def test_parameterized_constructor(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 5.0, 6.0)
        ln = pyrove.segment3(a, b)
        self.assertEqual(ln.A.x, 1.0)
        self.assertEqual(ln.A.y, 2.0)
        self.assertEqual(ln.A.z, 3.0)
        self.assertEqual(ln.B.x, 4.0)
        self.assertEqual(ln.B.y, 5.0)
        self.assertEqual(ln.B.z, 6.0)

    def test_construct(self):
        ln = pyrove.segment3()
        a = pyrove.vec3(7.0, 8.0, 9.0)
        b = pyrove.vec3(10.0, 11.0, 12.0)
        ln.construct(a, b)
        self.assertEqual(ln.A.x, 7.0)
        self.assertEqual(ln.A.y, 8.0)
        self.assertEqual(ln.A.z, 9.0)
        self.assertEqual(ln.B.x, 10.0)
        self.assertEqual(ln.B.y, 11.0)
        self.assertEqual(ln.B.z, 12.0)

    def test_direction(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 6.0, 9.0)
        ln = pyrove.segment3(a, b)
        d = ln.direction()
        self.assertAlmostEqual(d.x, 3.0)
        self.assertAlmostEqual(d.y, 4.0)
        self.assertAlmostEqual(d.z, 6.0)

    def test_to_ray(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 5.0, 6.0)
        ln = pyrove.segment3(a, b)
        r = ln.to_ray()
        self.assertAlmostEqual(r.r0.x, 1.0)
        self.assertAlmostEqual(r.r0.y, 2.0)
        self.assertAlmostEqual(r.r0.z, 3.0)
        self.assertAlmostEqual(r.a.x, 3.0)
        self.assertAlmostEqual(r.a.y, 3.0)
        self.assertAlmostEqual(r.a.z, 3.0)

    def test_bounds(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 5.0, 6.0)
        ln = pyrove.segment3(a, b)
        bb = ln.bounds()
        self.assertAlmostEqual(bb.lo.x, 1.0)
        self.assertAlmostEqual(bb.lo.y, 2.0)
        self.assertAlmostEqual(bb.lo.z, 3.0)
        self.assertAlmostEqual(bb.hi.x, 4.0)
        self.assertAlmostEqual(bb.hi.y, 5.0)
        self.assertAlmostEqual(bb.hi.z, 6.0)

    def test_length_sq(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 3.0, 6.0)
        ln = pyrove.segment3(a, b)
        self.assertAlmostEqual(ln.length_sq(), 49.0)  # 4 + 9 + 36 = 49

    def test_contains(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(4.0, 0.0, 0.0)
        ln = pyrove.segment3(a, b)

        # Point on the line segment
        self.assertTrue(ln.contains(pyrove.vec3(2.0, 0.0, 0.0)))

        # Point on the line but outside the segment
        self.assertFalse(ln.contains(pyrove.vec3(5.0, 0.0, 0.0)))

        # Point not on the line
        self.assertFalse(ln.contains(pyrove.vec3(2.0, 1.0, 0.0)))

    def test_distance(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(4.0, 0.0, 0.0)
        ln = pyrove.segment3(a, b)

        # Point on the segment has zero distance
        self.assertAlmostEqual(ln.distance(pyrove.vec3(2.0, 0.0, 0.0)), 0.0, places=5)

        # Point perpendicular to segment
        d = ln.distance(pyrove.vec3(2.0, 3.0, 4.0))
        expected = 5.0  # sqrt(9 + 16)
        self.assertAlmostEqual(d, expected, places=5)

    def test_distance_sq(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(4.0, 0.0, 0.0)
        ln = pyrove.segment3(a, b)

        # Point perpendicular to segment
        d_sq = ln.distance_sq(pyrove.vec3(2.0, 3.0, 4.0))
        self.assertAlmostEqual(d_sq, 25.0, places=5)  # 9 + 16 = 25

    def test_repr(self):
        ln = pyrove.segment3(pyrove.vec3(1.0, 2.0, 3.0), pyrove.vec3(4.0, 5.0, 6.0))
        s = repr(ln)
        self.assertIn("segment3", s)


class TestDoubleLine(unittest.TestCase):
    def test_segment2d(self):
        ln = pyrove.segment2d(pyrove.vec2d(1.0, 2.0), pyrove.vec2d(3.0, 4.0))
        self.assertEqual(ln.A.x, 1.0)
        self.assertEqual(ln.A.y, 2.0)
        self.assertEqual(ln.B.x, 3.0)
        self.assertEqual(ln.B.y, 4.0)

    def test_segment3d(self):
        ln = pyrove.segment3d(pyrove.vec3d(1.0, 2.0, 3.0), pyrove.vec3d(4.0, 5.0, 6.0))
        self.assertEqual(ln.A.x, 1.0)
        self.assertEqual(ln.A.y, 2.0)
        self.assertEqual(ln.A.z, 3.0)
        self.assertEqual(ln.B.x, 4.0)
        self.assertEqual(ln.B.y, 5.0)
        self.assertEqual(ln.B.z, 6.0)



class TestSegment(unittest.TestCase):
    """point, parameter and closest_point."""

    def test_degenerate_segment_is_a_point(self):
        a = pyrove.vec3d(3.0, 7.0, -2.0)
        s = pyrove.segment3d(a, a)
        p = pyrove.vec3d(10.0, 10.0, 10.0)
        self.assertAlmostEqual(s.distance(p), math.sqrt(49 + 9 + 144), places=12)
        self.assertEqual(s.parameter(p), 0.0)
        c = s.closest_point(p)
        self.assertEqual((c.x, c.y, c.z), (3.0, 7.0, -2.0))

    def test_degenerate_float_segment_is_finite(self):
        a = pyrove.vec2(3.0, 7.0)
        s = pyrove.segment2(a, a)
        self.assertAlmostEqual(s.distance(pyrove.vec2(6.0, 11.0)), 5.0, places=5)

    def test_parameter_clamps_and_projects(self):
        s = pyrove.segment2d(pyrove.vec2d(0.0, 0.0), pyrove.vec2d(10.0, 0.0))
        self.assertEqual(s.parameter(pyrove.vec2d(-5.0, 1.0)), 0.0)
        self.assertEqual(s.parameter(pyrove.vec2d(15.0, -1.0)), 1.0)
        self.assertAlmostEqual(s.parameter(pyrove.vec2d(4.0, 3.0)), 0.4, places=12)
        c = s.closest_point(pyrove.vec2d(4.0, 3.0))
        self.assertAlmostEqual(c.x, 4.0, places=12)
        self.assertAlmostEqual(c.y, 0.0, places=12)

    def test_point_is_exact_at_the_ends(self):
        a = pyrove.vec3(0.1, 0.7, -3.3)
        b = pyrove.vec3(5.9, -2.2, 1.3)
        s = pyrove.segment3(a, b)
        for t, e in ((0.0, a), (1.0, b)):
            p = s.point(t)
            self.assertEqual((p.x, p.y, p.z), (e.x, e.y, e.z))
        m = s.point(0.5)
        self.assertAlmostEqual(m.x, 3.0, places=5)

    def test_nan_point_gives_nan(self):
        s = pyrove.segment3d(pyrove.vec3d(0.0, 0.0, 0.0), pyrove.vec3d(1.0, 0.0, 0.0))
        p = pyrove.vec3d(float("nan"), 0.0, 0.0)
        self.assertTrue(math.isnan(s.parameter(p)))
        self.assertTrue(math.isnan(s.distance(p)))


if __name__ == "__main__":
    unittest.main(verbosity=2)
