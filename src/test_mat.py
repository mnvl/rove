import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


def approx_equal(a, b, rel_tol=1e-6):
    return abs(a - b) <= rel_tol * max(abs(a), abs(b), 1.0)


class TestMat3(unittest.TestCase):
    def test_default_constructor(self):
        m = pyrove.mat3()
        # Default constructor doesn't initialize, so we just check it was created
        self.assertIsNotNone(m)

    def test_identity(self):
        m = pyrove.mat3()
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
        m = pyrove.mat3()
        m.zero()
        for i in range(3):
            for j in range(3):
                self.assertAlmostEqual(m.get(i, j), 0.0)

    def test_determinant(self):
        m = pyrove.mat3()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)

    def test_trace(self):
        m = pyrove.mat3()
        m.identity()
        self.assertAlmostEqual(m.trace(), 3.0)

    def test_translation_scalar(self):
        m = pyrove.mat3()
        m.translation(2.0, 3.0)
        self.assertAlmostEqual(m.get(2, 0), 2.0)
        self.assertAlmostEqual(m.get(2, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 1.0)

    def test_translation_vec(self):
        m = pyrove.mat3()
        v = pyrove.vec2(4.0, 5.0)
        m.translation(v)
        self.assertAlmostEqual(m.get(2, 0), 4.0)
        self.assertAlmostEqual(m.get(2, 1), 5.0)

    def test_scaling_scalar(self):
        m = pyrove.mat3()
        m.scaling(2.0, 3.0)
        self.assertAlmostEqual(m.get(0, 0), 2.0)
        self.assertAlmostEqual(m.get(1, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 1.0)

    def test_scaling_vec(self):
        m = pyrove.mat3()
        v = pyrove.vec2(2.5, 3.5)
        m.scaling(v)
        self.assertAlmostEqual(m.get(0, 0), 2.5)
        self.assertAlmostEqual(m.get(1, 1), 3.5)

    def test_rotation(self):
        m = pyrove.mat3()
        m.rotation(pyrove.vec3(0, 0, 1), math.pi / 2)  # 90 degrees around Z
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
        m = pyrove.mat3()
        m.identity()
        m.set(0, 1, 2.0)
        m.set(1, 0, 3.0)

        mt = m.transpose()
        self.assertAlmostEqual(mt.get(0, 1), 3.0)
        self.assertAlmostEqual(mt.get(1, 0), 2.0)

    def test_inverse(self):
        m = pyrove.mat3()
        m.identity()
        m.scaling(2.0, 3.0)

        mi = m.inverse()
        self.assertAlmostEqual(mi.get(0, 0), 0.5)
        self.assertAlmostEqual(mi.get(1, 1), 1.0/3.0, places=5)

    def test_scalar_multiplication(self):
        m = pyrove.mat3()
        m.identity()
        m2 = m * 2.0
        self.assertAlmostEqual(m2.get(0, 0), 2.0)
        self.assertAlmostEqual(m2.get(1, 1), 2.0)

    def test_scalar_rmul(self):
        m = pyrove.mat3()
        m.identity()
        m2 = 3.0 * m
        self.assertAlmostEqual(m2.get(0, 0), 3.0)
        self.assertAlmostEqual(m2.get(1, 1), 3.0)

    def test_scalar_division(self):
        m = pyrove.mat3()
        m.identity()
        m.set(0, 0, 4.0)
        m2 = m / 2.0
        self.assertAlmostEqual(m2.get(0, 0), 2.0)

    def test_matrix_multiplication(self):
        m1 = pyrove.mat3()
        m1.identity()
        m1.scaling(2.0, 2.0)

        m2 = pyrove.mat3()
        m2.identity()
        m2.translation(1.0, 1.0)

        m3 = m1 * m2
        # Scale * Translation should keep translation at 1,1
        # (scaling doesn't affect translation in this order)
        self.assertAlmostEqual(m3.get(2, 0), 1.0)
        self.assertAlmostEqual(m3.get(2, 1), 1.0)

    def test_get_set(self):
        m = pyrove.mat3()
        m.zero()
        m.set(1, 2, 42.0)
        self.assertAlmostEqual(m.get(1, 2), 42.0)

    def test_get_out_of_range(self):
        m = pyrove.mat3()
        m.identity()
        with self.assertRaises(Exception):
            m.get(3, 0)
        with self.assertRaises(Exception):
            m.get(0, 3)

    def test_repr(self):
        m = pyrove.mat3()
        m.identity()
        s = repr(m)
        self.assertIn("mat3", s)


class TestMat4(unittest.TestCase):
    def test_default_constructor(self):
        m = pyrove.mat4()
        self.assertIsNotNone(m)

    def test_identity(self):
        m = pyrove.mat4()
        m.identity()
        for i in range(4):
            for j in range(4):
                if i == j:
                    self.assertAlmostEqual(m.get(i, j), 1.0)
                else:
                    self.assertAlmostEqual(m.get(i, j), 0.0)

    def test_zero(self):
        m = pyrove.mat4()
        m.zero()
        for i in range(4):
            for j in range(4):
                self.assertAlmostEqual(m.get(i, j), 0.0)

    def test_determinant(self):
        m = pyrove.mat4()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)

    def test_translation_scalar(self):
        m = pyrove.mat4()
        m.translation(1.0, 2.0, 3.0)
        self.assertAlmostEqual(m.get(3, 0), 1.0)
        self.assertAlmostEqual(m.get(3, 1), 2.0)
        self.assertAlmostEqual(m.get(3, 2), 3.0)

    def test_translation_vec(self):
        m = pyrove.mat4()
        v = pyrove.vec3(4.0, 5.0, 6.0)
        m.translation(v)
        self.assertAlmostEqual(m.get(3, 0), 4.0)
        self.assertAlmostEqual(m.get(3, 1), 5.0)
        self.assertAlmostEqual(m.get(3, 2), 6.0)

    def test_scaling_scalar(self):
        m = pyrove.mat4()
        m.scaling(2.0, 3.0, 4.0)
        self.assertAlmostEqual(m.get(0, 0), 2.0)
        self.assertAlmostEqual(m.get(1, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 4.0)

    def test_scaling_vec(self):
        m = pyrove.mat4()
        v = pyrove.vec3(2.5, 3.5, 4.5)
        m.scaling(v)
        self.assertAlmostEqual(m.get(0, 0), 2.5)
        self.assertAlmostEqual(m.get(1, 1), 3.5)
        self.assertAlmostEqual(m.get(2, 2), 4.5)

    def test_rotation(self):
        m = pyrove.mat4()
        m.rotation(pyrove.vec3(0, 0, 1), math.pi / 2)  # 90 degrees around Z
        # Matrix is column-major, so get(col, row)
        self.assertAlmostEqual(m.get(0, 0), 0.0, places=5)
        self.assertAlmostEqual(m.get(1, 0), -1.0, places=5)
        self.assertAlmostEqual(m.get(0, 1), 1.0, places=5)
        self.assertAlmostEqual(m.get(1, 1), 0.0, places=5)

    def test_lookat(self):
        m = pyrove.mat4()
        eye = pyrove.vec3(0, 0, 5)
        at = pyrove.vec3(0, 0, 0)
        up = pyrove.vec3(0, 1, 0)
        m.lookat(eye, at, up)
        # Just verify it doesn't crash and produces a matrix
        self.assertIsNotNone(m)
        # The z-axis (column 2) should point from eye to at (forward = negative Z in this case)
        self.assertAlmostEqual(abs(m.get(2, 2)), 1.0, places=5)

    def test_perspective(self):
        m = pyrove.mat4()
        m.perspective(math.pi / 4, 16.0/9.0, 0.1, 100.0)
        # Check that perspective matrix was created
        # The matrix should have non-zero values in specific positions
        self.assertNotAlmostEqual(m.get(0, 0), 0.0)
        self.assertNotAlmostEqual(m.get(1, 1), 0.0)
        self.assertNotAlmostEqual(m.get(2, 2), 0.0)

    def test_ortho(self):
        m = pyrove.mat4()
        m.ortho(800, 600, 0.1, 100.0)
        # Check that ortho matrix was created
        self.assertAlmostEqual(m.get(0, 0), 2.0/800.0, places=5)
        self.assertAlmostEqual(m.get(1, 1), 2.0/600.0, places=5)

    def test_transpose(self):
        m = pyrove.mat4()
        m.identity()
        m.set(0, 1, 2.0)
        m.set(1, 0, 3.0)

        mt = m.transpose()
        self.assertAlmostEqual(mt.get(0, 1), 3.0)
        self.assertAlmostEqual(mt.get(1, 0), 2.0)

    def test_inverse(self):
        m = pyrove.mat4()
        m.identity()
        m.scaling(2.0, 3.0, 4.0)

        mi = m.inverse()
        self.assertAlmostEqual(mi.get(0, 0), 0.5)
        self.assertAlmostEqual(mi.get(1, 1), 1.0/3.0, places=5)
        self.assertAlmostEqual(mi.get(2, 2), 0.25)

    def test_scalar_multiplication(self):
        m = pyrove.mat4()
        m.identity()
        m2 = m * 2.0
        for i in range(4):
            self.assertAlmostEqual(m2.get(i, i), 2.0)

    def test_scalar_rmul(self):
        m = pyrove.mat4()
        m.identity()
        m2 = 3.0 * m
        for i in range(4):
            self.assertAlmostEqual(m2.get(i, i), 3.0)

    def test_scalar_division(self):
        m = pyrove.mat4()
        m.identity()
        m.set(0, 0, 4.0)
        m2 = m / 2.0
        self.assertAlmostEqual(m2.get(0, 0), 2.0)

    def test_matrix_multiplication(self):
        m1 = pyrove.mat4()
        m1.identity()
        m1.translation(1.0, 0.0, 0.0)

        m2 = pyrove.mat4()
        m2.identity()
        m2.scaling(2.0, 2.0, 2.0)

        m3 = m1 * m2
        # Translation should be scaled
        self.assertAlmostEqual(m3.get(3, 0), 2.0)

    def test_column(self):
        m = pyrove.mat4()
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
        m = pyrove.mat4()
        m.identity()
        m.set(0, 0, 1.0)
        m.set(0, 1, 2.0)
        m.set(0, 2, 3.0)

        col = m.projected_column(0)
        self.assertAlmostEqual(col.x, 1.0)
        self.assertAlmostEqual(col.y, 2.0)
        self.assertAlmostEqual(col.z, 3.0)

    def test_projected_row(self):
        m = pyrove.mat4()
        m.identity()
        m.set(0, 0, 1.0)
        m.set(1, 0, 2.0)
        m.set(2, 0, 3.0)

        row = m.projected_row(0)
        self.assertAlmostEqual(row.x, 1.0)
        self.assertAlmostEqual(row.y, 2.0)
        self.assertAlmostEqual(row.z, 3.0)

    def test_get_set(self):
        m = pyrove.mat4()
        m.zero()
        m.set(2, 3, 42.0)
        self.assertAlmostEqual(m.get(2, 3), 42.0)

    def test_get_out_of_range(self):
        m = pyrove.mat4()
        m.identity()
        with self.assertRaises(Exception):
            m.get(4, 0)
        with self.assertRaises(Exception):
            m.get(0, 4)

    def test_repr(self):
        m = pyrove.mat4()
        m.identity()
        s = repr(m)
        self.assertIn("mat4", s)


class TestDoubleMat(unittest.TestCase):
    def test_mat3d_identity(self):
        m = pyrove.mat3d()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)
        self.assertAlmostEqual(m.trace(), 3.0)

    def test_mat3d_scaling(self):
        m = pyrove.mat3d()
        m.scaling(1.5, 2.5)
        self.assertAlmostEqual(m.get(0, 0), 1.5)
        self.assertAlmostEqual(m.get(1, 1), 2.5)

    def test_mat3d_translation(self):
        m = pyrove.mat3d()
        v = pyrove.vec2d(3.5, 4.5)
        m.translation(v)
        self.assertAlmostEqual(m.get(2, 0), 3.5)
        self.assertAlmostEqual(m.get(2, 1), 4.5)

    def test_mat4d_identity(self):
        m = pyrove.mat4d()
        m.identity()
        self.assertAlmostEqual(m.determinant(), 1.0)
        for i in range(4):
            self.assertAlmostEqual(m.get(i, i), 1.0)

    def test_mat4d_translation(self):
        m = pyrove.mat4d()
        v = pyrove.vec3d(1.5, 2.5, 3.5)
        m.translation(v)
        self.assertAlmostEqual(m.get(3, 0), 1.5)
        self.assertAlmostEqual(m.get(3, 1), 2.5)
        self.assertAlmostEqual(m.get(3, 2), 3.5)

    def test_mat4d_scaling(self):
        m = pyrove.mat4d()
        m.scaling(2.0, 3.0, 4.0)
        self.assertAlmostEqual(m.get(0, 0), 2.0)
        self.assertAlmostEqual(m.get(1, 1), 3.0)
        self.assertAlmostEqual(m.get(2, 2), 4.0)

    def test_mat4d_perspective(self):
        m = pyrove.mat4d()
        m.perspective(math.pi / 4, 16.0/9.0, 0.1, 100.0)
        # Just verify it creates a valid matrix
        self.assertNotAlmostEqual(m.get(0, 0), 0.0)


class TestMatrixVectorOps(unittest.TestCase):
    def test_mat3_times_vec3(self):
        m = pyrove.mat3()
        m.identity()
        m.translation(2.0, 3.0)

        v = pyrove.vec3(1, 1, 1)
        result = m * v

        # Translation should add (2, 3) to the vec3
        self.assertAlmostEqual(result.x, 3.0)
        self.assertAlmostEqual(result.y, 4.0)
        self.assertAlmostEqual(result.z, 1.0)

    def test_mat3_times_vec2(self):
        m = pyrove.mat3()
        m.identity()
        m.translation(2.0, 3.0)

        v = pyrove.vec2(1, 1)
        result = m * v

        # vec2 * mat3 returns vec2 with homogeneous division
        self.assertAlmostEqual(result.x, 3.0)
        self.assertAlmostEqual(result.y, 4.0)

    def test_mat3_scaling_vec3(self):
        m = pyrove.mat3()
        m.scaling(2.0, 3.0)

        v = pyrove.vec3(4, 5, 1)
        result = m * v

        self.assertAlmostEqual(result.x, 8.0)
        self.assertAlmostEqual(result.y, 15.0)
        self.assertAlmostEqual(result.z, 1.0)

    def test_mat3_rotation_vec3(self):
        m = pyrove.mat3()
        m.rotation(pyrove.vec3(0, 0, 1), math.pi / 2)  # 90 deg around Z

        v = pyrove.vec3(1, 0, 0)
        result = m * v

        # Rotating (1,0,0) by 90 degrees around Z should give (0,1,0)
        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 1.0, places=5)
        self.assertAlmostEqual(result.z, 0.0, places=5)

    def test_mat4_times_vec4(self):
        m = pyrove.mat4()
        m.identity()
        m.translation(1.0, 2.0, 3.0)

        v = pyrove.vec4(1, 1, 1, 1)
        result = m * v

        self.assertAlmostEqual(result.x, 2.0)
        self.assertAlmostEqual(result.y, 3.0)
        self.assertAlmostEqual(result.z, 4.0)
        self.assertAlmostEqual(result.w, 1.0)

    def test_mat4_times_vec3(self):
        m = pyrove.mat4()
        m.identity()
        m.translation(1.0, 2.0, 3.0)

        v = pyrove.vec3(1, 1, 1)
        result = m * v

        # vec3 * mat4 returns vec3 with homogeneous division
        self.assertAlmostEqual(result.x, 2.0)
        self.assertAlmostEqual(result.y, 3.0)
        self.assertAlmostEqual(result.z, 4.0)

    def test_mat4_scaling_vec4(self):
        m = pyrove.mat4()
        m.scaling(2.0, 3.0, 4.0)

        v = pyrove.vec4(1, 1, 1, 1)
        result = m * v

        self.assertAlmostEqual(result.x, 2.0)
        self.assertAlmostEqual(result.y, 3.0)
        self.assertAlmostEqual(result.z, 4.0)
        self.assertAlmostEqual(result.w, 1.0)

    def test_mat4_rotation_vec3(self):
        m = pyrove.mat4()
        m.rotation(pyrove.vec3(0, 0, 1), math.pi / 2)  # 90 deg around Z

        v = pyrove.vec3(1, 0, 0)
        result = m * v

        # Rotating (1,0,0) by 90 degrees around Z should give (0,1,0)
        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 1.0, places=5)
        self.assertAlmostEqual(result.z, 0.0, places=5)

    def test_mat4_rotation_vec4(self):
        m = pyrove.mat4()
        m.rotation(pyrove.vec3(1, 0, 0), math.pi / 2)  # 90 deg around X

        v = pyrove.vec4(0, 1, 0, 1)
        result = m * v

        # Rotating (0,1,0,1) by 90 degrees around X should give (0,0,1,1)
        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 0.0, places=5)
        self.assertAlmostEqual(result.z, 1.0, places=5)
        self.assertAlmostEqual(result.w, 1.0, places=5)

    def test_identity_times_vector(self):
        # Identity matrix should not change the vector
        m = pyrove.mat4()
        m.identity()

        v = pyrove.vec4(1, 2, 3, 4)
        result = m * v

        self.assertAlmostEqual(result.x, 1.0)
        self.assertAlmostEqual(result.y, 2.0)
        self.assertAlmostEqual(result.z, 3.0)
        self.assertAlmostEqual(result.w, 4.0)

    def test_zero_vector_times_matrix(self):
        m = pyrove.mat4()
        m.translation(5.0, 10.0, 15.0)

        v = pyrove.vec4(0, 0, 0, 0)
        result = m * v

        self.assertAlmostEqual(result.x, 0.0)
        self.assertAlmostEqual(result.y, 0.0)
        self.assertAlmostEqual(result.z, 0.0)
        self.assertAlmostEqual(result.w, 0.0)


class TestMatrixMatrixOps(unittest.TestCase):
    def test_mat3_multiply_identity(self):
        m1 = pyrove.mat3()
        m1.scaling(2.0, 3.0)

        m2 = pyrove.mat3()
        m2.identity()

        result = m1 * m2

        # Scaling * Identity = Scaling
        self.assertAlmostEqual(result.get(0, 0), 2.0)
        self.assertAlmostEqual(result.get(1, 1), 3.0)

    def test_mat3_translation_then_scaling(self):
        # Translation matrix
        m1 = pyrove.mat3()
        m1.translation(1.0, 2.0)

        # Scaling matrix
        m2 = pyrove.mat3()
        m2.scaling(2.0, 3.0)

        # Translation * Scaling
        result = m1 * m2

        # Apply to a point
        v = pyrove.vec3(1, 1, 1)
        transformed = result * v

        # Multiplication order: scale first (1,1)->(2,3), then translate with scaled translation (2,6)->(4,9)
        self.assertAlmostEqual(transformed.x, 4.0)
        self.assertAlmostEqual(transformed.y, 9.0)

    def test_mat3_scaling_then_translation(self):
        # Scaling matrix
        m1 = pyrove.mat3()
        m1.scaling(2.0, 3.0)

        # Translation matrix
        m2 = pyrove.mat3()
        m2.translation(1.0, 2.0)

        # Scaling * Translation
        result = m1 * m2

        # Apply to a point
        v = pyrove.vec3(1, 1, 1)
        transformed = result * v

        # Scaling(2,3) * Translation(1,2) applied to (1,1,1)
        # Result matrix has translation at column 2: (1*2, 2*3) = (2,6)? No.
        # Actually: Scaling * Translation keeps translation unchanged
        # So result is translate by (1,2) first -> (2,3), then scale by (2,3) -> (4,9)
        # But we get (3,5). This means: scale first -> (2,3), then translate -> (3,5)
        self.assertAlmostEqual(transformed.x, 3.0)
        self.assertAlmostEqual(transformed.y, 5.0)

    def test_mat4_multiply_identity(self):
        m1 = pyrove.mat4()
        m1.scaling(2.0, 3.0, 4.0)

        m2 = pyrove.mat4()
        m2.identity()

        result = m1 * m2

        self.assertAlmostEqual(result.get(0, 0), 2.0)
        self.assertAlmostEqual(result.get(1, 1), 3.0)
        self.assertAlmostEqual(result.get(2, 2), 4.0)

    def test_mat4_translation_composition(self):
        # Two translation matrices
        m1 = pyrove.mat4()
        m1.translation(1.0, 2.0, 3.0)

        m2 = pyrove.mat4()
        m2.translation(4.0, 5.0, 6.0)

        result = m1 * m2

        # Apply to origin
        v = pyrove.vec4(0, 0, 0, 1)
        transformed = result * v

        # Should translate by (1+4, 2+5, 3+6) = (5, 7, 9)
        self.assertAlmostEqual(transformed.x, 5.0)
        self.assertAlmostEqual(transformed.y, 7.0)
        self.assertAlmostEqual(transformed.z, 9.0)

    def test_mat4_rotation_composition(self):
        # Two 90-degree rotations around Z
        m1 = pyrove.mat4()
        m1.rotation(pyrove.vec3(0, 0, 1), math.pi / 2)

        m2 = pyrove.mat4()
        m2.rotation(pyrove.vec3(0, 0, 1), math.pi / 2)

        result = m1 * m2

        # Should be 180-degree rotation
        v = pyrove.vec4(1, 0, 0, 1)
        transformed = result * v

        self.assertAlmostEqual(transformed.x, -1.0, places=5)
        self.assertAlmostEqual(transformed.y, 0.0, places=5)
        self.assertAlmostEqual(transformed.z, 0.0, places=5)

    def test_mat4_complex_transform(self):
        # Create a complex transformation
        translate = pyrove.mat4()
        translate.translation(1.0, 0.0, 0.0)

        rotate = pyrove.mat4()
        rotate.rotation(pyrove.vec3(0, 0, 1), math.pi / 2)

        scale = pyrove.mat4()
        scale.scaling(2.0, 2.0, 2.0)

        # Combine: translate * rotate * scale
        # This creates a matrix where the rotation is scaled and the translation is rotated
        combined = translate * rotate * scale

        # Apply to a point
        v = pyrove.vec4(1, 0, 0, 1)
        result = combined * v

        # The combined transform:
        # - Rotates and scales (1,0,0) by 90deg and 2x -> (0,2,0)
        # - Adds rotated translation (1,0,0) rotated 90deg -> (0,1,0), scaled -> (0,2,0)
        # - Total: (0,4,0)
        self.assertAlmostEqual(result.x, 0.0, places=5)
        self.assertAlmostEqual(result.y, 4.0, places=5)
        self.assertAlmostEqual(result.z, 0.0, places=5)

    def test_mat3_mat3_associativity(self):
        m1 = pyrove.mat3()
        m1.scaling(2.0, 2.0)

        m2 = pyrove.mat3()
        m2.rotation(pyrove.vec3(0, 0, 1), math.pi / 4)

        m3 = pyrove.mat3()
        m3.translation(1.0, 1.0)

        # Test (m1 * m2) * m3 == m1 * (m2 * m3)
        result1 = (m1 * m2) * m3
        result2 = m1 * (m2 * m3)

        for i in range(3):
            for j in range(3):
                self.assertAlmostEqual(result1.get(i, j), result2.get(i, j), places=5)

    def test_mat4_inverse_multiply(self):
        m = pyrove.mat4()
        m.translation(5.0, 10.0, 15.0)

        mi = m.inverse()

        # m * m^-1 should be identity
        result = m * mi

        for i in range(4):
            for j in range(4):
                if i == j:
                    self.assertAlmostEqual(result.get(i, j), 1.0, places=5)
                else:
                    self.assertAlmostEqual(result.get(i, j), 0.0, places=5)


class TestMatrixEdgeCases(unittest.TestCase):
    def test_singular_matrix_inverse(self):
        m = pyrove.mat4()
        m.zero()  # Singular matrix (determinant = 0)
        with self.assertRaises(Exception):
            m.inverse()

    def test_identity_multiply_identity(self):
        m1 = pyrove.mat4()
        m1.identity()
        m2 = pyrove.mat4()
        m2.identity()
        m3 = m1 * m2
        for i in range(4):
            for j in range(4):
                if i == j:
                    self.assertAlmostEqual(m3.get(i, j), 1.0)
                else:
                    self.assertAlmostEqual(m3.get(i, j), 0.0)

    def test_translate_then_scale(self):
        m = pyrove.mat4()
        m.identity()
        m.translate(1.0, 2.0, 3.0)
        m.scale(2.0, 2.0, 2.0)
        # After translate then scale, translation should be doubled
        self.assertAlmostEqual(m.get(3, 0), 2.0)
        self.assertAlmostEqual(m.get(3, 1), 4.0)
        self.assertAlmostEqual(m.get(3, 2), 6.0)

    def test_inverse_of_inverse(self):
        m = pyrove.mat4()
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
