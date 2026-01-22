import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestPlane(unittest.TestCase):
    def test_default_constructor(self):
        p = pyrove.plane()
        self.assertEqual(p.A, 0.0)
        self.assertEqual(p.B, 0.0)
        self.assertEqual(p.C, 0.0)
        self.assertEqual(p.D, 0.0)

    def test_constructor_from_origin_normal(self):
        origin = pyrove.vec3(0.0, 0.0, 1.0)
        normal = pyrove.vec3(0.0, 0.0, 1.0)
        p = pyrove.plane(origin, normal)
        # Plane equation: 0*x + 0*y + 1*z + D = 0
        # At origin (0,0,1): 0 + 0 + 1 + D = 0, so D = -1
        self.assertAlmostEqual(p.A, 0.0)
        self.assertAlmostEqual(p.B, 0.0)
        self.assertAlmostEqual(p.C, 1.0)
        self.assertAlmostEqual(p.D, -1.0)

    def test_constructor_from_d_normal(self):
        d = -5.0
        normal = pyrove.vec3(1.0, 0.0, 0.0)
        p = pyrove.plane(d, normal)
        self.assertAlmostEqual(p.A, 1.0)
        self.assertAlmostEqual(p.B, 0.0)
        self.assertAlmostEqual(p.C, 0.0)
        self.assertAlmostEqual(p.D, -5.0)

    def test_set_from_origin_normal(self):
        p = pyrove.plane()
        origin = pyrove.vec3(1.0, 2.0, 3.0)
        normal = pyrove.vec3(0.0, 1.0, 0.0)
        p.set(origin, normal)
        # At point (1,2,3) with normal (0,1,0): 0*x + 1*y + 0*z + D = 0
        # 0 + 2 + 0 + D = 0, so D = -2
        self.assertAlmostEqual(p.A, 0.0)
        self.assertAlmostEqual(p.B, 1.0)
        self.assertAlmostEqual(p.C, 0.0)
        self.assertAlmostEqual(p.D, -2.0)

    def test_set_from_d_normal(self):
        p = pyrove.plane()
        p.set(-3.0, pyrove.vec3(1.0, 1.0, 1.0))
        self.assertAlmostEqual(p.A, 1.0)
        self.assertAlmostEqual(p.B, 1.0)
        self.assertAlmostEqual(p.C, 1.0)
        self.assertAlmostEqual(p.D, -3.0)

    def test_apply(self):
        # XY plane at z=0
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(0.0, 0.0, 1.0))

        # Points on the plane should evaluate to ~0
        self.assertAlmostEqual(p.apply(pyrove.vec3(1.0, 2.0, 0.0)), 0.0, places=5)

        # Point above the plane
        self.assertAlmostEqual(p.apply(pyrove.vec3(0.0, 0.0, 5.0)), 5.0, places=5)

        # Point below the plane
        self.assertAlmostEqual(p.apply(pyrove.vec3(0.0, 0.0, -3.0)), -3.0, places=5)

    def test_contains_point(self):
        # XY plane at z=0
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(0.0, 0.0, 1.0))

        # Point on the plane
        self.assertTrue(p.contains(pyrove.vec3(5.0, 3.0, 0.0)))

        # Point not on the plane
        self.assertFalse(p.contains(pyrove.vec3(0.0, 0.0, 0.1)))

    def test_contains_ray(self):
        # XY plane at z=0
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(0.0, 0.0, 1.0))

        # Ray in the plane
        r1 = pyrove.ray3(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(1.0, 0.0, 0.0))
        self.assertTrue(p.contains(r1))

        # Ray not in the plane
        r2 = pyrove.ray3(pyrove.vec3(0.0, 0.0, 1.0), pyrove.vec3(1.0, 0.0, 0.0))
        self.assertFalse(p.contains(r2))

    def test_test_intersection(self):
        # XY plane at z=0
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(0.0, 0.0, 1.0))

        # Ray that intersects the plane
        r1 = pyrove.ray3(pyrove.vec3(0.0, 0.0, 5.0), pyrove.vec3(0.0, 0.0, -1.0))
        self.assertTrue(p.test_intersection(r1))

        # Ray parallel to the plane
        r2 = pyrove.ray3(pyrove.vec3(0.0, 0.0, 5.0), pyrove.vec3(1.0, 0.0, 0.0))
        self.assertFalse(p.test_intersection(r2))

    def test_parallel(self):
        # XY plane at z=0
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(0.0, 0.0, 1.0))

        # Ray parallel to the plane
        r1 = pyrove.ray3(pyrove.vec3(0.0, 0.0, 5.0), pyrove.vec3(1.0, 0.0, 0.0))
        self.assertTrue(p.parallel(r1))

        # Ray that intersects the plane
        r2 = pyrove.ray3(pyrove.vec3(0.0, 0.0, 5.0), pyrove.vec3(0.0, 0.0, -1.0))
        self.assertFalse(p.parallel(r2))

    def test_trace(self):
        # XY plane at z=0
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(0.0, 0.0, 1.0))

        # Ray from (0,0,5) going down should hit plane at t=5
        r = pyrove.ray3(pyrove.vec3(0.0, 0.0, 5.0), pyrove.vec3(0.0, 0.0, -1.0))
        t = p.trace(r)
        self.assertAlmostEqual(t, 5.0, places=5)

    def test_get_normal(self):
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(1.0, 2.0, 3.0))
        n = p.get_normal()
        self.assertAlmostEqual(n.x, 1.0)
        self.assertAlmostEqual(n.y, 2.0)
        self.assertAlmostEqual(n.z, 3.0)

    def test_get_origin(self):
        origin = pyrove.vec3(1.0, 2.0, 3.0)
        normal = pyrove.vec3(0.0, 1.0, 0.0)
        p = pyrove.plane(origin, normal)
        o = p.get_origin()
        # The origin should be on the plane
        self.assertAlmostEqual(p.apply(o), 0.0, places=5)

    def test_is_correct(self):
        # Valid plane
        p1 = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(1.0, 0.0, 0.0))
        self.assertTrue(p1.is_correct())

        # Invalid plane (all coefficients zero)
        p2 = pyrove.plane()
        self.assertFalse(p2.is_correct())

    def test_classify_point(self):
        # XY plane at z=0
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(0.0, 0.0, 1.0))

        # Point above (positive side)
        c1 = p.classify(pyrove.vec3(0.0, 0.0, 5.0))
        self.assertEqual(c1, pyrove.plane_classification.POSITIVE)

        # Point below (negative side)
        c2 = p.classify(pyrove.vec3(0.0, 0.0, -5.0))
        self.assertEqual(c2, pyrove.plane_classification.NEGATIVE)

    def test_repr(self):
        p = pyrove.plane(pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec3(1.0, 0.0, 0.0))
        s = repr(p)
        self.assertIn("plane", s)

    def test_property_modification(self):
        p = pyrove.plane()
        p.A = 1.0
        p.B = 2.0
        p.C = 3.0
        p.D = 4.0
        self.assertEqual(p.A, 1.0)
        self.assertEqual(p.B, 2.0)
        self.assertEqual(p.C, 3.0)
        self.assertEqual(p.D, 4.0)


class TestDoublePlane(unittest.TestCase):
    def test_planed(self):
        p = pyrove.planed(pyrove.vec3d(0.0, 0.0, 0.0), pyrove.vec3d(1.0, 0.0, 0.0))
        self.assertAlmostEqual(p.A, 1.0)
        self.assertAlmostEqual(p.B, 0.0)
        self.assertAlmostEqual(p.C, 0.0)

    def test_planed_apply(self):
        p = pyrove.planed(pyrove.vec3d(0.0, 0.0, 0.0), pyrove.vec3d(0.0, 0.0, 1.0))
        result = p.apply(pyrove.vec3d(1.0, 2.0, 5.0))
        self.assertAlmostEqual(result, 5.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
