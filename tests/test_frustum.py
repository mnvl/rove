# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestFrustum(unittest.TestCase):
    def test_default_constructor(self):
        f = pyrove.frustum()
        self.assertEqual(len(f.planes), 6)

    def test_constructor_from_matrix(self):
        # Create a simple projection matrix
        m = pyrove.mat4()
        # Note: This is just testing that the constructor accepts a matrix
        # Actual frustum extraction from matrix is tested in C++
        f = pyrove.frustum(m)
        self.assertEqual(len(f.planes), 6)

    def test_load(self):
        f = pyrove.frustum()
        m = pyrove.mat4()
        f.load(m)
        # After loading, frustum should have 6 planes
        self.assertEqual(len(f.planes), 6)

    def test_planes_property(self):
        f = pyrove.frustum()
        # Can access planes array
        planes = f.planes
        self.assertEqual(len(planes), 6)

    def test_plane_indices_are_named_as_in_cpp(self):
        for cls in (pyrove.frustum, pyrove.frustumd):
            self.assertEqual(cls.left_plane, 0)
            self.assertEqual(cls.right_plane, 1)
            self.assertEqual(cls.top_plane, 2)
            self.assertEqual(cls.bottom_plane, 3)
            self.assertEqual(cls.near_plane, 4)
            self.assertEqual(cls.far_plane, 5)
            self.assertEqual(cls.planes_count, 6)
        self.assertFalse(hasattr(pyrove, "PLANE_LEFT"))
        self.assertFalse(hasattr(pyrove.frustum, "PLANE_LEFT"))
        self.assertFalse(hasattr(pyrove.frustum, "get_plane"))

    def test_plane_by_index(self):
        f = pyrove.frustum(pyrove.perspective_matrix(1.0, 1.0, 0.1, 100.0))
        for i in range(pyrove.frustum.planes_count):
            p = f.plane(i)
            self.assertAlmostEqual(p.A, f.planes[i].A)
            self.assertAlmostEqual(p.D, f.planes[i].D)
        near = f.plane(pyrove.frustum.near_plane)
        self.assertAlmostEqual(near.D, f.planes[4].D)
        with self.assertRaises(IndexError):
            f.plane(6)
        with self.assertRaises(IndexError):
            f.plane(-1)

    def test_contains_point(self):
        # Create a simple frustum using an identity matrix
        # This creates a frustum that doesn't really make sense geometrically,
        # but tests that the binding works
        f = pyrove.frustum()
        m = pyrove.mat4()
        f.load(m)

        # Test that contains method can be called
        point = pyrove.vec3(0.0, 0.0, 0.0)
        # Just verify the method is callable, result depends on matrix
        result = f.contains(point)
        self.assertIsInstance(result, bool)

    def test_test_intersection_aabb(self):
        f = pyrove.frustum()
        m = pyrove.mat4()
        f.load(m)

        # Create a simple AABB
        bb = pyrove.aabb3(pyrove.vec3(-1.0, -1.0, -1.0), pyrove.vec3(1.0, 1.0, 1.0))

        # Test that intersection method can be called
        result = f.test_intersection(bb)
        self.assertIsInstance(result, bool)

    def test_test_intersection_obb(self):
        f = pyrove.frustum()
        m = pyrove.mat4()
        f.load(m)

        # Create a simple OBB
        obb = pyrove.obb3()

        # Test that intersection method can be called
        result = f.test_intersection(obb)
        self.assertIsInstance(result, bool)

    def test_repr(self):
        f = pyrove.frustum()
        s = repr(f)
        self.assertIn("frustum", s)

    def test_property_modification(self):
        f = pyrove.frustum()

        # Create new planes array
        new_planes = [
            pyrove.plane(pyrove.vec3(1.0, 0.0, 0.0), pyrove.vec3(1.0, 0.0, 0.0)),
            pyrove.plane(pyrove.vec3(-1.0, 0.0, 0.0), pyrove.vec3(-1.0, 0.0, 0.0)),
            pyrove.plane(pyrove.vec3(0.0, 1.0, 0.0), pyrove.vec3(0.0, 1.0, 0.0)),
            pyrove.plane(pyrove.vec3(0.0, -1.0, 0.0), pyrove.vec3(0.0, -1.0, 0.0)),
            pyrove.plane(pyrove.vec3(0.0, 0.0, 1.0), pyrove.vec3(0.0, 0.0, 1.0)),
            pyrove.plane(pyrove.vec3(0.0, 0.0, -1.0), pyrove.vec3(0.0, 0.0, -1.0)),
        ]

        # Test we can set planes
        f.planes = new_planes
        self.assertEqual(len(f.planes), 6)

    def test_perspective_frustum(self):
        # Create a more realistic perspective projection matrix
        # This is a simplified perspective matrix
        fov = math.pi / 4.0  # 45 degrees
        aspect = 16.0 / 9.0
        near = 0.1
        far = 100.0

        # Create projection matrix (simplified version)
        f_val = 1.0 / math.tan(fov / 2.0)

        m = pyrove.mat4()
        # This is a simplified setup; proper matrix setup would require
        # implementing the full perspective projection
        # For now, just test that frustum can be created
        f = pyrove.frustum(m)

        self.assertEqual(len(f.planes), 6)

    def test_frustum_aabb_culling(self):
        # Test a practical frustum culling scenario
        f = pyrove.frustum()

        # Create an identity-based frustum
        m = pyrove.mat4()
        f.load(m)

        # Create AABB at origin
        bb1 = pyrove.aabb3(pyrove.vec3(-1.0, -1.0, -1.0), pyrove.vec3(1.0, 1.0, 1.0))
        r1 = f.test_intersection(bb1)

        # Create AABB far away (should be culled in most cases)
        bb2 = pyrove.aabb3(pyrove.vec3(1000.0, 1000.0, 1000.0), pyrove.vec3(1001.0, 1001.0, 1001.0))
        r2 = f.test_intersection(bb2)

        # Both should return boolean values
        self.assertIsInstance(r1, bool)
        self.assertIsInstance(r2, bool)


class TestDoubleFrustum(unittest.TestCase):
    def test_frustumd_default_constructor(self):
        f = pyrove.frustumd()
        self.assertEqual(len(f.planes), 6)

    def test_frustumd_from_matrix(self):
        m = pyrove.mat4d()
        f = pyrove.frustumd(m)
        self.assertEqual(len(f.planes), 6)

    def test_frustumd_load(self):
        f = pyrove.frustumd()
        m = pyrove.mat4d()
        f.load(m)
        self.assertEqual(len(f.planes), 6)

    def test_frustumd_test_intersection_aabb(self):
        f = pyrove.frustumd()

        bb = pyrove.aabb3d(pyrove.vec3d(-1.0, -1.0, -1.0), pyrove.vec3d(1.0, 1.0, 1.0))
        result = f.test_intersection(bb)
        self.assertIsInstance(result, bool)


if __name__ == "__main__":
    unittest.main(verbosity=2)
