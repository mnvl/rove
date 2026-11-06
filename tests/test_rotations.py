# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's quaternions and matrices by value (quaternion.h, matrix.h): rotating, inverting and interpolating rotations, rotations
between directions and to and from axis-angle and rotation vectors; points and vectors through matrices by name; the transforms
returned rather than set.  Oracles: numpy and the geometry (a rotation keeps lengths, an inverse undoes)."""
import math
import unittest

import numpy as np

import pyrove


def close(a, b, tol=1e-9):
    a, b = list(components(a)), list(components(b))
    return len(a) == len(b) and all(abs(x - y) <= tol for x, y in zip(a, b))


def components(v):
    if isinstance(v, (pyrove.quat, pyrove.quatd)):
        return (v.x, v.y, v.z, v.w)
    return tuple(v)


def quat_axis_angle(axis, angle, t=pyrove.quatd):
    n = math.sqrt(sum(c * c for c in axis))
    s = math.sin(angle / 2) / n
    return t(axis[0] * s, axis[1] * s, axis[2] * s, math.cos(angle / 2))


def mat_np(m):
    return np.array(m.to_numpy(), dtype=float)


class TestQuaternions(unittest.TestCase):
    def test_rotate_turns_a_vector_and_q_times_v_is_the_same(self):
        q = quat_axis_angle((0, 0, 1), math.pi / 2)
        v = pyrove.rotate(q, pyrove.vec3d(1, 0, 0))
        self.assertTrue(close(v, (0, 1, 0)))
        self.assertTrue(close(q * pyrove.vec3d(1, 0, 0), v))
        # a quaternion of any length but none: the same rotation
        self.assertTrue(close(pyrove.rotate(q * 3.0, pyrove.vec3d(1, 0, 0)), v))
        with self.assertRaises(ValueError):
            pyrove.rotate(pyrove.quatd(0, 0, 0, 0), pyrove.vec3d(1, 0, 0))

    def test_the_product_of_quaternions_is_the_composed_rotation(self):
        a, b = quat_axis_angle((0, 0, 1), 0.3), quat_axis_angle((1, 0, 0), 1.1)
        v = pyrove.vec3d(0.2, -0.7, 1.5)
        self.assertTrue(close(pyrove.rotate(a * b, v), pyrove.rotate(a, pyrove.rotate(b, v))))

    def test_inverse_undoes_and_refuses_no_rotation(self):
        q = quat_axis_angle((1, 2, 3), 0.8) * 2.0
        v = pyrove.vec3d(3, -1, 2)
        self.assertTrue(close(pyrove.rotate(pyrove.inverse(q), pyrove.rotate(q, v)), v))
        with self.assertRaises(ValueError):
            pyrove.inverse(pyrove.quat(0, 0, 0, 0))

    def test_normalize_of_no_quaternion_is_refused(self):
        with self.assertRaises(ValueError):
            pyrove.quat(0, 0, 0, 0).normalize()

    def test_rotation_matrix_and_to_quaternion_agree(self):
        q = quat_axis_angle((1, -1, 2), 1.3)
        m = pyrove.rotation_matrix(q)
        self.assertIsInstance(m, pyrove.mat4d)
        v = pyrove.vec3d(0.4, 0.1, -2)
        self.assertTrue(close(pyrove.transform_vector(m, v), pyrove.rotate(q, v)))
        back = pyrove.to_quaternion(m)
        self.assertGreaterEqual(back.w, 0)
        self.assertTrue(close(back, q) or close(back, -q))
        self.assertTrue(close(pyrove.rotate(pyrove.to_quaternion(pyrove.linear_part(m)), v), pyrove.rotate(q, v)))

    def test_rotation_between_takes_one_direction_onto_the_other(self):
        for a, b in [((1, 0, 0), (0, 1, 0)), ((1, 2, 3), (-3, 0.5, 2)), ((0, 0, 2), (0, 0, 5)), ((1, 0, 0), (-4, 0, 0)),
                     ((0, 1, 0), (0, -1, 0))]:
            q = pyrove.rotation_between(pyrove.vec3d(*a), pyrove.vec3d(*b))
            turned = pyrove.rotate(q, pyrove.vec3d(*a))
            nb = math.sqrt(sum(c * c for c in b))
            na = math.sqrt(sum(c * c for c in a))
            self.assertTrue(close([c / na for c in turned], [c / nb for c in b], 1e-9), (a, b, turned))
            # the shortest: by the angle between them
            _, angle = pyrove.to_axis_angle(q)
            self.assertAlmostEqual(angle, pyrove.angle_between(pyrove.vec3d(*a), pyrove.vec3d(*b)), places=9)
        with self.assertRaises(ValueError):
            pyrove.rotation_between(pyrove.vec3d(0, 0, 0), pyrove.vec3d(1, 0, 0))

    def test_axis_angle_and_rotation_vectors_round_trip(self):
        q = quat_axis_angle((0, 3, 4), 2.0)
        axis, angle = pyrove.to_axis_angle(q)
        self.assertTrue(close(axis, (0, 0.6, 0.8)))
        self.assertAlmostEqual(angle, 2.0, places=12)
        r = pyrove.rotation_vector(q)
        self.assertTrue(close(r, (0, 1.2, 1.6)))
        back = pyrove.from_rotation_vector(r)
        self.assertTrue(close(back, q))
        # no rotation: axis x, angle 0, the zero vector
        axis, angle = pyrove.to_axis_angle(pyrove.quatd(0, 0, 0, 1))
        self.assertEqual((tuple(axis), angle), ((1, 0, 0), 0))
        self.assertEqual(pyrove.rotation_vector(pyrove.quatd(0, 0, 0, 1)), pyrove.vec3d(0, 0, 0))
        # tiny turns keep their size
        tiny = pyrove.from_rotation_vector(pyrove.vec3d(1e-12, 0, 0))
        self.assertAlmostEqual(pyrove.rotation_vector(tiny).x, 1e-12, delta=1e-24)
        with self.assertRaises(ValueError):
            pyrove.to_axis_angle(pyrove.quatd(0, 0, 0, 0))
        with self.assertRaises(ValueError):
            pyrove.rotation_vector(pyrove.quatd(0, 0, 0, 0))

    def test_slerp_and_nlerp_take_the_shorter_arc(self):
        p, q = quat_axis_angle((0, 0, 1), 0.0), quat_axis_angle((0, 0, 1), 1.0)
        for f in (pyrove.slerp, pyrove.nlerp):
            self.assertTrue(close(f(p, q, 0.0), p))
            mid = f(p, -q, 0.5)                             # -q is q's rotation: halfway is half of it, not the long way round
            _, angle = pyrove.to_axis_angle(mid)
            self.assertAlmostEqual(angle, 0.5, places=9)
            end = f(p, q, 1.0)
            self.assertTrue(close(pyrove.rotate(end, pyrove.vec3d(1, 0, 0)), pyrove.rotate(q, pyrove.vec3d(1, 0, 0))))
        # slerp at constant speed
        _, angle = pyrove.to_axis_angle(pyrove.slerp(p, q, 0.25))
        self.assertAlmostEqual(angle, 0.25, places=9)
        # halfway between q and -q, the same rotation, is that rotation
        half = pyrove.nlerp(q, -q, 0.5)
        self.assertTrue(close(pyrove.rotate(half, pyrove.vec3d(1, 0, 0)), pyrove.rotate(q, pyrove.vec3d(1, 0, 0))))
        f = pyrove.slerp(pyrove.quat(0, 0, 0, 1), pyrove.quat(0, 0, 0.5, 0.8660254), 0.5)
        self.assertIsInstance(f, pyrove.quat)

    def test_nlerp_is_the_normalised_lerp_not_constant_speed(self):
        p, q = quat_axis_angle((0, 0, 1), 0.0), quat_axis_angle((0, 0, 1), 2.5)
        a, b = np.array(components(p)), np.array(components(q))
        for t in (0.1, 0.25, 0.6):
            want = (1 - t) * a + t * b
            want /= np.linalg.norm(want)
            self.assertTrue(close(pyrove.nlerp(p, q, t), want, 1e-12))
            _, angle = pyrove.to_axis_angle(pyrove.nlerp(p, q, t))
            self.assertGreater(abs(angle - 2.5 * t), 1e-3)           # off slerp's even pace mid-way


class TestMatricesByValue(unittest.TestCase):
    def test_transform_point_vector_and_project_point(self):
        m = pyrove.translation_matrix(pyrove.vec3d(1, 2, 3))
        self.assertIsInstance(m, pyrove.mat4d)
        self.assertEqual(pyrove.transform_point(m, pyrove.vec3d(1, 1, 1)), pyrove.vec3d(2, 3, 4))
        self.assertEqual(pyrove.transform_vector(m, pyrove.vec3d(1, 1, 1)), pyrove.vec3d(1, 1, 1))
        p = pyrove.perspective_matrix(math.pi / 2, 1.0, 1.0, 100.0)
        self.assertIsInstance(p, pyrove.mat4)
        # depth 0 at the near plane and 1 at the far, after the divide
        self.assertAlmostEqual(pyrove.project_point(p, pyrove.vec3(0, 0, 1)).z, 0.0, places=5)
        self.assertAlmostEqual(pyrove.project_point(p, pyrove.vec3(0, 0, 100)).z, 1.0, places=5)
        with self.assertRaises(ValueError):
            pyrove.project_point(p, pyrove.vec3(1, 1, 0))       # on the plane through the eye: w of zero
        # through a projection, transform_point does not divide and project_point does
        pd = mat_np(pyrove.perspective_matrix(1.0, 1.5, 0.5, 50.0))
        point = np.array([0.3, -0.2, 7.0, 1.0])
        clip = pd @ point
        self.assertTrue(close(pyrove.transform_point(pyrove.mat4d.from_numpy(pd), pyrove.vec3d(*point[:3])), clip[:3], 1e-9))
        self.assertTrue(close(pyrove.project_point(pyrove.mat4d.from_numpy(pd), pyrove.vec3d(*point[:3])), clip[:3] / clip[3], 1e-9))
        self.assertTrue(close(pyrove.transform_point(pyrove.mat4.from_numpy(pd.astype(np.float32)), pyrove.vec3(*point[:3])),
                              clip[:3], 1e-5))

    def test_3x3_point_maps(self):
        # x += 5 (the third column moves), and the third component doubles
        m = pyrove.mat3d.from_numpy(np.array([[1.0, 0.0, 5.0], [0.0, 1.0, 0.0], [0.0, 0.0, 2.0]]))
        self.assertEqual(pyrove.transform_point(m, pyrove.vec2d(1, 1)), pyrove.vec2d(6, 1))
        self.assertEqual(pyrove.project_point(m, pyrove.vec2d(1, 1)), pyrove.vec2d(3, 0.5))

    def test_inverse_transpose_and_their_refusal(self):
        m = pyrove.rotation_matrix(pyrove.vec3d(1, 2, 2), 0.7) * pyrove.scaling_matrix(pyrove.vec3d(2, 3, 4))
        self.assertTrue(np.allclose(mat_np(pyrove.inverse(m)) @ mat_np(m), np.eye(4), atol=1e-12))
        self.assertTrue(np.allclose(mat_np(pyrove.transpose(m)), mat_np(m).T))
        with self.assertRaises(ValueError):
            pyrove.inverse(pyrove.scaling_matrix(pyrove.vec3d(1, 0, 1)))
        m3 = pyrove.linear_part(m)
        self.assertIsInstance(m3, pyrove.mat3d)
        self.assertTrue(np.allclose(mat_np(m3), mat_np(m)[:3, :3]))
        self.assertTrue(np.allclose(mat_np(pyrove.inverse(m3)) @ mat_np(m3), np.eye(3), atol=1e-12))

    def test_rotation_matrix_is_right_handed_about_any_axis_length(self):
        m = pyrove.rotation_matrix(pyrove.vec3d(0, 0, 7), math.pi / 2)
        self.assertTrue(close(pyrove.transform_vector(m, pyrove.vec3d(1, 0, 0)), (0, 1, 0)))
        with self.assertRaises(ValueError):
            pyrove.rotation_matrix(pyrove.vec3d(0, 0, 0), 1.0)

    def test_normal_matrix_is_the_inverse_transpose_of_the_linear_part(self):
        m = pyrove.translation_matrix(pyrove.vec3d(5, 5, 5)) * pyrove.scaling_matrix(pyrove.vec3d(1, 4, 1))
        n = pyrove.normal_matrix(m)
        self.assertTrue(np.allclose(mat_np(n), np.linalg.inv(mat_np(m)[:3, :3]).T))
        # the normal of the plane y = x stays square to it once stretched
        tangent = np.array([1.0, 1.0, 0.0])
        normal = np.array([1.0, -1.0, 0.0])
        stretched_tangent = mat_np(m)[:3, :3] @ tangent
        self.assertAlmostEqual(float((mat_np(n) @ normal) @ stretched_tangent), 0.0, places=12)
        with self.assertRaises(ValueError):
            pyrove.normal_matrix(pyrove.scaling_matrix(pyrove.vec3d(0, 1, 1)))

    def test_look_at_and_ortho_are_the_setters_returned(self):
        eye, at, up = pyrove.vec3(1, 2, 3), pyrove.vec3(0, 0, 0), pyrove.vec3(0, 1, 0)
        set_ = pyrove.mat4()
        set_.lookat(eye, at, up)
        self.assertTrue(np.array_equal(mat_np(pyrove.look_at_matrix(eye, at, up)), mat_np(set_)))
        o = pyrove.ortho_matrix(-1.0, 3.0, -2.0, 2.0, 1.0, 11.0)
        self.assertTrue(close(pyrove.transform_point(o, pyrove.vec3(-1, -2, 1)), (-1, -1, 0), 1e-6))
        self.assertTrue(close(pyrove.transform_point(o, pyrove.vec3(3, 2, 11)), (1, 1, 1), 1e-6))
        with self.assertRaises(ValueError):
            pyrove.ortho_matrix(1.0, 1.0, -1.0, 1.0, 0.0, 1.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
