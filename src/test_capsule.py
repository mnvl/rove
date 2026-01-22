import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestCapsule2(unittest.TestCase):
    def test_default_constructor(self):
        cap = pyrove.capsule2()
        self.assertEqual(cap.axe.A.x, 0.0)
        self.assertEqual(cap.axe.A.y, 0.0)
        self.assertEqual(cap.axe.B.x, 0.0)
        self.assertEqual(cap.axe.B.y, 0.0)
        self.assertEqual(cap.radius, 0.0)

    def test_parameterized_constructor(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        r = 1.0
        cap = pyrove.capsule2(a, b, r)

        self.assertEqual(cap.axe.A.x, 0.0)
        self.assertEqual(cap.axe.A.y, 0.0)
        self.assertEqual(cap.axe.B.x, 4.0)
        self.assertEqual(cap.axe.B.y, 0.0)
        self.assertEqual(cap.radius, 1.0)

    def test_contains_on_axis(self):
        # Horizontal capsule along X axis
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        cap = pyrove.capsule2(a, b, 1.0)

        # Point on the axis line
        self.assertTrue(cap.contains(pyrove.vec2(2.0, 0.0)))

        # Point within radius
        self.assertTrue(cap.contains(pyrove.vec2(2.0, 0.5)))

    def test_contains_outside(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        cap = pyrove.capsule2(a, b, 1.0)

        # Point outside radius
        self.assertFalse(cap.contains(pyrove.vec2(2.0, 2.0)))

    def test_contains_at_endpoints(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        cap = pyrove.capsule2(a, b, 1.0)

        # Points at the endpoint spheres
        self.assertTrue(cap.contains(pyrove.vec2(0.0, 0.8)))
        self.assertTrue(cap.contains(pyrove.vec2(4.0, 0.8)))

    def test_distance_inside(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        cap = pyrove.capsule2(a, b, 1.0)

        # Point inside capsule should have distance 0
        d = cap.distance(pyrove.vec2(2.0, 0.5))
        self.assertAlmostEqual(d, 0.0, places=5)

    def test_distance_outside(self):
        a = pyrove.vec2(0.0, 0.0)
        b = pyrove.vec2(4.0, 0.0)
        cap = pyrove.capsule2(a, b, 1.0)

        # Point directly above center
        d = cap.distance(pyrove.vec2(2.0, 3.0))
        self.assertAlmostEqual(d, 2.0, places=5)  # 3.0 - 1.0 (radius)

    def test_repr(self):
        cap = pyrove.capsule2(pyrove.vec2(0.0, 0.0), pyrove.vec2(1.0, 0.0), 0.5)
        s = repr(cap)
        self.assertIn("capsule2", s)

    def test_property_modification(self):
        cap = pyrove.capsule2()
        cap.axe = pyrove.line2(pyrove.vec2(1.0, 2.0), pyrove.vec2(3.0, 4.0))
        cap.radius = 5.0

        self.assertEqual(cap.axe.A.x, 1.0)
        self.assertEqual(cap.axe.A.y, 2.0)
        self.assertEqual(cap.axe.B.x, 3.0)
        self.assertEqual(cap.axe.B.y, 4.0)
        self.assertEqual(cap.radius, 5.0)


class TestCapsule3(unittest.TestCase):
    def test_default_constructor(self):
        cap = pyrove.capsule3()
        self.assertEqual(cap.axe.A.x, 0.0)
        self.assertEqual(cap.axe.A.y, 0.0)
        self.assertEqual(cap.axe.A.z, 0.0)
        self.assertEqual(cap.axe.B.x, 0.0)
        self.assertEqual(cap.axe.B.y, 0.0)
        self.assertEqual(cap.axe.B.z, 0.0)
        self.assertEqual(cap.radius, 0.0)

    def test_parameterized_constructor(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 0.0, 4.0)
        r = 1.0
        cap = pyrove.capsule3(a, b, r)

        self.assertEqual(cap.axe.A.x, 0.0)
        self.assertEqual(cap.axe.A.y, 0.0)
        self.assertEqual(cap.axe.A.z, 0.0)
        self.assertEqual(cap.axe.B.x, 0.0)
        self.assertEqual(cap.axe.B.y, 0.0)
        self.assertEqual(cap.axe.B.z, 4.0)
        self.assertEqual(cap.radius, 1.0)

    def test_contains_on_axis(self):
        # Vertical capsule along Z axis
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 0.0, 4.0)
        cap = pyrove.capsule3(a, b, 1.0)

        # Point on the axis line
        self.assertTrue(cap.contains(pyrove.vec3(0.0, 0.0, 2.0)))

        # Point within radius
        self.assertTrue(cap.contains(pyrove.vec3(0.5, 0.0, 2.0)))
        self.assertTrue(cap.contains(pyrove.vec3(0.0, 0.5, 2.0)))

    def test_contains_outside(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 0.0, 4.0)
        cap = pyrove.capsule3(a, b, 1.0)

        # Point outside radius
        self.assertFalse(cap.contains(pyrove.vec3(2.0, 0.0, 2.0)))
        self.assertFalse(cap.contains(pyrove.vec3(0.0, 2.0, 2.0)))

    def test_contains_at_endpoints(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 0.0, 4.0)
        cap = pyrove.capsule3(a, b, 1.0)

        # Points at the endpoint spheres
        self.assertTrue(cap.contains(pyrove.vec3(0.8, 0.0, 0.0)))
        self.assertTrue(cap.contains(pyrove.vec3(0.0, 0.8, 0.0)))
        self.assertTrue(cap.contains(pyrove.vec3(0.8, 0.0, 4.0)))
        self.assertTrue(cap.contains(pyrove.vec3(0.0, 0.8, 4.0)))

    def test_distance_inside(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 0.0, 4.0)
        cap = pyrove.capsule3(a, b, 1.0)

        # Point inside capsule should have distance 0
        d = cap.distance(pyrove.vec3(0.0, 0.5, 2.0))
        self.assertAlmostEqual(d, 0.0, places=5)

    def test_distance_outside(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 0.0, 4.0)
        cap = pyrove.capsule3(a, b, 1.0)

        # Point directly to the side at center
        d = cap.distance(pyrove.vec3(3.0, 0.0, 2.0))
        self.assertAlmostEqual(d, 2.0, places=5)  # 3.0 - 1.0 (radius)

    def test_distance_perpendicular_to_axis(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 0.0, 4.0)
        cap = pyrove.capsule3(a, b, 1.0)

        # Point in XY plane at center
        d = cap.distance(pyrove.vec3(3.0, 4.0, 2.0))
        expected = 5.0 - 1.0  # sqrt(9+16) - radius
        self.assertAlmostEqual(d, expected, places=5)

    def test_repr(self):
        cap = pyrove.capsule3(
            pyrove.vec3(0.0, 0.0, 0.0),
            pyrove.vec3(0.0, 0.0, 1.0),
            0.5
        )
        s = repr(cap)
        self.assertIn("capsule3", s)

    def test_property_modification(self):
        cap = pyrove.capsule3()
        cap.axe = pyrove.line3(
            pyrove.vec3(1.0, 2.0, 3.0),
            pyrove.vec3(4.0, 5.0, 6.0)
        )
        cap.radius = 7.0

        self.assertEqual(cap.axe.A.x, 1.0)
        self.assertEqual(cap.axe.A.y, 2.0)
        self.assertEqual(cap.axe.A.z, 3.0)
        self.assertEqual(cap.axe.B.x, 4.0)
        self.assertEqual(cap.axe.B.y, 5.0)
        self.assertEqual(cap.axe.B.z, 6.0)
        self.assertEqual(cap.radius, 7.0)


class TestDoubleCapsule(unittest.TestCase):
    def test_capsule2d(self):
        cap = pyrove.capsule2d(
            pyrove.vec2d(0.0, 0.0),
            pyrove.vec2d(4.0, 0.0),
            1.0
        )
        self.assertEqual(cap.axe.A.x, 0.0)
        self.assertEqual(cap.axe.B.x, 4.0)
        self.assertEqual(cap.radius, 1.0)

    def test_capsule3d(self):
        cap = pyrove.capsule3d(
            pyrove.vec3d(0.0, 0.0, 0.0),
            pyrove.vec3d(0.0, 0.0, 4.0),
            1.0
        )
        self.assertEqual(cap.axe.A.z, 0.0)
        self.assertEqual(cap.axe.B.z, 4.0)
        self.assertEqual(cap.radius, 1.0)

    def test_capsule3d_contains(self):
        cap = pyrove.capsule3d(
            pyrove.vec3d(0.0, 0.0, 0.0),
            pyrove.vec3d(0.0, 0.0, 4.0),
            1.0
        )
        self.assertTrue(cap.contains(pyrove.vec3d(0.0, 0.0, 2.0)))
        self.assertFalse(cap.contains(pyrove.vec3d(2.0, 0.0, 2.0)))


if __name__ == "__main__":
    unittest.main(verbosity=2)
