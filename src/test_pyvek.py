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


class TestMat3(unittest.TestCase):
    def test_default_constructor(self):
        m = pyvek.mat3()
        # Default constructor doesn't initialize, so we just check it was created
        self.assertIsNotNone(m)

    def test_identity(self):
        m = pyvek.mat3()
        m.identity()
        self.assertAlmostEqual(m.get(0, 0), 1.0)
        self.assertAlmostEqual(m.get(1, 1), 1.0)
        self.assertAlmostEqual(m.get(2, 2), 1.0)
        self.assertAlmostEqual(m.get(0, 1), 0.0)
        self.assertAlmostEqual(m.get(0, 2), 0.0)
        self.assertAlmostEqual(m.get(1, 0), 0.0)
        self.assertAlmostEqual(m.get(1, 2), 0.0)
        self.assertAlmostEqual(m.get(2, 0), 0.0)
        self.assertAlmostEqual(m.get(2, 1), 0.0)

    def test_zero(self):
        m = pyvek.mat3()
        m.zero()
        for i in range(3):
            for j in range(3):
                self.assertAlmostEqual(m.get(i, j), 0.0)

    def test_determinant(self):
        m = pyvek.mat3()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)

    def test_trace(self):
        m = pyvek.mat3()
        m.identity()
        self.assertAlmostEqual(m.trace(), 3.0)

    def test_translation_scalar(self):
        m = pyvek.mat3()
        m.translation(2.0, 3.0)
        self.assertAlmostEqual(m.get(2, 0), 2.0)
        self.assertAlmostEqual(m.get(2, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 1.0)

    def test_translation_vec(self):
        m = pyvek.mat3()
        v = pyvek.vec2(4.0, 5.0)
        m.translation(v)
        self.assertAlmostEqual(m.get(2, 0), 4.0)
        self.assertAlmostEqual(m.get(2, 1), 5.0)

    def test_scaling_scalar(self):
        m = pyvek.mat3()
        m.scaling(2.0, 3.0)
        self.assertAlmostEqual(m.get(0, 0), 2.0)
        self.assertAlmostEqual(m.get(1, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 1.0)

    def test_scaling_vec(self):
        m = pyvek.mat3()
        v = pyvek.vec2(2.5, 3.5)
        m.scaling(v)
        self.assertAlmostEqual(m.get(0, 0), 2.5)
        self.assertAlmostEqual(m.get(1, 1), 3.5)

    def test_rotation(self):
        m = pyvek.mat3()
        m.rotation(pyvek.vec3(0, 0, 1), math.pi / 2)  # 90 degrees around Z
        # Matrix is column-major, so get(col, row)
        # Rotation matrix around Z by 90 degrees counter-clockwise:
        # Row 0: [0, -1, 0]  => get(0,0)=0, get(1,0)=-1, get(2,0)=0
        # Row 1: [1,  0, 0]  => get(0,1)=1, get(1,1)=0,  get(2,1)=0
        # Row 2: [0,  0, 1]  => get(0,2)=0, get(1,2)=0,  get(2,2)=1
        self.assertAlmostEqual(m.get(0, 0), 0.0, places=5)
        self.assertAlmostEqual(m.get(1, 0), -1.0, places=5)
        self.assertAlmostEqual(m.get(0, 1), 1.0, places=5)
        self.assertAlmostEqual(m.get(1, 1), 0.0, places=5)

    def test_transpose(self):
        m = pyvek.mat3()
        m.identity()
        m.set(0, 1, 2.0)
        m.set(1, 0, 3.0)

        mt = m.transpose()
        self.assertAlmostEqual(mt.get(0, 1), 3.0)
        self.assertAlmostEqual(mt.get(1, 0), 2.0)

    def test_inverse(self):
        m = pyvek.mat3()
        m.identity()
        m.scaling(2.0, 3.0)

        mi = m.inverse()
        self.assertAlmostEqual(mi.get(0, 0), 0.5)
        self.assertAlmostEqual(mi.get(1, 1), 1.0/3.0, places=5)

    def test_scalar_multiplication(self):
        m = pyvek.mat3()
        m.identity()
        m2 = m * 2.0
        self.assertAlmostEqual(m2.get(0, 0), 2.0)
        self.assertAlmostEqual(m2.get(1, 1), 2.0)

    def test_scalar_rmul(self):
        m = pyvek.mat3()
        m.identity()
        m2 = 3.0 * m
        self.assertAlmostEqual(m2.get(0, 0), 3.0)
        self.assertAlmostEqual(m2.get(1, 1), 3.0)

    def test_scalar_division(self):
        m = pyvek.mat3()
        m.identity()
        m.set(0, 0, 4.0)
        m2 = m / 2.0
        self.assertAlmostEqual(m2.get(0, 0), 2.0)

    def test_matrix_multiplication(self):
        m1 = pyvek.mat3()
        m1.identity()
        m1.scaling(2.0, 2.0)

        m2 = pyvek.mat3()
        m2.identity()
        m2.translation(1.0, 1.0)

        m3 = m1 * m2
        # Scale * Translation should keep translation at 1,1
        # (scaling doesn't affect translation in this order)
        self.assertAlmostEqual(m3.get(2, 0), 1.0)
        self.assertAlmostEqual(m3.get(2, 1), 1.0)

    def test_get_set(self):
        m = pyvek.mat3()
        m.zero()
        m.set(1, 2, 42.0)
        self.assertAlmostEqual(m.get(1, 2), 42.0)

    def test_get_out_of_range(self):
        m = pyvek.mat3()
        m.identity()
        with self.assertRaises(Exception):
            m.get(3, 0)
        with self.assertRaises(Exception):
            m.get(0, 3)

    def test_repr(self):
        m = pyvek.mat3()
        m.identity()
        s = repr(m)
        self.assertIn("mat3", s)


class TestMat4(unittest.TestCase):
    def test_default_constructor(self):
        m = pyvek.mat4()
        self.assertIsNotNone(m)

    def test_identity(self):
        m = pyvek.mat4()
        m.identity()
        for i in range(4):
            for j in range(4):
                if i == j:
                    self.assertAlmostEqual(m.get(i, j), 1.0)
                else:
                    self.assertAlmostEqual(m.get(i, j), 0.0)

    def test_zero(self):
        m = pyvek.mat4()
        m.zero()
        for i in range(4):
            for j in range(4):
                self.assertAlmostEqual(m.get(i, j), 0.0)

    def test_determinant(self):
        m = pyvek.mat4()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)

    def test_translation_scalar(self):
        m = pyvek.mat4()
        m.translation(1.0, 2.0, 3.0)
        self.assertAlmostEqual(m.get(3, 0), 1.0)
        self.assertAlmostEqual(m.get(3, 1), 2.0)
        self.assertAlmostEqual(m.get(3, 2), 3.0)

    def test_translation_vec(self):
        m = pyvek.mat4()
        v = pyvek.vec3(4.0, 5.0, 6.0)
        m.translation(v)
        self.assertAlmostEqual(m.get(3, 0), 4.0)
        self.assertAlmostEqual(m.get(3, 1), 5.0)
        self.assertAlmostEqual(m.get(3, 2), 6.0)

    def test_scaling_scalar(self):
        m = pyvek.mat4()
        m.scaling(2.0, 3.0, 4.0)
        self.assertAlmostEqual(m.get(0, 0), 2.0)
        self.assertAlmostEqual(m.get(1, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 4.0)

    def test_scaling_vec(self):
        m = pyvek.mat4()
        v = pyvek.vec3(2.5, 3.5, 4.5)
        m.scaling(v)
        self.assertAlmostEqual(m.get(0, 0), 2.5)
        self.assertAlmostEqual(m.get(1, 1), 3.5)
        self.assertAlmostEqual(m.get(2, 2), 4.5)

    def test_rotation(self):
        m = pyvek.mat4()
        m.rotation(pyvek.vec3(0, 0, 1), math.pi / 2)  # 90 degrees around Z
        # Matrix is column-major, so get(col, row)
        self.assertAlmostEqual(m.get(0, 0), 0.0, places=5)
        self.assertAlmostEqual(m.get(1, 0), -1.0, places=5)
        self.assertAlmostEqual(m.get(0, 1), 1.0, places=5)
        self.assertAlmostEqual(m.get(1, 1), 0.0, places=5)

    def test_lookat(self):
        m = pyvek.mat4()
        eye = pyvek.vec3(0, 0, 5)
        at = pyvek.vec3(0, 0, 0)
        up = pyvek.vec3(0, 1, 0)
        m.lookat(eye, at, up)
        # Just verify it doesn't crash and produces a matrix
        self.assertIsNotNone(m)
        # The z-axis (column 2) should point from eye to at (forward = negative Z in this case)
        self.assertAlmostEqual(abs(m.get(2, 2)), 1.0, places=5)

    def test_perspective(self):
        m = pyvek.mat4()
        m.perspective(math.pi / 4, 16.0/9.0, 0.1, 100.0)
        # Check that perspective matrix was created
        # The matrix should have non-zero values in specific positions
        self.assertNotAlmostEqual(m.get(0, 0), 0.0)
        self.assertNotAlmostEqual(m.get(1, 1), 0.0)
        self.assertNotAlmostEqual(m.get(2, 2), 0.0)

    def test_ortho(self):
        m = pyvek.mat4()
        m.ortho(800, 600, 0.1, 100.0)
        # Check that ortho matrix was created
        self.assertAlmostEqual(m.get(0, 0), 2.0/800.0, places=5)
        self.assertAlmostEqual(m.get(1, 1), 2.0/600.0, places=5)

    def test_transpose(self):
        m = pyvek.mat4()
        m.identity()
        m.set(0, 1, 2.0)
        m.set(1, 0, 3.0)

        mt = m.transpose()
        self.assertAlmostEqual(mt.get(0, 1), 3.0)
        self.assertAlmostEqual(mt.get(1, 0), 2.0)

    def test_inverse(self):
        m = pyvek.mat4()
        m.identity()
        m.scaling(2.0, 3.0, 4.0)

        mi = m.inverse()
        self.assertAlmostEqual(mi.get(0, 0), 0.5)
        self.assertAlmostEqual(mi.get(1, 1), 1.0/3.0, places=5)
        self.assertAlmostEqual(mi.get(2, 2), 0.25)

    def test_scalar_multiplication(self):
        m = pyvek.mat4()
        m.identity()
        m2 = m * 2.0
        for i in range(4):
            self.assertAlmostEqual(m2.get(i, i), 2.0)

    def test_scalar_rmul(self):
        m = pyvek.mat4()
        m.identity()
        m2 = 3.0 * m
        for i in range(4):
            self.assertAlmostEqual(m2.get(i, i), 3.0)

    def test_scalar_division(self):
        m = pyvek.mat4()
        m.identity()
        m.set(0, 0, 4.0)
        m2 = m / 2.0
        self.assertAlmostEqual(m2.get(0, 0), 2.0)

    def test_matrix_multiplication(self):
        m1 = pyvek.mat4()
        m1.identity()
        m1.translation(1.0, 0.0, 0.0)

        m2 = pyvek.mat4()
        m2.identity()
        m2.scaling(2.0, 2.0, 2.0)

        m3 = m1 * m2
        # Translation should be scaled
        self.assertAlmostEqual(m3.get(3, 0), 2.0)

    def test_column(self):
        m = pyvek.mat4()
        m.identity()
        m.set(0, 0, 1.0)
        m.set(0, 1, 2.0)
        m.set(0, 2, 3.0)
        m.set(0, 3, 4.0)

        col = m.column(0)
        self.assertAlmostEqual(col.x, 1.0)
        self.assertAlmostEqual(col.y, 2.0)
        self.assertAlmostEqual(col.z, 3.0)
        self.assertAlmostEqual(col.w, 4.0)

    def test_projected_column(self):
        m = pyvek.mat4()
        m.identity()
        m.set(0, 0, 1.0)
        m.set(0, 1, 2.0)
        m.set(0, 2, 3.0)

        col = m.projected_column(0)
        self.assertAlmostEqual(col.x, 1.0)
        self.assertAlmostEqual(col.y, 2.0)
        self.assertAlmostEqual(col.z, 3.0)

    def test_projected_row(self):
        m = pyvek.mat4()
        m.identity()
        m.set(0, 0, 1.0)
        m.set(1, 0, 2.0)
        m.set(2, 0, 3.0)

        row = m.projected_row(0)
        self.assertAlmostEqual(row.x, 1.0)
        self.assertAlmostEqual(row.y, 2.0)
        self.assertAlmostEqual(row.z, 3.0)

    def test_get_set(self):
        m = pyvek.mat4()
        m.zero()
        m.set(2, 3, 42.0)
        self.assertAlmostEqual(m.get(2, 3), 42.0)

    def test_get_out_of_range(self):
        m = pyvek.mat4()
        m.identity()
        with self.assertRaises(Exception):
            m.get(4, 0)
        with self.assertRaises(Exception):
            m.get(0, 4)

    def test_repr(self):
        m = pyvek.mat4()
        m.identity()
        s = repr(m)
        self.assertIn("mat4", s)


class TestDoubleMat(unittest.TestCase):
    def test_dmat3_identity(self):
        m = pyvek.dmat3()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)
        self.assertAlmostEqual(m.trace(), 3.0)

    def test_dmat3_scaling(self):
        m = pyvek.dmat3()
        m.scaling(1.5, 2.5)
        self.assertAlmostEqual(m.get(0, 0), 1.5)
        self.assertAlmostEqual(m.get(1, 1), 2.5)

    def test_dmat3_translation(self):
        m = pyvek.dmat3()
        v = pyvek.dvec2(3.5, 4.5)
        m.translation(v)
        self.assertAlmostEqual(m.get(2, 0), 3.5)
        self.assertAlmostEqual(m.get(2, 1), 4.5)

    def test_dmat4_identity(self):
        m = pyvek.dmat4()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)
        for i in range(4):
            self.assertAlmostEqual(m.get(i, i), 1.0)

    def test_dmat4_translation(self):
        m = pyvek.dmat4()
        v = pyvek.dvec3(1.5, 2.5, 3.5)
        m.translation(v)
        self.assertAlmostEqual(m.get(3, 0), 1.5)
        self.assertAlmostEqual(m.get(3, 1), 2.5)
        self.assertAlmostEqual(m.get(3, 2), 3.5)

    def test_dmat4_scaling(self):
        m = pyvek.dmat4()
        m.scaling(2.0, 3.0, 4.0)
        self.assertAlmostEqual(m.get(0, 0), 2.0)
        self.assertAlmostEqual(m.get(1, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 4.0)

    def test_dmat4_perspective(self):
        m = pyvek.dmat4()
        m.perspective(math.pi / 4, 16.0/9.0, 0.1, 100.0)
        # Just verify it creates a valid matrix
        self.assertNotAlmostEqual(m.get(0, 0), 0.0)


class TestMatrixEdgeCases(unittest.TestCase):
    def test_singular_matrix_inverse(self):
        m = pyvek.mat4()
        m.zero()  # Singular matrix (determinant = 0)
        with self.assertRaises(Exception):
            m.inverse()

    def test_identity_multiply_identity(self):
        m1 = pyvek.mat4()
        m1.identity()
        m2 = pyvek.mat4()
        m2.identity()
        m3 = m1 * m2
        for i in range(4):
            for j in range(4):
                if i == j:
                    self.assertAlmostEqual(m3.get(i, j), 1.0)
                else:
                    self.assertAlmostEqual(m3.get(i, j), 0.0)

    def test_translate_then_scale(self):
        m = pyvek.mat4()
        m.identity()
        m.translate(1.0, 2.0, 3.0)
        m.scale(2.0, 2.0, 2.0)
        # After translate then scale, translation should be doubled
        self.assertAlmostEqual(m.get(3, 0), 2.0)
        self.assertAlmostEqual(m.get(3, 1), 4.0)
        self.assertAlmostEqual(m.get(3, 2), 6.0)

    def test_inverse_of_inverse(self):
        m = pyvek.mat4()
        m.identity()
        m.scaling(2.0, 3.0, 4.0)

        mi = m.inverse()
        mii = mi.inverse()

        # Inverse of inverse should be close to original
        self.assertAlmostEqual(mii.get(0, 0), 2.0, places=5)
        self.assertAlmostEqual(mii.get(1, 1), 3.0, places=5)
        self.assertAlmostEqual(mii.get(2, 2), 4.0, places=5)


if __name__ == "__main__":
    unittest.main(verbosity=2)
