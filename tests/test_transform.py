# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's trs (transform.h): a transform kept as translation, rotation and scale, whose matrix is T R S; taken apart from a matrix,
composed, inverted and blended only where the result is one; and unproject_ray, the ray under a point of the screen."""
import math
import unittest

import numpy as np

import pyrove


def qaxis(axis, angle, t=pyrove.quatd):
    n = math.sqrt(sum(c * c for c in axis))
    s = math.sin(angle / 2) / n
    return t(axis[0] * s, axis[1] * s, axis[2] * s, math.cos(angle / 2))


def mat_np(m):
    return np.array(m.to_numpy(), dtype=float)


def trs(t, axis, angle, s):
    return pyrove.trsd(pyrove.vec3d(*t), qaxis(axis, angle), pyrove.vec3d(*s))


class TestTrs(unittest.TestCase):
    def test_default_is_no_transform(self):
        t = pyrove.trs()
        self.assertTrue(np.array_equal(mat_np(t.to_matrix()), np.eye(4)))
        self.assertEqual(tuple(t.scale), (1, 1, 1))

    def test_the_matrix_scales_then_turns_then_moves(self):
        t = trs((1, 2, 3), (0, 0, 1), math.pi / 2, (2, 3, 4))
        want = mat_np(pyrove.translation_matrix(t.translation)) @ mat_np(pyrove.rotation_matrix(t.rotation)) @ \
            mat_np(pyrove.scaling_matrix(t.scale))
        self.assertTrue(np.allclose(mat_np(t.to_matrix()), want, atol=1e-12))
        p = t.apply_point(pyrove.vec3d(1, 0, 0))                # (2, 0, 0) turned to (0, 2, 0), moved
        self.assertTrue(np.allclose(tuple(p), (1, 4, 3)))
        v = t.apply_vector(pyrove.vec3d(1, 0, 0))
        self.assertTrue(np.allclose(tuple(v), (0, 2, 0)))

    def test_to_trs_takes_a_matrix_apart_and_refuses_what_is_none(self):
        t = trs((1, -2, 3), (1, 1, 0), 0.7, (2, 2, 5))
        back = pyrove.to_trs(t.to_matrix())
        self.assertTrue(np.allclose(mat_np(back.to_matrix()), mat_np(t.to_matrix()), atol=1e-12))
        self.assertTrue(np.allclose(tuple(back.scale), (2, 2, 5)))
        mirror = pyrove.to_trs(pyrove.scaling_matrix(pyrove.vec3d(-1, 1, 1)))
        self.assertLess(mirror.scale.x, 0)
        with self.assertRaises(ValueError):
            pyrove.to_trs(pyrove.perspective_matrix(1.0, 1.0, 0.1, 10.0))       # a projection
        shear = np.eye(4)
        shear[0, 1] = 0.5
        with self.assertRaises(ValueError):
            pyrove.to_trs(pyrove.mat4d.from_numpy(shear))
        with self.assertRaises(ValueError):
            pyrove.to_trs(pyrove.scaling_matrix(pyrove.vec3d(1, 0, 1)))

    def test_composing_is_the_matrices_product_where_it_is_a_trs(self):
        parent = trs((5, 0, 0), (0, 1, 0), 0.4, (2, 2, 2))           # uniform scale
        child = trs((0, 1, 0), (1, 0, 0), 1.0, (1, 3, 0.5))
        both = parent * child
        self.assertTrue(np.allclose(mat_np(both.to_matrix()), mat_np(parent.to_matrix()) @ mat_np(child.to_matrix()), atol=1e-12))
        unturned = trs((1, 1, 1), (0, 0, 1), 0.0, (1, 2, 3))         # a child that does not turn, under a stretch
        stretched = trs((0, 0, 0), (0, 0, 1), 0.0, (1, 4, 1))
        self.assertTrue(np.allclose(mat_np((stretched * unturned).to_matrix()),
                                    mat_np(stretched.to_matrix()) @ mat_np(unturned.to_matrix()), atol=1e-12))
        with self.assertRaises(ValueError):
            trs((0, 0, 0), (0, 0, 1), 0.0, (1, 4, 1)) * trs((0, 0, 0), (0, 0, 1), 0.5, (1, 1, 1))     # a shear

    def test_inverse_undoes_where_it_is_a_trs(self):
        t = trs((1, 2, 3), (0, 1, 1), 1.2, (3, 3, 3))
        inv = pyrove.inverse(t)
        self.assertTrue(np.allclose(mat_np(inv.to_matrix()) @ mat_np(t.to_matrix()), np.eye(4), atol=1e-12))
        with self.assertRaises(ValueError):
            pyrove.inverse(trs((0, 0, 0), (0, 0, 1), 0.5, (1, 2, 1)))
        with self.assertRaises(ValueError):
            pyrove.inverse(trs((0, 0, 0), (0, 0, 1), 0.0, (0, 1, 1)))

    def test_lerp_lerps_translation_and_scale_and_slerps_rotation(self):
        a = trs((0, 0, 0), (0, 0, 1), 0.0, (1, 1, 1))
        b = trs((2, 4, 6), (0, 0, 1), 1.0, (3, 1, 1))
        m = pyrove.lerp(a, b, 0.25)
        self.assertTrue(np.allclose(tuple(m.translation), (0.5, 1, 1.5)))
        self.assertTrue(np.allclose(tuple(m.scale), (1.5, 1, 1)))
        _, angle = pyrove.to_axis_angle(m.rotation)
        self.assertAlmostEqual(angle, 0.25, places=9)


class TestUnprojectRay(unittest.TestCase):
    def test_the_ray_runs_from_the_near_plane_to_the_far_under_the_point(self):
        view = pyrove.look_at_matrix(pyrove.vec3d(0, 0, 0), pyrove.vec3d(0, 0, 1), pyrove.vec3d(0, 1, 0))
        proj = pyrove.perspective_matrix(math.pi / 2, 1.0, 1.0, 100.0)
        vp = mat_np(proj) @ mat_np(view)
        inv = pyrove.mat4d.from_numpy(np.linalg.inv(vp))
        r = pyrove.unproject_ray(inv, 0.0, 0.0)
        self.assertIsInstance(r, pyrove.ray3d)
        self.assertTrue(np.allclose(tuple(r.r0), (0, 0, 1), atol=1e-9))
        self.assertTrue(np.allclose(tuple(r.r0 + r.a), (0, 0, 100), atol=1e-6))
        # every point of it projects back to the same place of the screen
        corner = pyrove.unproject_ray(inv, 0.5, -0.25)
        for s in (0.0, 0.3, 1.0):
            p = vp @ np.array(list(corner.r0 + corner.a * s) + [1.0])
            self.assertTrue(np.allclose(p[:2] / p[3], (0.5, -0.25), atol=1e-9))
        with self.assertRaises(ValueError):
            pyrove.unproject_ray(pyrove.mat4d.from_numpy(np.zeros((4, 4))), 0.0, 0.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
