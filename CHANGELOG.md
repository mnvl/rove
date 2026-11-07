# Changelog

All notable changes to rove are written here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
versions follow [Semantic Versioning](https://semver.org/).

## [3.0.1] - 2026-11-07

What the independent review of 3.0.0 found.

### Fixed

- `aabb::trace`: a ray that starts or ends in the box, a touch at `t_max`, a flat box; it works in 2D too, where it was not implemented.
- `obb::test_collision`: every separating axis, for the whole of the time interval, and time forward from `t_min`.
- `obb::test_intersection`: all 15 axes.
- `heightfield`: columns and rows the right way round, `y_under` between rows, positions left of the field, and `trace` from inside its box,
  answering in world space with the time of the ray it was given.
- `cone::bounds` holds the whole cone.
- `sphere` against rays and segments, `plane::query_intersection(plane, ray&)`, the excentres of `triangle`, the contact point of
  `triangle::collide`, `triangle::query_intersection(triangle, segment&)` (it did not compile), `triangle::contains` far from the origin.
- `aabb::extend` by an empty box. `aabb<2>::edges_count`.
- `pcg32::uniform` and `weighted` of whole numbers.
- `first_contact` could never return when the tolerance was below float resolution.
- `m.inverse(m)` and `m.transpose(m)`. `axes_to_world` reports its failure. Matrices with no usable inverse and projections of no size throw.
- `vec<N>` for N of 5 or more is a whole type. `vec::to_axis_angle` is the rotation vector's axis and angle.
- Parallel lines are told by the angle between them, not the length of their directions.
- `segment::contains` includes its ends. `ray::distance` of a ray of no direction. `abs(0.0f)` is +0. `normalize` and `normalize_or_zero` agree
  at the extremes.
- `contact_info` is fully defined on a miss. `a_star` ends a search of an unbounded scorer: `limit_cells`, 2^26 by default.
- An octree build ends for any input, and is an error for items in a box that is empty or not finite.
- dfloat: every template takes it, as the README said, and it builds with warnings as errors.
- An installed rove links with any compiler: LTO is now opt-in. Its headers are SYSTEM to consumers, and rove's parent directory is off their
  include path; `<rove/src/x.h>` works in any checkout, from FetchContent and from an install. The headers compile after `<windows.h>`.
- pyrove: crashes on an index past a matrix, a malformed pickle, a tree used from another thread. `pip install -e` imports. Every value type
  compares, hashes, pickles and copies by value. `raycast_batch` at any distance, and read-only arrays. `first_contact` of two spheres, and the
  free functions the README names. `repr` shows every digit and the true type. Stubs and `py.typed`.

### Added

- `<rove/version.h>`: `ROVE_VERSION`. `a_star::limit_cells`. `octree::copy_unbuilt`. `pcg32::state`, `increment`, `from_state`.

### Changed

- `matrix::rotation(vec)` is documented as what it is: Euler angles, not a rotation vector.
- `Mat.inverse()` of a singular matrix raises `SingularMatrixError`, both a `ValueError` and the `RuntimeError` it was.

## [3.0.0] - 2026-11-06

First public release.
