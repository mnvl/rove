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

	bool happened, penetrated;
	vec_type position, normal;
	T time, penetration_depth;
	/// Which item of a collection was hit (hash_grid::trace); empty for one
	/// shape against another.
	std::optional<size_t> index;

	bool worse_than(contact_info const &ci) const {
		if (ci.happened == false) return false;
		if (happened == false) return true;
		if (penetrated == false && ci.penetrated == true) return true;
		if (penetrated == true && ci.penetrated == true) return penetration_depth > ci.penetration_depth;
		return time > ci.time;
	}
};

}
