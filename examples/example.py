#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""
Example script demonstrating pyrove Python bindings.

This script shows basic usage of vectors, matrices, and other geometric primitives.
"""

import pyrove

def main():
    print("=== pyrove Python Bindings Demo ===\n")

    # Vector operations
    print("1. Vector Operations")
    print("-" * 40)
    v1 = pyrove.vec3(1.0, 2.0, 3.0)
    v2 = pyrove.vec3(4.0, 5.0, 6.0)

    print(f"v1 = {v1}")
    print(f"v2 = {v2}")
    print(f"v1 + v2 = {v1 + v2}")
    print(f"v1 - v2 = {v1 - v2}")
    print(f"v1 * 2.0 = {v1 * 2.0}")
    print(f"v1.length() = {v1.length():.4f}")
    print(f"v1.dot(v2) = {v1.dot(v2):.4f}")

    # Normalization
    v3 = pyrove.vec3(3.0, 4.0, 0.0)
    print(f"\nv3 = {v3}")
    print(f"v3.length() = {v3.length():.4f}")
    v3.normalize()
    print(f"v3 after normalize() = {v3}")
    print(f"v3.length() after normalize = {v3.length():.4f}")

    # 2D vectors
    print("\n2. 2D Vectors")
    print("-" * 40)
    v2d1 = pyrove.vec2(3.0, 4.0)
    v2d2 = pyrove.vec2(1.0, 0.0)
    print(f"v2d1 = {v2d1}")
    print(f"v2d1.length() = {v2d1.length():.4f}")
    perp = v2d1.perpendicular()
    print(f"v2d1.perpendicular() = {perp}")
    print(f"v2d1.dot(perpendicular) = {v2d1.dot(perp):.4f} (should be ~0)")

    # Matrix operations
    print("\n3. Matrix Operations")
    print("-" * 40)
    mat_identity = pyrove.mat4()
    print("mat4() is the identity")

    mat_trans = pyrove.translation_matrix(pyrove.vec3(1.0, 2.0, 3.0))
    print(f"translation_matrix((1, 2, 3)) moves the origin to {pyrove.transform_point(mat_trans, pyrove.vec3(0, 0, 0))}")

    mat_scale = pyrove.scaling_matrix(pyrove.vec3(2.0, 2.0, 2.0))
    print(f"scaling_matrix((2, 2, 2)) takes (1, 1, 1) to {pyrove.transform_point(mat_scale, pyrove.vec3(1, 1, 1))}")

    # Quaternions
    print("\n4. Quaternions")
    print("-" * 40)
    q = pyrove.quat(0.0, 0.0, 0.0, 1.0)
    print(f"Quaternion: {q}")

    # Geometric primitives
    print("\n5. Geometric Primitives")
    print("-" * 40)

    # Ray
    ray_origin = pyrove.vec3(0.0, 0.0, -10.0)
    ray_dir = pyrove.vec3(0.0, 0.0, 1.0)
    ray = pyrove.ray3(ray_origin, ray_dir)
    point_on_ray = ray.apply(5.0)
    print(f"Ray from {ray_origin} along {ray_dir}")
    print(f"  Point at t=5: {point_on_ray}")

    # Line segment
    line = pyrove.segment3(pyrove.vec3(0, 0, 0), pyrove.vec3(10, 0, 0))
    line_length_sq = line.length_sq()
    test_point = pyrove.vec3(5, 3, 0)
    dist = line.distance(test_point)
    print(f"\nLine from (0,0,0) to (10,0,0)")
    print(f"  Length squared: {line_length_sq:.2f}")
    print(f"  Distance to point (5,3,0): {dist:.2f}")

    # Plane
    plane = pyrove.plane(pyrove.vec3(0, 0, 0), pyrove.vec3(0, 1, 0))
    print(f"\nPlane at origin with normal (0,1,0)")
    downward = pyrove.ray3(pyrove.vec3(1, 2, 3), pyrove.vec3(0, -1, 0))
    if plane.test_intersection(downward):
        t = plane.trace(downward)
        hit_point = downward.apply(t)
        print(f"  Ray intersects plane at t={t:.2f}, point={hit_point}")

    # Triangle
    tri = pyrove.triangle3(
        pyrove.vec3(0, 0, 0),
        pyrove.vec3(3, 0, 0),
        pyrove.vec3(0, 4, 0)
    )
    area = tri.area()
    centroid = tri.cog()
    circumcenter = tri.circumcenter()
    print(f"\nTriangle with vertices (0,0,0), (3,0,0), (0,4,0)")
    print(f"  Area: {area:.2f}")
    print(f"  Centroid: {centroid}")
    print(f"  Circumcenter: {circumcenter}")

    # Capsule
    capsule = pyrove.capsule3(
        pyrove.vec3(0, 0, 0),
        pyrove.vec3(0, 2, 0),
        0.5
    )
    test_cap_point = pyrove.vec3(0, 1, 0.3)
    inside = capsule.contains(test_cap_point)
    print(f"\nCapsule from (0,0,0) to (0,2,0) with radius 0.5")
    print(f"  Point (0,1,0.3) is {'inside' if inside else 'outside'}")

    # Sphere
    sphere_center = pyrove.vec3(0.0, 0.0, 0.0)
    sphere = pyrove.sphere3(sphere_center, 5.0)
    print(f"\nSphere at origin with radius 5.0")

    # AABB
    aabb_min = pyrove.vec3(-1.0, -1.0, -1.0)
    aabb_max = pyrove.vec3(1.0, 1.0, 1.0)
    aabb = pyrove.aabb3(aabb_min, aabb_max)
    aabb_center = aabb.centre()
    print(f"\nAABB from {aabb_min} to {aabb_max}")
    print(f"  Center: {aabb_center}")

    print("\n=== Demo Complete ===")
    print("\nFor more, see README.md")

if __name__ == "__main__":
    main()
