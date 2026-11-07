// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

// What <windows.h> defines without NOMINMAX, before rove's headers.  It compiles here on any platform: this is the test that MSVC would
// not have to be run for.  (The standard library goes first: libstdc++ is not protected from these, MSVC's own is.)

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <memory>
#include <numbers>
#include <optional>
#include <ostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#define near
#define far
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#include <rove/a_star.h>
#include <rove/aabb.h>
#include <rove/bresenham_supercover.h>
#include <rove/capsule.h>
#include <rove/collision.h>
#include <rove/cone.h>
#include <rove/contact_info.h>
#include <rove/frustum.h>
#include <rove/hash.h>
#include <rove/hash_grid.h>
#include <rove/heightfield.h>
#include <rove/intersect.h>
#include <rove/interval.h>
#include <rove/matrix.h>
#include <rove/noise.h>
#include <rove/obb.h>
#include <rove/octree.h>
#include <rove/plane.h>
#include <rove/quaternion.h>
#include <rove/random.h>
#include <rove/ray.h>
#include <rove/scalar.h>
#include <rove/segment.h>
#include <rove/sphere.h>
#include <rove/traits.h>
#include <rove/transform.h>
#include <rove/triangle.h>
#include <rove/vec.h>

#undef near
#undef far
#undef min
#undef max

// Instantiated with the macros gone from here on, but parsed under them above, which is what they break.
template class rove::aabb<3, float>;
