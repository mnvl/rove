import unittest
import sys
import os
import numpy as np

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


def approx_equal(a, b, rel_tol=1e-6):
    return abs(a - b) <= rel_tol * max(abs(a), abs(b), 1.0)


class TestNumpyConversions(unittest.TestCase):
    def test_vec2_from_numpy(self):
        arr = np.array([1.0, 2.0], dtype=np.float32)
        v = pyrove.vec2.from_numpy(arr)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)

    def test_vec2_to_numpy(self):
        v = pyrove.vec2(3.0, 4.0)
        arr = v.to_numpy()
        self.assertEqual(arr.shape, (2,))
        self.assertEqual(arr[0], 3.0)
        self.assertEqual(arr[1], 4.0)

    def test_vec2_roundtrip(self):
        v1 = pyrove.vec2(5.0, 6.0)
        arr = v1.to_numpy()
        v2 = pyrove.vec2.from_numpy(arr)
        self.assertEqual(v1.x, v2.x)
        self.assertEqual(v1.y, v2.y)

    def test_vec3_from_numpy(self):
        arr = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        v = pyrove.vec3.from_numpy(arr)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_vec3_to_numpy(self):
        v = pyrove.vec3(4.0, 5.0, 6.0)
        arr = v.to_numpy()
        self.assertEqual(arr.shape, (3,))
        self.assertEqual(arr[0], 4.0)
        self.assertEqual(arr[1], 5.0)
        self.assertEqual(arr[2], 6.0)

    def test_vec3_roundtrip(self):
        v1 = pyrove.vec3(7.0, 8.0, 9.0)
        arr = v1.to_numpy()
        v2 = pyrove.vec3.from_numpy(arr)
        self.assertEqual(v1.x, v2.x)
        self.assertEqual(v1.y, v2.y)
        self.assertEqual(v1.z, v2.z)

    def test_vec4_from_numpy(self):
        arr = np.array([1.0, 2.0, 3.0, 4.0], dtype=np.float32)
        v = pyrove.vec4.from_numpy(arr)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)
        self.assertEqual(v.w, 4.0)

    def test_vec4_to_numpy(self):
        v = pyrove.vec4(5.0, 6.0, 7.0, 8.0)
        arr = v.to_numpy()
        self.assertEqual(arr.shape, (4,))
        self.assertEqual(arr[0], 5.0)
        self.assertEqual(arr[1], 6.0)
        self.assertEqual(arr[2], 7.0)
        self.assertEqual(arr[3], 8.0)

    def test_vec4_roundtrip(self):
        v1 = pyrove.vec4(9.0, 10.0, 11.0, 12.0)
        arr = v1.to_numpy()
        v2 = pyrove.vec4.from_numpy(arr)
        self.assertEqual(v1.x, v2.x)
        self.assertEqual(v1.y, v2.y)
        self.assertEqual(v1.z, v2.z)
        self.assertEqual(v1.w, v2.w)

    def test_mat3_from_numpy(self):
        arr = np.array([[1, 2, 3],
                        [4, 5, 6],
                        [7, 8, 9]], dtype=np.float32)
        m = pyrove.mat3.from_numpy(arr)
        # Matrix is column-major, so m.get(col, row)
        for row in range(3):
            for col in range(3):
                self.assertEqual(m.get(col, row), arr[row, col])

    def test_mat3_to_numpy(self):
        m = pyrove.mat3()
        m.identity()
        arr = m.to_numpy()
        self.assertEqual(arr.shape, (3, 3))
        # Identity matrix
        for i in range(3):
            for j in range(3):
                if i == j:
                    self.assertEqual(arr[i, j], 1.0)
                else:
                    self.assertEqual(arr[i, j], 0.0)

    def test_mat3_roundtrip(self):
        arr1 = np.array([[1, 2, 3],
                         [4, 5, 6],
                         [7, 8, 9]], dtype=np.float32)
        m = pyrove.mat3.from_numpy(arr1)
        arr2 = m.to_numpy()
        np.testing.assert_array_almost_equal(arr1, arr2)

    def test_mat4_from_numpy(self):
        arr = np.array([[1, 2, 3, 4],
                        [5, 6, 7, 8],
                        [9, 10, 11, 12],
                        [13, 14, 15, 16]], dtype=np.float32)
        m = pyrove.mat4.from_numpy(arr)
        # Matrix is column-major, so m.get(col, row)
        for row in range(4):
            for col in range(4):
                self.assertEqual(m.get(col, row), arr[row, col])

    def test_mat4_to_numpy(self):
        m = pyrove.mat4()
        m.identity()
        arr = m.to_numpy()
        self.assertEqual(arr.shape, (4, 4))
        # Identity matrix
        for i in range(4):
            for j in range(4):
                if i == j:
                    self.assertEqual(arr[i, j], 1.0)
                else:
                    self.assertEqual(arr[i, j], 0.0)

    def test_mat4_roundtrip(self):
        arr1 = np.array([[1, 2, 3, 4],
                         [5, 6, 7, 8],
                         [9, 10, 11, 12],
                         [13, 14, 15, 16]], dtype=np.float32)
        m = pyrove.mat4.from_numpy(arr1)
        arr2 = m.to_numpy()
        np.testing.assert_array_almost_equal(arr1, arr2)

    def test_mat4_translation_numpy(self):
        m = pyrove.mat4()
        m.translation(1.0, 2.0, 3.0)
        arr = m.to_numpy()
        # In a translation matrix, the translation vector is in the last column
        # But numpy array is row-major, so it's in the last row
        self.assertEqual(arr[0, 3], 1.0)
        self.assertEqual(arr[1, 3], 2.0)
        self.assertEqual(arr[2, 3], 3.0)
        self.assertEqual(arr[3, 3], 1.0)

    def test_quat_from_numpy(self):
        arr = np.array([1.0, 2.0, 3.0, 4.0], dtype=np.float32)
        q = pyrove.quat.from_numpy(arr)
        self.assertEqual(q.x, 1.0)
        self.assertEqual(q.y, 2.0)
        self.assertEqual(q.z, 3.0)
        self.assertEqual(q.w, 4.0)

    def test_quat_to_numpy(self):
        q = pyrove.quat(5.0, 6.0, 7.0, 8.0)
        arr = q.to_numpy()
        self.assertEqual(arr.shape, (4,))
        self.assertEqual(arr[0], 5.0)
        self.assertEqual(arr[1], 6.0)
        self.assertEqual(arr[2], 7.0)
        self.assertEqual(arr[3], 8.0)

    def test_quat_roundtrip(self):
        q1 = pyrove.quat(0.0, 0.0, 0.0, 1.0)
        q1.normalize()
        arr = q1.to_numpy()
        q2 = pyrove.quat.from_numpy(arr)
        self.assertAlmostEqual(q1.x, q2.x, places=6)
        self.assertAlmostEqual(q1.y, q2.y, places=6)
        self.assertAlmostEqual(q1.z, q2.z, places=6)
        self.assertAlmostEqual(q1.w, q2.w, places=6)

    def test_vec2d_from_numpy(self):
        arr = np.array([1.0, 2.0], dtype=np.float64)
        v = pyrove.vec2d.from_numpy(arr)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)

    def test_vec3d_from_numpy(self):
        arr = np.array([1.0, 2.0, 3.0], dtype=np.float64)
        v = pyrove.vec3d.from_numpy(arr)
        self.assertEqual(v.x, 1.0)
        self.assertEqual(v.y, 2.0)
        self.assertEqual(v.z, 3.0)

    def test_mat3d_from_numpy(self):
        arr = np.eye(3, dtype=np.float64)
        m = pyrove.mat3d.from_numpy(arr)
        arr2 = m.to_numpy()
        np.testing.assert_array_almost_equal(arr, arr2)

    def test_mat4d_from_numpy(self):
        arr = np.eye(4, dtype=np.float64)
        m = pyrove.mat4d.from_numpy(arr)
        arr2 = m.to_numpy()
        np.testing.assert_array_almost_equal(arr, arr2)

    def test_quatd_from_numpy(self):
        arr = np.array([0.0, 0.0, 0.0, 1.0], dtype=np.float64)
        q = pyrove.quatd.from_numpy(arr)
        self.assertEqual(q.x, 0.0)
        self.assertEqual(q.y, 0.0)
        self.assertEqual(q.z, 0.0)
        self.assertEqual(q.w, 1.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
