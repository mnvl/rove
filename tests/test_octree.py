# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""Pickling OctreeTriangle3: a tree built in one process is used in another.

The state is octree::save()'s bytes -- the items and the built nodes -- so a
round trip must give back the same tree, not merely the same triangles: the
same payloads for every query, in the same order, which only the same node
structure produces.
"""
import math
import pickle
import random

import numpy as np
import pytest

import pyrove
from ray_scan import F32, octree_of, random_rays, random_triangles, scaled_about_centre, trace_triangles, unit_rows


def _random_tree(n: int, seed: int) -> pyrove.OctreeTriangle3:
    rng = random.Random(seed)

    def point():
        return pyrove.vec3(rng.uniform(-5, 5), rng.uniform(-5, 5), rng.uniform(-1, 1))

    tree = pyrove.OctreeTriangle3(
        pyrove.aabb3(pyrove.vec3(-5, -5, -1), pyrove.vec3(5, 5, 1)))
    for i in range(n):
        tree.insert(pyrove.triangle3(point(), point(), point()), 3 * i + 1)
    tree.build()
    return tree


def _items(tree) -> list:
    return [((t.A.x, t.A.y, t.A.z, t.B.x, t.B.y, t.B.z, t.C.x, t.C.y, t.C.z), p)
            for t, p in tree.query_aabb_items(tree.bounds)]


def test_pickle_round_trip_is_the_same_tree():
    tree = _random_tree(400, seed=1)
    copy = pickle.loads(pickle.dumps(tree))

    assert type(copy) is pyrove.OctreeTriangle3
    assert copy.size == tree.size == 400
    assert (copy.bounds.lo.x, copy.bounds.hi.z) == (tree.bounds.lo.x, tree.bounds.hi.z)
    assert _items(copy) == _items(tree)

    rng = random.Random(2)
    for _ in range(100):
        c = pyrove.vec3(rng.uniform(-5, 5), rng.uniform(-5, 5), rng.uniform(-1, 1))
        h = pyrove.vec3(rng.uniform(0.1, 2), rng.uniform(0.1, 2), rng.uniform(0.1, 1))
        box = pyrove.aabb3(c - h, c + h)
        assert copy.query_aabb(box) == tree.query_aabb(box)
        sphere = pyrove.sphere3(c, rng.uniform(0.1, 2))
        assert copy.query_sphere(sphere) == tree.query_sphere(sphere)

    directions = np.array(
        [[math.cos(a), math.sin(a), -0.05] for a in np.linspace(0, 2 * math.pi, 360)],
        dtype=np.float32)
    origin = pyrove.vec3(0.3, -0.2, 0.0)
    np.testing.assert_array_equal(
        copy.raycast_batch(origin, directions, 20.0),
        tree.raycast_batch(origin, directions, 20.0))

    # Pickling the copy gives the same bytes: nothing was rebuilt on the way.
    assert tree.__getstate__() == copy.__getstate__()


def test_an_empty_built_tree_round_trips():
    tree = _random_tree(0, seed=3)
    copy = pickle.loads(pickle.dumps(tree))
    assert copy.size == 0
    assert copy.query_aabb(copy.bounds) == []


def test_an_unbuilt_tree_does_not_pickle():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1)))
    tree.insert(pyrove.triangle3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0), pyrove.vec3(0, 1, 0)), 0)
    with pytest.raises(Exception, match="build"):
        pickle.dumps(tree)


def test_a_damaged_state_is_refused():
    state = _random_tree(20, seed=4).__getstate__()
    blank = pyrove.OctreeTriangle3.__new__(pyrove.OctreeTriangle3)
    with pytest.raises(RuntimeError, match="end early"):
        blank.__setstate__(state[:-1])
    blank = pyrove.OctreeTriangle3.__new__(pyrove.OctreeTriangle3)
    with pytest.raises(RuntimeError, match="past the saved tree"):
        blank.__setstate__(state + b"\0")
    blank = pyrove.OctreeTriangle3.__new__(pyrove.OctreeTriangle3)
    with pytest.raises(RuntimeError, match="not a saved octree"):
        blank.__setstate__(b"\0" * len(state))


def test_raycast_batch_gives_the_distance_to_the_nearest_hit():
    # a floor at z = -1 and a roof at z = 2 over the origin: a ray going down
    # at z component dz meets the floor 1 / |dz| away, one going up the roof
    # 2 / dz away, unless that is past max_distance; a level ray meets neither
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-50, -50, -5), pyrove.vec3(50, 50, 5)))
    tree.insert(pyrove.triangle3(pyrove.vec3(-40, -40, -1), pyrove.vec3(40, -40, -1), pyrove.vec3(0, 40, -1)), 1)
    tree.insert(pyrove.triangle3(pyrove.vec3(-40, -40, 2), pyrove.vec3(40, -40, 2), pyrove.vec3(0, 40, 2)), 2)
    tree.build()
    directions = np.array([[0, 0, -1], [0.6, 0, -0.8], [0, 0.8, 0.6], [0, 0, 1], [1, 0, 0], [0.995, 0, -0.0998749]],
                          dtype=np.float32)
    directions /= np.linalg.norm(directions, axis=1, keepdims=True)
    got = tree.raycast_batch(pyrove.vec3(0, 0, 0), directions, 5.0)
    expected = [1.0, 1.25, 2 / 0.6, 2.0, math.inf, math.inf]      # the last would meet the floor 10 away
    np.testing.assert_allclose(got, expected, rtol=1e-5)


# raycast_batch_hits: per-ray origins, and what each ray hits.  Against the scan: a ray that grazes an edge may go either way, so
# what the tree finds must be the nearest of the triangles the scan hits even shrunk by 0.1% (robust), or no farther, and what
# it reports must be hit by the triangle grown by 0.1% (loose).

def test_raycast_batch_hits_are_the_scans_and_its_distances_raycast_batchs():
    rng = np.random.default_rng(41)
    triangles = random_triangles(rng, 400) * 4
    payloads = np.arange(400) * 7 - 50
    tree = octree_of(triangles, payloads)
    origins, directions = random_rays(rng, 3000, 4.0)
    distance, item, payload, normal = tree.raycast_batch_hits(origins, directions, 12.0)
    assert distance.dtype == F32 and item.dtype == np.int64 and payload.dtype == np.int32 and normal.dtype == F32
    o, d = origins.astype(float), unit_rows(directions.astype(float))
    tris = triangles.astype(float)
    exact = trace_triangles(o, d, tris, 12.0)
    robust = trace_triangles(o, d, scaled_about_centre(tris, 1 - 1e-3, 1), 12.0).min(axis=1)
    loose = trace_triangles(o, d, scaled_about_centre(tris, 1 + 1e-3, 1), 12.0 + 1e-3)
    hits = 0
    for r in range(len(origins)):
        if not np.isfinite(distance[r]):
            assert not np.isfinite(robust[r])
            assert item[r] == -1 and payload[r] == -1 and np.isnan(normal[r]).all()
            continue
        hits += 1
        t = float(distance[r])
        assert t <= robust[r] + 2e-4 * (1 + t)
        assert np.isfinite(loose[r, item[r]])
        assert payload[r] == payloads[item[r]]
        if np.isfinite(exact[r, item[r]]):
            assert abs(exact[r, item[r]] - t) <= 2e-4 * (1 + t)
        tri = tris[item[r]]
        face = np.cross(tri[1] - tri[0], tri[2] - tri[1])
        assert np.linalg.norm(np.cross(normal[r], face / np.linalg.norm(face))) < 1e-4
        assert normal[r] @ d[r] <= 1e-6 and abs(np.linalg.norm(normal[r]) - 1) < 1e-5
    assert hits > 500
    # one shared origin and unit directions: raycast_batch's distances, but for a graze
    shared = np.repeat(origins[:1], len(origins), axis=0)
    unit = unit_rows(directions.astype(float)).astype(F32)
    old = tree.raycast_batch(pyrove.vec3(*map(float, origins[0])), unit, 12.0)
    new = tree.raycast_batch_hits(shared, unit, 12.0)[0]
    assert (np.isfinite(old) != np.isfinite(new)).sum() <= 3
    both = np.isfinite(old) & np.isfinite(new)
    np.testing.assert_allclose(old[both], new[both], rtol=1e-4, atol=1e-5)


def test_raycast_batch_hits_takes_a_direction_of_any_length_it_can_tell_from_none():
    rng = np.random.default_rng(7)
    tree = octree_of(random_triangles(rng, 400) * 4, range(400))
    origins, directions = random_rays(rng, 1000, 4.0)
    distance, _, _, _ = tree.raycast_batch_hits(origins, directions, 12.0)
    hit = np.isfinite(distance)
    assert hit.sum() > 300                                              # the rays meet something
    for scale in (1e-5, 1e30):                                          # short, and so long that its square overflows float
        scaled = (directions * F32(scale)).astype(F32)
        assert np.isfinite(scaled).all()
        with np.errstate(over="ignore"):
            assert scale < 1 or not np.isfinite((scaled ** 2).sum(axis=1)).any()
        d, _, _, _ = tree.raycast_batch_hits(origins, scaled, 12.0)
        assert (np.isfinite(d) != hit).sum() <= 5                      # a graze may go either way
        both = hit & np.isfinite(d)
        np.testing.assert_allclose(d[both], distance[both], rtol=1e-4, atol=1e-5)


def test_raycast_batch_hits_refuses_what_is_no_ray():
    tree = octree_of(random_triangles(np.random.default_rng(42), 10), range(10))
    o = np.zeros((2, 3), F32)
    d = np.array([[1, 0, 0], [0, 0, 0]], F32)
    with pytest.raises(ValueError, match="ray 1"):
        tree.raycast_batch_hits(o, d, 1.0)
    for bad in (np.nan, np.inf, -np.inf):                               # in the origin or the direction of the second row
        for k in range(3):
            row = np.ones((2, 3), F32)
            row[1, k] = bad
            with pytest.raises(ValueError, match="ray 1"):
                tree.raycast_batch_hits(row, np.ones((2, 3), F32), 1.0)
            with pytest.raises(ValueError, match="ray 1"):
                tree.raycast_batch_hits(o, row, 1.0)
    with pytest.raises(ValueError, match="ray 1"):                     # too short to tell from rounding noise
        tree.raycast_batch_hits(o, np.array([[1, 0, 0], [1e-7, 0, 0]], F32), 1.0)
    with pytest.raises(ValueError, match="distance"):
        tree.raycast_batch_hits(o, np.ones((2, 3), F32), -1.0)
    with pytest.raises(ValueError, match="same number"):
        tree.raycast_batch_hits(o, np.ones((3, 3), F32), 1.0)
    with pytest.raises(ValueError, match="same number"):                # more origins than directions: none read past
        tree.raycast_batch_hits(np.zeros((3, 3), F32), np.ones((2, 3), F32), 1.0)
    with pytest.raises(TypeError):
        tree.raycast_batch_hits(o.astype(np.float64), np.ones((2, 3), F32), 1.0)
    unbuilt = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1)))
    with pytest.raises(Exception, match="build"):
        unbuilt.raycast_batch_hits(o, np.ones((2, 3), F32), 1.0)
    with pytest.raises(Exception, match="build"):                  # with no rays to trace too
        unbuilt.raycast_batch_hits(np.zeros((0, 3), F32), np.zeros((0, 3), F32), 1.0)
