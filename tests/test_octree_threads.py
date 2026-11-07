# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""Bad arguments to an octree are errors, and so is nothing done from another thread: it waits for its turn, it does not crash."""
import threading

import numpy as np
import pytest

import pyrove


def _triangle(x):
    return pyrove.triangle3(pyrove.vec3(x, 0, 0), pyrove.vec3(x + 0.01, 0, 0), pyrove.vec3(x, 0.01, 0))


def test_building_in_a_box_with_nothing_in_it_is_an_error():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3())
    for i in range(16):
        tree.insert(_triangle(i * 0.1), i)
    with pytest.raises(ValueError):
        tree.build()


def test_building_in_an_infinite_box_is_an_error():
    inf = float("inf")
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-inf, -inf, -inf), pyrove.vec3(inf, inf, inf)))
    for i in range(16):
        tree.insert(_triangle(i * 0.1), i)
    with pytest.raises(ValueError):
        tree.build()


def test_an_empty_box_with_no_items_builds():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3())
    tree.build()
    assert tree.size == 0


def test_coincident_triangles_with_no_limit_on_cell_size_end():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1)), 8, 0.0)
    point = pyrove.vec3(0.25, 0.25, 0.25)
    for i in range(17):
        tree.insert(pyrove.triangle3(point, point, point), i)
    tree.build()
    found = tree.query_aabb(pyrove.aabb3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 1, 1)))
    assert sorted(found) == list(range(17))


def test_nan_triangles_end_too():
    nan = float("nan")
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1)), 8, 0.0)
    for i in range(17):
        tree.insert(pyrove.triangle3(pyrove.vec3(nan, nan, nan), pyrove.vec3(nan, nan, nan), pyrove.vec3(nan, nan, nan)), i)
    tree.build()
    assert tree.size == 17


def _a_big_tree():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-100, -100, -100), pyrove.vec3(100, 100, 100)))
    for i in range(30000):
        x = (i % 197) - 98.0
        y = (i % 89) - 44.0
        z = (i % 41) - 20.0
        tree.insert(pyrove.triangle3(pyrove.vec3(x, y, z), pyrove.vec3(x + 0.5, y, z), pyrove.vec3(x, y + 0.5, z)), i)
    return tree


def test_inserting_and_querying_while_it_builds_waits():
    tree = _a_big_tree()
    errors = []

    def other():
        box = pyrove.aabb3(pyrove.vec3(-100, -100, -100), pyrove.vec3(100, 100, 100))
        for i in range(200):
            try:
                tree.query_aabb(box)
            except RuntimeError:
                pass           # not built (yet, or again): an error it may say, never a crash
            except Exception as e:      # anything else is not what it says
                errors.append(e)
            tree.insert(_triangle(0.0), -1)

    worker = threading.Thread(target=other)
    worker.start()
    for _ in range(3):
        tree.build()
    worker.join(timeout=15)
    assert not worker.is_alive()
    assert not errors


def test_threads_inserting_at_once_lose_nothing():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1)))

    def inserting(k):
        for i in range(2000):
            tree.insert(_triangle(0.0), k * 10000 + i)

    threads = [threading.Thread(target=inserting, args=(k,)) for k in range(8)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    assert tree.size == 16000


# --- raycast_batch ------------------------------------------------------------

def _ground():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-10, -1, -10), pyrove.vec3(10, 1, 10)))
    tree.insert(pyrove.triangle3(pyrove.vec3(-9, 0, -9), pyrove.vec3(9, 0, -9), pyrove.vec3(0, 0, 9)), 1)
    tree.build()
    return tree


@pytest.mark.parametrize("distance", [10.0, 1e10, 1e20, 1e30, float("inf")])
def test_raycast_batch_at_any_distance(distance):
    down = np.array([[0, -1, 0]], dtype=np.float32)
    hits = _ground().raycast_batch(pyrove.vec3(0, 1, 0), down, distance)
    assert hits[0] == pytest.approx(1.0)


def test_raycast_batch_within_its_distance():
    down = np.array([[0, -1, 0]], dtype=np.float32)
    assert np.isinf(_ground().raycast_batch(pyrove.vec3(0, 1, 0), down, 0.5)[0])


def test_raycast_batch_of_a_negative_distance_is_an_error():
    down = np.array([[0, -1, 0]], dtype=np.float32)
    with pytest.raises(ValueError):
        _ground().raycast_batch(pyrove.vec3(0, 1, 0), down, -1.0)
    with pytest.raises(ValueError):
        _ground().raycast_batch(pyrove.vec3(0, 1, 0), down, float("nan"))


def test_raycast_batch_takes_a_read_only_array():
    down = np.array([[0, -1, 0]], dtype=np.float32)
    down.setflags(write=False)
    assert _ground().raycast_batch(pyrove.vec3(0, 1, 0), down, 10.0)[0] == pytest.approx(1.0)


def test_raycast_batch_measures_a_direction_of_any_length():
    down = np.array([[0, -5, 0]], dtype=np.float32)
    assert _ground().raycast_batch(pyrove.vec3(0, 1, 0), down, 10.0)[0] == pytest.approx(1.0)
