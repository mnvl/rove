# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's long calls let other Python threads run: an octree's build(), raycast_batch() and raycast_batch_hits() release the
GIL while they work in C++ (a program that steps simulators from threads would otherwise stop every one of them while one
built or traced a tree).

The oracle: while one thread is in the call, the main thread wakes every millisecond to take a timestamp.  With the GIL held through
the call it cannot wake until the call is over; released, its longest gap is a few milliseconds, far below the call.  (The queries
that return items keep the GIL: building their Python lists is most of their time, and it needs it.)"""
import sys
import threading
import time
import unittest

import numpy as np

import pyrove

GIL = sys.version_info < (3, 13) or sys._is_gil_enabled()


def _grid_tree(n, built=True):
    """A ground of 2 n^2 triangles over [0, n]^2"""
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(0, 0, -1), pyrove.vec3(n, n, 1)), 8)
    for i in range(n):
        for j in range(n):
            a, b = pyrove.vec3(i, j, 0), pyrove.vec3(i + 1, j, 0)
            c, d = pyrove.vec3(i + 1, j + 1, 0), pyrove.vec3(i, j + 1, 0)
            tree.insert(pyrove.triangle3(a, b, c), 2 * (i * n + j))
            tree.insert(pyrove.triangle3(a, c, d), 2 * (i * n + j) + 1)
    if built:
        tree.build()
    return tree


def _downward_rays(n, lo, hi):
    """n rays from (x, y, 0.5) over [lo, hi]^2, down at every slant: unit directions and the distance to z = 0 of each"""
    rng = np.random.default_rng(2)
    origins = np.column_stack([rng.uniform(lo, hi, n), rng.uniform(lo, hi, n), np.full(n, 0.5)]).astype(np.float32)
    directions = rng.normal(size=(n, 3)).astype(np.float32)
    directions[:, 2] = -np.abs(directions[:, 2]) - 0.1
    directions /= np.linalg.norm(directions, axis=1, keepdims=True)
    return origins, directions, 0.5 / -directions[:, 2]


def _longest_stall(call):
    """(how long call took in its thread, the longest the main thread could not wake meanwhile)"""
    done = threading.Event()
    span = {}

    def worker():
        start = time.perf_counter()
        call()
        span["took"] = time.perf_counter() - start
        done.set()

    t = threading.Thread(target=worker)
    ticks = [time.perf_counter()]
    t.start()
    while not done.is_set():
        time.sleep(0.001)
        ticks.append(time.perf_counter())
    t.join()
    ticks.append(time.perf_counter())       # (the call can be over before the loop first looks)
    return span["took"], float(np.diff(ticks).max())


class TestTheGilIsReleased(unittest.TestCase):
    def assert_runs_alongside(self, call, what, repeats=1):
        """call makes `repeats` calls of what is checked: the main thread must run within a quarter of one of them"""
        took, stall = _longest_stall(call)
        # A free-threaded Python has no GIL for the call to hold, so a stall there says nothing about it.  They do happen: on macOS
        # with 3.14t, the main thread has stood still for half a call.  The tests still make the calls and check their answers.
        if not GIL:
            return
        self.assertGreater(took, 0.1, f"{what} took {took:.3f} s: too short to tell (make it longer)")
        self.assertLess(stall, took / (4 * repeats),
                        f"{what}: the main thread could not run for {stall:.3f} s of {took:.3f} s ({repeats} calls)")

    def test_a_call_that_ends_at_once_is_measured(self):
        for _ in range(300):     # it can be over before the main thread first looks
            took, stall = _longest_stall(lambda: None)
            self.assertGreaterEqual(took, 0.0)
            self.assertGreaterEqual(stall, 0.0)

    def test_a_call_that_keeps_the_gil_is_caught(self):
        keeps_the_gil = lambda: sum(range(30_000_000))      # all in C, and sum() does not let go
        if GIL:
            with self.assertRaisesRegex(AssertionError, "could not run"):
                self.assert_runs_alongside(keeps_the_gil, "sum")
        else:
            self.assert_runs_alongside(keeps_the_gil, "sum")

    def test_raycast_batch(self):
        tree = _grid_tree(100)
        rng = np.random.default_rng(1)
        directions = rng.normal(size=(20_000, 3)).astype(np.float32)
        directions[:, 2] = -np.abs(directions[:, 2]) - 0.1
        directions /= np.linalg.norm(directions, axis=1, keepdims=True)
        origin = pyrove.vec3(50, 50, 0.5)
        out = {}
        self.assert_runs_alongside(lambda: out.setdefault("d", tree.raycast_batch(origin, directions, 1000.0)), "raycast_batch")
        # and the answers are still the rays' own: each hits the ground 0.5 below the origin
        np.testing.assert_allclose(out["d"][:1000], 0.5 / -directions[:1000, 2], rtol=1e-4)
        down = np.array([[0, 0, -1]], dtype=np.float32)
        self.assertAlmostEqual(float(tree.raycast_batch(origin, down, 10.0)[0]), 0.5, places=5)

    def test_raycast_batch_refuses_what_it_did(self):
        tree = _grid_tree(2)
        with self.assertRaises(ValueError):
            tree.raycast_batch(pyrove.vec3(0, 0, 1), np.zeros((4, 2), dtype=np.float32), 1.0)

    def test_raycast_batch_hits(self):
        tree = _grid_tree(100)
        origins, directions, ground = _downward_rays(100_000, 40, 60)       # far from the edges: each meets the ground
        out = {}
        self.assert_runs_alongside(lambda: out.setdefault("r", tree.raycast_batch_hits(origins, directions, 1000.0)),
                                   "raycast_batch_hits")
        np.testing.assert_allclose(out["r"][0], ground, rtol=1e-4)

    def test_build(self):
        tree = _grid_tree(800, built=False)       # 600 built in under 0.1 s with clang, too short to tell
        self.assert_runs_alongside(tree.build, "build")
        self.assertEqual(len(tree.query_aabb_items(pyrove.aabb3(pyrove.vec3(10.2, 10.2, -1), pyrove.vec3(10.3, 10.3, 1)))), 2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
