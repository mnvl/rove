# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

import unittest
import sys
import os
import math

# Add build directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))
import pyrove


class TestOBB3Construction(unittest.TestCase):
    def test_from_vectors(self):
        o = pyrove.vec3(1, 2, 3)
        t = pyrove.vec3(4, 0, 0)
        n = pyrove.vec3(0, 5, 0)
        b = pyrove.vec3(0, 0, 6)
        box = pyrove.obb3(o, t, n, b)

        self.assertAlmostEqual(box.origin.x, 1.0)
        self.assertAlmostEqual(box.tangent.x, 4.0)
        self.assertAlmostEqual(box.normal.y, 5.0)
        self.assertAlmostEqual(box.binormal.z, 6.0)


class TestOBB3Copy(unittest.TestCase):
    """The copy constructor exists so callers of the mutating transform()
    can take a copy without rebuilding the box from its four components."""

    def _box(self):
        return pyrove.obb3(
            pyrove.vec3(1, 2, 3),
            pyrove.vec3(4, 0, 0),
            pyrove.vec3(0, 5, 0),
            pyrove.vec3(0, 0, 6),
        )

    def test_copy_reproduces_every_component(self):
        original = self._box()
        clone = pyrove.obb3(original)
        for attr in ("origin", "tangent", "normal", "binormal"):
            a = getattr(original, attr)
            b = getattr(clone, attr)
            self.assertAlmostEqual(a.x, b.x)
            self.assertAlmostEqual(a.y, b.y)
            self.assertAlmostEqual(a.z, b.z)

    def test_copy_is_independent(self):
        original = self._box()
        clone = pyrove.obb3(original)
        clone.origin = pyrove.vec3(99, 99, 99)
        self.assertAlmostEqual(original.origin.x, 1.0)

    def test_transform_does_not_reach_through_a_copy(self):
        """The reason this exists: transform() mutates in place, and callers
        reuse one world-space box across many objects in a frame."""
        original = self._box()
        clone = pyrove.obb3(original)

        m = pyrove.mat4()
        m.identity()
        m.translate(10, 0, 0)
        clone.transform(m)

        self.assertAlmostEqual(original.origin.x, 1.0)
        self.assertAlmostEqual(clone.origin.x, 11.0)

    def test_copying_an_aabb_derived_box_still_works(self):
        """The copy overload must not shadow the aabb3 constructor."""
        from_aabb = pyrove.obb3(
            pyrove.aabb3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 2, 3)))
        clone = pyrove.obb3(from_aabb)
        self.assertAlmostEqual(clone.tangent.x, 1.0)
        self.assertAlmostEqual(clone.normal.y, 2.0)
        self.assertAlmostEqual(clone.binormal.z, 3.0)


class TestOBB2Copy(unittest.TestCase):
    def test_copy_is_independent(self):
        original = pyrove.obb2(
            pyrove.aabb2(pyrove.vec2(0, 0), pyrove.vec2(4, 5)))
        clone = pyrove.obb2(original)
        clone.origin = pyrove.vec2(99, 99)
        self.assertAlmostEqual(original.origin.x, 0.0)
        self.assertAlmostEqual(clone.origin.x, 99.0)


class TestMakeSweepVolume(unittest.TestCase):
    def test_returns_six_obbs(self):
        box = pyrove.obb3(
            pyrove.vec3(0, 0, 0),
            pyrove.vec3(2, 0, 0),
            pyrove.vec3(0, 3, 0),
            pyrove.vec3(0, 0, 4),
        )
        disp = pyrove.vec3(10, 0, 0)
        result = pyrove.obb3.make_sweep_volume(box, disp)
        self.assertEqual(len(result), 6)
        for obb in result:
            self.assertIsInstance(obb, pyrove.obb3)

    def test_origins(self):
        box = pyrove.obb3(
            pyrove.vec3(1, 2, 3),
            pyrove.vec3(4, 0, 0),
            pyrove.vec3(0, 5, 0),
            pyrove.vec3(0, 0, 6),
        )
        disp = pyrove.vec3(10, 0, 0)
        out = pyrove.obb3.make_sweep_volume(box, disp)

        # bottom face origin = box.origin
        self.assertAlmostEqual(out[0].origin.x, 1.0)
        self.assertAlmostEqual(out[0].origin.y, 2.0)
        self.assertAlmostEqual(out[0].origin.z, 3.0)

        # top face origin = box.origin + box.binormal
        self.assertAlmostEqual(out[1].origin.x, 1.0)
        self.assertAlmostEqual(out[1].origin.y, 2.0)
        self.assertAlmostEqual(out[1].origin.z, 9.0)

        # front face origin = box.origin + box.tangent
        self.assertAlmostEqual(out[5].origin.x, 5.0)
        self.assertAlmostEqual(out[5].origin.y, 2.0)
        self.assertAlmostEqual(out[5].origin.z, 3.0)

    def test_tunneling_detection(self):
        """Sweep OBBs detect a thin wall that the final-position OBB misses."""
        box = pyrove.obb3(
            pyrove.vec3(-5, -0.5, -0.5),
            pyrove.vec3(1, 0, 0),
            pyrove.vec3(0, 1, 0),
            pyrove.vec3(0, 0, 1),
        )
        disp = pyrove.vec3(10, 0, 0)

        wall = pyrove.triangle3(
            pyrove.vec3(0, -2, -2),
            pyrove.vec3(0, 2, -2),
            pyrove.vec3(0, 0, 2),
        )

        # Final-position OBB (at x=5) should NOT hit the wall at x=0
        final_box = pyrove.obb3(
            pyrove.vec3(5, -0.5, -0.5),
            pyrove.vec3(1, 0, 0),
            pyrove.vec3(0, 1, 0),
            pyrove.vec3(0, 0, 1),
        )
        self.assertFalse(final_box.test_intersection(wall))

        # Sweep OBBs should catch the wall
        sweep = pyrove.obb3.make_sweep_volume(box, disp)
        self.assertTrue(any(s.test_intersection(wall) for s in sweep))

    def test_zero_displacement(self):
        """Zero displacement produces 6 degenerate (flat) OBBs."""
        box = pyrove.obb3(
            pyrove.vec3(0, 0, 0),
            pyrove.vec3(2, 0, 0),
            pyrove.vec3(0, 3, 0),
            pyrove.vec3(0, 0, 4),
        )
        disp = pyrove.vec3(0, 0, 0)
        sweep = pyrove.obb3.make_sweep_volume(box, disp)
        self.assertEqual(len(sweep), 6)
        for s in sweep:
            # The displacement edge (binormal of each sweep OBB) should be zero
            self.assertAlmostEqual(s.binormal.x, 0.0)
            self.assertAlmostEqual(s.binormal.y, 0.0)
            self.assertAlmostEqual(s.binormal.z, 0.0)


class TestIsDegenerate(unittest.TestCase):
    def test_axis_aligned_box_not_degenerate(self):
        """Well-formed axis-aligned box is not degenerate."""
        box = pyrove.obb3(
            pyrove.vec3(0, 0, 0),
            pyrove.vec3(2, 0, 0),
            pyrove.vec3(0, 3, 0),
            pyrove.vec3(0, 0, 4),
        )
        self.assertFalse(box.is_degenerate())
        self.assertTrue(box.is_basis_linearly_independent())

    def test_parallel_tangent_binormal(self):
        """Tangent and binormal both along X -> degenerate."""
        box = pyrove.obb3(
            pyrove.vec3(0, 0, 0),
            pyrove.vec3(2, 0, 0),
            pyrove.vec3(0, 3, 0),
            pyrove.vec3(0.5, 0, 0),
        )
        self.assertTrue(box.is_degenerate())
        self.assertFalse(box.is_basis_linearly_independent())

    def test_zero_length_edge(self):
        """Zero-length binormal -> degenerate."""
        box = pyrove.obb3(
            pyrove.vec3(0, 0, 0),
            pyrove.vec3(2, 0, 0),
            pyrove.vec3(0, 3, 0),
            pyrove.vec3(0, 0, 0),
        )
        self.assertTrue(box.is_degenerate())

    def test_sweep_obbs_forward_displacement(self):
        """Sweep OBBs with displacement parallel to tangent are degenerate."""
        box = pyrove.obb3(
            pyrove.vec3(-0.8, 0.15, 0.2),
            pyrove.vec3(-0.4, 0, 0),
            pyrove.vec3(0, -0.3, 0),
            pyrove.vec3(0, 0, 0.4),
        )
        disp = pyrove.vec3(-0.03, 0, 0)
        sweep = pyrove.obb3.make_sweep_volume(box, disp)
        # Bottom/top/left/right faces: tangent parallel to displacement.
        self.assertTrue(sweep[0].is_degenerate())
        self.assertTrue(sweep[1].is_degenerate())
        self.assertTrue(sweep[2].is_degenerate())
        self.assertTrue(sweep[3].is_degenerate())
        # Back/front faces: tangent NOT parallel to displacement.
        self.assertFalse(sweep[4].is_degenerate())
        self.assertFalse(sweep[5].is_degenerate())

    def test_transform_translation(self):
        """Transform with a translation matrix moves the OBB."""
        box = pyrove.obb3(
            pyrove.vec3(0, 0, 0),
            pyrove.vec3(1, 0, 0),
            pyrove.vec3(0, 1, 0),
            pyrove.vec3(0, 0, 1),
        )
        m = pyrove.mat4()
        m.identity()
        m.translate(10, 20, 30)
        box.transform(m)
        self.assertAlmostEqual(box.origin.x, 10, places=5)
        self.assertAlmostEqual(box.origin.y, 20, places=5)
        self.assertAlmostEqual(box.origin.z, 30, places=5)
        # Edge vectors unchanged by pure translation.
        self.assertAlmostEqual(box.tangent.x, 1, places=5)
        self.assertAlmostEqual(box.normal.y, 1, places=5)
        self.assertAlmostEqual(box.binormal.z, 1, places=5)

    def test_transform_scale(self):
        """Transform with a scale matrix scales the OBB."""
        box = pyrove.obb3(
            pyrove.vec3(1, 2, 3),
            pyrove.vec3(1, 0, 0),
            pyrove.vec3(0, 1, 0),
            pyrove.vec3(0, 0, 1),
        )
        m = pyrove.mat4()
        m.identity()
        m.scale(2, 2, 2)
        box.transform(m)
        self.assertAlmostEqual(box.origin.x, 2, places=5)
        self.assertAlmostEqual(box.origin.y, 4, places=5)
        self.assertAlmostEqual(box.origin.z, 6, places=5)
        self.assertAlmostEqual(box.tangent.x, 2, places=5)
        self.assertAlmostEqual(box.normal.y, 2, places=5)
        self.assertAlmostEqual(box.binormal.z, 2, places=5)

    def test_rotated_box_not_degenerate(self):
        """45-degree rotated box is not degenerate."""
        box = pyrove.obb3(
            pyrove.vec3(0, 0, 0),
            pyrove.vec3(1, 1, 0),
            pyrove.vec3(-1, 1, 0),
            pyrove.vec3(0, 0, 2),
        )
        self.assertFalse(box.is_degenerate())


if __name__ == '__main__':
    unittest.main()
