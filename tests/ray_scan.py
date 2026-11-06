# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""What the batched ray tests share: random triangles, an OctreeTriangle3 of them, and a scan in numpy that traces every ray
against every triangle (Moller-Trumbore in float64, without edge slack), the oracle the batches are held to."""
import numpy as np

import pyrove

F32 = np.float32


def random_triangles(rng, n):
    """(n, 3, 3) local triangles about the origin; some reach out of the tree's bounds ([-1, 1]^3), as built meshes' may"""
    centres = rng.uniform(-1, 1, (n, 1, 3))
    return (centres + rng.uniform(-0.5, 0.5, (n, 3, 3))).astype(F32)


def octree_of(triangles, payloads):
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1)), 4, 0.01)
    for t, p in zip(triangles, payloads):
        tree.insert(pyrove.triangle3(*(pyrove.vec3(*map(float, v)) for v in t)), int(p))
    tree.build()
    return tree


def unit_rows(v):
    return v / np.linalg.norm(v, axis=1, keepdims=True)


def scaled_about_centre(points, factor, axis):
    centre = points.mean(axis=axis, keepdims=True)
    return centre + (points - centre) * factor


def trace_triangles(origins, directions, triangles, t_max):
    """(rays, triangles) times of Moller-Trumbore over [0, t_max] without edge slack, inf for a miss (float64)"""
    a, b, c = triangles[:, 0], triangles[:, 1], triangles[:, 2]
    e1, e2 = b - a, c - a
    p = np.cross(directions[:, None, :], e2[None])
    det = np.einsum("tk,rtk->rt", e1, p)
    with np.errstate(divide="ignore", invalid="ignore"):
        inv = 1.0 / det
        s = origins[:, None, :] - a[None]
        u = np.einsum("rtk,rtk->rt", s, p) * inv
        q = np.cross(s, e1[None])
        v = np.einsum("rk,rtk->rt", directions, q) * inv
        t = np.einsum("tk,rtk->rt", e2, q) * inv
    hit = (det != 0) & (u >= 0) & (v >= 0) & (u + v <= 1) & (t >= 0) & (t <= t_max)
    return np.where(hit, t, np.inf)


def random_rays(rng, n, spread):
    origins = rng.uniform(-1.5 * spread, 1.5 * spread, (n, 3)).astype(F32)
    towards = rng.uniform(-spread, spread, (n, 3))
    directions = ((towards - origins) * rng.uniform(0.2, 3, (n, 1))).astype(F32)      # of no one length
    return origins, directions
