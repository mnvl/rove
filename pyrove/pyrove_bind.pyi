from collections.abc import Iterator, Sequence
from typing import Annotated, overload

from numpy.typing import ArrayLike


class OctreeTriangle3:
    """
    A static octree of triangles of float32, each with an int payload: insert, then build, then query
    """

    def __init__(self, bounds: aabb3, leaf_items_threshold: int = 16, min_subcluster_diagonal_sq: float = 9.999999747378752e-06) -> None: ...

    def insert(self, triangle: triangle3, payload: int) -> None:
        """Adds an item; other threads' use of the tree waits for it"""

    def build(self) -> None:
        """
        Builds the tree, letting other Python threads run meanwhile: one that uses this tree waits until it returns.  ValueError for items in a box that is empty or not finite
        """

    @property
    def size(self) -> int: ...

    def __repr__(self) -> str: ...

    @property
    def bounds(self) -> aabb3: ...

    def __getstate__(self) -> bytes: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __setstate__(self, state: bytes) -> None: ...

    def query_aabb(self, bounds: aabb3) -> list[int]: ...

    def query_sphere(self, sphere: sphere3) -> list[int]: ...

    def query_aabb_items(self, bounds: aabb3) -> list[tuple[triangle3, int]]: ...

    def query_sphere_items(self, sphere: sphere3) -> list[tuple[triangle3, int]]: ...

    def query_line_items(self, line: segment3) -> list[tuple[triangle3, int]]:
        """
        Triangles the segment `line` (A to B) actually hits, exact (not an AABB approximation), tree-pruned.
        """

    def raycast_batch(self, origin: vec3, directions: Annotated[ArrayLike, dict(dtype='float32', shape=(None, None), order='C', device='cpu', writable=False)], max_distance: float) -> Annotated[ArrayLike, dict(dtype='float32')]:
        """
        Nearest-hit distance for every ray in `directions` (shape (N,3), of any length, read-only or not; numbers of another type are rounded to float32), from a shared `origin`, within max_distance (may be inf; ValueError if negative). One octree traversal per ray, all looped in C++ -- a single Python call per octree per frame. Returns an (N,) float array; inf where a ray hits nothing within max_distance or has no direction. Other Python threads run while it traces (none may change the directions meanwhile; one that changes the tree waits).
        """

    def raycast_batch_hits(self, origins: Annotated[ArrayLike, dict(dtype='float32', shape=(None, 3), order='C', device='cpu', writable=False)], directions: Annotated[ArrayLike, dict(dtype='float32', shape=(None, 3), order='C', device='cpu', writable=False)], max_distance: float) -> tuple:
        """
        The nearest hit of every ray, origins[i] towards directions[i] (both (N, 3) float32 in C order; a direction of any length but none), within max_distance (may be inf), traced in C++ with other Python threads running. Returns (distance (N,) float32, item (N,) int64: the triangle's index in the tree, payload (N,) int32, normal (N, 3) float32: unit, facing against the ray). A miss is distance inf, item and payload -1 and a NaN normal. ValueError naming a ray that is no ray (NaN, infinite, no length), for a negative max_distance or arrays of different lengths; TypeError for arrays of another type, shape or order. raycast_batch, one origin and distances only, is unchanged.
        """

class SingularMatrixError(ValueError, RuntimeError):
    """
    A matrix with no inverse: a ValueError, as every other refusal, and a RuntimeError, as Mat.inverse() raised before
    """

class a_star:
    """A* over a grid whose cells a Python function scores"""

    def __init__(self, start_x: int, start_y: int, goal_x: int, goal_y: int, weights: Annotated[ArrayLike, dict(dtype='float32', shape=(None, None), order='C', device='cpu')]) -> None: ...

    def calculate_path(self) -> None: ...

    def calculate_cost_map(self) -> None: ...

    def have_failed(self) -> bool: ...

    def build_path(self) -> list: ...

    def build_cost_map(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(None, None))]: ...

class aabb2:
    """An axis-aligned rectangle of float32"""

    @overload
    def __init__(self) -> None:
        """
        An empty box: it contains nothing, and the first extend() makes it that point
        """

    @overload
    def __init__(self, centre: vec2) -> None:
        """Construct degenerate AABB from single point"""

    @overload
    def __init__(self, lo: vec2, hi: vec2) -> None:
        """Construct AABB from two corner points"""

    @overload
    def __init__(self, centre: vec2, side_length: float) -> None:
        """
        Construct a cube AABB from its centre and full side length (centre +- side_length / 2 on every axis)
        """

    def is_empty(self) -> bool:
        """
        Whether the box holds no point: lo past hi on some axis (a box of a single point is not empty)
        """

    def grown(self, margin: float) -> aabb2:
        """
        The box with every side moved out by margin (in by a negative one); an empty box stays empty, and one shrunk past nothing is empty
        """

    @property
    def lo(self) -> vec2:
        """Minimum corner (lower bounds)"""

    @lo.setter
    def lo(self, arg: vec2, /) -> None: ...

    @property
    def hi(self) -> vec2:
        """Maximum corner (upper bounds)"""

    @hi.setter
    def hi(self, arg: vec2, /) -> None: ...

    def null(self) -> None:
        """Reset AABB to null state for accumulation"""

    def normalize(self) -> None:
        """Ensure lo <= hi in all dimensions"""

    def diagonal(self) -> vec2:
        """Get AABB diagonal vector (hi - lo)"""

    def centre(self) -> vec2:
        """Get AABB center point"""

    @overload
    def extend(self, point: vec2) -> None:
        """Extend AABB to include a point"""

    @overload
    def extend(self, other: aabb2) -> None:
        """Extend AABB to include another AABB"""

    @overload
    def contains(self, point: vec2) -> bool:
        """Test if point is inside AABB"""

    @overload
    def contains(self, other: aabb2) -> bool:
        """Test if another AABB is fully contained"""

    def test_intersection(self, other: aabb2) -> bool:
        """Test intersection with another AABB"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class aabb2d:
    """An axis-aligned rectangle of float64"""

    @overload
    def __init__(self) -> None:
        """
        An empty box: it contains nothing, and the first extend() makes it that point
        """

    @overload
    def __init__(self, centre: vec2d) -> None:
        """Construct degenerate AABB from single point"""

    @overload
    def __init__(self, lo: vec2d, hi: vec2d) -> None:
        """Construct AABB from two corner points"""

    @overload
    def __init__(self, centre: vec2d, side_length: float) -> None:
        """
        Construct a cube AABB from its centre and full side length (centre +- side_length / 2 on every axis)
        """

    def is_empty(self) -> bool:
        """
        Whether the box holds no point: lo past hi on some axis (a box of a single point is not empty)
        """

    def grown(self, margin: float) -> aabb2d:
        """
        The box with every side moved out by margin (in by a negative one); an empty box stays empty, and one shrunk past nothing is empty
        """

    @property
    def lo(self) -> vec2d:
        """Minimum corner (lower bounds)"""

    @lo.setter
    def lo(self, arg: vec2d, /) -> None: ...

    @property
    def hi(self) -> vec2d:
        """Maximum corner (upper bounds)"""

    @hi.setter
    def hi(self, arg: vec2d, /) -> None: ...

    def null(self) -> None:
        """Reset AABB to null state for accumulation"""

    def normalize(self) -> None:
        """Ensure lo <= hi in all dimensions"""

    def diagonal(self) -> vec2d:
        """Get AABB diagonal vector (hi - lo)"""

    def centre(self) -> vec2d:
        """Get AABB center point"""

    @overload
    def extend(self, point: vec2d) -> None:
        """Extend AABB to include a point"""

    @overload
    def extend(self, other: aabb2d) -> None:
        """Extend AABB to include another AABB"""

    @overload
    def contains(self, point: vec2d) -> bool:
        """Test if point is inside AABB"""

    @overload
    def contains(self, other: aabb2d) -> bool:
        """Test if another AABB is fully contained"""

    def test_intersection(self, other: aabb2d) -> bool:
        """Test intersection with another AABB"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class aabb3:
    """An axis-aligned box of float32"""

    @overload
    def __init__(self) -> None:
        """
        An empty box: it contains nothing, and the first extend() makes it that point
        """

    @overload
    def __init__(self, centre: vec3) -> None:
        """Construct degenerate AABB from single point"""

    @overload
    def __init__(self, lo: vec3, hi: vec3) -> None:
        """Construct AABB from two corner points"""

    @overload
    def __init__(self, centre: vec3, side_length: float) -> None:
        """
        Construct a cube AABB from its centre and full side length (centre +- side_length / 2 on every axis)
        """

    def is_empty(self) -> bool:
        """
        Whether the box holds no point: lo past hi on some axis (a box of a single point is not empty)
        """

    def grown(self, margin: float) -> aabb3:
        """
        The box with every side moved out by margin (in by a negative one); an empty box stays empty, and one shrunk past nothing is empty
        """

    @property
    def lo(self) -> vec3:
        """Minimum corner (lower bounds)"""

    @lo.setter
    def lo(self, arg: vec3, /) -> None: ...

    @property
    def hi(self) -> vec3:
        """Maximum corner (upper bounds)"""

    @hi.setter
    def hi(self, arg: vec3, /) -> None: ...

    def null(self) -> None:
        """Reset AABB to null state for accumulation"""

    def normalize(self) -> None:
        """Ensure lo <= hi in all dimensions"""

    def diagonal(self) -> vec3:
        """Get AABB diagonal vector (hi - lo)"""

    def centre(self) -> vec3:
        """Get AABB center point"""

    @overload
    def extend(self, point: vec3) -> None:
        """Extend AABB to include a point"""

    @overload
    def extend(self, other: aabb3) -> None:
        """Extend AABB to include another AABB"""

    @overload
    def contains(self, point: vec3) -> bool:
        """Test if point is inside AABB"""

    @overload
    def contains(self, other: aabb3) -> bool:
        """Test if another AABB is fully contained"""

    def test_intersection(self, other: aabb3) -> bool:
        """Test intersection with another AABB"""

    def trace(self, ray: ray3, t_min: float = 0.0, t_max: float = 1.0) -> object:
        """
        Ray-AABB intersection. Returns (t_enter, t_exit) tuple of ray-parameter times, or None if the ray misses the AABB in [t_min, t_max].
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class aabb3d:
    """An axis-aligned box of float64"""

    @overload
    def __init__(self) -> None:
        """
        An empty box: it contains nothing, and the first extend() makes it that point
        """

    @overload
    def __init__(self, centre: vec3d) -> None:
        """Construct degenerate AABB from single point"""

    @overload
    def __init__(self, lo: vec3d, hi: vec3d) -> None:
        """Construct AABB from two corner points"""

    @overload
    def __init__(self, centre: vec3d, side_length: float) -> None:
        """
        Construct a cube AABB from its centre and full side length (centre +- side_length / 2 on every axis)
        """

    def is_empty(self) -> bool:
        """
        Whether the box holds no point: lo past hi on some axis (a box of a single point is not empty)
        """

    def grown(self, margin: float) -> aabb3d:
        """
        The box with every side moved out by margin (in by a negative one); an empty box stays empty, and one shrunk past nothing is empty
        """

    @property
    def lo(self) -> vec3d:
        """Minimum corner (lower bounds)"""

    @lo.setter
    def lo(self, arg: vec3d, /) -> None: ...

    @property
    def hi(self) -> vec3d:
        """Maximum corner (upper bounds)"""

    @hi.setter
    def hi(self, arg: vec3d, /) -> None: ...

    def null(self) -> None:
        """Reset AABB to null state for accumulation"""

    def normalize(self) -> None:
        """Ensure lo <= hi in all dimensions"""

    def diagonal(self) -> vec3d:
        """Get AABB diagonal vector (hi - lo)"""

    def centre(self) -> vec3d:
        """Get AABB center point"""

    @overload
    def extend(self, point: vec3d) -> None:
        """Extend AABB to include a point"""

    @overload
    def extend(self, other: aabb3d) -> None:
        """Extend AABB to include another AABB"""

    @overload
    def contains(self, point: vec3d) -> bool:
        """Test if point is inside AABB"""

    @overload
    def contains(self, other: aabb3d) -> bool:
        """Test if another AABB is fully contained"""

    def test_intersection(self, other: aabb3d) -> bool:
        """Test intersection with another AABB"""

    def trace(self, ray: ray3d, t_min: float = 0.0, t_max: float = 1.0) -> object:
        """
        Ray-AABB intersection. Returns (t_enter, t_exit) tuple of ray-parameter times, or None if the ray misses the AABB in [t_min, t_max].
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

@overload
def abs(v: vec2) -> vec2:
    """Component by component"""

@overload
def abs(v: vec3) -> vec3: ...

@overload
def abs(v: vec4) -> vec4: ...

@overload
def abs(v: vec2d) -> vec2d: ...

@overload
def abs(v: vec3d) -> vec3d: ...

@overload
def abs(v: vec4d) -> vec4d: ...

@overload
def all_greater(a: vec2, b: vec2) -> bool: ...

@overload
def all_greater(a: vec3, b: vec3) -> bool: ...

@overload
def all_greater(a: vec4, b: vec4) -> bool: ...

@overload
def all_greater(a: vec2d, b: vec2d) -> bool: ...

@overload
def all_greater(a: vec3d, b: vec3d) -> bool: ...

@overload
def all_greater(a: vec4d, b: vec4d) -> bool: ...

@overload
def all_greater_equal(a: vec2, b: vec2) -> bool: ...

@overload
def all_greater_equal(a: vec3, b: vec3) -> bool: ...

@overload
def all_greater_equal(a: vec4, b: vec4) -> bool: ...

@overload
def all_greater_equal(a: vec2d, b: vec2d) -> bool: ...

@overload
def all_greater_equal(a: vec3d, b: vec3d) -> bool: ...

@overload
def all_greater_equal(a: vec4d, b: vec4d) -> bool: ...

@overload
def all_less(a: vec2, b: vec2) -> bool:
    """Whether every component of a is less than b's"""

@overload
def all_less(a: vec3, b: vec3) -> bool: ...

@overload
def all_less(a: vec4, b: vec4) -> bool: ...

@overload
def all_less(a: vec2d, b: vec2d) -> bool: ...

@overload
def all_less(a: vec3d, b: vec3d) -> bool: ...

@overload
def all_less(a: vec4d, b: vec4d) -> bool: ...

@overload
def all_less_equal(a: vec2, b: vec2) -> bool: ...

@overload
def all_less_equal(a: vec3, b: vec3) -> bool: ...

@overload
def all_less_equal(a: vec4, b: vec4) -> bool: ...

@overload
def all_less_equal(a: vec2d, b: vec2d) -> bool: ...

@overload
def all_less_equal(a: vec3d, b: vec3d) -> bool: ...

@overload
def all_less_equal(a: vec4d, b: vec4d) -> bool: ...

@overload
def angle_between(a: vec2, b: vec2) -> float:
    """
    The angle between two directions, 0 to pi, exact for tiny ones; ValueError for a vector of no length
    """

@overload
def angle_between(a: vec3, b: vec3) -> float: ...

@overload
def angle_between(a: vec2d, b: vec2d) -> float: ...

@overload
def angle_between(a: vec3d, b: vec3d) -> float: ...

@overload
def axis_angle(axis: vec3, angle: float) -> quat:
    """
    The unit quaternion turning by angle (radians) about an axis of any length; ValueError for an axis of no length
    """

@overload
def axis_angle(axis: vec3d, angle: float) -> quatd: ...

@overload
def bounds_of(points: Sequence[vec3]) -> aabb3:
    """The box of the points; empty for none"""

@overload
def bounds_of(points: Sequence[vec2]) -> aabb2: ...

@overload
def bounds_of(points: Sequence[vec3d]) -> aabb3d: ...

@overload
def bounds_of(points: Sequence[vec2d]) -> aabb2d: ...

class capsule2:
    """A 2D capsule of float32: a segment and a radius around it"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec2, b: vec2, radius: float) -> None:
        """Construct capsule from two endpoints and radius"""

    @property
    def axe(self) -> segment2:
        """Central axis line segment of the capsule"""

    @axe.setter
    def axe(self, arg: segment2, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the capsule"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def contains(self, point: vec2) -> bool:
        """Test if point is inside the capsule"""

    def distance(self, point: vec2) -> float:
        """Get distance from capsule surface to point (0 if inside)"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class capsule2d:
    """A 2D capsule of float64: a segment and a radius around it"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec2d, b: vec2d, radius: float) -> None:
        """Construct capsule from two endpoints and radius"""

    @property
    def axe(self) -> segment2d:
        """Central axis line segment of the capsule"""

    @axe.setter
    def axe(self, arg: segment2d, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the capsule"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def contains(self, point: vec2d) -> bool:
        """Test if point is inside the capsule"""

    def distance(self, point: vec2d) -> float:
        """Get distance from capsule surface to point (0 if inside)"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class capsule3:
    """A 3D capsule of float32: a segment and a radius around it"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec3, b: vec3, radius: float) -> None:
        """Construct capsule from two endpoints and radius"""

    @property
    def axe(self) -> segment3:
        """Central axis line segment of the capsule"""

    @axe.setter
    def axe(self, arg: segment3, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the capsule"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def contains(self, point: vec3) -> bool:
        """Test if point is inside the capsule"""

    def distance(self, point: vec3) -> float:
        """Get distance from capsule surface to point (0 if inside)"""

    @overload
    def test_intersection(self, triangle: triangle3) -> bool:
        """Test if capsule intersects a triangle"""

    @overload
    def test_intersection(self, other: capsule3) -> bool:
        """Test if capsule intersects another capsule"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class capsule3d:
    """A 3D capsule of float64: a segment and a radius around it"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec3d, b: vec3d, radius: float) -> None:
        """Construct capsule from two endpoints and radius"""

    @property
    def axe(self) -> segment3d:
        """Central axis line segment of the capsule"""

    @axe.setter
    def axe(self, arg: segment3d, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the capsule"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def contains(self, point: vec3d) -> bool:
        """Test if point is inside the capsule"""

    def distance(self, point: vec3d) -> float:
        """Get distance from capsule surface to point (0 if inside)"""

    @overload
    def test_intersection(self, triangle: triangle3d) -> bool:
        """Test if capsule intersects a triangle"""

    @overload
    def test_intersection(self, other: capsule3d) -> bool:
        """Test if capsule intersects another capsule"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

@overload
def clamp(v: vec2, lo: vec2, hi: vec2) -> vec2:
    """Each component between lo's and hi's"""

@overload
def clamp(v: vec2, lo: float, hi: float) -> vec2:
    """Each component between lo and hi"""

@overload
def clamp(v: vec3, lo: vec3, hi: vec3) -> vec3:
    """Each component between lo's and hi's"""

@overload
def clamp(v: vec3, lo: float, hi: float) -> vec3:
    """Each component between lo and hi"""

@overload
def clamp(v: vec4, lo: vec4, hi: vec4) -> vec4:
    """Each component between lo's and hi's"""

@overload
def clamp(v: vec4, lo: float, hi: float) -> vec4:
    """Each component between lo and hi"""

@overload
def clamp(v: vec2d, lo: vec2d, hi: vec2d) -> vec2d:
    """Each component between lo's and hi's"""

@overload
def clamp(v: vec2d, lo: float, hi: float) -> vec2d:
    """Each component between lo and hi"""

@overload
def clamp(v: vec3d, lo: vec3d, hi: vec3d) -> vec3d:
    """Each component between lo's and hi's"""

@overload
def clamp(v: vec3d, lo: float, hi: float) -> vec3d:
    """Each component between lo and hi"""

@overload
def clamp(v: vec4d, lo: vec4d, hi: vec4d) -> vec4d:
    """Each component between lo's and hi's"""

@overload
def clamp(v: vec4d, lo: float, hi: float) -> vec4d:
    """Each component between lo and hi"""

@overload
def conjugate(q: quat) -> quat:
    """The quaternion with x, y and z negated"""

@overload
def conjugate(q: quatd) -> quatd: ...

@overload
def critically_damped(value: float, velocity: float, target: float, omega: float, dt: float) -> tuple[float, float]:
    """
    One step of dt of a critically damped spring, its exact solution: (value, velocity) after it. ValueError for a negative omega or dt. For numbers and vectors.
    """

@overload
def critically_damped(value: vec2, velocity: vec2, target: vec2, omega: float, dt: float) -> tuple[vec2, vec2]: ...

@overload
def critically_damped(value: vec3, velocity: vec3, target: vec3, omega: float, dt: float) -> tuple[vec3, vec3]: ...

@overload
def critically_damped(value: vec2d, velocity: vec2d, target: vec2d, omega: float, dt: float) -> tuple[vec2d, vec2d]: ...

@overload
def critically_damped(value: vec3d, velocity: vec3d, target: vec3d, omega: float, dt: float) -> tuple[vec3d, vec3d]: ...

@overload
def cross(a: vec3, b: vec3) -> vec3:
    """The cross product"""

@overload
def cross(a: vec3d, b: vec3d) -> vec3d: ...

@overload
def damp(current: float, target: float, rate: float, dt: float) -> float:
    """
    current moved towards target by the share 1 - exp(-rate dt): however dt is cut up, the same place; never past the target. ValueError for a negative rate or dt. For numbers and vectors.
    """

@overload
def damp(current: vec2, target: vec2, rate: float, dt: float) -> vec2: ...

@overload
def damp(current: vec3, target: vec3, rate: float, dt: float) -> vec3: ...

@overload
def damp(current: vec2d, target: vec2d, rate: float, dt: float) -> vec2d: ...

@overload
def damp(current: vec3d, target: vec3d, rate: float, dt: float) -> vec3d: ...

def degrees(radians: float) -> float:
    """Radians to degrees"""

def delta_angle(from_: float, to: float) -> float:
    """The turn (-pi, pi] from one angle to another, the short way round"""

@overload
def distance(a: vec2, b: vec2) -> float: ...

@overload
def distance(a: vec3, b: vec3) -> float: ...

@overload
def distance(a: vec4, b: vec4) -> float: ...

@overload
def distance(a: vec2d, b: vec2d) -> float: ...

@overload
def distance(a: vec3d, b: vec3d) -> float: ...

@overload
def distance(a: vec4d, b: vec4d) -> float: ...

@overload
def distance_sq(a: vec2, b: vec2) -> float: ...

@overload
def distance_sq(a: vec3, b: vec3) -> float: ...

@overload
def distance_sq(a: vec4, b: vec4) -> float: ...

@overload
def distance_sq(a: vec2d, b: vec2d) -> float: ...

@overload
def distance_sq(a: vec3d, b: vec3d) -> float: ...

@overload
def distance_sq(a: vec4d, b: vec4d) -> float: ...

@overload
def dot(a: vec2, b: vec2) -> float:
    """The dot product"""

@overload
def dot(a: vec3, b: vec3) -> float: ...

@overload
def dot(a: vec4, b: vec4) -> float: ...

@overload
def dot(a: vec2d, b: vec2d) -> float: ...

@overload
def dot(a: vec3d, b: vec3d) -> float: ...

@overload
def dot(a: vec4d, b: vec4d) -> float: ...

@overload
def dot(p: quat, q: quat) -> float: ...

@overload
def dot(p: quatd, q: quatd) -> float: ...

def fbm(x: float, y: float, octaves: int, lacunarity: float, gain: float, seed: int) -> float:
    """
    Octaves of 2D value noise, each lacunarity times finer and gain times fainter; ValueError for no octaves.  Other Python threads run meanwhile.
    """

@overload
def first_contact(still: sphere3, ball: sphere3, move: vec3) -> object:
    """
    When a sphere moving by `move` over a step of unit time first touches a sphere standing still.  None if they do not touch, else a dict with time (not bounded above: a touch further on than `move` has a time past 1), position (the moving centre at the touch, or on a penetration the deepest point of it in the still one), normal (from the still sphere's centre to the moving one), penetrated and penetration_depth.
    """

@overload
def first_contact(still: sphere3d, ball: sphere3d, move: vec3d) -> object:
    """
    When a sphere moving by `move` over a step of unit time first touches a sphere standing still.  None if they do not touch, else a dict with time (not bounded above: a touch further on than `move` has a time past 1), position (the moving centre at the touch, or on a penetration the deepest point of it in the still one), normal (from the still sphere's centre to the moving one), penetrated and penetration_depth.
    """

@overload
def first_contact(still: sphere2, ball: sphere2, move: vec2) -> object:
    """
    When a sphere moving by `move` over a step of unit time first touches a sphere standing still.  None if they do not touch, else a dict with time (not bounded above: a touch further on than `move` has a time past 1), position (the moving centre at the touch, or on a penetration the deepest point of it in the still one), normal (from the still sphere's centre to the moving one), penetrated and penetration_depth.
    """

@overload
def first_contact(still: sphere2d, ball: sphere2d, move: vec2d) -> object:
    """
    When a sphere moving by `move` over a step of unit time first touches a sphere standing still.  None if they do not touch, else a dict with time (not bounded above: a touch further on than `move` has a time past 1), position (the moving centre at the touch, or on a penetration the deepest point of it in the still one), normal (from the still sphere's centre to the moving one), penetrated and penetration_depth.
    """

@overload
def first_contact(start: capsule3, end: capsule3, ball: sphere3, move: vec3, tolerance: float) -> object:
    """
    When a sphere and a capsule, both moving linearly through a step of unit time, first touch: the sphere's centre moves by `move`, the capsule's ends (and radius) go from `start` to `end`.  None if they do not touch within the step, else a dict with time (in [0, 1]), position (on the capsule's surface), normal (from the capsule's axis to the sphere's centre), penetrated and penetration_depth (if they overlap at the start).  The touch is reported when the gap is at most `tolerance`, never after the surfaces meet.
    """

@overload
def first_contact(start: capsule3d, end: capsule3d, ball: sphere3d, move: vec3d, tolerance: float) -> object:
    """
    When a sphere and a capsule, both moving linearly through a step of unit time, first touch: the sphere's centre moves by `move`, the capsule's ends (and radius) go from `start` to `end`.  None if they do not touch within the step, else a dict with time (in [0, 1]), position (on the capsule's surface), normal (from the capsule's axis to the sphere's centre), penetrated and penetration_depth (if they overlap at the start).  The touch is reported when the gap is at most `tolerance`, never after the surfaces meet.
    """

@overload
def first_contact(start: capsule2, end: capsule2, ball: sphere2, move: vec2, tolerance: float) -> object:
    """
    When a sphere and a capsule, both moving linearly through a step of unit time, first touch: the sphere's centre moves by `move`, the capsule's ends (and radius) go from `start` to `end`.  None if they do not touch within the step, else a dict with time (in [0, 1]), position (on the capsule's surface), normal (from the capsule's axis to the sphere's centre), penetrated and penetration_depth (if they overlap at the start).  The touch is reported when the gap is at most `tolerance`, never after the surfaces meet.
    """

@overload
def first_contact(start: capsule2d, end: capsule2d, ball: sphere2d, move: vec2d, tolerance: float) -> object:
    """
    When a sphere and a capsule, both moving linearly through a step of unit time, first touch: the sphere's centre moves by `move`, the capsule's ends (and radius) go from `start` to `end`.  None if they do not touch within the step, else a dict with time (in [0, 1]), position (on the capsule's surface), normal (from the capsule's axis to the sphere's centre), penetrated and penetration_depth (if they overlap at the start).  The touch is reported when the gap is at most `tolerance`, never after the surfaces meet.
    """

def fract(x: float) -> float:
    """What floor leaves, in [0, 1): fract(-0.25) is 0.75"""

@overload
def from_rotation_vector(r: vec3) -> quat:
    """The unit quaternion of a turn of |r| about r: the exp map"""

@overload
def from_rotation_vector(r: vec3d) -> quatd: ...

class frustum:
    """A view frustum of float32, from a view-projection matrix"""

    @overload
    def __init__(self) -> None:
        """Default constructor"""

    @overload
    def __init__(self, matrix: mat4) -> None:
        """Construct frustum from view-projection matrix"""

    def load(self, matrix: mat4) -> None:
        """Load frustum planes from view-projection matrix"""

    def contains(self, point: vec3) -> bool:
        """Test if point is inside the frustum"""

    @overload
    def test_intersection(self, sphere: sphere3) -> bool:
        """
        Test whether the sphere is in view: False only where it lies wholly outside a plane
        """

    @overload
    def test_intersection(self, aabb: aabb3) -> bool:
        """Test intersection with axis-aligned bounding box"""

    @overload
    def test_intersection(self, obb: obb3) -> bool:
        """Test intersection with oriented bounding box"""

    def plane(self, index: int) -> plane:
        """
        The plane of that index: frustum.left_plane, right_plane, top_plane, bottom_plane, near_plane, far_plane (0 to 5)
        """

    @property
    def planes(self) -> list[plane]:
        """
        The 6 planes, in the order of the indices left_plane, right_plane, top_plane, bottom_plane, near_plane, far_plane
        """

    @planes.setter
    def planes(self, arg: Sequence[plane], /) -> None: ...

    planes_count: int = ...
    """6"""

    left_plane: int = ...
    """The left plane's index"""

    right_plane: int = ...
    """The right plane's index"""

    top_plane: int = ...
    """The top plane's index"""

    bottom_plane: int = ...
    """The bottom plane's index"""

    near_plane: int = ...
    """The near plane's index"""

    far_plane: int = ...
    """The far plane's index"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class frustumd:
    """A view frustum of float64, from a view-projection matrix"""

    @overload
    def __init__(self) -> None:
        """Default constructor"""

    @overload
    def __init__(self, matrix: mat4d) -> None:
        """Construct frustum from view-projection matrix"""

    def load(self, matrix: mat4d) -> None:
        """Load frustum planes from view-projection matrix"""

    def contains(self, point: vec3d) -> bool:
        """Test if point is inside the frustum"""

    @overload
    def test_intersection(self, sphere: sphere3d) -> bool:
        """
        Test whether the sphere is in view: False only where it lies wholly outside a plane
        """

    @overload
    def test_intersection(self, aabb: aabb3d) -> bool:
        """Test intersection with axis-aligned bounding box"""

    @overload
    def test_intersection(self, obb: obb3d) -> bool:
        """Test intersection with oriented bounding box"""

    def plane(self, index: int) -> planed:
        """
        The plane of that index: frustum.left_plane, right_plane, top_plane, bottom_plane, near_plane, far_plane (0 to 5)
        """

    @property
    def planes(self) -> list[planed]:
        """
        The 6 planes, in the order of the indices left_plane, right_plane, top_plane, bottom_plane, near_plane, far_plane
        """

    @planes.setter
    def planes(self, arg: Sequence[planed], /) -> None: ...

    planes_count: int = ...
    """6"""

    left_plane: int = ...
    """The left plane's index"""

    right_plane: int = ...
    """The right plane's index"""

    top_plane: int = ...
    """The top plane's index"""

    bottom_plane: int = ...
    """The bottom plane's index"""

    near_plane: int = ...
    """The near plane's index"""

    far_plane: int = ...
    """The far plane's index"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

def hash_combine(seed: int, value: int) -> int:
    """A seed and a value mixed into one hash (order matters)"""

def hash_to_unit(h: int) -> float:
    """A hash into [0, 1), from its top 53 bits"""

@overload
def inverse(q: quat) -> quat:
    """
    The opposite rotation, of a quaternion of any length but none (ValueError)
    """

@overload
def inverse(q: quatd) -> quatd:
    """
    The opposite rotation, of a quaternion of any length but none (ValueError)
    """

@overload
def inverse(m: mat4) -> mat4:
    """The inverse; ValueError for a singular matrix"""

@overload
def inverse(m: mat3) -> mat3:
    """The inverse; ValueError for a singular matrix"""

@overload
def inverse(m: mat4d) -> mat4d:
    """The inverse; ValueError for a singular matrix"""

@overload
def inverse(m: mat3d) -> mat3d:
    """The inverse; ValueError for a singular matrix"""

@overload
def inverse(t: trs) -> trs:
    """
    The opposite transform, where t's scale is uniform or t does not turn; ValueError otherwise and for a scale of zero
    """

@overload
def inverse(t: trsd) -> trsd:
    """
    The opposite transform, where t's scale is uniform or t does not turn; ValueError otherwise and for a scale of zero
    """

def inverse_lerp(a: float, b: float, x: float) -> float:
    """Where x is from a (0) to b (1), unclamped; ValueError for a == b"""

@overload
def is_finite(v: vec2) -> bool: ...

@overload
def is_finite(v: vec3) -> bool: ...

@overload
def is_finite(v: vec4) -> bool: ...

@overload
def is_finite(v: vec2d) -> bool: ...

@overload
def is_finite(v: vec3d) -> bool: ...

@overload
def is_finite(v: vec4d) -> bool: ...

@overload
def length(v: vec2) -> float: ...

@overload
def length(v: vec3) -> float: ...

@overload
def length(v: vec4) -> float: ...

@overload
def length(v: vec2d) -> float: ...

@overload
def length(v: vec3d) -> float: ...

@overload
def length(v: vec4d) -> float: ...

@overload
def length_sq(v: vec2) -> float: ...

@overload
def length_sq(v: vec3) -> float: ...

@overload
def length_sq(v: vec4) -> float: ...

@overload
def length_sq(v: vec2d) -> float: ...

@overload
def length_sq(v: vec3d) -> float: ...

@overload
def length_sq(v: vec4d) -> float: ...

@overload
def lerp(a: vec2, b: vec2, t: float) -> vec2:
    """a + (b - a) t, component by component, unclamped"""

@overload
def lerp(a: vec3, b: vec3, t: float) -> vec3:
    """a + (b - a) t, component by component, unclamped"""

@overload
def lerp(a: vec4, b: vec4, t: float) -> vec4:
    """a + (b - a) t, component by component, unclamped"""

@overload
def lerp(a: vec2d, b: vec2d, t: float) -> vec2d:
    """a + (b - a) t, component by component, unclamped"""

@overload
def lerp(a: vec3d, b: vec3d, t: float) -> vec3d:
    """a + (b - a) t, component by component, unclamped"""

@overload
def lerp(a: vec4d, b: vec4d, t: float) -> vec4d:
    """a + (b - a) t, component by component, unclamped"""

@overload
def lerp(a: float, b: float, t: float) -> float:
    """a + (b - a) t, unclamped"""

@overload
def lerp(a: trs, b: trs, u: float) -> trs:
    """Translation and scale lerped, rotation slerped"""

@overload
def lerp(a: trsd, b: trsd, u: float) -> trsd:
    """Translation and scale lerped, rotation slerped"""

@overload
def linear_part(m: mat4) -> mat3:
    """The upper-left 3x3"""

@overload
def linear_part(m: mat4d) -> mat3d: ...

@overload
def look_at_matrix(eye: vec3, at: vec3, up: vec3) -> mat4:
    """mat4.lookat, returned"""

@overload
def look_at_matrix(eye: vec3d, at: vec3d, up: vec3d) -> mat4d: ...

class mat3:
    """A 3 x 3 matrix of float32, column-major"""

    def __init__(self) -> None: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float32', shape=(3, 3), writable=False)]) -> mat3:
        """Create mat3 from numpy array"""

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(3, 3))]:
        """Convert mat3 to numpy array"""

    def identity(self) -> None:
        """Set to identity matrix"""

    def zero(self) -> None:
        """Set to zero matrix"""

    @overload
    def translation(self, x: float, y: float) -> None:
        """Set to translation matrix"""

    @overload
    def translation(self, p: vec2) -> None: ...

    @overload
    def translate(self, x: float, y: float) -> None:
        """Apply translation"""

    @overload
    def translate(self, p: vec2) -> None: ...

    @overload
    def scaling(self, x: float, y: float) -> None:
        """Set to scaling matrix"""

    @overload
    def scaling(self, p: vec2) -> None: ...

    @overload
    def scale(self, x: float, y: float) -> None:
        """Apply scaling"""

    @overload
    def scale(self, p: vec2) -> None: ...

    @overload
    def rotation(self, axis: vec3, angle: float) -> None:
        """Set to rotation matrix"""

    @overload
    def rotation(self, q: quat) -> None:
        """Set to rotation matrix from quaternion"""

    @overload
    def rotate(self, axis: vec3, angle: float) -> None:
        """Apply rotation"""

    @overload
    def rotate(self, q: quat) -> None:
        """Apply rotation from quaternion"""

    def determinant(self) -> float:
        """Compute determinant"""

    def trace(self) -> float:
        """Compute trace"""

    def invert(self) -> bool:
        """Invert matrix in place"""

    def inverse(self) -> mat3:
        """The inverse; SingularMatrixError, a ValueError, for a singular matrix"""

    def transpose(self) -> mat3:
        """Return transposed matrix"""

    @overload
    def __imul__(self, k: float) -> mat3: ...

    @overload
    def __imul__(self, m: mat3) -> mat3: ...

    def __itruediv__(self, k: float) -> mat3: ...

    @overload
    def __mul__(self, arg: float, /) -> mat3: ...

    @overload
    def __mul__(self, arg: mat3, /) -> mat3: ...

    @overload
    def __mul__(self, arg: vec2, /) -> vec2:
        """
        Transform vec2 by mat3 as a homogeneous point, divided by z, returning vec2
        """

    @overload
    def __mul__(self, arg: vec3, /) -> vec3:
        """Transform vec3 by mat3"""

    def __rmul__(self, arg: float, /) -> mat3: ...

    def __truediv__(self, arg: float, /) -> mat3: ...

    def __repr__(self) -> str: ...

    def get(self, i: int, j: int) -> float:
        """Get matrix element at [i][j]"""

    def set(self, i: int, j: int, value: float) -> None:
        """Set matrix element at [i][j]"""

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class mat3d:
    """A 3 x 3 matrix of float64, column-major"""

    def __init__(self) -> None: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float64', shape=(3, 3), writable=False)]) -> mat3d:
        """Create mat3 from numpy array"""

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float64', shape=(3, 3))]:
        """Convert mat3 to numpy array"""

    def identity(self) -> None:
        """Set to identity matrix"""

    def zero(self) -> None:
        """Set to zero matrix"""

    @overload
    def translation(self, x: float, y: float) -> None:
        """Set to translation matrix"""

    @overload
    def translation(self, p: vec2d) -> None: ...

    @overload
    def translate(self, x: float, y: float) -> None:
        """Apply translation"""

    @overload
    def translate(self, p: vec2d) -> None: ...

    @overload
    def scaling(self, x: float, y: float) -> None:
        """Set to scaling matrix"""

    @overload
    def scaling(self, p: vec2d) -> None: ...

    @overload
    def scale(self, x: float, y: float) -> None:
        """Apply scaling"""

    @overload
    def scale(self, p: vec2d) -> None: ...

    @overload
    def rotation(self, axis: vec3d, angle: float) -> None:
        """Set to rotation matrix"""

    @overload
    def rotation(self, q: quatd) -> None:
        """Set to rotation matrix from quaternion"""

    @overload
    def rotate(self, axis: vec3d, angle: float) -> None:
        """Apply rotation"""

    @overload
    def rotate(self, q: quatd) -> None:
        """Apply rotation from quaternion"""

    def determinant(self) -> float:
        """Compute determinant"""

    def trace(self) -> float:
        """Compute trace"""

    def invert(self) -> bool:
        """Invert matrix in place"""

    def inverse(self) -> mat3d:
        """The inverse; SingularMatrixError, a ValueError, for a singular matrix"""

    def transpose(self) -> mat3d:
        """Return transposed matrix"""

    @overload
    def __imul__(self, k: float) -> mat3d: ...

    @overload
    def __imul__(self, m: mat3d) -> mat3d: ...

    def __itruediv__(self, k: float) -> mat3d: ...

    @overload
    def __mul__(self, arg: float, /) -> mat3d: ...

    @overload
    def __mul__(self, arg: mat3d, /) -> mat3d: ...

    @overload
    def __mul__(self, arg: vec2d, /) -> vec2d:
        """
        Transform vec2 by mat3 as a homogeneous point, divided by z, returning vec2
        """

    @overload
    def __mul__(self, arg: vec3d, /) -> vec3d:
        """Transform vec3 by mat3"""

    def __rmul__(self, arg: float, /) -> mat3d: ...

    def __truediv__(self, arg: float, /) -> mat3d: ...

    def __repr__(self) -> str: ...

    def get(self, i: int, j: int) -> float:
        """Get matrix element at [i][j]"""

    def set(self, i: int, j: int, value: float) -> None:
        """Set matrix element at [i][j]"""

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class mat4:
    """A 4 x 4 matrix of float32, column-major"""

    def __init__(self) -> None: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float32', shape=(4, 4), writable=False)]) -> mat4:
        """Create mat4 from numpy array"""

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(4, 4))]:
        """Convert mat4 to numpy array"""

    def identity(self) -> None:
        """Set to identity matrix"""

    def zero(self) -> None:
        """Set to zero matrix"""

    @overload
    def translation(self, x: float, y: float, z: float) -> None:
        """Set to translation matrix"""

    @overload
    def translation(self, p: vec3) -> None: ...

    @overload
    def translate(self, x: float, y: float, z: float) -> None:
        """Apply translation"""

    @overload
    def translate(self, p: vec3) -> None: ...

    @overload
    def scaling(self, x: float, y: float, z: float) -> None:
        """Set to scaling matrix"""

    @overload
    def scaling(self, p: vec3) -> None: ...

    @overload
    def scale(self, x: float, y: float, z: float) -> None:
        """Apply scaling"""

    @overload
    def scale(self, p: vec3) -> None: ...

    @overload
    def rotation(self, axis: vec3, angle: float) -> None:
        """Set to rotation matrix"""

    @overload
    def rotation(self, q: quat) -> None:
        """Set to rotation matrix from quaternion"""

    @overload
    def rotate(self, axis: vec3, angle: float) -> None:
        """Apply rotation"""

    @overload
    def rotate(self, q: quat) -> None:
        """Apply rotation from quaternion"""

    def lookat(self, eye: vec3, at: vec3, up: vec3) -> None:
        """Set to lookat matrix"""

    def ortho(self, width: float, height: float, z_near: float, z_far: float) -> None:
        """Set to orthographic projection matrix"""

    def perspective(self, fovy: float, aspect: float, z_near: float, z_far: float) -> None:
        """Set to perspective projection matrix"""

    def determinant(self) -> float:
        """Compute determinant"""

    def invert(self) -> bool:
        """Invert matrix in place"""

    def inverse(self) -> mat4:
        """The inverse; SingularMatrixError, a ValueError, for a singular matrix"""

    def transpose(self) -> mat4:
        """Return transposed matrix"""

    def transpose_inplace(self) -> None:
        """Transpose matrix in place"""

    def column(self, index: int) -> vec4:
        """Column 0 to 3 as a vec4; IndexError past them"""

    def projected_column(self, index: int) -> vec3:
        """Column 0 to 3 as a vec3; IndexError past them"""

    def projected_row(self, index: int) -> vec3:
        """Row 0 to 3 as a vec3; IndexError past them"""

    @overload
    def __imul__(self, k: float) -> mat4: ...

    @overload
    def __imul__(self, m: mat4) -> mat4: ...

    def __itruediv__(self, k: float) -> mat4: ...

    @overload
    def __mul__(self, arg: float, /) -> mat4: ...

    @overload
    def __mul__(self, arg: mat4, /) -> mat4: ...

    @overload
    def __mul__(self, arg: vec3, /) -> vec3:
        """Transform vec3 by mat4, returning vec3"""

    @overload
    def __mul__(self, arg: vec4, /) -> vec4:
        """Transform vec4 by mat4"""

    def __rmul__(self, arg: float, /) -> mat4: ...

    def __truediv__(self, arg: float, /) -> mat4: ...

    def __repr__(self) -> str: ...

    def get(self, i: int, j: int) -> float:
        """Get matrix element at [i][j]"""

    def set(self, i: int, j: int, value: float) -> None:
        """Set matrix element at [i][j]"""

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class mat4d:
    """A 4 x 4 matrix of float64, column-major"""

    def __init__(self) -> None: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float64', shape=(4, 4), writable=False)]) -> mat4d:
        """Create mat4 from numpy array"""

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float64', shape=(4, 4))]:
        """Convert mat4 to numpy array"""

    def identity(self) -> None:
        """Set to identity matrix"""

    def zero(self) -> None:
        """Set to zero matrix"""

    @overload
    def translation(self, x: float, y: float, z: float) -> None:
        """Set to translation matrix"""

    @overload
    def translation(self, p: vec3d) -> None: ...

    @overload
    def translate(self, x: float, y: float, z: float) -> None:
        """Apply translation"""

    @overload
    def translate(self, p: vec3d) -> None: ...

    @overload
    def scaling(self, x: float, y: float, z: float) -> None:
        """Set to scaling matrix"""

    @overload
    def scaling(self, p: vec3d) -> None: ...

    @overload
    def scale(self, x: float, y: float, z: float) -> None:
        """Apply scaling"""

    @overload
    def scale(self, p: vec3d) -> None: ...

    @overload
    def rotation(self, axis: vec3d, angle: float) -> None:
        """Set to rotation matrix"""

    @overload
    def rotation(self, q: quatd) -> None:
        """Set to rotation matrix from quaternion"""

    @overload
    def rotate(self, axis: vec3d, angle: float) -> None:
        """Apply rotation"""

    @overload
    def rotate(self, q: quatd) -> None:
        """Apply rotation from quaternion"""

    def lookat(self, eye: vec3d, at: vec3d, up: vec3d) -> None:
        """Set to lookat matrix"""

    def ortho(self, width: float, height: float, z_near: float, z_far: float) -> None:
        """Set to orthographic projection matrix"""

    def perspective(self, fovy: float, aspect: float, z_near: float, z_far: float) -> None:
        """Set to perspective projection matrix"""

    def determinant(self) -> float:
        """Compute determinant"""

    def invert(self) -> bool:
        """Invert matrix in place"""

    def inverse(self) -> mat4d:
        """The inverse; SingularMatrixError, a ValueError, for a singular matrix"""

    def transpose(self) -> mat4d:
        """Return transposed matrix"""

    def transpose_inplace(self) -> None:
        """Transpose matrix in place"""

    def column(self, index: int) -> vec4d:
        """Column 0 to 3 as a vec4; IndexError past them"""

    def projected_column(self, index: int) -> vec3d:
        """Column 0 to 3 as a vec3; IndexError past them"""

    def projected_row(self, index: int) -> vec3d:
        """Row 0 to 3 as a vec3; IndexError past them"""

    @overload
    def __imul__(self, k: float) -> mat4d: ...

    @overload
    def __imul__(self, m: mat4d) -> mat4d: ...

    def __itruediv__(self, k: float) -> mat4d: ...

    @overload
    def __mul__(self, arg: float, /) -> mat4d: ...

    @overload
    def __mul__(self, arg: mat4d, /) -> mat4d: ...

    @overload
    def __mul__(self, arg: vec3d, /) -> vec3d:
        """Transform vec3 by mat4, returning vec3"""

    @overload
    def __mul__(self, arg: vec4d, /) -> vec4d:
        """Transform vec4 by mat4"""

    def __rmul__(self, arg: float, /) -> mat4d: ...

    def __truediv__(self, arg: float, /) -> mat4d: ...

    def __repr__(self) -> str: ...

    def get(self, i: int, j: int) -> float:
        """Get matrix element at [i][j]"""

    def set(self, i: int, j: int, value: float) -> None:
        """Set matrix element at [i][j]"""

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

@overload
def max(a: vec2, b: vec2) -> vec2:
    """Component by component"""

@overload
def max(a: vec3, b: vec3) -> vec3: ...

@overload
def max(a: vec4, b: vec4) -> vec4: ...

@overload
def max(a: vec2d, b: vec2d) -> vec2d: ...

@overload
def max(a: vec3d, b: vec3d) -> vec3d: ...

@overload
def max(a: vec4d, b: vec4d) -> vec4d: ...

@overload
def min(a: vec2, b: vec2) -> vec2:
    """Component by component"""

@overload
def min(a: vec3, b: vec3) -> vec3: ...

@overload
def min(a: vec4, b: vec4) -> vec4: ...

@overload
def min(a: vec2d, b: vec2d) -> vec2d: ...

@overload
def min(a: vec3d, b: vec3d) -> vec3d: ...

@overload
def min(a: vec4d, b: vec4d) -> vec4d: ...

@overload
def move_towards(current: float, target: float, max_delta: float) -> float:
    """
    current moved towards target by max_delta at most, landing on it exactly; ValueError for a negative max_delta. For numbers and vectors.
    """

@overload
def move_towards(current: vec2, target: vec2, max_delta: float) -> vec2: ...

@overload
def move_towards(current: vec3, target: vec3, max_delta: float) -> vec3: ...

@overload
def move_towards(current: vec2d, target: vec2d, max_delta: float) -> vec2d: ...

@overload
def move_towards(current: vec3d, target: vec3d, max_delta: float) -> vec3d: ...

@overload
def nlerp(p: quat, q: quat, t: float) -> quat:
    """
    Normalised linear interpolation from p to q along the shorter arc; p and q of any length but none (ValueError)
    """

@overload
def nlerp(p: quatd, q: quatd, t: float) -> quatd: ...

@overload
def normal_matrix(m: mat4) -> mat3:
    """
    The inverse transpose of the linear part: what turns normals as m turns the surface; ValueError for a singular one
    """

@overload
def normal_matrix(m: mat4d) -> mat3d: ...

@overload
def normalize(v: vec2) -> vec2:
    """
    v at unit length, as a new vector; ValueError for one of no length or not finite
    """

@overload
def normalize(v: vec3) -> vec3: ...

@overload
def normalize(v: vec4) -> vec4: ...

@overload
def normalize(v: vec2d) -> vec2d: ...

@overload
def normalize(v: vec3d) -> vec3d: ...

@overload
def normalize(v: vec4d) -> vec4d: ...

@overload
def normalize_or_zero(v: vec2) -> vec2:
    """
    v at unit length, as a new vector, or v as it is where it has no length
    """

@overload
def normalize_or_zero(v: vec3) -> vec3: ...

@overload
def normalize_or_zero(v: vec4) -> vec4: ...

@overload
def normalize_or_zero(v: vec2d) -> vec2d: ...

@overload
def normalize_or_zero(v: vec3d) -> vec3d: ...

@overload
def normalize_or_zero(v: vec4d) -> vec4d: ...

class obb2:
    """An oriented rectangle of float32: a corner and two edges"""

    @overload
    def __init__(self) -> None:
        """Default constructor (uninitialized)"""

    @overload
    def __init__(self, aabb: aabb2) -> None:
        """Construct OBB from axis-aligned bounding box"""

    @overload
    def __init__(self, other: obb2) -> None:
        """Copy constructor"""

    @property
    def origin(self) -> vec2:
        """Origin corner of the OBB"""

    @origin.setter
    def origin(self, arg: vec2, /) -> None: ...

    @property
    def tangent(self) -> vec2:
        """First edge vector (width direction)"""

    @tangent.setter
    def tangent(self, arg: vec2, /) -> None: ...

    @property
    def normal(self) -> vec2:
        """Second edge vector (height direction)"""

    @normal.setter
    def normal(self, arg: vec2, /) -> None: ...

    def world_to_local_point(self, point: vec2) -> vec2:
        """Transform point from world to local OBB coordinates"""

    def world_to_local_vector(self, vector: vec2) -> vec2:
        """Transform vector from world to local OBB coordinates"""

    def contains(self, point: vec2) -> bool:
        """Test if point is inside OBB"""

    def vertex(self, vertex_number: int) -> vec2:
        """Get vertex position by index (0-3)"""

    def side_normal(self, side_number: int) -> vec2:
        """Get outward normal for a side (0-3)"""

    def bounds(self) -> aabb2:
        """Get axis-aligned bounding box containing this OBB"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class obb2d:
    """An oriented rectangle of float64: a corner and two edges"""

    @overload
    def __init__(self) -> None:
        """Default constructor (uninitialized)"""

    @overload
    def __init__(self, aabb: aabb2d) -> None:
        """Construct OBB from axis-aligned bounding box"""

    @overload
    def __init__(self, other: obb2d) -> None:
        """Copy constructor"""

    @property
    def origin(self) -> vec2d:
        """Origin corner of the OBB"""

    @origin.setter
    def origin(self, arg: vec2d, /) -> None: ...

    @property
    def tangent(self) -> vec2d:
        """First edge vector (width direction)"""

    @tangent.setter
    def tangent(self, arg: vec2d, /) -> None: ...

    @property
    def normal(self) -> vec2d:
        """Second edge vector (height direction)"""

    @normal.setter
    def normal(self, arg: vec2d, /) -> None: ...

    def world_to_local_point(self, point: vec2d) -> vec2d:
        """Transform point from world to local OBB coordinates"""

    def world_to_local_vector(self, vector: vec2d) -> vec2d:
        """Transform vector from world to local OBB coordinates"""

    def contains(self, point: vec2d) -> bool:
        """Test if point is inside OBB"""

    def vertex(self, vertex_number: int) -> vec2d:
        """Get vertex position by index (0-3)"""

    def side_normal(self, side_number: int) -> vec2d:
        """Get outward normal for a side (0-3)"""

    def bounds(self) -> aabb2d:
        """Get axis-aligned bounding box containing this OBB"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class obb3:
    """An oriented box of float32: a corner and three edges"""

    @overload
    def __init__(self) -> None:
        """Default constructor (uninitialized)"""

    @overload
    def __init__(self, aabb: aabb3) -> None:
        """Construct OBB from axis-aligned bounding box"""

    @overload
    def __init__(self, origin: vec3, tangent: vec3, normal: vec3, binormal: vec3) -> None:
        """Construct OBB from origin and three edge vectors"""

    @overload
    def __init__(self, other: obb3) -> None:
        """Copy constructor"""

    @property
    def origin(self) -> vec3:
        """Origin corner of the OBB"""

    @origin.setter
    def origin(self, arg: vec3, /) -> None: ...

    @property
    def tangent(self) -> vec3:
        """First edge vector (X-like direction)"""

    @tangent.setter
    def tangent(self, arg: vec3, /) -> None: ...

    @property
    def normal(self) -> vec3:
        """Second edge vector (Y-like direction)"""

    @normal.setter
    def normal(self, arg: vec3, /) -> None: ...

    @property
    def binormal(self) -> vec3:
        """Third edge vector (Z-like direction)"""

    @binormal.setter
    def binormal(self, arg: vec3, /) -> None: ...

    def transform(self, matrix: mat4) -> None:
        """Apply a 4x4 transformation matrix to the OBB"""

    def centre(self) -> vec3:
        """Get center point of the OBB"""

    def volume(self) -> float:
        """Get volume of the OBB"""

    def bounds(self) -> aabb3:
        """Get axis-aligned bounding box containing this OBB"""

    def vertex(self, vertex_number: int) -> vec3:
        """Get vertex position by index (0-7)"""

    def contains(self, point: vec3) -> bool:
        """Test if point is inside OBB"""

    @overload
    def test_intersection(self, other: obb3) -> bool:
        """Test intersection with another OBB"""

    @overload
    def test_intersection(self, tri: triangle3) -> bool:
        """Test intersection with a triangle (SAT)"""

    @overload
    def test_intersection(self, cap: capsule3) -> bool:
        """Test intersection with a capsule (exact)"""

    def trace(self, ray: ray3, t_min: float = 0.0, t_max: float = 1.0) -> object:
        """
        Ray-OBB intersection. Returns (t_enter, t_exit) tuple of ray-parameter times, or None if the ray misses the OBB in [t_min, t_max].  When the ray origin is inside the OBB, t_enter == t_min.
        """

    def is_degenerate(self, tolerance: float = 0.009999999776482582) -> bool:
        """Test whether this OBB has near-zero volume (parallel edges)"""

    def is_basis_linearly_independent(self, tolerance: float = 0.009999999776482582) -> bool:
        """Test whether edge vectors are linearly independent"""

    @staticmethod
    def make_sweep_volume(box: obb3, displacement: vec3) -> list[obb3]:
        """Decompose swept volume into 6 face-sweep OBBs for tunneling prevention"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class obb3d:
    """An oriented box of float64: a corner and three edges"""

    @overload
    def __init__(self) -> None:
        """Default constructor (uninitialized)"""

    @overload
    def __init__(self, aabb: aabb3d) -> None:
        """Construct OBB from axis-aligned bounding box"""

    @overload
    def __init__(self, origin: vec3d, tangent: vec3d, normal: vec3d, binormal: vec3d) -> None:
        """Construct OBB from origin and three edge vectors"""

    @overload
    def __init__(self, other: obb3d) -> None:
        """Copy constructor"""

    @property
    def origin(self) -> vec3d:
        """Origin corner of the OBB"""

    @origin.setter
    def origin(self, arg: vec3d, /) -> None: ...

    @property
    def tangent(self) -> vec3d:
        """First edge vector (X-like direction)"""

    @tangent.setter
    def tangent(self, arg: vec3d, /) -> None: ...

    @property
    def normal(self) -> vec3d:
        """Second edge vector (Y-like direction)"""

    @normal.setter
    def normal(self, arg: vec3d, /) -> None: ...

    @property
    def binormal(self) -> vec3d:
        """Third edge vector (Z-like direction)"""

    @binormal.setter
    def binormal(self, arg: vec3d, /) -> None: ...

    def transform(self, matrix: mat4d) -> None:
        """Apply a 4x4 transformation matrix to the OBB"""

    def centre(self) -> vec3d:
        """Get center point of the OBB"""

    def volume(self) -> float:
        """Get volume of the OBB"""

    def bounds(self) -> aabb3d:
        """Get axis-aligned bounding box containing this OBB"""

    def vertex(self, vertex_number: int) -> vec3d:
        """Get vertex position by index (0-7)"""

    def contains(self, point: vec3d) -> bool:
        """Test if point is inside OBB"""

    @overload
    def test_intersection(self, other: obb3d) -> bool:
        """Test intersection with another OBB"""

    @overload
    def test_intersection(self, tri: triangle3d) -> bool:
        """Test intersection with a triangle (SAT)"""

    @overload
    def test_intersection(self, cap: capsule3d) -> bool:
        """Test intersection with a capsule (exact)"""

    def trace(self, ray: ray3d, t_min: float = 0.0, t_max: float = 1.0) -> object:
        """
        Ray-OBB intersection. Returns (t_enter, t_exit) tuple of ray-parameter times, or None if the ray misses the OBB in [t_min, t_max].  When the ray origin is inside the OBB, t_enter == t_min.
        """

    def is_degenerate(self, tolerance: float = 0.01) -> bool:
        """Test whether this OBB has near-zero volume (parallel edges)"""

    def is_basis_linearly_independent(self, tolerance: float = 0.01) -> bool:
        """Test whether edge vectors are linearly independent"""

    @staticmethod
    def make_sweep_volume(box: obb3d, displacement: vec3d) -> list[obb3d]:
        """Decompose swept volume into 6 face-sweep OBBs for tunneling prevention"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

def ortho_matrix(left: float, right: float, bottom: float, top: float, z_near: float, z_far: float) -> mat4:
    """
    An orthographic projection off centre: [left, right] and [bottom, top] to -1..1, depth 0 at z_near to 1 at z_far; ValueError for a box of no width, height or depth
    """

class pcg32:
    """
    O'Neill's PCG32: the same numbers on every machine, and its state is its value (a copy is a rollback). The reals are doubles, as Python's floats; unit_vector() is a vec3 of floats, as rove's C++ draws one in float.
    """

    def __init__(self, seed: int, stream: int = 15726070495360670683) -> None:
        """
        The generator of seed on stream: different streams give unrelated numbers from the same seed
        """

    def next(self) -> int:
        """The next 32 random bits"""

    def below(self, n: int) -> int:
        """
        A whole number in [0, n), each as likely (no modulo's bias); ValueError for n of 0
        """

    def unit(self) -> float:
        """
        A number in [0, 1) of 53 random bits: the next number high, the one after low
        """

    def uniform(self, lo: float, hi: float) -> float:
        """lo + (hi - lo) unit(); ValueError for lo above hi"""

    def chance(self, p: float) -> bool:
        """True with probability p: never for 0, always for 1"""

    def normal(self, mean: float, deviation: float) -> float:
        """Normally distributed (Box-Muller); ValueError for a negative deviation"""

    def unit_vector(self) -> vec3:
        """A direction uniformly over the sphere"""

    def shuffle(self, items: list) -> None:
        """
        Puts the list in a random order in place (Fisher-Yates: the same order everywhere)
        """

    def weighted(self, weights: Sequence[float]) -> int:
        """
        An index into the weights, each as likely as its share; ValueError for a negative weight or a sum of none
        """

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __repr__(self) -> str: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

def perspective_matrix(fovy: float, aspect: float, z_near: float, z_far: float) -> mat4:
    """mat4.perspective, returned: depth 0 at z_near to 1 at z_far"""

class plane:
    """A plane of float32: A x + B y + C z + D = 0"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, origin: vec3, normal: vec3) -> None:
        """Construct plane from a point on the plane and normal vector"""

    @overload
    def __init__(self, d: float, normal: vec3) -> None:
        """Construct plane from distance D and normal vector"""

    @property
    def A(self) -> float:
        """X coefficient in plane equation Ax + By + Cz + D = 0"""

    @A.setter
    def A(self, arg: float, /) -> None: ...

    @property
    def B(self) -> float:
        """Y coefficient in plane equation Ax + By + Cz + D = 0"""

    @B.setter
    def B(self, arg: float, /) -> None: ...

    @property
    def C(self) -> float:
        """Z coefficient in plane equation Ax + By + Cz + D = 0"""

    @C.setter
    def C(self, arg: float, /) -> None: ...

    @property
    def D(self) -> float:
        """Distance coefficient in plane equation Ax + By + Cz + D = 0"""

    @D.setter
    def D(self, arg: float, /) -> None: ...

    @overload
    def set(self, origin: vec3, normal: vec3) -> None:
        """Set plane from a point on the plane and normal vector"""

    @overload
    def set(self, d: float, normal: vec3) -> None:
        """Set plane from distance D and normal vector"""

    def apply(self, point: vec3) -> float:
        """Evaluate plane equation at point: Ax + By + Cz + D"""

    def test_intersection(self, ray: ray3, epsilon: float = 9.999999974752427e-07) -> bool:
        """Test if ray intersects with the plane"""

    @overload
    def contains(self, point: vec3, epsilon: float = 9.999999974752427e-07) -> bool:
        """Test if point lies on the plane"""

    @overload
    def contains(self, ray: ray3, epsilon: float = 9.999999974752427e-07) -> bool:
        """Test if ray lies on the plane"""

    def parallel(self, ray: ray3, epsilon: float = 9.999999974752427e-07) -> bool:
        """Test if ray is parallel to the plane"""

    def trace(self, ray: ray3) -> float:
        """Get parameter t where ray intersects plane"""

    def origin(self) -> vec3:
        """Get a point on the plane"""

    def normal(self) -> vec3:
        """Get plane normal vector"""

    def is_correct(self) -> bool:
        """
        Check if plane equation is valid (at least one coefficient is non-zero)
        """

    def classify(self, point: vec3, epsilon: float = 9.999999974752427e-07) -> int:
        """
        Classify point position relative to plane (returns int: 1=positive, 2=intersects, 3=negative)
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class planed:
    """A plane of float64: A x + B y + C z + D = 0"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, origin: vec3d, normal: vec3d) -> None:
        """Construct plane from a point on the plane and normal vector"""

    @overload
    def __init__(self, d: float, normal: vec3d) -> None:
        """Construct plane from distance D and normal vector"""

    @property
    def A(self) -> float:
        """X coefficient in plane equation Ax + By + Cz + D = 0"""

    @A.setter
    def A(self, arg: float, /) -> None: ...

    @property
    def B(self) -> float:
        """Y coefficient in plane equation Ax + By + Cz + D = 0"""

    @B.setter
    def B(self, arg: float, /) -> None: ...

    @property
    def C(self) -> float:
        """Z coefficient in plane equation Ax + By + Cz + D = 0"""

    @C.setter
    def C(self, arg: float, /) -> None: ...

    @property
    def D(self) -> float:
        """Distance coefficient in plane equation Ax + By + Cz + D = 0"""

    @D.setter
    def D(self, arg: float, /) -> None: ...

    @overload
    def set(self, origin: vec3d, normal: vec3d) -> None:
        """Set plane from a point on the plane and normal vector"""

    @overload
    def set(self, d: float, normal: vec3d) -> None:
        """Set plane from distance D and normal vector"""

    def apply(self, point: vec3d) -> float:
        """Evaluate plane equation at point: Ax + By + Cz + D"""

    def test_intersection(self, ray: ray3d, epsilon: float = 1e-12) -> bool:
        """Test if ray intersects with the plane"""

    @overload
    def contains(self, point: vec3d, epsilon: float = 1e-12) -> bool:
        """Test if point lies on the plane"""

    @overload
    def contains(self, ray: ray3d, epsilon: float = 1e-12) -> bool:
        """Test if ray lies on the plane"""

    def parallel(self, ray: ray3d, epsilon: float = 1e-12) -> bool:
        """Test if ray is parallel to the plane"""

    def trace(self, ray: ray3d) -> float:
        """Get parameter t where ray intersects plane"""

    def origin(self) -> vec3d:
        """Get a point on the plane"""

    def normal(self) -> vec3d:
        """Get plane normal vector"""

    def is_correct(self) -> bool:
        """
        Check if plane equation is valid (at least one coefficient is non-zero)
        """

    def classify(self, point: vec3d, epsilon: float = 1e-12) -> int:
        """
        Classify point position relative to plane (returns int: 1=positive, 2=intersects, 3=negative)
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

@overload
def project(v: vec2, onto: vec2) -> vec2:
    """The part of v along onto (any length but none: ValueError)"""

@overload
def project(v: vec3, onto: vec3) -> vec3: ...

@overload
def project(v: vec4, onto: vec4) -> vec4: ...

@overload
def project(v: vec2d, onto: vec2d) -> vec2d: ...

@overload
def project(v: vec3d, onto: vec3d) -> vec3d: ...

@overload
def project(v: vec4d, onto: vec4d) -> vec4d: ...

@overload
def project_point(m: mat4, p: vec3) -> vec3:
    """
    m p with w = 1, divided by the w that comes out; ValueError for w of zero
    """

@overload
def project_point(m: mat3, p: vec2) -> vec2:
    """
    (x, y, 1) through m, divided by the third component; ValueError for zero there
    """

@overload
def project_point(m: mat4d, p: vec3d) -> vec3d:
    """
    m p with w = 1, divided by the w that comes out; ValueError for w of zero
    """

@overload
def project_point(m: mat3d, p: vec2d) -> vec2d:
    """
    (x, y, 1) through m, divided by the third component; ValueError for zero there
    """

class quat:
    """A quaternion (x, y, z, w) of float32"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float, z: float, w: float) -> None: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float32', shape=(4), writable=False)]) -> quat:
        """Create quaternion from numpy array"""

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(4))]:
        """Convert quaternion to numpy array"""

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def w(self) -> float: ...

    @w.setter
    def w(self, arg: float, /) -> None: ...

    def identity(self) -> None:
        """Set to identity quaternion (0, 0, 0, 1)"""

    def norm(self) -> float:
        """Compute quaternion norm"""

    def normalize(self) -> None:
        """Normalize quaternion in place"""

    def normalized(self) -> quat:
        """Return normalized quaternion"""

    def scale(self, s: float) -> None:
        """Scale quaternion by scalar"""

    def __neg__(self) -> quat: ...

    def __add__(self, arg: quat, /) -> quat: ...

    def __sub__(self, arg: quat, /) -> quat: ...

    @overload
    def __mul__(self, arg: float, /) -> quat: ...

    @overload
    def __mul__(self, arg: quat, /) -> quat:
        """The composed rotation: q first, then p"""

    @overload
    def __mul__(self, arg: vec3, /) -> vec3:
        """v turned by q (any length but none: ValueError), as rotate(q, v)"""

    def __rmul__(self, arg: float, /) -> quat: ...

    def __repr__(self) -> str: ...

    def dot(self, other: quat) -> float:
        """Compute dot product with another quaternion"""

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class quat_slerper:
    """
    Spherical interpolation between two quaternions of float32, set up once for many steps
    """

    def __init__(self) -> None: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def setup(self, p: quat, q: quat) -> None:
        """Setup SLERP between two quaternions"""

    def interpolate(self, t: float) -> quat:
        """Interpolate between quaternions at parameter t [0,1]"""

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class quat_slerperd:
    """
    Spherical interpolation between two quaternions of float64, set up once for many steps
    """

    def __init__(self) -> None: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def setup(self, p: quatd, q: quatd) -> None:
        """Setup SLERP between two quaternions"""

    def interpolate(self, t: float) -> quatd:
        """Interpolate between quaternions at parameter t [0,1]"""

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class quatd:
    """A quaternion (x, y, z, w) of float64"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float, z: float, w: float) -> None: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float64', shape=(4), writable=False)]) -> quatd:
        """Create quaternion from numpy array"""

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float64', shape=(4))]:
        """Convert quaternion to numpy array"""

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def w(self) -> float: ...

    @w.setter
    def w(self, arg: float, /) -> None: ...

    def identity(self) -> None:
        """Set to identity quaternion (0, 0, 0, 1)"""

    def norm(self) -> float:
        """Compute quaternion norm"""

    def normalize(self) -> None:
        """Normalize quaternion in place"""

    def normalized(self) -> quatd:
        """Return normalized quaternion"""

    def scale(self, s: float) -> None:
        """Scale quaternion by scalar"""

    def __neg__(self) -> quatd: ...

    def __add__(self, arg: quatd, /) -> quatd: ...

    def __sub__(self, arg: quatd, /) -> quatd: ...

    @overload
    def __mul__(self, arg: float, /) -> quatd: ...

    @overload
    def __mul__(self, arg: quatd, /) -> quatd:
        """The composed rotation: q first, then p"""

    @overload
    def __mul__(self, arg: vec3d, /) -> vec3d:
        """v turned by q (any length but none: ValueError), as rotate(q, v)"""

    def __rmul__(self, arg: float, /) -> quatd: ...

    def __repr__(self) -> str: ...

    def dot(self, other: quatd) -> float:
        """Compute dot product with another quaternion"""

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

def radians(degrees: float) -> float:
    """Degrees to radians: 180 is exactly pi"""

class ray2:
    """
    A 2D ray of float32: an origin r0 and a direction a; apply(t) is r0 + a t
    """

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, origin: vec2, direction: vec2) -> None:
        """Construct ray from origin and direction"""

    @property
    def r0(self) -> vec2:
        """Ray origin point"""

    @r0.setter
    def r0(self, arg: vec2, /) -> None: ...

    @property
    def a(self) -> vec2:
        """Ray direction vector"""

    @a.setter
    def a(self, arg: vec2, /) -> None: ...

    def set(self, origin: vec2, direction: vec2) -> None:
        """Set ray origin and direction"""

    def apply(self, t: float) -> vec2:
        """Get point on ray at parameter t: r0 + a * t"""

    def contains(self, point: vec2, epsilon: float = 9.999999974752427e-07) -> bool:
        """Test if point lies on the ray"""

    def distance_sq(self, point: vec2) -> float:
        """Get squared distance from ray to point"""

    @overload
    def distance(self, point: vec2) -> float:
        """Get distance from ray to point"""

    @overload
    def distance(self, other: ray2) -> float:
        """
        Distance between the two lines: zero where they cross, the closest approach where they are skew, how far apart they are where they are parallel
        """

    def test_intersection(self, other: ray2, epsilon: float = 9.999999974752427e-07) -> bool:
        """Whether the two lines meet: distance(other) < epsilon"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class ray2d:
    """
    A 2D ray of float64: an origin r0 and a direction a; apply(t) is r0 + a t
    """

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, origin: vec2d, direction: vec2d) -> None:
        """Construct ray from origin and direction"""

    @property
    def r0(self) -> vec2d:
        """Ray origin point"""

    @r0.setter
    def r0(self, arg: vec2d, /) -> None: ...

    @property
    def a(self) -> vec2d:
        """Ray direction vector"""

    @a.setter
    def a(self, arg: vec2d, /) -> None: ...

    def set(self, origin: vec2d, direction: vec2d) -> None:
        """Set ray origin and direction"""

    def apply(self, t: float) -> vec2d:
        """Get point on ray at parameter t: r0 + a * t"""

    def contains(self, point: vec2d, epsilon: float = 1e-12) -> bool:
        """Test if point lies on the ray"""

    def distance_sq(self, point: vec2d) -> float:
        """Get squared distance from ray to point"""

    @overload
    def distance(self, point: vec2d) -> float:
        """Get distance from ray to point"""

    @overload
    def distance(self, other: ray2d) -> float:
        """
        Distance between the two lines: zero where they cross, the closest approach where they are skew, how far apart they are where they are parallel
        """

    def test_intersection(self, other: ray2d, epsilon: float = 1e-12) -> bool:
        """Whether the two lines meet: distance(other) < epsilon"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class ray3:
    """
    A 3D ray of float32: an origin r0 and a direction a; apply(t) is r0 + a t
    """

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, origin: vec3, direction: vec3) -> None:
        """Construct ray from origin and direction"""

    @property
    def r0(self) -> vec3:
        """Ray origin point"""

    @r0.setter
    def r0(self, arg: vec3, /) -> None: ...

    @property
    def a(self) -> vec3:
        """Ray direction vector"""

    @a.setter
    def a(self, arg: vec3, /) -> None: ...

    def set(self, origin: vec3, direction: vec3) -> None:
        """Set ray origin and direction"""

    def apply(self, t: float) -> vec3:
        """Get point on ray at parameter t: r0 + a * t"""

    def contains(self, point: vec3, epsilon: float = 9.999999974752427e-07) -> bool:
        """Test if point lies on the ray"""

    def distance_sq(self, point: vec3) -> float:
        """Get squared distance from ray to point"""

    @overload
    def distance(self, point: vec3) -> float:
        """Get distance from ray to point"""

    @overload
    def distance(self, other: ray3) -> float:
        """
        Distance between the two lines: zero where they cross, the closest approach where they are skew, how far apart they are where they are parallel
        """

    def test_intersection(self, other: ray3, epsilon: float = 9.999999974752427e-07) -> bool:
        """Whether the two lines meet: distance(other) < epsilon"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class ray3d:
    """
    A 3D ray of float64: an origin r0 and a direction a; apply(t) is r0 + a t
    """

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, origin: vec3d, direction: vec3d) -> None:
        """Construct ray from origin and direction"""

    @property
    def r0(self) -> vec3d:
        """Ray origin point"""

    @r0.setter
    def r0(self, arg: vec3d, /) -> None: ...

    @property
    def a(self) -> vec3d:
        """Ray direction vector"""

    @a.setter
    def a(self, arg: vec3d, /) -> None: ...

    def set(self, origin: vec3d, direction: vec3d) -> None:
        """Set ray origin and direction"""

    def apply(self, t: float) -> vec3d:
        """Get point on ray at parameter t: r0 + a * t"""

    def contains(self, point: vec3d, epsilon: float = 1e-12) -> bool:
        """Test if point lies on the ray"""

    def distance_sq(self, point: vec3d) -> float:
        """Get squared distance from ray to point"""

    @overload
    def distance(self, point: vec3d) -> float:
        """Get distance from ray to point"""

    @overload
    def distance(self, other: ray3d) -> float:
        """
        Distance between the two lines: zero where they cross, the closest approach where they are skew, how far apart they are where they are parallel
        """

    def test_intersection(self, other: ray3d, epsilon: float = 1e-12) -> bool:
        """Whether the two lines meet: distance(other) < epsilon"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

@overload
def reflect(v: vec2, n: vec2) -> vec2:
    """v bounced off a surface of unit normal n"""

@overload
def reflect(v: vec3, n: vec3) -> vec3: ...

@overload
def reflect(v: vec4, n: vec4) -> vec4: ...

@overload
def reflect(v: vec2d, n: vec2d) -> vec2d: ...

@overload
def reflect(v: vec3d, n: vec3d) -> vec3d: ...

@overload
def reflect(v: vec4d, n: vec4d) -> vec4d: ...

@overload
def reject(v: vec2, onto: vec2) -> vec2:
    """The part of v square to onto (any length but none: ValueError)"""

@overload
def reject(v: vec3, onto: vec3) -> vec3: ...

@overload
def reject(v: vec4, onto: vec4) -> vec4: ...

@overload
def reject(v: vec2d, onto: vec2d) -> vec2d: ...

@overload
def reject(v: vec3d, onto: vec3d) -> vec3d: ...

@overload
def reject(v: vec4d, onto: vec4d) -> vec4d: ...

def remap(x: float, a0: float, a1: float, b0: float, b1: float) -> float:
    """x moved from [a0, a1] to [b0, b1], unclamped; ValueError for a0 == a1"""

@overload
def rotate(q: quat, v: vec3) -> vec3:
    """v turned by q, a quaternion of any length but none (ValueError)"""

@overload
def rotate(q: quatd, v: vec3d) -> vec3d: ...

@overload
def rotation_between(from_: vec3, to: vec3) -> quat:
    """
    The shortest rotation turning direction from_ onto direction to (any lengths but none: ValueError); half a turn for opposite ones
    """

@overload
def rotation_between(from_: vec3d, to: vec3d) -> quatd: ...

@overload
def rotation_matrix(q: quat) -> mat4:
    """The 4x4 rotation of a quaternion of any length but none (ValueError)"""

@overload
def rotation_matrix(q: quatd) -> mat4d:
    """The 4x4 rotation of a quaternion of any length but none (ValueError)"""

@overload
def rotation_matrix(axis: vec3, angle: float) -> mat4:
    """
    A right-handed turn of angle radians about axis (any length but none: ValueError)
    """

@overload
def rotation_matrix(axis: vec3d, angle: float) -> mat4d:
    """
    A right-handed turn of angle radians about axis (any length but none: ValueError)
    """

@overload
def rotation_vector(q: quat) -> vec3:
    """
    The axis times the angle (0 to pi): the log map; ValueError for no quaternion
    """

@overload
def rotation_vector(q: quatd) -> vec3d: ...

@overload
def scaling_matrix(factors: vec3) -> mat4: ...

@overload
def scaling_matrix(factors: vec3d) -> mat4d: ...

class segment2:
    """A 2D segment of float32, from A to B"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec2, b: vec2) -> None:
        """Construct line segment from two endpoints"""

    @property
    def A(self) -> vec2:
        """First endpoint of line segment"""

    @A.setter
    def A(self, arg: vec2, /) -> None: ...

    @property
    def B(self) -> vec2:
        """Second endpoint of line segment"""

    @B.setter
    def B(self, arg: vec2, /) -> None: ...

    def construct(self, a: vec2, b: vec2) -> None:
        """Set line segment endpoints"""

    def direction(self) -> vec2:
        """Get direction vector (B - A)"""

    def to_ray(self) -> ray2:
        """Get ray representation of this line segment"""

    def bounds(self) -> aabb2:
        """Get axis-aligned bounding box containing this line segment"""

    def length_sq(self) -> float:
        """Get squared length of line segment"""

    def contains(self, point: vec2) -> bool:
        """Test if point lies on the line segment"""

    def distance_sq(self, point: vec2) -> float:
        """Get squared distance from line segment to point"""

    def distance(self, point: vec2) -> float:
        """Get distance from line segment to point"""

    def point(self, t: float) -> vec2:
        """Get the point A + (B - A) t; exactly A at t = 0, exactly B at t = 1"""

    def parameter(self, point: vec2) -> float:
        """Get t in [0, 1] of the nearest point on the segment; 0 when A == B"""

    def closest_point(self, point: vec2) -> vec2:
        """Get the point on the segment nearest to point"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class segment2d:
    """A 2D segment of float64, from A to B"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec2d, b: vec2d) -> None:
        """Construct line segment from two endpoints"""

    @property
    def A(self) -> vec2d:
        """First endpoint of line segment"""

    @A.setter
    def A(self, arg: vec2d, /) -> None: ...

    @property
    def B(self) -> vec2d:
        """Second endpoint of line segment"""

    @B.setter
    def B(self, arg: vec2d, /) -> None: ...

    def construct(self, a: vec2d, b: vec2d) -> None:
        """Set line segment endpoints"""

    def direction(self) -> vec2d:
        """Get direction vector (B - A)"""

    def to_ray(self) -> ray2d:
        """Get ray representation of this line segment"""

    def bounds(self) -> aabb2d:
        """Get axis-aligned bounding box containing this line segment"""

    def length_sq(self) -> float:
        """Get squared length of line segment"""

    def contains(self, point: vec2d) -> bool:
        """Test if point lies on the line segment"""

    def distance_sq(self, point: vec2d) -> float:
        """Get squared distance from line segment to point"""

    def distance(self, point: vec2d) -> float:
        """Get distance from line segment to point"""

    def point(self, t: float) -> vec2d:
        """Get the point A + (B - A) t; exactly A at t = 0, exactly B at t = 1"""

    def parameter(self, point: vec2d) -> float:
        """Get t in [0, 1] of the nearest point on the segment; 0 when A == B"""

    def closest_point(self, point: vec2d) -> vec2d:
        """Get the point on the segment nearest to point"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class segment3:
    """A 3D segment of float32, from A to B"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec3, b: vec3) -> None:
        """Construct line segment from two endpoints"""

    @property
    def A(self) -> vec3:
        """First endpoint of line segment"""

    @A.setter
    def A(self, arg: vec3, /) -> None: ...

    @property
    def B(self) -> vec3:
        """Second endpoint of line segment"""

    @B.setter
    def B(self, arg: vec3, /) -> None: ...

    def construct(self, a: vec3, b: vec3) -> None:
        """Set line segment endpoints"""

    def direction(self) -> vec3:
        """Get direction vector (B - A)"""

    def to_ray(self) -> ray3:
        """Get ray representation of this line segment"""

    def bounds(self) -> aabb3:
        """Get axis-aligned bounding box containing this line segment"""

    def length_sq(self) -> float:
        """Get squared length of line segment"""

    def contains(self, point: vec3) -> bool:
        """Test if point lies on the line segment"""

    def distance_sq(self, point: vec3) -> float:
        """Get squared distance from line segment to point"""

    def distance(self, point: vec3) -> float:
        """Get distance from line segment to point"""

    def point(self, t: float) -> vec3:
        """Get the point A + (B - A) t; exactly A at t = 0, exactly B at t = 1"""

    def parameter(self, point: vec3) -> float:
        """Get t in [0, 1] of the nearest point on the segment; 0 when A == B"""

    def closest_point(self, point: vec3) -> vec3:
        """Get the point on the segment nearest to point"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class segment3d:
    """A 3D segment of float64, from A to B"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec3d, b: vec3d) -> None:
        """Construct line segment from two endpoints"""

    @property
    def A(self) -> vec3d:
        """First endpoint of line segment"""

    @A.setter
    def A(self, arg: vec3d, /) -> None: ...

    @property
    def B(self) -> vec3d:
        """Second endpoint of line segment"""

    @B.setter
    def B(self, arg: vec3d, /) -> None: ...

    def construct(self, a: vec3d, b: vec3d) -> None:
        """Set line segment endpoints"""

    def direction(self) -> vec3d:
        """Get direction vector (B - A)"""

    def to_ray(self) -> ray3d:
        """Get ray representation of this line segment"""

    def bounds(self) -> aabb3d:
        """Get axis-aligned bounding box containing this line segment"""

    def length_sq(self) -> float:
        """Get squared length of line segment"""

    def contains(self, point: vec3d) -> bool:
        """Test if point lies on the line segment"""

    def distance_sq(self, point: vec3d) -> float:
        """Get squared distance from line segment to point"""

    def distance(self, point: vec3d) -> float:
        """Get distance from line segment to point"""

    def point(self, t: float) -> vec3d:
        """Get the point A + (B - A) t; exactly A at t = 0, exactly B at t = 1"""

    def parameter(self, point: vec3d) -> float:
        """Get t in [0, 1] of the nearest point on the segment; 0 when A == B"""

    def closest_point(self, point: vec3d) -> vec3d:
        """Get the point on the segment nearest to point"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

@overload
def signed_angle(a: vec2, b: vec2) -> float:
    """
    The turn (-pi to pi) from a's direction to b's, positive from x towards y
    """

@overload
def signed_angle(a: vec3, b: vec3, axis: vec3) -> float:
    """
    The turn about axis (right-handed, -pi to pi) taking a's direction to b's, seen across the axis; ValueError for an axis of no length or a or b along it
    """

@overload
def signed_angle(a: vec2d, b: vec2d) -> float:
    """
    The turn (-pi to pi) from a's direction to b's, positive from x towards y
    """

@overload
def signed_angle(a: vec3d, b: vec3d, axis: vec3d) -> float:
    """
    The turn about axis (right-handed, -pi to pi) taking a's direction to b's, seen across the axis; ValueError for an axis of no length or a or b along it
    """

@overload
def slerp(p: quat, q: quat, t: float) -> quat:
    """
    Spherical interpolation from p (t = 0) to q (t = 1) along the shorter arc, at constant speed
    """

@overload
def slerp(p: quatd, q: quatd, t: float) -> quatd: ...

def smoothstep(edge0: float, edge1: float, x: float) -> float:
    """GLSL's smoothstep; ValueError for equal edges"""

class sphere2:
    """A circle: a centre and a radius, of float32"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, centre: vec2, radius: float) -> None:
        """Construct sphere from center and radius"""

    @overload
    def __init__(self, aabb: aabb2) -> None:
        """Construct bounding sphere from AABB"""

    @property
    def centre(self) -> vec2:
        """Center point of the sphere"""

    @centre.setter
    def centre(self, arg: vec2, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the sphere"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def set(self, centre: vec2, radius: float) -> None:
        """Set sphere center and radius"""

    def construct(self, aabb: aabb2) -> None:
        """Initialize sphere from AABB"""

    def bounds(self) -> aabb2:
        """Get axis-aligned bounding box containing this sphere"""

    def contains(self, point: vec2) -> bool:
        """Test if point is inside sphere"""

    @overload
    def test_intersection(self, other: sphere2) -> bool:
        """Test intersection with another sphere"""

    @overload
    def test_intersection(self, ray: ray2) -> bool:
        """Test if a ray intersects this circle"""

    @overload
    def test_intersection(self, line: segment2) -> bool:
        """Test if a line segment intersects this circle"""

    @overload
    def query_intersection(self, ray: ray2) -> tuple:
        """
        Intersect circle with ray. Returns (count, p1, p2); count is 0, 1, or 2.
        """

    @overload
    def query_intersection(self, line: segment2) -> tuple:
        """
        Intersect circle with segment. Returns (count, p1, p2); count is 0, 1, or 2.
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class sphere2d:
    """A circle: a centre and a radius, of float64"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, centre: vec2d, radius: float) -> None:
        """Construct sphere from center and radius"""

    @overload
    def __init__(self, aabb: aabb2d) -> None:
        """Construct bounding sphere from AABB"""

    @property
    def centre(self) -> vec2d:
        """Center point of the sphere"""

    @centre.setter
    def centre(self, arg: vec2d, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the sphere"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def set(self, centre: vec2d, radius: float) -> None:
        """Set sphere center and radius"""

    def construct(self, aabb: aabb2d) -> None:
        """Initialize sphere from AABB"""

    def bounds(self) -> aabb2d:
        """Get axis-aligned bounding box containing this sphere"""

    def contains(self, point: vec2d) -> bool:
        """Test if point is inside sphere"""

    @overload
    def test_intersection(self, other: sphere2d) -> bool:
        """Test intersection with another sphere"""

    @overload
    def test_intersection(self, ray: ray2d) -> bool:
        """Test if a ray intersects this circle"""

    @overload
    def test_intersection(self, line: segment2d) -> bool:
        """Test if a line segment intersects this circle"""

    @overload
    def query_intersection(self, ray: ray2d) -> tuple:
        """
        Intersect circle with ray. Returns (count, p1, p2); count is 0, 1, or 2.
        """

    @overload
    def query_intersection(self, line: segment2d) -> tuple:
        """
        Intersect circle with segment. Returns (count, p1, p2); count is 0, 1, or 2.
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class sphere3:
    """A sphere: a centre and a radius, of float32"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, centre: vec3, radius: float) -> None:
        """Construct sphere from center and radius"""

    @overload
    def __init__(self, aabb: aabb3) -> None:
        """Construct bounding sphere from AABB"""

    @property
    def centre(self) -> vec3:
        """Center point of the sphere"""

    @centre.setter
    def centre(self, arg: vec3, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the sphere"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def set(self, centre: vec3, radius: float) -> None:
        """Set sphere center and radius"""

    def construct(self, aabb: aabb3) -> None:
        """Initialize sphere from AABB"""

    def bounds(self) -> aabb3:
        """Get axis-aligned bounding box containing this sphere"""

    def contains(self, point: vec3) -> bool:
        """Test if point is inside sphere"""

    def test_intersection(self, other: sphere3) -> bool:
        """Test intersection with another sphere"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class sphere3d:
    """A sphere: a centre and a radius, of float64"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, centre: vec3d, radius: float) -> None:
        """Construct sphere from center and radius"""

    @overload
    def __init__(self, aabb: aabb3d) -> None:
        """Construct bounding sphere from AABB"""

    @property
    def centre(self) -> vec3d:
        """Center point of the sphere"""

    @centre.setter
    def centre(self, arg: vec3d, /) -> None: ...

    @property
    def radius(self) -> float:
        """Radius of the sphere"""

    @radius.setter
    def radius(self, arg: float, /) -> None: ...

    def set(self, centre: vec3d, radius: float) -> None:
        """Set sphere center and radius"""

    def construct(self, aabb: aabb3d) -> None:
        """Initialize sphere from AABB"""

    def bounds(self) -> aabb3d:
        """Get axis-aligned bounding box containing this sphere"""

    def contains(self, point: vec3d) -> bool:
        """Test if point is inside sphere"""

    def test_intersection(self, other: sphere3d) -> bool:
        """Test intersection with another sphere"""

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

def splitmix64(x: int) -> int:
    """Vigna's splitmix64 mix of 64 bits"""

@overload
def to_axis_angle(q: quat) -> tuple:
    """
    (axis, angle): the unit axis and angle (0 to pi) of q's rotation; (x, 0) for none; ValueError for no quaternion
    """

@overload
def to_axis_angle(q: quatd) -> tuple: ...

@overload
def to_quaternion(m: mat3) -> quat:
    """The unit quaternion (w >= 0) of a rotation matrix"""

@overload
def to_quaternion(m: mat4) -> quat:
    """The unit quaternion (w >= 0) of a rotation matrix's upper 3x3"""

@overload
def to_quaternion(m: mat3d) -> quatd:
    """The unit quaternion (w >= 0) of a rotation matrix"""

@overload
def to_quaternion(m: mat4d) -> quatd:
    """The unit quaternion (w >= 0) of a rotation matrix's upper 3x3"""

@overload
def to_trs(m: mat4) -> trs:
    """
    The trs whose matrix is m (x's scale negative for a mirror); ValueError for a projection, a shear or a scale of no length
    """

@overload
def to_trs(m: mat4d) -> trsd: ...

@overload
def transform_aabb(box: aabb3, m: mat4) -> aabb3:
    """
    The box of the box's corners through an affine matrix; an empty box stays empty; ValueError for a projection
    """

@overload
def transform_aabb(box: aabb3d, m: mat4d) -> aabb3d: ...

@overload
def transform_point(m: mat4, p: vec3) -> vec3:
    """m p with w = 1, not divided (an affine transform's point)"""

@overload
def transform_point(m: mat3, p: vec2) -> vec2:
    """(x, y, 1) through m, not divided"""

@overload
def transform_point(m: mat4d, p: vec3d) -> vec3d:
    """m p with w = 1, not divided (an affine transform's point)"""

@overload
def transform_point(m: mat3d, p: vec2d) -> vec2d:
    """(x, y, 1) through m, not divided"""

@overload
def transform_vector(m: mat4, v: vec3) -> vec3:
    """m v with w = 0: turned and scaled, not moved"""

@overload
def transform_vector(m: mat4d, v: vec3d) -> vec3d: ...

@overload
def translation_matrix(offset: vec3) -> mat4: ...

@overload
def translation_matrix(offset: vec3d) -> mat4d: ...

@overload
def transpose(m: mat4) -> mat4: ...

@overload
def transpose(m: mat3) -> mat3: ...

@overload
def transpose(m: mat4d) -> mat4d: ...

@overload
def transpose(m: mat3d) -> mat3d: ...

class triangle3:
    """A 3D triangle of float32, of corners A, B and C"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec3, b: vec3, c: vec3) -> None:
        """Construct triangle from three vertices"""

    @property
    def A(self) -> vec3:
        """First vertex of triangle"""

    @A.setter
    def A(self, arg: vec3, /) -> None: ...

    @property
    def B(self) -> vec3:
        """Second vertex of triangle"""

    @B.setter
    def B(self, arg: vec3, /) -> None: ...

    @property
    def C(self) -> vec3:
        """Third vertex of triangle"""

    @C.setter
    def C(self, arg: vec3, /) -> None: ...

    def construct(self, a: vec3, b: vec3, c: vec3) -> None:
        """Set triangle vertices"""

    def normal(self) -> vec3:
        """Get normalized normal vector of the triangle"""

    def to_plane(self) -> plane:
        """Get the plane containing this triangle"""

    def perimeter(self) -> float:
        """Get perimeter of the triangle"""

    def area(self) -> float:
        """Get area of the triangle"""

    def cog(self) -> vec3:
        """Get center of gravity (centroid)"""

    def orthocenter(self) -> vec3:
        """Get orthocenter (concurrence of altitudes)"""

    def circumcenter(self) -> vec3:
        """Get circumcenter (center of circumscribed circle)"""

    def circumcircle_radius(self) -> float:
        """Get radius of circumscribed circle"""

    def circumcircle(self) -> sphere3:
        """Get circumscribed sphere (passes through all three vertices)"""

    def incenter(self) -> vec3:
        """Get incenter (center of inscribed circle)"""

    def incircle_radius(self) -> float:
        """Get radius of inscribed circle"""

    def incircle(self) -> sphere3:
        """Get inscribed sphere (tangent to all three edges)"""

    def excenter_A(self) -> vec3:
        """Get excenter opposite to vertex A"""

    def excircle_A_radius(self) -> float:
        """Get radius of excircle opposite to vertex A"""

    def excenter_B(self) -> vec3:
        """Get excenter opposite to vertex B"""

    def excircle_B_radius(self) -> float:
        """Get radius of excircle opposite to vertex B"""

    def excenter_C(self) -> vec3:
        """Get excenter opposite to vertex C"""

    def excircle_C_radius(self) -> float:
        """Get radius of excircle opposite to vertex C"""

    def feuerbach_center(self) -> vec3:
        """Get Feuerbach circle center (nine-point circle)"""

    def feuerbach_radius(self) -> float:
        """Get Feuerbach circle radius"""

    def contains_on_plane(self, point: vec3) -> bool:
        """Test if point on triangle's plane is inside the triangle"""

    def contains(self, point: vec3) -> bool:
        """Test if 3D point is inside the triangle"""

    def trace(self, ray: ray3, t_min: float = 0, t_max: float = 1, epsilon: float = 9.999999974752427e-07) -> bool:
        """
        Test if ray intersects triangle within parameter range; a determinant under epsilon counts as parallel
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class triangle3d:
    """A 3D triangle of float64, of corners A, B and C"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, a: vec3d, b: vec3d, c: vec3d) -> None:
        """Construct triangle from three vertices"""

    @property
    def A(self) -> vec3d:
        """First vertex of triangle"""

    @A.setter
    def A(self, arg: vec3d, /) -> None: ...

    @property
    def B(self) -> vec3d:
        """Second vertex of triangle"""

    @B.setter
    def B(self, arg: vec3d, /) -> None: ...

    @property
    def C(self) -> vec3d:
        """Third vertex of triangle"""

    @C.setter
    def C(self, arg: vec3d, /) -> None: ...

    def construct(self, a: vec3d, b: vec3d, c: vec3d) -> None:
        """Set triangle vertices"""

    def normal(self) -> vec3d:
        """Get normalized normal vector of the triangle"""

    def to_plane(self) -> planed:
        """Get the plane containing this triangle"""

    def perimeter(self) -> float:
        """Get perimeter of the triangle"""

    def area(self) -> float:
        """Get area of the triangle"""

    def cog(self) -> vec3d:
        """Get center of gravity (centroid)"""

    def orthocenter(self) -> vec3d:
        """Get orthocenter (concurrence of altitudes)"""

    def circumcenter(self) -> vec3d:
        """Get circumcenter (center of circumscribed circle)"""

    def circumcircle_radius(self) -> float:
        """Get radius of circumscribed circle"""

    def circumcircle(self) -> sphere3d:
        """Get circumscribed sphere (passes through all three vertices)"""

    def incenter(self) -> vec3d:
        """Get incenter (center of inscribed circle)"""

    def incircle_radius(self) -> float:
        """Get radius of inscribed circle"""

    def incircle(self) -> sphere3d:
        """Get inscribed sphere (tangent to all three edges)"""

    def excenter_A(self) -> vec3d:
        """Get excenter opposite to vertex A"""

    def excircle_A_radius(self) -> float:
        """Get radius of excircle opposite to vertex A"""

    def excenter_B(self) -> vec3d:
        """Get excenter opposite to vertex B"""

    def excircle_B_radius(self) -> float:
        """Get radius of excircle opposite to vertex B"""

    def excenter_C(self) -> vec3d:
        """Get excenter opposite to vertex C"""

    def excircle_C_radius(self) -> float:
        """Get radius of excircle opposite to vertex C"""

    def feuerbach_center(self) -> vec3d:
        """Get Feuerbach circle center (nine-point circle)"""

    def feuerbach_radius(self) -> float:
        """Get Feuerbach circle radius"""

    def contains_on_plane(self, point: vec3d) -> bool:
        """Test if point on triangle's plane is inside the triangle"""

    def contains(self, point: vec3d) -> bool:
        """Test if 3D point is inside the triangle"""

    def trace(self, ray: ray3d, t_min: float = 0, t_max: float = 1, epsilon: float = 1e-12) -> bool:
        """
        Test if ray intersects triangle within parameter range; a determinant under epsilon counts as parallel
        """

    def __repr__(self) -> str: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class trs:
    """
    A transform kept as its translation, rotation and scale, whose matrix scales, then turns, then moves (T R S, as glTF's nodes and a skeleton's joints)
    """

    @overload
    def __init__(self) -> None:
        """No transform: no move, no turn, a scale of 1"""

    @overload
    def __init__(self, translation: vec3, rotation: quat, scale: vec3) -> None: ...

    @property
    def translation(self) -> vec3: ...

    @translation.setter
    def translation(self, arg: vec3, /) -> None: ...

    @property
    def rotation(self) -> quat: ...

    @rotation.setter
    def rotation(self, arg: quat, /) -> None: ...

    @property
    def scale(self) -> vec3: ...

    @scale.setter
    def scale(self, arg: vec3, /) -> None: ...

    def to_matrix(self) -> mat4:
        """T R S"""

    def apply_point(self, p: vec3) -> vec3:
        """The point scaled, turned and moved"""

    def apply_vector(self, v: vec3) -> vec3:
        """The vector scaled and turned, not moved"""

    def __mul__(self, arg: trs, /) -> trs:
        """
        a then b's parent: the matrices' product, where a's scale is uniform or b does not turn (a shear otherwise: ValueError)
        """

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __repr__(self) -> str: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

class trsd:
    """
    A transform kept as its translation, rotation and scale, whose matrix scales, then turns, then moves (T R S, as glTF's nodes and a skeleton's joints)
    """

    @overload
    def __init__(self) -> None:
        """No transform: no move, no turn, a scale of 1"""

    @overload
    def __init__(self, translation: vec3d, rotation: quatd, scale: vec3d) -> None: ...

    @property
    def translation(self) -> vec3d: ...

    @translation.setter
    def translation(self, arg: vec3d, /) -> None: ...

    @property
    def rotation(self) -> quatd: ...

    @rotation.setter
    def rotation(self, arg: quatd, /) -> None: ...

    @property
    def scale(self) -> vec3d: ...

    @scale.setter
    def scale(self, arg: vec3d, /) -> None: ...

    def to_matrix(self) -> mat4d:
        """T R S"""

    def apply_point(self, p: vec3d) -> vec3d:
        """The point scaled, turned and moved"""

    def apply_vector(self, v: vec3d) -> vec3d:
        """The vector scaled and turned, not moved"""

    def __mul__(self, arg: trsd, /) -> trsd:
        """
        a then b's parent: the matrices' product, where a's scale is uniform or b does not turn (a shear otherwise: ValueError)
        """

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __repr__(self) -> str: ...

    def __eq__(self, arg: object, /) -> object: ...

    def __ne__(self, arg: object, /) -> object: ...

    def __hash__(self) -> int: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

@overload
def unproject_ray(inverse_view_projection: mat4, ndc_x: float, ndc_y: float) -> ray3:
    """
    The ray under a point of the screen (-1 to 1, +y up), from the near plane (r0) to the far (r0 + a); ValueError for a matrix that takes the point to no finite place
    """

@overload
def unproject_ray(inverse_view_projection: mat4d, ndc_x: float, ndc_y: float) -> ray3d: ...

@overload
def value_noise(x: float, *, seed: int) -> float:
    """Value noise in [-1, 1], smooth, the seed's own"""

@overload
def value_noise(x: float, y: float, *, seed: int) -> float: ...

@overload
def value_noise(x: float, y: float, z: float, *, seed: int) -> float: ...

class vec2:
    """A 2D vector of float32"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def xy(self) -> tuple: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float32', shape=(2), writable=False)]) -> vec2:
        """
        A vec2 from a numpy array of 2 numbers, read-only or not; numbers of another type are converted to this one's (rounded, for float64 to a float32 vec2)
        """

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(2))]:
        """Convert vec2 to numpy array"""

    def set(self, x: float, y: float) -> None: ...

    def set_all(self, a: float) -> None: ...

    def length(self) -> float: ...

    def length_sq(self) -> float: ...

    def manhattan_length(self) -> float: ...

    def normalize(self) -> None:
        """
        Unit length in place; ValueError for a vector of no length or not finite
        """

    def negate(self) -> None: ...

    def perpendicular(self) -> vec2: ...

    def is_collinear(self, v: vec2, epsilon: float = 9.999999974752427e-07) -> bool: ...

    def __neg__(self) -> vec2: ...

    def __add__(self, arg: vec2, /) -> vec2: ...

    def __sub__(self, arg: vec2, /) -> vec2: ...

    @overload
    def __mul__(self, arg: vec2, /) -> vec2: ...

    @overload
    def __mul__(self, arg: float, /) -> vec2: ...

    def __rmul__(self, arg: float, /) -> vec2: ...

    @overload
    def __truediv__(self, arg: vec2, /) -> vec2: ...

    @overload
    def __truediv__(self, arg: float, /) -> vec2: ...

    def __iadd__(self, v: vec2) -> vec2: ...

    def __isub__(self, v: vec2) -> vec2: ...

    @overload
    def __imul__(self, v: vec2) -> vec2: ...

    @overload
    def __imul__(self, k: float) -> vec2: ...

    @overload
    def __itruediv__(self, v: vec2) -> vec2: ...

    @overload
    def __itruediv__(self, k: float) -> vec2: ...

    def __eq__(self, arg: vec2, /) -> bool: ...

    def __ne__(self, arg: vec2, /) -> bool: ...

    def __hash__(self) -> int: ...

    def __len__(self) -> int: ...

    def __getitem__(self, arg: int, /) -> float: ...

    def __setitem__(self, arg0: int, arg1: float, /) -> None: ...

    def __iter__(self) -> Iterator[float]: ...

    def __repr__(self) -> str: ...

    def dot(self, arg: vec2, /) -> float: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class vec2d:
    """A 2D vector of float64"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def xy(self) -> tuple: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float64', shape=(2), writable=False)]) -> vec2d:
        """
        A vec2 from a numpy array of 2 numbers, read-only or not; numbers of another type are converted to this one's (rounded, for float64 to a float32 vec2)
        """

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float64', shape=(2))]:
        """Convert vec2 to numpy array"""

    def set(self, x: float, y: float) -> None: ...

    def set_all(self, a: float) -> None: ...

    def length(self) -> float: ...

    def length_sq(self) -> float: ...

    def manhattan_length(self) -> float: ...

    def normalize(self) -> None:
        """
        Unit length in place; ValueError for a vector of no length or not finite
        """

    def negate(self) -> None: ...

    def perpendicular(self) -> vec2d: ...

    def is_collinear(self, v: vec2d, epsilon: float = 1e-12) -> bool: ...

    def __neg__(self) -> vec2d: ...

    def __add__(self, arg: vec2d, /) -> vec2d: ...

    def __sub__(self, arg: vec2d, /) -> vec2d: ...

    @overload
    def __mul__(self, arg: vec2d, /) -> vec2d: ...

    @overload
    def __mul__(self, arg: float, /) -> vec2d: ...

    def __rmul__(self, arg: float, /) -> vec2d: ...

    @overload
    def __truediv__(self, arg: vec2d, /) -> vec2d: ...

    @overload
    def __truediv__(self, arg: float, /) -> vec2d: ...

    def __iadd__(self, v: vec2d) -> vec2d: ...

    def __isub__(self, v: vec2d) -> vec2d: ...

    @overload
    def __imul__(self, v: vec2d) -> vec2d: ...

    @overload
    def __imul__(self, k: float) -> vec2d: ...

    @overload
    def __itruediv__(self, v: vec2d) -> vec2d: ...

    @overload
    def __itruediv__(self, k: float) -> vec2d: ...

    def __eq__(self, arg: vec2d, /) -> bool: ...

    def __ne__(self, arg: vec2d, /) -> bool: ...

    def __hash__(self) -> int: ...

    def __len__(self) -> int: ...

    def __getitem__(self, arg: int, /) -> float: ...

    def __setitem__(self, arg0: int, arg1: float, /) -> None: ...

    def __iter__(self) -> Iterator[float]: ...

    def __repr__(self) -> str: ...

    def dot(self, arg: vec2d, /) -> float: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class vec3:
    """A 3D vector of float32"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float, z: float) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def xy(self) -> tuple: ...

    @property
    def xyz(self) -> tuple: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float32', shape=(3), writable=False)]) -> vec3:
        """
        A vec3 from a numpy array of 3 numbers, read-only or not; numbers of another type are converted to this one's (rounded, for float64 to a float32 vec3)
        """

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(3))]:
        """Convert vec3 to numpy array"""

    def set(self, x: float, y: float, z: float) -> None: ...

    def set_all(self, a: float) -> None: ...

    def length(self) -> float: ...

    def length_sq(self) -> float: ...

    def manhattan_length(self) -> float: ...

    def normalize(self) -> None:
        """
        Unit length in place; ValueError for a vector of no length or not finite
        """

    def negate(self) -> None: ...

    def is_collinear(self, v: vec3, epsilon: float = 9.999999974752427e-07) -> bool: ...

    def __neg__(self) -> vec3: ...

    def __add__(self, arg: vec3, /) -> vec3: ...

    def __sub__(self, arg: vec3, /) -> vec3: ...

    @overload
    def __mul__(self, arg: vec3, /) -> vec3: ...

    @overload
    def __mul__(self, arg: float, /) -> vec3: ...

    def __rmul__(self, arg: float, /) -> vec3: ...

    @overload
    def __truediv__(self, arg: vec3, /) -> vec3: ...

    @overload
    def __truediv__(self, arg: float, /) -> vec3: ...

    def __iadd__(self, v: vec3) -> vec3: ...

    def __isub__(self, v: vec3) -> vec3: ...

    @overload
    def __imul__(self, v: vec3) -> vec3: ...

    @overload
    def __imul__(self, k: float) -> vec3: ...

    @overload
    def __itruediv__(self, v: vec3) -> vec3: ...

    @overload
    def __itruediv__(self, k: float) -> vec3: ...

    def __eq__(self, arg: vec3, /) -> bool: ...

    def __ne__(self, arg: vec3, /) -> bool: ...

    def __hash__(self) -> int: ...

    def __len__(self) -> int: ...

    def __getitem__(self, arg: int, /) -> float: ...

    def __setitem__(self, arg0: int, arg1: float, /) -> None: ...

    def __iter__(self) -> Iterator[float]: ...

    def __repr__(self) -> str: ...

    def dot(self, arg: vec3, /) -> float: ...

    def cross(self, arg: vec3, /) -> vec3: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class vec3d:
    """A 3D vector of float64"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float, z: float) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def xy(self) -> tuple: ...

    @property
    def xyz(self) -> tuple: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float64', shape=(3), writable=False)]) -> vec3d:
        """
        A vec3 from a numpy array of 3 numbers, read-only or not; numbers of another type are converted to this one's (rounded, for float64 to a float32 vec3)
        """

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float64', shape=(3))]:
        """Convert vec3 to numpy array"""

    def set(self, x: float, y: float, z: float) -> None: ...

    def set_all(self, a: float) -> None: ...

    def length(self) -> float: ...

    def length_sq(self) -> float: ...

    def manhattan_length(self) -> float: ...

    def normalize(self) -> None:
        """
        Unit length in place; ValueError for a vector of no length or not finite
        """

    def negate(self) -> None: ...

    def is_collinear(self, v: vec3d, epsilon: float = 1e-12) -> bool: ...

    def __neg__(self) -> vec3d: ...

    def __add__(self, arg: vec3d, /) -> vec3d: ...

    def __sub__(self, arg: vec3d, /) -> vec3d: ...

    @overload
    def __mul__(self, arg: vec3d, /) -> vec3d: ...

    @overload
    def __mul__(self, arg: float, /) -> vec3d: ...

    def __rmul__(self, arg: float, /) -> vec3d: ...

    @overload
    def __truediv__(self, arg: vec3d, /) -> vec3d: ...

    @overload
    def __truediv__(self, arg: float, /) -> vec3d: ...

    def __iadd__(self, v: vec3d) -> vec3d: ...

    def __isub__(self, v: vec3d) -> vec3d: ...

    @overload
    def __imul__(self, v: vec3d) -> vec3d: ...

    @overload
    def __imul__(self, k: float) -> vec3d: ...

    @overload
    def __itruediv__(self, v: vec3d) -> vec3d: ...

    @overload
    def __itruediv__(self, k: float) -> vec3d: ...

    def __eq__(self, arg: vec3d, /) -> bool: ...

    def __ne__(self, arg: vec3d, /) -> bool: ...

    def __hash__(self) -> int: ...

    def __len__(self) -> int: ...

    def __getitem__(self, arg: int, /) -> float: ...

    def __setitem__(self, arg0: int, arg1: float, /) -> None: ...

    def __iter__(self) -> Iterator[float]: ...

    def __repr__(self) -> str: ...

    def dot(self, arg: vec3d, /) -> float: ...

    def cross(self, arg: vec3d, /) -> vec3d: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class vec4:
    """A 4D vector of float32"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float, z: float, w: float) -> None: ...

    @overload
    def __init__(self, xyz: vec3, w: float) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def w(self) -> float: ...

    @w.setter
    def w(self, arg: float, /) -> None: ...

    @property
    def xy(self) -> tuple: ...

    @property
    def xyz(self) -> tuple: ...

    @property
    def xyzw(self) -> tuple: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float32', shape=(4), writable=False)]) -> vec4:
        """
        A vec4 from a numpy array of 4 numbers, read-only or not; numbers of another type are converted to this one's (rounded, for float64 to a float32 vec4)
        """

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float32', shape=(4))]:
        """Convert vec4 to numpy array"""

    def set(self, x: float, y: float, z: float, w: float) -> None: ...

    def set_all(self, a: float) -> None: ...

    def length(self) -> float: ...

    def length_sq(self) -> float: ...

    def manhattan_length(self) -> float: ...

    def normalize(self) -> None:
        """
        Unit length in place; ValueError for a vector of no length or not finite
        """

    def negate(self) -> None: ...

    def __neg__(self) -> vec4: ...

    def __add__(self, arg: vec4, /) -> vec4: ...

    def __sub__(self, arg: vec4, /) -> vec4: ...

    @overload
    def __mul__(self, arg: vec4, /) -> vec4: ...

    @overload
    def __mul__(self, arg: float, /) -> vec4: ...

    def __rmul__(self, arg: float, /) -> vec4: ...

    @overload
    def __truediv__(self, arg: vec4, /) -> vec4: ...

    @overload
    def __truediv__(self, arg: float, /) -> vec4: ...

    def __iadd__(self, v: vec4) -> vec4: ...

    def __isub__(self, v: vec4) -> vec4: ...

    @overload
    def __imul__(self, v: vec4) -> vec4: ...

    @overload
    def __imul__(self, k: float) -> vec4: ...

    @overload
    def __itruediv__(self, v: vec4) -> vec4: ...

    @overload
    def __itruediv__(self, k: float) -> vec4: ...

    def __eq__(self, arg: vec4, /) -> bool: ...

    def __ne__(self, arg: vec4, /) -> bool: ...

    def __hash__(self) -> int: ...

    def __len__(self) -> int: ...

    def __getitem__(self, arg: int, /) -> float: ...

    def __setitem__(self, arg0: int, arg1: float, /) -> None: ...

    def __iter__(self) -> Iterator[float]: ...

    def __repr__(self) -> str: ...

    def dot(self, arg: vec4, /) -> float: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

class vec4d:
    """A 4D vector of float64"""

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, x: float, y: float, z: float, w: float) -> None: ...

    @overload
    def __init__(self, xyz: vec3d, w: float) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def w(self) -> float: ...

    @w.setter
    def w(self, arg: float, /) -> None: ...

    @property
    def xy(self) -> tuple: ...

    @property
    def xyz(self) -> tuple: ...

    @property
    def xyzw(self) -> tuple: ...

    @staticmethod
    def from_numpy(array: Annotated[ArrayLike, dict(dtype='float64', shape=(4), writable=False)]) -> vec4d:
        """
        A vec4 from a numpy array of 4 numbers, read-only or not; numbers of another type are converted to this one's (rounded, for float64 to a float32 vec4)
        """

    def to_numpy(self) -> Annotated[ArrayLike, dict(dtype='float64', shape=(4))]:
        """Convert vec4 to numpy array"""

    def set(self, x: float, y: float, z: float, w: float) -> None: ...

    def set_all(self, a: float) -> None: ...

    def length(self) -> float: ...

    def length_sq(self) -> float: ...

    def manhattan_length(self) -> float: ...

    def normalize(self) -> None:
        """
        Unit length in place; ValueError for a vector of no length or not finite
        """

    def negate(self) -> None: ...

    def __neg__(self) -> vec4d: ...

    def __add__(self, arg: vec4d, /) -> vec4d: ...

    def __sub__(self, arg: vec4d, /) -> vec4d: ...

    @overload
    def __mul__(self, arg: vec4d, /) -> vec4d: ...

    @overload
    def __mul__(self, arg: float, /) -> vec4d: ...

    def __rmul__(self, arg: float, /) -> vec4d: ...

    @overload
    def __truediv__(self, arg: vec4d, /) -> vec4d: ...

    @overload
    def __truediv__(self, arg: float, /) -> vec4d: ...

    def __iadd__(self, v: vec4d) -> vec4d: ...

    def __isub__(self, v: vec4d) -> vec4d: ...

    @overload
    def __imul__(self, v: vec4d) -> vec4d: ...

    @overload
    def __imul__(self, k: float) -> vec4d: ...

    @overload
    def __itruediv__(self, v: vec4d) -> vec4d: ...

    @overload
    def __itruediv__(self, k: float) -> vec4d: ...

    def __eq__(self, arg: vec4d, /) -> bool: ...

    def __ne__(self, arg: vec4d, /) -> bool: ...

    def __hash__(self) -> int: ...

    def __len__(self) -> int: ...

    def __getitem__(self, arg: int, /) -> float: ...

    def __setitem__(self, arg0: int, arg1: float, /) -> None: ...

    def __iter__(self) -> Iterator[float]: ...

    def __repr__(self) -> str: ...

    def dot(self, arg: vec4d, /) -> float: ...

    def __getstate__(self) -> tuple: ...

    def __setstate__(self, arg: tuple, /) -> None: ...

    def __reduce_ex__(self, protocol: int) -> tuple: ...

    def __array__(self, dtype: object | None = None, copy: object | None = None) -> object: ...

    __array_ufunc__: None = None

def wrap(x: float, lo: float, hi: float) -> float:
    """x into [lo, hi), which must hold zero (ValueError otherwise)"""

def wrap_to_pi(a: float) -> float:
    """An angle into (-pi, pi]"""

def wrap_to_two_pi(a: float) -> float:
    """An angle into [0, 2 pi)"""
