import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


def approx_equal(a, b, rel_tol=1e-6):
    return abs(a - b) <= rel_tol * max(abs(a), abs(b), 1.0)


class TestQuaternion(unittest.TestCase):
    def test_default_constructor(self):
        q = pyrove.quat()
        # Default constructor doesn't initialize
        self.assertIsNotNone(q)

    def test_value_constructor(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_identity(self):
        q = pyrove.quat()
        q.identity()
        self.assertEqual(q.x, 0.0)
        self.assertEqual(q.y, 0.0)
        self.assertEqual(q.z, 0.0)
        self.assertEqual(q.w, 1.0)

    def test_norm(self):
        q = pyrove.quat(3.0, 4.0, 0.0, 0.0)
        self.assertAlmostEqual(q.norm(), 5.0)

    def test_normalize(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        original_norm = q.norm()
        q.normalize()
        self.assertAlmostEqual(q.norm(), 1.0)
        # Check that direction is preserved
        self.assertAlmostEqual(q.x * original_norm, 1.0, places=5)
        self.assertAlmostEqual(q.y * original_norm, 2.0, places=5)

    def test_normalized(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        qn = q.normalized()
        self.assertAlmostEqual(qn.norm(), 1.0)
        # Original should be unchanged
        self.assertNotAlmostEqual(q.norm(), 1.0)

    def test_scale(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        q.scale(2.0)
        self.assertEqual(q.x, 2.0)
        self.assertEqual(q.y, 4.0)
        self.assertEqual(q.z, 6.0)
        self.assertEqual(q.w, 8.0)

    def test_addition(self):
        q1 = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        q2 = pyrove.quat(5.0, 6.0, 7.0, 8.0)
        q3 = q1 + q2
        self.assertEqual(q3.x, 6.0)
        self.assertEqual(q3.y, 8.0)
        self.assertEqual(q3.z, 10.0)
        self.assertEqual(q3.w, 12.0)

    def test_subtraction(self):
        q1 = pyrove.quat(5.0, 6.0, 7.0, 8.0)
        q2 = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        q3 = q1 - q2
        self.assertEqual(q3.x, 4.0)
        self.assertEqual(q3.y, 4.0)
        self.assertEqual(q3.z, 4.0)
        self.assertEqual(q3.w, 4.0)

    def test_scalar_multiplication(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        q2 = q * 3.0
        self.assertEqual(q2.x, 3.0)
        self.assertEqual(q2.y, 6.0)
        self.assertEqual(q2.z, 9.0)
        self.assertEqual(q2.w, 12.0)

    def test_scalar_rmul(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        q2 = 3.0 * q
        self.assertEqual(q2.x, 3.0)
        self.assertEqual(q2.y, 6.0)

    def test_negation(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        qn = -q
        self.assertEqual(qn.x, -1.0)
        self.assertEqual(qn.y, -2.0)
        self.assertEqual(qn.z, -3.0)
        self.assertEqual(qn.w, -4.0)

    def test_dot_product(self):
        q1 = pyrove.quat(1.0, 0.0, 0.0, 0.0)
        q2 = pyrove.quat(0.0, 1.0, 0.0, 0.0)
        self.assertAlmostEqual(q1.dot(q2), 0.0)

        q3 = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        q4 = pyrove.quat(5.0, 6.0, 7.0, 8.0)
        expected = 1*5 + 2*6 + 3*7 + 4*8
        self.assertAlmostEqual(q3.dot(q4), expected)

    def test_dot_function(self):
        q1 = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        q2 = pyrove.quat(5.0, 6.0, 7.0, 8.0)
        expected = 1*5 + 2*6 + 3*7 + 4*8
        self.assertAlmostEqual(pyrove.dot(q1, q2), expected)

    def test_repr(self):
        q = pyrove.quat(1.0, 2.0, 3.0, 4.0)
        s = repr(q)
        self.assertIn("quat", s)
        self.assertIn("1", s)

    def test_property_access(self):
        q = pyrove.quat()
        q.x = 1.0
        q.y = 2.0
        q.z = 3.0
        q.w = 4.0
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)


class TestQuaternionSlerp(unittest.TestCase):
    def test_slerp_identity_to_90deg(self):
        q1 = pyrove.quat(0, 0, 0, 1)  # Identity
        q2 = pyrove.quat(0, 0, 0.7071068, 0.7071068)  # 90 deg around Z

        # At t=0, should be q1
        q_start = pyrove.slerp(q1, q2, 0.0)
        self.assertAlmostEqual(q_start.w, 1.0, places=5)

        # At t=1, should be q2
        q_end = pyrove.slerp(q1, q2, 1.0)
        self.assertAlmostEqual(q_end.z, 0.7071068, places=5)
        self.assertAlmostEqual(q_end.w, 0.7071068, places=5)

        # At t=0.5, should be halfway
        q_mid = pyrove.slerp(q1, q2, 0.5)
        self.assertAlmostEqual(q_mid.norm(), 1.0, places=5)

    def test_slerp_class(self):
        q1 = pyrove.quat(0, 0, 0, 1)
        q2 = pyrove.quat(0, 0, 1, 0)

        slerper = pyrove.quat_slerper()
        slerper.setup(q1, q2)

        q_mid = slerper.interpolate(0.5)
        self.assertAlmostEqual(q_mid.norm(), 1.0, places=5)

    def test_slerp_preserves_norm(self):
        q1 = pyrove.quat(1, 0, 0, 0)
        q1.normalize()
        q2 = pyrove.quat(0, 1, 0, 0)
        q2.normalize()

        for t in [0.0, 0.25, 0.5, 0.75, 1.0]:
            q = pyrove.slerp(q1, q2, t)
            self.assertAlmostEqual(q.norm(), 1.0, places=5)


class TestQuaternionMatrix(unittest.TestCase):
    def test_mat3_rotation_from_quat(self):
        # 90 degree rotation around Z axis
        q = pyrove.quat(0, 0, 0.7071068, 0.7071068)
        m = pyrove.mat3()
        m.rotation(q)

        v = pyrove.vec3(1, 0, 0)
        result = m * v

        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 1.0, places=5)
        self.assertAlmostEqual(result.z, 0.0, places=5)

    def test_mat4_rotation_from_quat(self):
        # 90 degree rotation around Z axis
        q = pyrove.quat(0, 0, 0.7071068, 0.7071068)
        m = pyrove.mat4()
        m.rotation(q)

        v = pyrove.vec4(1, 0, 0, 1)
        result = m * v

        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 1.0, places=5)
        self.assertAlmostEqual(result.z, 0.0, places=5)
        self.assertAlmostEqual(result.w, 1.0, places=5)

    def test_mat4_rotation_around_x(self):
        # 90 degree rotation around X axis
        q = pyrove.quat(0.7071068, 0, 0, 0.7071068)
        m = pyrove.mat4()
        m.rotation(q)

        v = pyrove.vec4(0, 1, 0, 1)
        result = m * v

        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 0.0, places=5)
        self.assertAlmostEqual(result.z, 1.0, places=5)

    def test_mat4_rotation_around_y(self):
        # 90 degree rotation around Y axis
        q = pyrove.quat(0, 0.7071068, 0, 0.7071068)
        m = pyrove.mat4()
        m.rotation(q)

        v = pyrove.vec4(1, 0, 0, 1)
        result = m * v

        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 0.0, places=5)
        self.assertAlmostEqual(result.z, -1.0, places=5)

    def test_mat4_rotate_quaternion(self):
        # Start with identity
        m = pyrove.mat4()
        m.identity()

        # Apply 90 degree rotation around Z
        q = pyrove.quat(0, 0, 0.7071068, 0.7071068)
        m.rotate(q)

        v = pyrove.vec4(1, 0, 0, 1)
        result = m * v

        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 1.0, places=5)

    def test_identity_quaternion_gives_identity_matrix(self):
        q = pyrove.quat(0, 0, 0, 1)
        m = pyrove.mat4()
        m.rotation(q)

        # Should be identity matrix
        for i in range(4):
            for j in range(4):
                if i == j:
                    self.assertAlmostEqual(m.get(i, j), 1.0, places=5)
                else:
                    self.assertAlmostEqual(m.get(i, j), 0.0, places=5)

    def test_quaternion_rotation_composition(self):
        # Two 45-degree rotations around Z should equal 90 degrees
        q45 = pyrove.quat(0, 0, 0.3826834, 0.9238795)  # 45 deg

        m1 = pyrove.mat4()
        m1.rotation(q45)

        m2 = pyrove.mat4()
        m2.rotation(q45)

        combined = m1 * m2

        v = pyrove.vec4(1, 0, 0, 1)
        result = combined * v

        # Should be approximately 90 degree rotation
        self.assertAlmostEqual(result.x, 0.0, places=4)
        self.assertAlmostEqual(result.y, 1.0, places=4)


class TestDoubleQuaternion(unittest.TestCase):
    def test_quatd_basic(self):
        q = pyrove.quatd(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_quatd_norm(self):
        q = pyrove.quatd(3.0, 4.0, 0.0, 0.0)
        self.assertAlmostEqual(q.norm(), 5.0)

    def test_quatd_slerp(self):
        slerper = pyrove.quat_slerperd()
        q1 = pyrove.quatd(0, 0, 0, 1)
        q2 = pyrove.quatd(0, 0, 1, 0)
        slerper.setup(q1, q2)
        q_mid = slerper.interpolate(0.5)
        self.assertAlmostEqual(q_mid.norm(), 1.0, places=5)


if __name__ == "__main__":
    unittest.main(verbosity=2)
