import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyvek


def approx_equal(a, b, rel_tol=1e-6):
    return abs(a - b) <= rel_tol * max(abs(a), abs(b), 1.0)


class TestVec2(unittest.TestCase):
    def test_default_constructor(self):
        v = pyvek.vec2()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)

    def test_value_constructor(self):
        v = pyvek.vec2(3.0, 4.0)
        self.assertEqual(v.x, 3.0)
        self.assertEqual(v.y, 4.0)

    def test_set(self):
        v = pyvek.vec2()
        v.set(1.0, 2.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)

    def test_set_all(self):
        v = pyvek.vec2()
        v.set_all(5.0)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 5.0)

    def test_length(self):
        v = pyvek.vec2(3.0, 4.0)
        self.assertAlmostEqual(v.length(), 5.0)

    def test_length_sq(self):
        v = pyvek.vec2(3.0, 4.0)
        self.assertAlmostEqual(v.length_sq(), 25.0)

    def test_length_manhattan(self):
        v = pyvek.vec2(3.0, 4.0)
        self.assertAlmostEqual(v.length_manhattan(), 7.0)

    def test_normalize(self):
        v = pyvek.vec2(3.0, 4.0)
        v.normalize()
        self.assertAlmostEqual(v.length(), 1.0)
        self.assertAlmostEqual(v.x, 0.6)
        self.assertAlmostEqual(v.y, 0.8)

    def test_negate(self):
        v = pyvek.vec2(3.0, 4.0)
        v.negate()
        self.assertEqual(v.x, -3.0)
        self.assertEqual(v.y, -4.0)

    def test_perpendicular(self):
        v = pyvek.vec2(1.0, 0.0)
        p = v.perpendicular()
        self.assertAlmostEqual(p.x, 0.0)
        self.assertAlmostEqual(abs(p.y), 1.0)

    def test_add(self):
        a = pyvek.vec2(1.0, 2.0)
        b = pyvek.vec2(3.0, 4.0)
        c = a + b
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 6.0)

    def test_sub(self):
        a = pyvek.vec2(3.0, 4.0)
        b = pyvek.vec2(1.0, 1.0)
        c = a - b
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 3.0)

    def test_mul_vec(self):
        a = pyvek.vec2(2.0, 3.0)
        b = pyvek.vec2(4.0, 5.0)
        c = a * b
        self.assertEqual(c.x, 8.0)
        self.assertEqual(c.y, 15.0)

    def test_mul_scalar(self):
        v = pyvek.vec2(2.0, 3.0)
        c = v * 2.0
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 6.0)

    def test_rmul_scalar(self):
        v = pyvek.vec2(2.0, 3.0)
        c = 2.0 * v
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 6.0)

    def test_div_vec(self):
        a = pyvek.vec2(8.0, 15.0)
        b = pyvek.vec2(4.0, 5.0)
        c = a / b
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 3.0)

    def test_div_scalar(self):
        v = pyvek.vec2(4.0, 6.0)
        c = v / 2.0
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 3.0)

    def test_neg(self):
        v = pyvek.vec2(3.0, 4.0)
        n = -v
        self.assertEqual(n.x, -3.0)
        self.assertEqual(n.y, -4.0)

    def test_eq(self):
        a = pyvek.vec2(1.0, 2.0)
        b = pyvek.vec2(1.0, 2.0)
        c = pyvek.vec2(1.0, 3.0)
        self.assertTrue(a == b)
        self.assertFalse(a == c)

    def test_ne(self):
        a = pyvek.vec2(1.0, 2.0)
        b = pyvek.vec2(1.0, 3.0)
        self.assertTrue(a != b)

    def test_dot(self):
        a = pyvek.vec2(1.0, 2.0)
        b = pyvek.vec2(3.0, 4.0)
        self.assertAlmostEqual(a.dot(b), 11.0)

    def test_repr(self):
        v = pyvek.vec2(1.0, 2.0)
        self.assertIn("vec2", repr(v))
        self.assertIn("1", repr(v))
        self.assertIn("2", repr(v))

    def test_is_collinear(self):
        a = pyvek.vec2(1.0, 2.0)
        b = pyvek.vec2(2.0, 4.0)
        c = pyvek.vec2(1.0, 3.0)
        self.assertTrue(a.is_collinear(b))
        self.assertFalse(a.is_collinear(c))


class TestVec3(unittest.TestCase):
    def test_default_constructor(self):
        v = pyvek.vec3()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)

    def test_value_constructor(self):
        v = pyvek.vec3(1.0, 2.0, 3.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_set(self):
        v = pyvek.vec3()
        v.set(1.0, 2.0, 3.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_set_all(self):
        v = pyvek.vec3()
        v.set_all(5.0)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 5.0)
        self.assertEqual(v.z, 5.0)

    def test_length(self):
        v = pyvek.vec3(2.0, 3.0, 6.0)
        self.assertAlmostEqual(v.length(), 7.0)

    def test_length_sq(self):
        v = pyvek.vec3(2.0, 3.0, 6.0)
        self.assertAlmostEqual(v.length_sq(), 49.0)

    def test_normalize(self):
        v = pyvek.vec3(2.0, 3.0, 6.0)
        v.normalize()
        self.assertAlmostEqual(v.length(), 1.0)

    def test_negate(self):
        v = pyvek.vec3(1.0, 2.0, 3.0)
        v.negate()
        self.assertEqual(v.x, -1.0)
        self.assertEqual(v.y, -2.0)
        self.assertEqual(v.z, -3.0)

    def test_add(self):
        a = pyvek.vec3(1.0, 2.0, 3.0)
        b = pyvek.vec3(4.0, 5.0, 6.0)
        c = a + b
        self.assertEqual(c.x, 5.0)
        self.assertEqual(c.y, 7.0)
        self.assertEqual(c.z, 9.0)

    def test_sub(self):
        a = pyvek.vec3(4.0, 5.0, 6.0)
        b = pyvek.vec3(1.0, 2.0, 3.0)
        c = a - b
        self.assertEqual(c.x, 3.0)
        self.assertEqual(c.y, 3.0)
        self.assertEqual(c.z, 3.0)

    def test_mul_vec(self):
        a = pyvek.vec3(2.0, 3.0, 4.0)
        b = pyvek.vec3(5.0, 6.0, 7.0)
        c = a * b
        self.assertEqual(c.x, 10.0)
        self.assertEqual(c.y, 18.0)
        self.assertEqual(c.z, 28.0)

    def test_mul_scalar(self):
        v = pyvek.vec3(1.0, 2.0, 3.0)
        c = v * 2.0
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)

    def test_rmul_scalar(self):
        v = pyvek.vec3(1.0, 2.0, 3.0)
        c = 2.0 * v
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)

    def test_div_scalar(self):
        v = pyvek.vec3(2.0, 4.0, 6.0)
        c = v / 2.0
        self.assertEqual(c.x, 1.0)
        self.assertEqual(c.y, 2.0)
        self.assertEqual(c.z, 3.0)

    def test_neg(self):
        v = pyvek.vec3(1.0, 2.0, 3.0)
        n = -v
        self.assertEqual(n.x, -1.0)
        self.assertEqual(n.y, -2.0)
        self.assertEqual(n.z, -3.0)

    def test_eq(self):
        a = pyvek.vec3(1.0, 2.0, 3.0)
        b = pyvek.vec3(1.0, 2.0, 3.0)
        self.assertTrue(a == b)

    def test_ne(self):
        a = pyvek.vec3(1.0, 2.0, 3.0)
        b = pyvek.vec3(1.0, 2.0, 4.0)
        self.assertTrue(a != b)

    def test_dot(self):
        a = pyvek.vec3(1.0, 2.0, 3.0)
        b = pyvek.vec3(4.0, 5.0, 6.0)
        self.assertAlmostEqual(a.dot(b), 32.0)

    def test_cross(self):
        a = pyvek.vec3(1.0, 0.0, 0.0)
        b = pyvek.vec3(0.0, 1.0, 0.0)
        c = a.cross(b)
        self.assertAlmostEqual(c.x, 0.0)
        self.assertAlmostEqual(c.y, 0.0)
        self.assertAlmostEqual(c.z, 1.0)

    def test_cross_anticommutative(self):
        a = pyvek.vec3(1.0, 2.0, 3.0)
        b = pyvek.vec3(4.0, 5.0, 6.0)
        c1 = a.cross(b)
        c2 = b.cross(a)
        self.assertAlmostEqual(c1.x, -c2.x)
        self.assertAlmostEqual(c1.y, -c2.y)
        self.assertAlmostEqual(c1.z, -c2.z)

    def test_repr(self):
        v = pyvek.vec3(1.0, 2.0, 3.0)
        self.assertIn("vec3", repr(v))

    def test_is_collinear(self):
        a = pyvek.vec3(1.0, 2.0, 3.0)
        b = pyvek.vec3(2.0, 4.0, 6.0)
        c = pyvek.vec3(1.0, 2.0, 4.0)
        self.assertTrue(a.is_collinear(b))
        self.assertFalse(a.is_collinear(c))


class TestVec4(unittest.TestCase):
    def test_default_constructor(self):
        v = pyvek.vec4()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)
        self.assertEqual(v.w, 0.0)

    def test_value_constructor(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertEqual(v.w, 4.0)

    def test_set(self):
        v = pyvek.vec4()
        v.set(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertEqual(v.w, 4.0)

    def test_set_all(self):
        v = pyvek.vec4()
        v.set_all(5.0)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 5.0)
        self.assertEqual(v.z, 5.0)
        self.assertEqual(v.w, 5.0)

    def test_length(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        expected = math.sqrt(1 + 4 + 9 + 16)
        self.assertAlmostEqual(v.length(), expected, places=6)

    def test_length_sq(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertAlmostEqual(v.length_sq(), 30.0)

    def test_normalize(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        v.normalize()
        self.assertAlmostEqual(v.length(), 1.0, places=6)

    def test_negate(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        v.negate()
        self.assertEqual(v.x, -1.0)
        self.assertEqual(v.y, -2.0)
        self.assertEqual(v.z, -3.0)
        self.assertEqual(v.w, -4.0)

    def test_add(self):
        a = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyvek.vec4(5.0, 6.0, 7.0, 8.0)
        c = a + b
        self.assertEqual(c.x, 6.0)
        self.assertEqual(c.y, 8.0)
        self.assertEqual(c.z, 10.0)
        self.assertEqual(c.w, 12.0)

    def test_sub(self):
        a = pyvek.vec4(5.0, 6.0, 7.0, 8.0)
        b = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        c = a - b
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 4.0)
        self.assertEqual(c.w, 4.0)

    def test_mul_scalar(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        c = v * 2.0
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)
        self.assertEqual(c.w, 8.0)

    def test_rmul_scalar(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        c = 2.0 * v
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)
        self.assertEqual(c.w, 8.0)

    def test_div_scalar(self):
        v = pyvek.vec4(2.0, 4.0, 6.0, 8.0)
        c = v / 2.0
        self.assertEqual(c.x, 1.0)
        self.assertEqual(c.y, 2.0)
        self.assertEqual(c.z, 3.0)
        self.assertEqual(c.w, 4.0)

    def test_neg(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        n = -v
        self.assertEqual(n.x, -1.0)
        self.assertEqual(n.y, -2.0)
        self.assertEqual(n.z, -3.0)
        self.assertEqual(n.w, -4.0)

    def test_eq(self):
        a = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertTrue(a == b)

    def test_ne(self):
        a = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyvek.vec4(1.0, 2.0, 3.0, 5.0)
        self.assertTrue(a != b)

    def test_dot(self):
        a = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyvek.vec4(5.0, 6.0, 7.0, 8.0)
        self.assertAlmostEqual(a.dot(b), 70.0)

    def test_repr(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertIn("vec4", repr(v))


class TestDoubleVec(unittest.TestCase):
    def test_dvec2(self):
        v = pyvek.dvec2(1.0, 2.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertAlmostEqual(v.length(), math.sqrt(5))

    def test_dvec3(self):
        v = pyvek.dvec3(1.0, 2.0, 3.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertAlmostEqual(v.length(), math.sqrt(14))

    def test_dvec4(self):
        v = pyvek.dvec4(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertEqual(v.w, 4.0)
        self.assertAlmostEqual(v.length(), math.sqrt(30))

    def test_dvec3_cross(self):
        a = pyvek.dvec3(1.0, 0.0, 0.0)
        b = pyvek.dvec3(0.0, 1.0, 0.0)
        c = a.cross(b)
        self.assertAlmostEqual(c.z, 1.0)


class TestFreeFunctions(unittest.TestCase):
    def test_dot_vec2(self):
        a = pyvek.vec2(1.0, 2.0)
        b = pyvek.vec2(3.0, 4.0)
        self.assertAlmostEqual(pyvek.dot(a, b), 11.0)

    def test_dot_vec3(self):
        a = pyvek.vec3(1.0, 2.0, 3.0)
        b = pyvek.vec3(4.0, 5.0, 6.0)
        self.assertAlmostEqual(pyvek.dot(a, b), 32.0)

    def test_dot_vec4(self):
        a = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyvek.vec4(5.0, 6.0, 7.0, 8.0)
        self.assertAlmostEqual(pyvek.dot(a, b), 70.0)

    def test_cross(self):
        a = pyvek.vec3(1.0, 0.0, 0.0)
        b = pyvek.vec3(0.0, 1.0, 0.0)
        c = pyvek.cross(a, b)
        self.assertAlmostEqual(c.x, 0.0)
        self.assertAlmostEqual(c.y, 0.0)
        self.assertAlmostEqual(c.z, 1.0)

    def test_normalize_vec2(self):
        v = pyvek.vec2(3.0, 4.0)
        n = pyvek.normalize(v)
        self.assertAlmostEqual(n.length(), 1.0)
        self.assertAlmostEqual(n.x, 0.6)
        self.assertAlmostEqual(n.y, 0.8)

    def test_normalize_vec3(self):
        v = pyvek.vec3(2.0, 3.0, 6.0)
        n = pyvek.normalize(v)
        self.assertAlmostEqual(n.length(), 1.0)

    def test_normalize_vec4(self):
        v = pyvek.vec4(1.0, 2.0, 3.0, 4.0)
        n = pyvek.normalize(v)
        self.assertAlmostEqual(n.length(), 1.0, places=6)


class TestEdgeCases(unittest.TestCase):
    def test_zero_vector_length(self):
        v = pyvek.vec3(0.0, 0.0, 0.0)
        self.assertEqual(v.length(), 0.0)

    def test_unit_vectors(self):
        x = pyvek.vec3(1.0, 0.0, 0.0)
        y = pyvek.vec3(0.0, 1.0, 0.0)
        z = pyvek.vec3(0.0, 0.0, 1.0)
        self.assertAlmostEqual(x.length(), 1.0)
        self.assertAlmostEqual(y.length(), 1.0)
        self.assertAlmostEqual(z.length(), 1.0)
        self.assertAlmostEqual(x.dot(y), 0.0)
        self.assertAlmostEqual(y.dot(z), 0.0)
        self.assertAlmostEqual(z.dot(x), 0.0)

    def test_negative_values(self):
        v = pyvek.vec3(-1.0, -2.0, -3.0)
        self.assertEqual(v.x, -1.0)
        self.assertEqual(v.y, -2.0)
        self.assertEqual(v.z, -3.0)
        self.assertAlmostEqual(v.length(), math.sqrt(14), places=6)

    def test_property_modification(self):
        v = pyvek.vec3(1.0, 2.0, 3.0)
        v.x = 10.0
        v.y = 20.0
        v.z = 30.0
        self.assertEqual(v.x, 10.0)
        self.assertEqual(v.y, 20.0)
        self.assertEqual(v.z, 30.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
