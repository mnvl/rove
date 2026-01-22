import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestRay2(unittest.TestCase):
    def test_default_constructor(self):
        r = pyrove.ray2()
        self.assertEqual(r.r0.x, 0.0)
        self.assertEqual(r.r0.y, 0.0)
        self.assertEqual(r.a.x, 0.0)
        self.assertEqual(r.a.y, 0.0)

    def test_parameterized_constructor(self):
        origin = pyrove.vec2(1.0, 2.0)
        direction = pyrove.vec2(3.0, 4.0)
        r = pyrove.ray2(origin, direction)
        self.assertEqual(r.r0.x, 1.0)
        self.assertEqual(r.r0.y, 2.0)
        self.assertEqual(r.a.x, 3.0)
        self.assertEqual(r.a.y, 4.0)

    def test_set(self):
        r = pyrove.ray2()
        origin = pyrove.vec2(5.0, 6.0)
        direction = pyrove.vec2(7.0, 8.0)
        r.set(origin, direction)
        self.assertEqual(r.r0.x, 5.0)
        self.assertEqual(r.r0.y, 6.0)
        self.assertEqual(r.a.x, 7.0)
        self.assertEqual(r.a.y, 8.0)

    def test_apply(self):
        origin = pyrove.vec2(1.0, 2.0)
        direction = pyrove.vec2(3.0, 4.0)
        r = pyrove.ray2(origin, direction)

        # Point at t=0 should be origin
        p0 = r.apply(0.0)
        self.assertAlmostEqual(p0.x, 1.0)
        self.assertAlmostEqual(p0.y, 2.0)

        # Point at t=1 should be origin + direction
        p1 = r.apply(1.0)
        self.assertAlmostEqual(p1.x, 4.0)
        self.assertAlmostEqual(p1.y, 6.0)

        # Point at t=2
        p2 = r.apply(2.0)
        self.assertAlmostEqual(p2.x, 7.0)
        self.assertAlmostEqual(p2.y, 10.0)

    def test_contains(self):
        origin = pyrove.vec2(0.0, 0.0)
        direction = pyrove.vec2(1.0, 0.0)
        r = pyrove.ray2(origin, direction)

        # Point on the ray
        self.assertTrue(r.contains(pyrove.vec2(5.0, 0.0)))

        # Point not on the ray
        self.assertFalse(r.contains(pyrove.vec2(0.0, 1.0)))

    def test_distance(self):
        origin = pyrove.vec2(0.0, 0.0)
        direction = pyrove.vec2(1.0, 0.0)
        r = pyrove.ray2(origin, direction)

        # Point on the ray has zero distance
        self.assertAlmostEqual(r.distance(pyrove.vec2(5.0, 0.0)), 0.0, places=5)

        # Point perpendicular to ray
        d = r.distance(pyrove.vec2(5.0, 3.0))
        self.assertAlmostEqual(d, 3.0, places=5)

    def test_distance_sq(self):
        origin = pyrove.vec2(0.0, 0.0)
        direction = pyrove.vec2(1.0, 0.0)
        r = pyrove.ray2(origin, direction)

        # Point perpendicular to ray
        d_sq = r.distance_sq(pyrove.vec2(5.0, 3.0))
        self.assertAlmostEqual(d_sq, 9.0, places=5)

    def test_test_intersection(self):
        r1 = pyrove.ray2(pyrove.vec2(0.0, 0.0), pyrove.vec2(1.0, 0.0))
        r2 = pyrove.ray2(pyrove.vec2(0.0, 1.0), pyrove.vec2(1.0, 0.0))

        # Parallel rays should not intersect
        self.assertFalse(r1.test_intersection(r2))

    def test_repr(self):
        r = pyrove.ray2(pyrove.vec2(1.0, 2.0), pyrove.vec2(3.0, 4.0))
        s = repr(r)
        self.assertIn("ray2", s)

    def test_property_modification(self):
        r = pyrove.ray2()
        r.r0 = pyrove.vec2(10.0, 20.0)
        r.a = pyrove.vec2(30.0, 40.0)
        self.assertEqual(r.r0.x, 10.0)
        self.assertEqual(r.r0.y, 20.0)
        self.assertEqual(r.a.x, 30.0)
        self.assertEqual(r.a.y, 40.0)


class TestRay3(unittest.TestCase):
    def test_default_constructor(self):
        r = pyrove.ray3()
        self.assertEqual(r.r0.x, 0.0)
        self.assertEqual(r.r0.y, 0.0)
        self.assertEqual(r.r0.z, 0.0)
        self.assertEqual(r.a.x, 0.0)
        self.assertEqual(r.a.y, 0.0)
        self.assertEqual(r.a.z, 0.0)

    def test_parameterized_constructor(self):
        origin = pyrove.vec3(1.0, 2.0, 3.0)
        direction = pyrove.vec3(4.0, 5.0, 6.0)
        r = pyrove.ray3(origin, direction)
        self.assertEqual(r.r0.x, 1.0)
        self.assertEqual(r.r0.y, 2.0)
        self.assertEqual(r.r0.z, 3.0)
        self.assertEqual(r.a.x, 4.0)
        self.assertEqual(r.a.y, 5.0)
        self.assertEqual(r.a.z, 6.0)

    def test_set(self):
        r = pyrove.ray3()
        origin = pyrove.vec3(7.0, 8.0, 9.0)
        direction = pyrove.vec3(10.0, 11.0, 12.0)
        r.set(origin, direction)
        self.assertEqual(r.r0.x, 7.0)
        self.assertEqual(r.r0.y, 8.0)
        self.assertEqual(r.r0.z, 9.0)
        self.assertEqual(r.a.x, 10.0)
        self.assertEqual(r.a.y, 11.0)
        self.assertEqual(r.a.z, 12.0)

    def test_apply(self):
        origin = pyrove.vec3(1.0, 2.0, 3.0)
        direction = pyrove.vec3(4.0, 5.0, 6.0)
        r = pyrove.ray3(origin, direction)

        # Point at t=0 should be origin
        p0 = r.apply(0.0)
        self.assertAlmostEqual(p0.x, 1.0)
        self.assertAlmostEqual(p0.y, 2.0)
        self.assertAlmostEqual(p0.z, 3.0)

        # Point at t=1 should be origin + direction
        p1 = r.apply(1.0)
        self.assertAlmostEqual(p1.x, 5.0)
        self.assertAlmostEqual(p1.y, 7.0)
        self.assertAlmostEqual(p1.z, 9.0)

    def test_contains(self):
        origin = pyrove.vec3(0.0, 0.0, 0.0)
        direction = pyrove.vec3(1.0, 0.0, 0.0)
        r = pyrove.ray3(origin, direction)

        # Point on the ray
        self.assertTrue(r.contains(pyrove.vec3(5.0, 0.0, 0.0)))

        # Point not on the ray
        self.assertFalse(r.contains(pyrove.vec3(0.0, 1.0, 0.0)))

    def test_distance(self):
        origin = pyrove.vec3(0.0, 0.0, 0.0)
        direction = pyrove.vec3(1.0, 0.0, 0.0)
        r = pyrove.ray3(origin, direction)

        # Point on the ray has zero distance
        self.assertAlmostEqual(r.distance(pyrove.vec3(5.0, 0.0, 0.0)), 0.0, places=5)

        # Point perpendicular to ray
        d = r.distance(pyrove.vec3(5.0, 3.0, 4.0))
        self.assertAlmostEqual(d, 5.0, places=5)  # sqrt(3^2 + 4^2) = 5

    def test_distance_sq(self):
        origin = pyrove.vec3(0.0, 0.0, 0.0)
        direction = pyrove.vec3(1.0, 0.0, 0.0)
        r = pyrove.ray3(origin, direction)

        # Point perpendicular to ray
        d_sq = r.distance_sq(pyrove.vec3(5.0, 3.0, 4.0))
        self.assertAlmostEqual(d_sq, 25.0, places=5)  # 3^2 + 4^2 = 25

    def test_repr(self):
        r = pyrove.ray3(pyrove.vec3(1.0, 2.0, 3.0), pyrove.vec3(4.0, 5.0, 6.0))
        s = repr(r)
        self.assertIn("ray3", s)


class TestDoubleRay(unittest.TestCase):
    def test_ray2d(self):
        r = pyrove.ray2d(pyrove.vec2d(1.0, 2.0), pyrove.vec2d(3.0, 4.0))
        self.assertEqual(r.r0.x, 1.0)
        self.assertEqual(r.r0.y, 2.0)
        self.assertEqual(r.a.x, 3.0)
        self.assertEqual(r.a.y, 4.0)

    def test_ray3d(self):
        r = pyrove.ray3d(pyrove.vec3d(1.0, 2.0, 3.0), pyrove.vec3d(4.0, 5.0, 6.0))
        self.assertEqual(r.r0.x, 1.0)
        self.assertEqual(r.r0.y, 2.0)
        self.assertEqual(r.r0.z, 3.0)
        self.assertEqual(r.a.x, 4.0)
        self.assertEqual(r.a.y, 5.0)
        self.assertEqual(r.a.z, 6.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
