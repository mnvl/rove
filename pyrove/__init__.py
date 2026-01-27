"""
pyrove - Python bindings for the rove C++ math and geometry library

This package provides Python bindings for the rove C++ library, which includes:
- Vector math (vec2, vec3, vec4)
- Matrix transformations (mat3, mat4)
- Quaternions
- Geometric primitives (ray, line, plane, triangle, sphere, capsule)
- Bounding volumes (AABB, OBB)
- Collision detection
- Pathfinding (A*)
- Heightfield terrain
"""

from __future__ import annotations

__version__ = "1.0.0"

# Import the compiled C++ extension module
try:
    from .pyrove_bind import *
except ImportError as e:
    raise ImportError(
        "Failed to import the pyrove C++ extension module. "
        "Make sure the package was installed correctly with pip install."
    ) from e

# Define public API
__all__ = [
    # Core types (float)
    "vec2", "vec3", "vec4",
    "mat3", "mat4",
    "quat", "quat_slerper",
    # Core types (double)
    "vec2d", "vec3d", "vec4d",
    "mat3d", "mat4d",
    "quatd", "quat_slerperd",
    # Geometric primitives
    "ray3",
    "line3",
    "plane",
    "triangle",
    "sphere2", "sphere3",
    "sphere2d", "sphere3d",
    "capsule",
    # Bounding volumes (float)
    "aabb2", "aabb3",
    "obb2", "obb3",
    # Bounding volumes (double)
    "aabb2d", "aabb3d",
    "obb2d", "obb3d",
    "frustum",
    # Collision
    "contact_info",
    # Algorithms
    "heightfield",
    # Constants
    "PI", "EPSILON",
]
