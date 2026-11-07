// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <cstddef>
#include <optional>

#include "scalar.h"
#include "vec.h"

namespace rove
{

template<int N, class T = scalar>
struct [[nodiscard]] contact_info {
	static constexpr size_t dimensions = N;
	using vec_type = vec<dimensions, T>;

	// (all of it defined on a miss too: nothing here is left unset for a reader to trip on)
	bool happened = false, penetrated = false;
	vec_type position, normal;
	T time = T(0), penetration_depth = T(0);
	/// Which item of a collection was hit (hash_grid::trace); empty for one
	/// shape against another.
	std::optional<size_t> index;

	/// Whether `ci` comes before this one: any contact over none, a penetration over a touch, the shallower of two penetrations, the
	/// earlier of two touches
	bool worse_than(contact_info const &ci) const {
		if (ci.happened == false) return false;
		if (happened == false) return true;
		if (penetrated == false && ci.penetrated == true) return true;
		if (penetrated == true && ci.penetrated == true) return penetration_depth > ci.penetration_depth;
		return time > ci.time;
	}
};

}
