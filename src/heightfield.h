// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <vector>
#include "vec.h"
#include "aabb.h"
#include "segment.h"
#include "matrix.h"
#include "collision.h"

namespace rove
{

template<class T = scalar>
class heightfield {
public:
	using value_type = T;
	using cell_type = vec<2, int>;

	heightfield(matrix<4,4> const &tf, size_t ncols, size_t nrows);
	~heightfield();

	value_type local_y_at(int col, int row) const { return heights_.at(row * ncols_ + col); }
	value_type &local_y_at(int col, int row) { return heights_.at(row * ncols_ + col); }
	vec<3> local_vertex_at(int col, int row) const { return vec<3>(scalar(col), local_y_at(col, row), scalar(row)); }

	// Wrong by the house rule, kept as it was: off the field (and, for max_y_in_cell, outside a cell) these answer default_value, a
	// plausible height, instead of saying there is none.  A caller passes a value it cannot mistake (a NaN) and checks for it.  The
	// class is templated on T but works in scalar and matrix<4,4> whatever T is.
	scalar y_under(vec<3> const &position, scalar default_value = 0);
	scalar max_y_in_cell(int col, int row, scalar default_value = 0) const;
	scalar max_y_in_cell(vec<3> const &position, scalar default_value = 0) const;

	size_t ncols() const { return ncols_; }
	size_t nrows() const { return nrows_; }
	matrix<4,4> const &get_local_to_world() const { return local_to_world_; }
	matrix<4,4> const &get_world_to_local() const { return world_to_local_; }
	aabb<3> const &get_local_aabb() const { return local_aabb_; }

	void set_local_to_world(rove::matrix<4, 4> const &tf);
	cell_type world_position_to_cell(vec<3> const &position) const;
	vec<3> cell_to_world_position(cell_type const &cell) const;
	contact_info<3> trace(ray<3> const &r, scalar max_distance = 1000.0f) const;
	void load_from_raw_buffer(value_type const *heights);
	void resize(size_t ncols, size_t nrows);
	std::vector<cell_type> build_path(vec<3> const &from, vec<3> const &to, value_type min_value = 0,
		std::vector<cell_type> const &obstacles = std::vector<cell_type>()) const;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & local_to_world_ & world_to_local_ & ncols_ & nrows_ & heights_ & local_aabb_;
	}

private:
	matrix<4,4> local_to_world_, world_to_local_;
	size_t ncols_, nrows_;
	std::vector<value_type> heights_;
	aabb<3> local_aabb_;

	size_t nsamples()
	{
		return nrows_ * ncols_;
	}

	void post_load();
};

}
