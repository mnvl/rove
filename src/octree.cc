// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "octree.h"

namespace rove
{

template class octree<aabb<3>, int>;
template class octree<sphere<3>, int>;
template class octree<triangle<3>, int>;

}
