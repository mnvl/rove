// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

// Compiled with -Wall -Wextra -Werror and the headers as non-system (CMakeLists.txt): a consumer's flags must not fail on them.

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
#include <rove/format.h>

namespace
{
// Templates only warn once instantiated
[[maybe_unused]] rove::aabb<3> moved_box(rove::aabb<3> const &b, rove::matrix<4, 4> const &m) { return rove::transform_aabb(b, m); }
[[maybe_unused]] rove::ray<3> unprojected(rove::matrix<4, 4> const &m) { return rove::unproject_ray(m, 0.0f, 0.0f); }
}
