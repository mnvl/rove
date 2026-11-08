# rove

A C++20 math and geometry library for robotics, simulation and games, with Python bindings (`pyrove`).

rove has the types a 3D program needs (vectors, matrices, quaternions, transforms, rays, segments, planes, triangles,
spheres, boxes, capsules, frustums), the queries between them, continuous collision detection, spatial indices, A*
pathfinding and reproducible random numbers. It has no dependencies, compiles as one small static library, and gives
Python the same types with NumPy conversion, at C++ speed.

It began as the math of a game engine written from 2004 on, and was brought up to C++20 for robotics simulation, where
Python's geometry libraries were too slow or too limited and NumPy alone made per-object geometry awkward. Much of the
binding code, refactorings and documentation were written with Claude and curated by the author.

It is the math of [LIGA Football 11v11](https://liga.manvel.dev/), a free eleven-a-side football (soccer) game that plays in
the browser, and of its deterministic simulation.

Our benchmarks show that this library is 2x faster on average than what's available for Python right now when working
on single objects (see examples/benchmark.py).


## Features

### Core Math Types
- **Vectors**: `vec<N, T>` (`vec<2>`, `vec<3>`, `vec<4>`, any N), float, double or `dfloat`; colours are vectors named in the order of their memory (`rgb<T>`, `bgr<T>`, `rgba<T>`, `bgra<T>`, with `.r/.g/.b/.a`), and one order is not taken for another; `xz<T>` is the ground of a Y-up world
- **Matrices**: 3x3 and 4x4, column-major (GLSL's and glTF's layout); setters (`m.translation(...)`) and the same transforms by value (`translation_matrix(...)`)
- **Quaternions**: rotation of vectors, slerp and nlerp, axis-angle, rotation vectors (log and exp maps), the shortest rotation between two directions
- **Transforms** (`transform.h`): `trs<T>`, a translation, rotation and scale, composed, inverted and taken apart from a matrix
- **Scalar helpers** (`scalar.h`): `lerp`, `inverse_lerp`, `remap`, `smoothstep`, `clamp`, angle wraps, `radians`/`degrees`, frame-rate-independent `damp`, `move_towards`, a critically damped spring
- **dfloat** (`dfloat.h`, `ROVE_DFLOAT`): a float that gives the same bits on every platform, for lockstep simulations

### Geometric Primitives
- **Rays**: an origin and a direction (2D and 3D); `apply(t)` is the point at t (the distance functions take the whole line); `ray::unit(origin, direction)` makes the direction unit length, so t is a distance
- **Line segments**: `segment<N>`, with closest points (2D and 3D)
- **Planes**: infinite planes with intersection and classification tests (3D)
- **Triangles**: ray intersection, closest points, and the classical centres and circles (3D)
- **Capsules**, **spheres**, **AABBs** (with `transform_aabb`, `bounds_of`, `grown`), **OBBs** (2D and 3D)
- **Frustums**: view frustums for culling boxes and spheres (3D)

### Advanced Features
- **Collision detection** (`collision.h`): `first_contact`, when a moving shape first touches another over a step; empty when it does not
  - a moving sphere against a sphere, a segment, an OBB or an AABB, in closed form
  - a moving sphere against a capsule that moves, turns and stretches over the same step (conservative advancement: never past the touch, no tunnelling)
- **A* pathfinding**: Grid-based pathfinding with custom scoring
- **Spatial indices**: a static octree, and a hash grid of columns that takes inserts at any time (both with box, segment and sphere queries and the nearest hit along a ray)
- **Random numbers** (`random.h`): `pcg32`, the same numbers on every machine, with its own distributions (`below`, `uniform`, `normal`, `unit_vector`, `shuffle`, `weighted`)
- **Hashes and noise** (`hash.h`, `noise.h`): `splitmix64`, `hash_combine`, `hash_to_unit`; value noise in 1 to 3 dimensions and `fbm`
- **Formatting** (`format.h`, opt-in): `std::format("{:.3f}", v)` for vectors (every storage), quaternions, matrices (by rows) and AABBs; the spec applies to each component
- **Heightfield**: Terrain heightfield with ray tracing
- **Bresenham algorithm**: Line rasterization

### Python Bindings
- Python API via nanobind for the types above, `trs`, `pcg32`, the hashes, noise, the octree of triangles and
  the free functions (but the ground helpers `to_xz`, `from_xz` and `horizontal_length`, and `dfloat`)
- NumPy integration for seamless array conversions
- Float and double precision support

## Python

```bash
pip install pyrove                                   # from PyPI
pip install git+https://github.com/mnvl/rove.git     # from source: needs CMake and a C++20 compiler
```

More ways to install, and what each needs: [docs/install.md](https://github.com/mnvl/rove/blob/main/docs/install.md).

```python
import pyrove
import numpy as np

# Vectors
v1 = pyrove.vec3(1.0, 2.0, 3.0)
v2 = pyrove.vec3(4.0, 5.0, 6.0)

# Vector operations
dot = pyrove.dot(v1, v2)
cross = pyrove.cross(v1, v2)
normalized = pyrove.normalize(v1)
# a vector of no length has no direction: pyrove.normalize raises ValueError, pyrove.normalize_or_zero hands it back

# Matrices
m = pyrove.mat4()
m.identity()
m.translate(1.0, 2.0, 3.0)
m.rotate(pyrove.vec3(0.0, 1.0, 0.0), 3.14159 / 4.0)

# Transform vectors
v_transformed = m * v1

# Quaternions
q = pyrove.quat(0.0, 0.0, 0.0, 1.0)
q.normalize()

# SLERP
q1 = pyrove.quat(0.0, 0.0, 0.0, 1.0)
q2 = pyrove.quat(0.0, 0.707, 0.0, 0.707)
q_mid = pyrove.slerp(q1, q2, 0.5)

# NumPy integration
arr = np.array([1.0, 2.0, 3.0], dtype=np.float32)
v = pyrove.vec3.from_numpy(arr)
arr_back = v.to_numpy()

# Matrix from NumPy
mat_array = np.eye(4, dtype=np.float32)
mat = pyrove.mat4.from_numpy(mat_array)
mat_back = mat.to_numpy()

# Double precision types
dv = pyrove.vec3d(1.0, 2.0, 3.0)
dm = pyrove.mat4d()
dq = pyrove.quatd(0.0, 0.0, 0.0, 1.0)

# Geometric primitives

# Rays for ray casting
ray = pyrove.ray3(pyrove.vec3(0, 2, 0), pyrove.vec3(0, -1, 0))
point_on_ray = ray.apply(5.0)  # Get point at t=5

# Line segments
line = pyrove.segment3(pyrove.vec3(0, 0, 0), pyrove.vec3(10, 0, 0))
distance = line.distance(pyrove.vec3(5, 3, 0))

# Planes with intersection tests
plane = pyrove.plane(pyrove.vec3(0, 0, 0), pyrove.vec3(0, 1, 0))
if plane.test_intersection(ray):
    t = plane.trace(ray)
    hit_point = ray.apply(t)

# Triangles with extensive geometric properties
tri = pyrove.triangle3(
    pyrove.vec3(0, 0, 0),
    pyrove.vec3(1, 0, 0),
    pyrove.vec3(0, 1, 0)
)
area = tri.area()
centroid = tri.cog()
circumcenter = tri.circumcenter()
if tri.trace(ray, 0.0, 100.0):
    print("Ray hits triangle!")

# Capsules for collision detection
capsule = pyrove.capsule3(
    pyrove.vec3(0, 0, 0),
    pyrove.vec3(0, 2, 0),
    0.5
)
if capsule.contains(pyrove.vec3(0, 1, 0.3)):
    print("Point inside capsule")

# Frustum culling
view_projection = pyrove.perspective_matrix(1.0, 16 / 9, 0.1, 100.0)
frustum = pyrove.frustum(view_projection)
bbox = pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1))
if frustum.test_intersection(bbox):
    print("Object is visible")
```

The free functions take vectors, matrices and quaternions of either precision and answer in it; those of plain numbers take and
give Python's floats (doubles), and `perspective_matrix` and `ortho_matrix` give a `mat4`.  What C++ throws as
`std::invalid_argument` is a `ValueError`.

```python
# scalar.h: radians, degrees, fract, lerp, inverse_lerp, remap, smoothstep, wrap, wrap_to_pi, wrap_to_two_pi, delta_angle
pyrove.delta_angle(3.0, -3.0)                         # 0.283: the short way, across the seam
x = pyrove.damp(x, target, 4.0, dt)                   # numbers, vec2 and vec3: the same end however dt is cut
x, v = pyrove.critically_damped(x, v, target, 6.0, dt)    # returns (value, velocity)

# vec.h: all_less (and _equal, all_greater...), min, max, abs, clamp, is_finite, project, reject, reflect, angle_between,
# signed_angle (2D, or 3D about an axis); vec4(vec3, w); vec4 * vec4
pyrove.signed_angle(a, b, pyrove.vec3(0, 0, 1))

# quaternion.h: q * v, q * q, rotate, inverse, rotation_matrix, to_quaternion, rotation_between, to_axis_angle (a tuple),
# rotation_vector, from_rotation_vector, slerp, nlerp
q = pyrove.rotation_between(pyrove.vec3(1, 0, 0), heading)

# matrix.h: transform_point, transform_vector, project_point (4x4 and 3x3), inverse, transpose, translation_matrix,
# scaling_matrix, rotation_matrix, look_at_matrix, perspective_matrix, ortho_matrix, linear_part, normal_matrix
p = pyrove.project_point(view_projection, world_point)    # divided by w: ValueError on the plane through the eye

# transform.h: trs and trsd (translation, rotation, scale; to_matrix, apply_point, apply_vector, a * b), to_trs, inverse, lerp,
# unproject_ray (a ray3 from the near plane to the far)
# aabb.h: aabb.is_empty(), aabb.grown(margin), bounds_of([points]), transform_aabb(box, m); frustum.test_intersection(sphere3)

# random.h, hash.h, noise.h: the same numbers as C++ from the same seed
rng = pyrove.pcg32(42)
rng.below(6), rng.unit(), rng.uniform(-1.0, 1.0), rng.normal(0.0, 1.0), rng.unit_vector()
rng.shuffle(items)                                     # in place
pyrove.value_noise(x, y, seed=7), pyrove.fbm(x, y, 4, 2.0, 0.5, 7), pyrove.splitmix64(k)

# octree.h: OctreeTriangle3.raycast_batch_hits(origins, directions, max_distance): per-ray origins ((N, 3) float32 in C order,
# as the directions), and each hit's (distance, item, payload, normal) as arrays (a miss: inf, -1, -1, NaN), without the GIL;
# raycast_batch(origin, directions, max_distance), one origin and distances only, is as it was
```

`abs`, `min` and `max` are rove's, on vectors (`pyrove.min(a, b)`): `from pyrove import *` leaves them out, so Python's own stay.

## C++

rove can be a git submodule, fetched by CMake, a Conan or vcpkg package, or installed from source; each way gives the
target `rove::rove`. [docs/install.md](https://github.com/mnvl/rove/blob/main/docs/install.md) has the steps for each. As a submodule:

```bash
git submodule add https://github.com/mnvl/rove external/rove
```

```cmake
add_subdirectory(external/rove)
target_link_libraries(my_app PRIVATE rove::rove)
```

```cpp
#include <rove/vec.h>
#include <rove/quaternion.h>
#include <rove/transform.h>
#include <rove/collision.h>
#include <rove/random.h>

using namespace rove;

// Vectors
vec<3> position(1, 2, 3), up(0, 1, 0);
float d = dot(position, up);
vec<3> side = cross(position, up);
vec<3> unit = direction(position);              // throws for a vector of no length (or too short to tell from rounding)
float turn = signed_angle(vec<3>(1, 0, 0), vec<3>(0, 0, -1), up);

// Transforms
matrix<4, 4> world = translation_matrix(vec<3>(1, 2, 3)) * rotation_matrix(up, PI / 4);
vec<3> moved = transform_point(world, position);          // w = 1, no divide: what world * position is
matrix<4, 4> projection = perspective_matrix(1.0f, 16.0f / 9, 0.1f, 100.0f);
vec<3> ndc = project_point(projection, moved);            // divided by w

trs<float> joint(vec<3>(0, 1, 0), axis_angle(up, 0.5f), vec<3>(1, 1, 1));
matrix<4, 4> m = joint.to_matrix();
trs<float> back = to_trs(m);

// Quaternions
quaternion<float> q = rotation_between(vec<3>(1, 0, 0), vec<3>(0, 0, 1));
vec<3> turned = q * vec<3>(1, 0, 0);
quaternion<float> halfway = slerp(quaternion<float>(), q, 0.5f);

// Smoothing that does not depend on the frame rate
speed = damp(speed, 10.0f, 4.0f, dt);      // speed: a float kept from frame to frame

// Random numbers, the same on every machine
pcg32 rng(seed);
float x = rng.uniform(-1.0f, 1.0f);
std::uint32_t card = rng.below(52);

// Collision detection
sphere<3, float> s1(vec<3>(0, 0, 0), 1.0f);
sphere<3, float> s2(vec<3>(1.5f, 0, 0), 1.0f);
// s2 moves by (-1, 0, 0) over the step; the touch, if any, comes back as a contact_info
std::optional<contact_info<3, float>> contact = first_contact(s1, s2, vec<3>(-1, 0, 0));
```

## Benchmarks

`examples/benchmark.py` times pyrove against NumPy, [eigenpy](https://github.com/stack-of-tasks/eigenpy) and
[minieigen](https://github.com/eudoxos/minieigen) from Python, on the same double-precision inputs, each library in a process of its
own and in the fastest form found for it. All must give the same answers before any time is printed, and one that cannot be imported
stops the run; `--libs pyrove,numpy` leaves the others out.

```bash
pip install numpy eigenpy
sudo apt install python3-minieigen   # Debian and Ubuntu; not on PyPI, and a venv needs --system-site-packages
python3 examples/benchmark.py
pytest -n 4 --timeout=20 examples/test_benchmark.py   # the checks on the benchmark itself
```


## Architecture

The library's sources and headers are in `src/` under the `rove` namespace; `include/rove/` forwards to them, `python/` has the
bindings, `tests/` the C++ (Boost.Test) and Python (pytest) tests.

### Conventions
- **Names**: a type names its parts as the standard library does: `<class>_type` aliases (`vec_type`, `aabb_type`,
  `scalar_type`, `segment_type`; an index's `primitive_type`, `payload_type`, `index_type`, `contact_type`), lower case constants
  (`dimensions`, a matrix's `columns` and `rows`, `obb::vertices_count`, `octree::subclusters_count`) and enumerators
  (`plane::positive`, `frustum::left_plane`).  Accessors have no `get_` (`bounds()`, `normal()`, `vertex(i)`, `centre()`); what
  makes another type is `to_` (`to_ray()`, `to_plane()`).  One name for each thing: `dot`, `cross`, `m * v`, `segment`
- **Conversions between shapes are explicit**: `aabb(centre)`, `sphere(box)`, `obb(box)`, `frustum(matrix)`; a function taking a
  sphere does not take a box and make one of it
- **Structured bindings**: `auto [x, y, z] = v;` for any vector (`auto &` writes through); a quaternion's `x, y, z, w` too
- Vectors are their components and nothing else, aligned as the scalar (`sizeof(vec<3>) == 12`); unions name them (`x`, `r`, `i[]`)
- Matrices are column-major (`ij[column][row]`, GLSL's and glTF's layout); vectors are columns: `a * b` applies `b` first
- **Defaults**: `vec()` is zero, `matrix()` the identity, `quaternion()` the identity, `aabb()` empty (`is_empty()`: it contains
  nothing and the first `extend()` makes it that point), `sphere()`, `capsule()`, `plane()`, `cone()`, `interval()` zero, `trs()`
  no transform.
- **Projections** (`perspective`, `ortho`, `perspective_matrix`, `ortho_matrix`): view space looks along +Z; depth runs from 0 at the
  near plane to 1 at the far (Vulkan's and Direct3D's range, not OpenGL's -1..1); y up.  `frustum` takes its near plane at depth 0.
- **`lookat()` is a mirror**: forward = at - eye, right = forward x up, up' = right x forward.  Looking along world +Z with +Y up,
  world +X lands on view -X.  A right-handed world (glTF's) seen through it and edge's renderer comes out as it was authored (edge's
  README, Coordinate System).  It is not `D3DXMatrixLookAtLH`, whose right is up x forward.
- **Euler angles**: `rotation(alpha, beta, gamma)` is Rz(gamma) Ry(beta) Rx(alpha): a turn about x first, then y, then z.
- **A point through a matrix**: `m * p` of a `mat4` and a `vec3` (and `transform_point`) is affine: w taken as 1 and never divided by (a
  `mat3` and a `vec2` is different: a homogeneous 2D point, divided by the third component; Python's `get(i, j)` is [column][row], as the
  matrix is stored, and `to_numpy()[i, j]` is [row][column], as numpy reads it; and the setters `m.translate(...); m.scale(...)` each
  multiply on the right of what they are told to do first, S x T, where `m *= T; m *= S` is T x S); `project_point` divides
  by the w that comes out.  `transform_vector` leaves out the translation.
- **What has no answer throws** `std::invalid_argument` rather than returning a plausible value: `direction()` of a zero vector,
  `normalize()` of a zero vector (`normalize_or_zero()` hands it back) or quaternion, `inverse()` of a singular matrix,
  `project_point` where w is zero, an angle with a zero vector, `to_trs` of a sheared matrix, a turn about an axis of no length (`rotation`, `rotate`, `rotation_matrix`: it was a matrix
  of NaN), `ray::unit` of a direction of no length, `heightfield::set_local_to_world` of a singular matrix, `lookat` from an
  eye at its target and `matrix::direction` along a zero vector.  Inside rove, where a zero vector comes of valid input (the
  normal of a triangle of no area, spheres of one centre, a view straight up), the answer is what it always was.
- **Checks**: what rove checks, a caller checks the same way, and each throws `std::invalid_argument` with `what` first and returns
  the object, so it chains (`axis.check_nonzero("a hinge axis").length()`):
  - `vec`: `check_finite(what)` ("… that is not finite"), `check_nonzero(what, epsilon = zero_length)` ("… of length 0")
  - `matrix`: `check_finite(what)`, `check_invertible(what)` ("… of determinant 0"), and for 4x4 `check_affine(what)` ("… whose last
    row is not (0, 0, 0, 1)")
  - `aabb`, `ray`: `check_finite(what)`
- **dfloat**: every template takes `dfloat` for T; the functions reach its own `sqrt`, `sin`, `atan2`, `fmod` by argument lookup
  (`using std::sin; sin(x)`), never `std::` directly.
- **Bits**: a fix to a function changes only what it answers for inputs that were wrong (a zero quaternion, an uninitialised box);
  for a valid input it keeps its bits, so a lockstep simulation's recordings play back across versions.
- `first_contact` returns `std::optional<contact_info>`: the first touch, or empty when there is none
- **`[[nodiscard]]`**: the value types (vectors, matrices, quaternions, `trs`, the shapes, `contact_info`), `first_contact`,
  `matrix::inverse(M)` and the `trace` functions: a result thrown away is a bug (`normalize(v);` where `v.normalize();` was meant,
  an `inverse(M)` of a singular matrix unnoticed)
- **Concepts** check what a template is given, so a wrong one is a single error naming what is missing: `spatial_traits_for` (an
  index's `Traits`), `spatial_query_traits_for` (a query's), `trace_test` (`trace`'s test), `a_star_scorer` (`a_star`'s scorer)

### Known traps (kept as they are)
- `heightfield::y_under` and `max_y_in_cell` answer their `default_value` off the field
- `plane::trace(r, t, t_min = 0, t_max = 1)` defaults to a segment of the ray, `ray::distance` measures to the whole line
- `matrix::direction` along a vector straight up or down, and `lookat` with `up` along the view, give zero axes: there is no
  side to take

## Documentation

The headers' comments (`///`, which an IDE shows) and this README. [CHANGELOG.md](https://github.com/mnvl/rove/blob/main/CHANGELOG.md) lists what changed between
versions.

## API Reference

### Vectors (`vec.h`)
- `length`, `length_sq`, `manhattan_length`, `distance`, `distance_sq`, `dot`, `cross`, `normalize` (throws for no length or not
  finite), `normalize_or_zero`, `direction` (throws for `zero_length` or less, or not finite; for both, a long vector whose square
  overflows is scaled first), `lerp`
- `all_less`, `all_less_equal`, `all_greater`, `all_greater_equal`; component-wise `min`, `max`, `abs`, `clamp`; `is_finite`;
  `v.check_finite(what)`, `v.check_nonzero(what)` (and `check_finite` of an `aabb` or a `ray`)
- `project`, `reject`, `reflect`, `angle_between`, `signed_angle`, `move_towards`
- `to_xz`, `from_xz`, `horizontal_length`; `vec<4>(vec<3>, w)`, `xyz()`, `xy()`
- `std::less` (lexicographic) and `std::hash` of any vector

### Scalars (`scalar.h`)
- `lerp`, `inverse_lerp`, `remap`, `smoothstep`, `clamp`, `fract`, `radians`, `degrees`
- `wrap`, `wrap_to_pi`, `wrap_to_two_pi`, `delta_angle` (float, double and dfloat)
- `damp` (exponential, the same however the time is cut into steps), `move_towards`, `critically_damped` (an exact spring step)

### Matrices (`matrix.h`)
- Setters: `identity()`, `zero()`, `translation()`, `translate()`, `rotation()`, `rotate()`, `scaling()`, `scale()`, `lookat()`,
  `perspective()`, `ortho()`; `inverse(M)`, `transpose(M)`, `determinant()`, `trace()`; `check_finite`, `check_invertible`,
  `check_affine` (4x4)
- By value: `translation_matrix`, `scaling_matrix`, `rotation_matrix` (axis and angle, or a quaternion), `look_at_matrix`,
  `perspective_matrix`, `ortho_matrix` (off centre), `inverse(m)` (throws for a singular matrix), `transpose(m)`, `linear_part`,
  `normal_matrix`
- `transform_point`, `transform_vector`, `project_point` (2D and 3D)

### Quaternions (`quaternion.h`)
- `identity()`, `norm()`, `normalize()` (throws for no length), `set_unit(matrix)`, `to_quaternion(matrix)`
- `q * v` and `rotate(q, v)`, `conjugate`, `inverse`, `axis_angle`, `to_axis_angle`, `rotation_vector`, `from_rotation_vector`
- `slerp`, `nlerp`, `quaternion_slerper`, `power`, `angle_between`, `rotation_between`

### Transforms (`transform.h`)
- `trs<T>`: `to_matrix()`, `apply_point()`, `apply_vector()`; `to_trs(matrix)`, `a * b`, `inverse(t)`, `lerp`
- `unproject_ray(inverse_view_projection, ndc_x, ndc_y)`

### Bounds
- `aabb`: `extend`, `contains`, `test_intersection`, `closest_point`, `is_empty`, `grown`; `transform_aabb(box, matrix)`,
  `bounds_of(first, last)`
- `frustum`: `contains` (a point), `test_intersection` (an AABB, an OBB, a sphere)

### Spatial indices (`octree.h`, `hash_grid.h`)
- `octree<Primitive, Payload>`: `insert`, `build`, `is_built`; `query_intersection` (payloads), `query_intersection_items`,
  `query_indices` (`operator[]`'s indices); `trace` (the nearest hit, a `contact_info`); `save` and `load`
- `hash_grid<Primitive, Payload>`: the same queries and `trace`, with inserts at any time and no `build`

### Randomness and noise
- `pcg32`: `next`, `below`, `unit`, `uniform`, `chance`, `normal`, `unit_vector`, `shuffle`, `weighted`
- `splitmix64`, `hash_combine`, `hash_to_unit`; `value_noise` (1, 2, 3 dimensions), `fbm`

## Type Aliases

### C++
rove declares no short names: `vec<3>` is `vec<3, float>` (`scalar` is float), `matrix<4, 4>` and `quaternion<>` likewise, and a
project names its own (`using vec3 = rove::vec<3>;`).  The colours (`rgb<T>`, `bgra<T>` ...) and the ground `xz<T>` are aliases of
`vec`.

### Python
- **Vectors**: `vec2`, `vec3`, `vec4` (float), `vec2d`, `vec3d`, `vec4d` (double)
- **Matrices**: `mat3`, `mat4` (float), `mat3d`, `mat4d` (double)
- **Quaternions**: `quat` (float), `quatd` (double)
- **Transforms**: `trs` (float), `trsd` (double)
- **Random numbers**: `pcg32`
- **Rays**: `ray2`, `ray3` (float), `ray2d`, `ray3d` (double)
- **Segments**: `segment2`, `segment3` (float), `segment2d`, `segment3d` (double)
- **Planes**: `plane` (float), `planed` (double)
- **Triangles**: `triangle3` (float), `triangle3d` (double)
- **Capsules**: `capsule2`, `capsule3` (float), `capsule2d`, `capsule3d` (double)
- **Spheres**: `sphere2`, `sphere3` (float), `sphere2d`, `sphere3d` (double)
- **AABB**: `aabb2`, `aabb3` (float), `aabb2d`, `aabb3d` (double)
- **OBB**: `obb2`, `obb3` (float), `obb2d`, `obb3d` (double)
- **Frustums**: `frustum` (float), `frustumd` (double)
- **Spatial**: `OctreeTriangle3` (float)

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `ROVE_TESTS` | on when rove is built by itself | `rove_tests` (Boost.Test), and pyrove's pytest suite with `ROVE_PYTHON` |
| `ROVE_PYTHON` | off | the nanobind module `pyrove_bind` |
| `ROVE_DFLOAT` | off | `rove::dfloat` and rove's instantiations for it; adds `-ffp-contract=off` to rove and whoever links it |
| `ROVE_LTO` | on when rove is built by itself | link-time optimisation of rove in Release and RelWithDebInfo; added by a project, rove leaves it (and the build type) to it |
| `ROVE_WARNINGS` | on when rove is built by itself | compiler warnings for rove's own sources |
| `ROVE_INSTALL` | on when rove is built by itself | install rules and the CMake package (`find_package(rove)`) |

Building and testing:

```bash
cmake -S . -B build -DROVE_PYTHON=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The tests need Boost.Test; the Python tests need pytest and NumPy.

## License

MIT; see [LICENSE](https://github.com/mnvl/rove/blob/main/LICENSE.md).

## Contributing

See [CONTRIBUTING.md](https://github.com/mnvl/rove/blob/main/CONTRIBUTING.md).
