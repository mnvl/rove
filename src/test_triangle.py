import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestTriangle3(unittest.TestCase):
    def test_default_constructor(self):
        t = pyrove.triangle3()
        self.assertEqual(t.A.x, 0.0)
        self.assertEqual(t.A.y, 0.0)
        self.assertEqual(t.A.z, 0.0)
        self.assertEqual(t.B.x, 0.0)
        self.assertEqual(t.B.y, 0.0)
        self.assertEqual(t.B.z, 0.0)
        self.assertEqual(t.C.x, 0.0)
        self.assertEqual(t.C.y, 0.0)
        self.assertEqual(t.C.z, 0.0)

    def test_parameterized_constructor(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(1.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 1.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        self.assertEqual(t.A.x, 0.0)
        self.assertEqual(t.A.y, 0.0)
        self.assertEqual(t.A.z, 0.0)
        self.assertEqual(t.B.x, 1.0)
        self.assertEqual(t.B.y, 0.0)
        self.assertEqual(t.B.z, 0.0)
        self.assertEqual(t.C.x, 0.0)
        self.assertEqual(t.C.y, 1.0)
        self.assertEqual(t.C.z, 0.0)

    def test_construct(self):
        t = pyrove.triangle3()
        a = pyrove.vec3(1.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 1.0, 0.0)
        c = pyrove.vec3(0.0, 0.0, 1.0)
        t.construct(a, b, c)
        self.assertEqual(t.A.x, 1.0)
        self.assertEqual(t.B.y, 1.0)
        self.assertEqual(t.C.z, 1.0)

    def test_get_normal(self):
        # Right triangle in XY plane
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(1.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 1.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        n = t.get_normal()

        # Normal should point up (positive Z)
        self.assertAlmostEqual(abs(n.z), 1.0, places=5)
        self.assertAlmostEqual(n.length(), 1.0, places=5)

    def test_get_plane(self):
        # Triangle in XY plane at z=0
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(1.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 1.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        p = t.get_plane()

        # All vertices should be on the plane
        self.assertAlmostEqual(p.apply(a), 0.0, places=5)
        self.assertAlmostEqual(p.apply(b), 0.0, places=5)
        self.assertAlmostEqual(p.apply(c), 0.0, places=5)

    def test_perimeter(self):
        # Right triangle with sides 3, 4, 5
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(3.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 4.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        perimeter = t.perimeter()
        self.assertAlmostEqual(perimeter, 12.0, places=5)  # 3 + 4 + 5

    def test_area(self):
        # Right triangle with base 3 and height 4
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(3.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 4.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        area = t.area()
        self.assertAlmostEqual(area, 6.0, places=5)  # 0.5 * 3 * 4

    def test_cog(self):
        # Triangle with vertices at (0,0,0), (3,0,0), (0,3,0)
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(3.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 3.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        cog = t.cog()

        # Center of gravity should be at (1, 1, 0)
        self.assertAlmostEqual(cog.x, 1.0, places=5)
        self.assertAlmostEqual(cog.y, 1.0, places=5)
        self.assertAlmostEqual(cog.z, 0.0, places=5)

    def test_orthocenter(self):
        # Right triangle at origin
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(1.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 1.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        o = t.orthocenter()

        # For a right triangle, orthocenter is at the right angle vertex
        self.assertAlmostEqual(o.x, 0.0, places=4)
        self.assertAlmostEqual(o.y, 0.0, places=4)
        self.assertAlmostEqual(o.z, 0.0, places=4)

    def test_circumcenter(self):
        # Right triangle
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 2.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        cc = t.circumcenter()

        # For a right triangle, circumcenter is at midpoint of hypotenuse
        self.assertAlmostEqual(cc.x, 1.0, places=4)
        self.assertAlmostEqual(cc.y, 1.0, places=4)
        self.assertAlmostEqual(cc.z, 0.0, places=4)

    def test_circumcircle_radius(self):
        # Right triangle with legs 3 and 4 (hypotenuse = 5)
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(3.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 4.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        r = t.circumcircle_radius()

        # For a right triangle, circumradius = hypotenuse / 2
        self.assertAlmostEqual(r, 2.5, places=4)  # 5 / 2

    def test_get_circumcircle(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 2.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        s = t.get_circumcircle()

        # All vertices should be on the sphere
        dist_a = (s.centre - a).length()
        dist_b = (s.centre - b).length()
        dist_c = (s.centre - c).length()

        self.assertAlmostEqual(dist_a, s.radius, places=4)
        self.assertAlmostEqual(dist_b, s.radius, places=4)
        self.assertAlmostEqual(dist_c, s.radius, places=4)

    def test_incenter(self):
        # Equilateral triangle
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(1.0, math.sqrt(3.0), 0.0)
        t = pyrove.triangle3(a, b, c)
        ic = t.incenter()

        # For equilateral triangle, incenter = centroid
        cog = t.cog()
        self.assertAlmostEqual(ic.x, cog.x, places=4)
        self.assertAlmostEqual(ic.y, cog.y, places=4)

    def test_incircle_radius(self):
        # Right triangle with legs 3 and 4
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(3.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 4.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        r = t.incircle_radius()

        # For this right triangle, inradius = (a + b - c) / 2 = (3 + 4 - 5) / 2 = 1
        self.assertAlmostEqual(r, 1.0, places=4)

    def test_get_incircle(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(3.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 4.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        s = t.get_incircle()

        self.assertAlmostEqual(s.radius, 1.0, places=4)

    def test_feuerbach_center(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 2.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        fc = t.feuerbach_center()

        # Feuerbach center should exist
        self.assertIsNotNone(fc)

    def test_feuerbach_radius(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 2.0, 0.0)
        t = pyrove.triangle3(a, b, c)
        fr = t.feuerbach_radius()

        # Feuerbach radius should be half the circumradius
        cr = t.circumcircle_radius()
        self.assertAlmostEqual(fr, cr / 2.0, places=4)

    def test_contains_on_plane(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 2.0, 0.0)
        t = pyrove.triangle3(a, b, c)

        # Point inside the triangle (on plane)
        self.assertTrue(t.contains_on_plane(pyrove.vec3(0.5, 0.5, 0.0)))

        # Point outside the triangle (on plane)
        self.assertFalse(t.contains_on_plane(pyrove.vec3(3.0, 3.0, 0.0)))

    def test_contains(self):
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 2.0, 0.0)
        t = pyrove.triangle3(a, b, c)

        # Point inside the triangle
        self.assertTrue(t.contains(pyrove.vec3(0.5, 0.5, 0.0)))

        # Point outside the triangle
        self.assertFalse(t.contains(pyrove.vec3(3.0, 3.0, 0.0)))

        # Point on the plane but outside
        self.assertFalse(t.contains(pyrove.vec3(-1.0, -1.0, 0.0)))

        # Point not on the plane
        self.assertFalse(t.contains(pyrove.vec3(0.5, 0.5, 1.0)))

    def test_trace(self):
        # Triangle in XY plane
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(2.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 2.0, 0.0)
        t = pyrove.triangle3(a, b, c)

        # Ray from above going down, should hit triangle
        r1 = pyrove.ray3(pyrove.vec3(0.5, 0.5, 5.0), pyrove.vec3(0.0, 0.0, -1.0))
        self.assertTrue(t.trace(r1, 0.0, 10.0))

        # Ray from above going down, but misses triangle
        r2 = pyrove.ray3(pyrove.vec3(5.0, 5.0, 5.0), pyrove.vec3(0.0, 0.0, -1.0))
        self.assertFalse(t.trace(r2, 0.0, 10.0))

    def test_test_intersection(self):
        # Two triangles in the same plane
        t1 = pyrove.triangle3(
            pyrove.vec3(0.0, 0.0, 0.0),
            pyrove.vec3(2.0, 0.0, 0.0),
            pyrove.vec3(0.0, 2.0, 0.0)
        )

        # Overlapping triangle
        t2 = pyrove.triangle3(
            pyrove.vec3(1.0, 0.0, 0.0),
            pyrove.vec3(3.0, 0.0, 0.0),
            pyrove.vec3(1.0, 2.0, 0.0)
        )

        # These should intersect
        self.assertTrue(t1.test_intersection(t2))

    def test_repr(self):
        t = pyrove.triangle3(
            pyrove.vec3(0.0, 0.0, 0.0),
            pyrove.vec3(1.0, 0.0, 0.0),
            pyrove.vec3(0.0, 1.0, 0.0)
        )
        s = repr(t)
        self.assertIn("triangle3", s)

    def test_property_modification(self):
        t = pyrove.triangle3()
        t.A = pyrove.vec3(1.0, 2.0, 3.0)
        t.B = pyrove.vec3(4.0, 5.0, 6.0)
        t.C = pyrove.vec3(7.0, 8.0, 9.0)
        self.assertEqual(t.A.x, 1.0)
        self.assertEqual(t.B.y, 5.0)
        self.assertEqual(t.C.z, 9.0)

    def test_excenter_methods(self):
        # Simple triangle
        a = pyrove.vec3(0.0, 0.0, 0.0)
        b = pyrove.vec3(3.0, 0.0, 0.0)
        c = pyrove.vec3(0.0, 4.0, 0.0)
        t = pyrove.triangle3(a, b, c)

        # Test that excenter methods return values
        ea = t.excenter_A()
        eb = t.excenter_B()
        ec = t.excenter_C()

        self.assertIsNotNone(ea)
        self.assertIsNotNone(eb)
        self.assertIsNotNone(ec)

        # Test excircle radius methods
        ra = t.excircle_A_radius()
        rb = t.excircle_B_radius()
        rc = t.excircle_C_radius()

        self.assertGreater(ra, 0.0)
        self.assertGreater(rb, 0.0)
        self.assertGreater(rc, 0.0)


class TestDoubleTriangle(unittest.TestCase):
    def test_triangle3d(self):
        t = pyrove.triangle3d(
            pyrove.vec3d(0.0, 0.0, 0.0),
            pyrove.vec3d(1.0, 0.0, 0.0),
            pyrove.vec3d(0.0, 1.0, 0.0)
        )
        self.assertEqual(t.A.x, 0.0)
        self.assertEqual(t.B.x, 1.0)
        self.assertEqual(t.C.y, 1.0)

    def test_triangle3d_area(self):
        t = pyrove.triangle3d(
            pyrove.vec3d(0.0, 0.0, 0.0),
            pyrove.vec3d(3.0, 0.0, 0.0),
            pyrove.vec3d(0.0, 4.0, 0.0)
        )
        area = t.area()
        self.assertAlmostEqual(area, 6.0, places=5)


if __name__ == "__main__":
    unittest.main(verbosity=2)
