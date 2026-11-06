# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's boxes and frustum (aabb.h, frustum.h): an empty box as a state, a box grown, the box of points, a box through a matrix,
and a frustum against a sphere."""
import math
import unittest

import pyrove


def box(lo, hi, t=pyrove.aabb3):
    v = pyrove.vec3 if t is pyrove.aabb3 else pyrove.vec3d
    return t(v(*lo), v(*hi))


class TestEmptyAndGrown(unittest.TestCase):
    def test_a_box_made_empty_is_empty_and_a_point_is_not(self):
        b = pyrove.aabb3()
        self.assertTrue(b.is_empty())               # aabb() is empty (it was uninitialised)
        b.extend(pyrove.vec3(1, 2, 3))
        self.assertFalse(b.is_empty())              # a single point is a box
        self.assertTrue(box((0, 0, 0), (1, -1, 1)).is_empty())
        self.assertTrue(pyrove.aabb2d().is_empty())

    def test_grown_moves_every_side_and_an_empty_box_stays_empty(self):
        g = box((0, 0, 0), (1, 2, 3)).grown(0.5)
        self.assertEqual((tuple(g.lo), tuple(g.hi)), ((-0.5, -0.5, -0.5), (1.5, 2.5, 3.5)))
        self.assertTrue(pyrove.aabb3().grown(10.0).is_empty())
        self.assertTrue(box((0, 0, 0), (1, 2, 3)).grown(-0.6).is_empty())     # shrunk past nothing on x
        self.assertFalse(box((0, 0, 0), (1, 2, 3)).grown(-0.4).is_empty())


class TestBoundsOf(unittest.TestCase):
    def test_the_box_of_points(self):
        b = pyrove.bounds_of([pyrove.vec3(1, 5, -1), pyrove.vec3(-2, 0, 4), pyrove.vec3(0, 7, 0)])
        self.assertIsInstance(b, pyrove.aabb3)
        self.assertEqual((tuple(b.lo), tuple(b.hi)), ((-2, 0, -1), (1, 7, 4)))
        b2 = pyrove.bounds_of([pyrove.vec2d(1, 1), pyrove.vec2d(-1, 3)])
        self.assertIsInstance(b2, pyrove.aabb2d)
        self.assertEqual((tuple(b2.lo), tuple(b2.hi)), ((-1, 1), (1, 3)))

    def test_no_points_is_the_empty_box(self):
        self.assertTrue(pyrove.bounds_of([]).is_empty())


class TestTransformAabb(unittest.TestCase):
    def test_the_box_of_the_moved_corners(self):
        b = box((-1, -1, -1), (1, 1, 1), pyrove.aabb3d)
        m = pyrove.translation_matrix(pyrove.vec3d(10, 0, 0)) * pyrove.rotation_matrix(pyrove.vec3d(0, 0, 1), math.pi / 4)
        t = pyrove.transform_aabb(b, m)
        r = math.sqrt(2)
        for got, want in zip(list(t.lo) + list(t.hi), [10 - r, -r, -1, 10 + r, r, 1]):
            self.assertAlmostEqual(got, want, places=12)

    def test_an_empty_box_stays_empty_and_a_projection_is_refused(self):
        m = pyrove.translation_matrix(pyrove.vec3d(1, 2, 3))
        self.assertTrue(pyrove.transform_aabb(pyrove.aabb3d(), m).is_empty())
        p = pyrove.perspective_matrix(1.0, 1.0, 0.1, 10.0)
        with self.assertRaises(ValueError):
            pyrove.transform_aabb(box((0, 0, 1), (1, 1, 2)), p)


class TestFrustumSphere(unittest.TestCase):
    def setUp(self):
        view = pyrove.look_at_matrix(pyrove.vec3(0, 0, 0), pyrove.vec3(0, 0, 1), pyrove.vec3(0, 1, 0))
        proj = pyrove.perspective_matrix(math.pi / 2, 1.0, 1.0, 100.0)
        self.f = pyrove.frustum(proj * view)

    def test_a_sphere_in_view_on_an_edge_or_out(self):
        self.assertTrue(self.f.test_intersection(pyrove.sphere3(pyrove.vec3(0, 0, 50), 1.0)))
        self.assertTrue(self.f.test_intersection(pyrove.sphere3(pyrove.vec3(0, 0, 0.5), 0.6)))     # across the near plane
        self.assertFalse(self.f.test_intersection(pyrove.sphere3(pyrove.vec3(0, 0, 0.5), 0.4)))    # before it
        self.assertFalse(self.f.test_intersection(pyrove.sphere3(pyrove.vec3(0, 0, -5), 1.0)))     # behind
        # beside the 45-degree side plane x = z: the distance to it is (x - z) / sqrt(2)
        self.assertFalse(self.f.test_intersection(pyrove.sphere3(pyrove.vec3(20, 0, 10), 7.0)))
        self.assertTrue(self.f.test_intersection(pyrove.sphere3(pyrove.vec3(20, 0, 10), 7.2)))


if __name__ == "__main__":
    unittest.main(verbosity=2)
