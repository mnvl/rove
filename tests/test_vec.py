# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


def approx_equal(a, b, rel_tol=1e-6):
    return abs(a - b) <= rel_tol * max(abs(a), abs(b), 1.0)


class TestVec2(unittest.TestCase):
    def test_default_constructor(self):
        v = pyrove.vec2()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)

    def test_value_constructor(self):
        v = pyrove.vec2(3.0, 4.0)
        self.assertEqual(v.x, 3.0)
        self.assertEqual(v.y, 4.0)

    def test_set(self):
        v = pyrove.vec2()
        v.set(1.0, 2.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)

    def test_set_all(self):
        v = pyrove.vec2()
        v.set_all(5.0)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 5.0)

    def test_length(self):
        v = pyrove.vec2(3.0, 4.0)
        self.assertAlmostEqual(v.length(), 5.0)

    def test_length_sq(self):
        v = pyrove.vec2(3.0, 4.0)
        self.assertAlmostEqual(v.length_sq(), 25.0)

    def test_manhattan_length(self):
        v = pyrove.vec2(3.0, 4.0)
        self.assertAlmostEqual(v.manhattan_length(), 7.0)

    def test_normalize(self):
        v = pyrove.vec2(3.0, 4.0)
        v.normalize()
        self.assertAlmostEqual(v.length(), 1.0)
        self.assertAlmostEqual(v.x, 0.6)
        self.assertAlmostEqual(v.y, 0.8)

    def test_negate(self):
        v = pyrove.vec2(3.0, 4.0)
        v.negate()
        self.assertEqual(v.x, -3.0)
        self.assertEqual(v.y, -4.0)

    def test_perpendicular(self):
        v = pyrove.vec2(1.0, 0.0)
        p = v.perpendicular()
        # a quarter clockwise: (x, y) to (y, -x)
        self.assertAlmostEqual(p.x, 0.0)
        self.assertAlmostEqual(p.y, -1.0)
        q = pyrove.vec2(3.0, 4.0).perpendicular()
        self.assertAlmostEqual(q.x, 4.0)
        self.assertAlmostEqual(q.y, -3.0)

    def test_add(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(3.0, 4.0)
        c = a + b
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 6.0)

    def test_sub(self):
        a = pyrove.vec2(3.0, 4.0)
        b = pyrove.vec2(1.0, 1.0)
        c = a - b
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 3.0)

    def test_mul_vec(self):
        a = pyrove.vec2(2.0, 3.0)
        b = pyrove.vec2(4.0, 5.0)
        c = a * b
        self.assertEqual(c.x, 8.0)
        self.assertEqual(c.y, 15.0)

    def test_mul_scalar(self):
        v = pyrove.vec2(2.0, 3.0)
        c = v * 2.0
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 6.0)

    def test_rmul_scalar(self):
        v = pyrove.vec2(2.0, 3.0)
        c = 2.0 * v
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 6.0)

    def test_div_vec(self):
        a = pyrove.vec2(8.0, 15.0)
        b = pyrove.vec2(4.0, 5.0)
        c = a / b
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 3.0)

    def test_div_scalar(self):
        v = pyrove.vec2(4.0, 6.0)
        c = v / 2.0
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 3.0)

    def test_neg(self):
        v = pyrove.vec2(3.0, 4.0)
        n = -v
        self.assertEqual(n.x, -3.0)
        self.assertEqual(n.y, -4.0)

    def test_eq(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(1.0, 2.0)
        c = pyrove.vec2(1.0, 3.0)
        self.assertTrue(a == b)
        self.assertFalse(a == c)

    def test_ne(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(1.0, 3.0)
        self.assertTrue(a != b)

    def test_dot(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(3.0, 4.0)
        self.assertAlmostEqual(a.dot(b), 11.0)

    def test_repr(self):
        v = pyrove.vec2(1.0, 2.0)
        self.assertIn("vec2", repr(v))
        self.assertIn("1", repr(v))
        self.assertIn("2", repr(v))

    def test_is_collinear(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(2.0, 4.0)
        c = pyrove.vec2(1.0, 3.0)
        self.assertTrue(a.is_collinear(b))
        self.assertFalse(a.is_collinear(c))


class TestVec3(unittest.TestCase):
    def test_default_constructor(self):
        v = pyrove.vec3()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)

    def test_value_constructor(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_set(self):
        v = pyrove.vec3()
        v.set(1.0, 2.0, 3.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_set_all(self):
        v = pyrove.vec3()
        v.set_all(5.0)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 5.0)
        self.assertEqual(v.z, 5.0)

    def test_length(self):
        v = pyrove.vec3(2.0, 3.0, 6.0)
        self.assertAlmostEqual(v.length(), 7.0)

    def test_length_sq(self):
        v = pyrove.vec3(2.0, 3.0, 6.0)
        self.assertAlmostEqual(v.length_sq(), 49.0)

    def test_normalize(self):
        v = pyrove.vec3(2.0, 3.0, 6.0)
        v.normalize()
        self.assertAlmostEqual(v.length(), 1.0)

    def test_negate(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        v.negate()
        self.assertEqual(v.x, -1.0)
        self.assertEqual(v.y, -2.0)
        self.assertEqual(v.z, -3.0)

    def test_add(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 5.0, 6.0)
        c = a + b
        self.assertEqual(c.x, 5.0)
        self.assertEqual(c.y, 7.0)
        self.assertEqual(c.z, 9.0)

    def test_sub(self):
        a = pyrove.vec3(4.0, 5.0, 6.0)
        b = pyrove.vec3(1.0, 2.0, 3.0)
        c = a - b
        self.assertEqual(c.x, 3.0)
        self.assertEqual(c.y, 3.0)
        self.assertEqual(c.z, 3.0)

    def test_mul_vec(self):
        a = pyrove.vec3(2.0, 3.0, 4.0)
        b = pyrove.vec3(5.0, 6.0, 7.0)
        c = a * b
        self.assertEqual(c.x, 10.0)
        self.assertEqual(c.y, 18.0)
        self.assertEqual(c.z, 28.0)

    def test_mul_scalar(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        c = v * 2.0
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)

    def test_rmul_scalar(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        c = 2.0 * v
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)

    def test_div_scalar(self):
        v = pyrove.vec3(2.0, 4.0, 6.0)
        c = v / 2.0
        self.assertEqual(c.x, 1.0)
        self.assertEqual(c.y, 2.0)
        self.assertEqual(c.z, 3.0)

    def test_neg(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        n = -v
        self.assertEqual(n.x, -1.0)
        self.assertEqual(n.y, -2.0)
        self.assertEqual(n.z, -3.0)

    def test_eq(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(1.0, 2.0, 3.0)
        self.assertTrue(a == b)

    def test_ne(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(1.0, 2.0, 4.0)
        self.assertTrue(a != b)

    def test_dot(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 5.0, 6.0)
        self.assertAlmostEqual(a.dot(b), 32.0)

    def test_cross(self):
        a = pyrove.vec3(1.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 1.0, 0.0)
        c = a.cross(b)
        self.assertAlmostEqual(c.x, 0.0)
        self.assertAlmostEqual(c.y, 0.0)
        self.assertAlmostEqual(c.z, 1.0)

    def test_cross_anticommutative(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 5.0, 6.0)
        c1 = a.cross(b)
        c2 = b.cross(a)
        self.assertAlmostEqual(c1.x, -c2.x)
        self.assertAlmostEqual(c1.y, -c2.y)
        self.assertAlmostEqual(c1.z, -c2.z)

    def test_repr(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        self.assertIn("vec3", repr(v))

    def test_is_collinear(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(2.0, 4.0, 6.0)
        c = pyrove.vec3(1.0, 2.0, 4.0)
        self.assertTrue(a.is_collinear(b))
        self.assertFalse(a.is_collinear(c))


class TestVec4(unittest.TestCase):
    def test_default_constructor(self):
        v = pyrove.vec4()
        self.assertEqual(v.x, 0.0)
        self.assertEqual(v.y, 0.0)
        self.assertEqual(v.z, 0.0)
        self.assertEqual(v.w, 0.0)

    def test_value_constructor(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertEqual(v.w, 4.0)

    def test_set(self):
        v = pyrove.vec4()
        v.set(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertEqual(v.w, 4.0)

    def test_set_all(self):
        v = pyrove.vec4()
        v.set_all(5.0)
        self.assertEqual(v.x, 5.0)
        self.assertEqual(v.y, 5.0)
        self.assertEqual(v.z, 5.0)
        self.assertEqual(v.w, 5.0)

    def test_length(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        expected = math.sqrt(1 + 4 + 9 + 16)
        self.assertAlmostEqual(v.length(), expected, places=6)

    def test_length_sq(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertAlmostEqual(v.length_sq(), 30.0)

    def test_normalize(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        v.normalize()
        self.assertAlmostEqual(v.length(), 1.0, places=6)

    def test_negate(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        v.negate()
        self.assertEqual(v.x, -1.0)
        self.assertEqual(v.y, -2.0)
        self.assertEqual(v.z, -3.0)
        self.assertEqual(v.w, -4.0)

    def test_add(self):
        a = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyrove.vec4(5.0, 6.0, 7.0, 8.0)
        c = a + b
        self.assertEqual(c.x, 6.0)
        self.assertEqual(c.y, 8.0)
        self.assertEqual(c.z, 10.0)
        self.assertEqual(c.w, 12.0)

    def test_sub(self):
        a = pyrove.vec4(5.0, 6.0, 7.0, 8.0)
        b = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        c = a - b
        self.assertEqual(c.x, 4.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 4.0)
        self.assertEqual(c.w, 4.0)

    def test_mul_scalar(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        c = v * 2.0
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)
        self.assertEqual(c.w, 8.0)

    def test_rmul_scalar(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        c = 2.0 * v
        self.assertEqual(c.x, 2.0)
        self.assertEqual(c.y, 4.0)
        self.assertEqual(c.z, 6.0)
        self.assertEqual(c.w, 8.0)

    def test_div_scalar(self):
        v = pyrove.vec4(2.0, 4.0, 6.0, 8.0)
        c = v / 2.0
        self.assertEqual(c.x, 1.0)
        self.assertEqual(c.y, 2.0)
        self.assertEqual(c.z, 3.0)
        self.assertEqual(c.w, 4.0)

    def test_neg(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        n = -v
        self.assertEqual(n.x, -1.0)
        self.assertEqual(n.y, -2.0)
        self.assertEqual(n.z, -3.0)
        self.assertEqual(n.w, -4.0)

    def test_eq(self):
        a = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertTrue(a == b)

    def test_ne(self):
        a = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyrove.vec4(1.0, 2.0, 3.0, 5.0)
        self.assertTrue(a != b)

    def test_dot(self):
        a = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyrove.vec4(5.0, 6.0, 7.0, 8.0)
        self.assertAlmostEqual(a.dot(b), 70.0)

    def test_repr(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertIn("vec4", repr(v))


class TestDoubleVec(unittest.TestCase):
    def test_vec2d(self):
        v = pyrove.vec2d(1.0, 2.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertAlmostEqual(v.length(), math.sqrt(5))

    def test_vec3d(self):
        v = pyrove.vec3d(1.0, 2.0, 3.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertAlmostEqual(v.length(), math.sqrt(14))

    def test_vec4d(self):
        v = pyrove.vec4d(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertEqual(v.w, 4.0)
        self.assertAlmostEqual(v.length(), math.sqrt(30))

    def test_vec3d_cross(self):
        a = pyrove.vec3d(1.0, 0.0, 0.0)
        b = pyrove.vec3d(0.0, 1.0, 0.0)
        c = a.cross(b)
        self.assertAlmostEqual(c.z, 1.0)


class TestFreeFunctions(unittest.TestCase):
    def test_dot_vec2(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(3.0, 4.0)
        self.assertAlmostEqual(pyrove.dot(a, b), 11.0)

    def test_dot_vec3(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(4.0, 5.0, 6.0)
        self.assertAlmostEqual(pyrove.dot(a, b), 32.0)

    def test_dot_vec4(self):
        a = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyrove.vec4(5.0, 6.0, 7.0, 8.0)
        self.assertAlmostEqual(pyrove.dot(a, b), 70.0)

    def test_cross(self):
        a = pyrove.vec3(1.0, 0.0, 0.0)
        b = pyrove.vec3(0.0, 1.0, 0.0)
        c = pyrove.cross(a, b)
        self.assertAlmostEqual(c.x, 0.0)
        self.assertAlmostEqual(c.y, 0.0)
        self.assertAlmostEqual(c.z, 1.0)

    def test_normalize_vec2(self):
        v = pyrove.vec2(3.0, 4.0)
        n = pyrove.normalize(v)
        self.assertAlmostEqual(n.length(), 1.0)
        self.assertAlmostEqual(n.x, 0.6)
        self.assertAlmostEqual(n.y, 0.8)

    def test_normalize_vec3(self):
        v = pyrove.vec3(2.0, 3.0, 6.0)
        n = pyrove.normalize(v)
        self.assertAlmostEqual(n.length(), 1.0)

    def test_normalize_vec4(self):
        v = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        n = pyrove.normalize(v)
        self.assertAlmostEqual(n.length(), 1.0, places=6)

    def test_normalize_of_no_length_raises(self):
        for v in (pyrove.vec2(0.0, 0.0), pyrove.vec3(0.0, 0.0, 0.0), pyrove.vec4(0.0, 0.0, 0.0, 0.0)):
            with self.assertRaises(ValueError):
                pyrove.normalize(v)
            with self.assertRaises(ValueError):
                v.normalize()
            self.assertEqual(v.length(), 0.0)
        with self.assertRaises(ValueError):
            pyrove.normalize(pyrove.vec3(float("nan"), 1.0, 0.0))

    def test_normalize_or_zero(self):
        z = pyrove.normalize_or_zero(pyrove.vec3(0.0, 0.0, 0.0))
        self.assertEqual((z.x, z.y, z.z), (0.0, 0.0, 0.0))
        n = pyrove.normalize_or_zero(pyrove.vec2(3.0, 4.0))
        self.assertAlmostEqual(n.x, 0.6)

    def test_manhattan_length(self):
        self.assertAlmostEqual(pyrove.vec2(3.0, -4.0).manhattan_length(), 7.0)
        self.assertAlmostEqual(pyrove.vec3(1.0, -2.0, 3.0).manhattan_length(), 6.0)
        self.assertAlmostEqual(pyrove.vec4(1.0, -1.0, 1.0, -1.0).manhattan_length(), 4.0)


class TestSequenceProtocol(unittest.TestCase):
    def test_len(self):
        self.assertEqual(len(pyrove.vec2(1.0, 2.0)), 2)
        self.assertEqual(len(pyrove.vec3(1.0, 2.0, 3.0)), 3)
        self.assertEqual(len(pyrove.vec4(1.0, 2.0, 3.0, 4.0)), 4)

    def test_getitem_positive(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        self.assertEqual(v[0], 1.0)
        self.assertEqual(v[1], 2.0)
        self.assertEqual(v[2], 3.0)

    def test_getitem_negative(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        self.assertEqual(v[-1], 3.0)
        self.assertEqual(v[-2], 2.0)
        self.assertEqual(v[-3], 1.0)

    def test_getitem_out_of_range(self):
        v = pyrove.vec2(1.0, 2.0)
        with self.assertRaises(IndexError):
            _ = v[2]
        with self.assertRaises(IndexError):
            _ = v[-3]
        v3 = pyrove.vec3(1.0, 2.0, 3.0)
        with self.assertRaises(IndexError):
            _ = v3[3]
        with self.assertRaises(IndexError):
            _ = v3[-4]
        v4 = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        with self.assertRaises(IndexError):
            _ = v4[4]
        with self.assertRaises(IndexError):
            _ = v4[-5]

    def test_unpack_vec2(self):
        x, y = pyrove.vec2(1.0, 2.0)
        self.assertEqual((x, y), (1.0, 2.0))

    def test_unpack_vec3(self):
        x, y, z = pyrove.vec3(1.0, 2.0, 3.0)
        self.assertEqual((x, y, z), (1.0, 2.0, 3.0))

    def test_unpack_vec4(self):
        x, y, z, w = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertEqual((x, y, z, w), (1.0, 2.0, 3.0, 4.0))

    def test_unpack_wrong_arity_raises(self):
        # vec2 cannot unpack into 3 names
        with self.assertRaises(ValueError):
            x, y, z = pyrove.vec2(1.0, 2.0)

    def test_iter_list_tuple(self):
        self.assertEqual(list(pyrove.vec2(1.0, 2.0)), [1.0, 2.0])
        self.assertEqual(list(pyrove.vec3(1.0, 2.0, 3.0)), [1.0, 2.0, 3.0])
        self.assertEqual(tuple(pyrove.vec4(1.0, 2.0, 3.0, 4.0)), (1.0, 2.0, 3.0, 4.0))

    def test_iter_for_loop(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        collected = []
        for c in v:
            collected.append(c)
        self.assertEqual(collected, [1.0, 2.0, 3.0])

    def test_double_variants_iterable(self):
        self.assertEqual(list(pyrove.vec2d(1.0, 2.0)), [1.0, 2.0])
        self.assertEqual(list(pyrove.vec3d(1.0, 2.0, 3.0)), [1.0, 2.0, 3.0])
        self.assertEqual(list(pyrove.vec4d(1.0, 2.0, 3.0, 4.0)), [1.0, 2.0, 3.0, 4.0])


class TestHash(unittest.TestCase):
    def test_eq_implies_hash_equal_vec2(self):
        a = pyrove.vec2(1.0, 2.0)
        b = pyrove.vec2(1.0, 2.0)
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))

    def test_eq_implies_hash_equal_vec3(self):
        a = pyrove.vec3(1.0, 2.0, 3.0)
        b = pyrove.vec3(1.0, 2.0, 3.0)
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))

    def test_eq_implies_hash_equal_vec4(self):
        a = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        b = pyrove.vec4(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))

    def test_distinct_vectors_distinct_hash(self):
        # Not strictly required, but a permutation collision would suggest
        # the combine is order-insensitive — that would be a real bug.
        self.assertNotEqual(hash(pyrove.vec2(1.0, 2.0)), hash(pyrove.vec2(2.0, 1.0)))
        self.assertNotEqual(hash(pyrove.vec3(1.0, 2.0, 3.0)), hash(pyrove.vec3(3.0, 2.0, 1.0)))

    def test_set_dedup(self):
        s = {pyrove.vec2(1.0, 2.0), pyrove.vec2(1.0, 2.0), pyrove.vec2(3.0, 4.0)}
        self.assertEqual(len(s), 2)

    def test_dict_key(self):
        d = {pyrove.vec3(1.0, 2.0, 3.0): "a"}
        self.assertEqual(d[pyrove.vec3(1.0, 2.0, 3.0)], "a")

    def test_double_variant_hash(self):
        a = pyrove.vec3d(1.0, 2.0, 3.0)
        b = pyrove.vec3d(1.0, 2.0, 3.0)
        self.assertEqual(hash(a), hash(b))


class TestEdgeCases(unittest.TestCase):
    def test_zero_vector_length(self):
        v = pyrove.vec3(0.0, 0.0, 0.0)
        self.assertEqual(v.length(), 0.0)

    def test_unit_vectors(self):
        x = pyrove.vec3(1.0, 0.0, 0.0)
        y = pyrove.vec3(0.0, 1.0, 0.0)
        z = pyrove.vec3(0.0, 0.0, 1.0)
        self.assertAlmostEqual(x.length(), 1.0)
        self.assertAlmostEqual(y.length(), 1.0)
        self.assertAlmostEqual(z.length(), 1.0)
        self.assertAlmostEqual(x.dot(y), 0.0)
        self.assertAlmostEqual(y.dot(z), 0.0)
        self.assertAlmostEqual(z.dot(x), 0.0)

    def test_negative_values(self):
        v = pyrove.vec3(-1.0, -2.0, -3.0)
        self.assertEqual(v.x, -1.0)
        self.assertEqual(v.y, -2.0)
        self.assertEqual(v.z, -3.0)
        self.assertAlmostEqual(v.length(), math.sqrt(14), places=6)

    def test_property_modification(self):
        v = pyrove.vec3(1.0, 2.0, 3.0)
        v.x = 10.0
        v.y = 20.0
        v.z = 30.0
        self.assertEqual(v.x, 10.0)
        self.assertEqual(v.y, 20.0)
        self.assertEqual(v.z, 30.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
