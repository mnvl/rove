// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <stdexcept>
#include <utility>
#include <cstdint>
#include <tuple>
#include <unordered_set>
#include "a_star.h"
#include "bresenham_supercover.h"
#include "heightfield.h"

namespace rove
{

template<class T>
heightfield<T>::heightfield(matrix<4,4> const &tf, size_t nrows, size_t ncols):
	ncols_(ncols),
	nrows_(nrows)
{
	set_local_to_world(tf);

	heights_.resize(nrows * ncols);

	local_aabb_.lo.set(0, -1, 0);
	local_aabb_.hi.set(rove::scalar(ncols_ - 1), 1, rove::scalar(nrows_ - 1));
}

template<class T>
heightfield<T>::~heightfield()
{
}

template<class T>
scalar heightfield<T>::y_under(vec<3> const &position, scalar default_value)
{
	vec<3> local_position = world_to_local_ * position;

	int col = int(local_position.x);
	int row = int(local_position.z);

	if (col < 0 || row < 0 || col >= int(ncols_) - 1 || row >= int(nrows_) - 1) return default_value;

	scalar v1 = local_y_at(col + 0, row + 0);
	scalar v2 = local_y_at(col + 1, row + 0);
	scalar v3 = local_y_at(col + 0, row + 1);
	scalar v4 = local_y_at(col + 1, row + 1);

	scalar k1 = local_position.x - col;
	scalar k2 = local_position.z - col;

	scalar v12 = v1 * (1 - k1) + v2 * k1;
	scalar v34 = v3 * (1 - k1) + v4 * k1;

	return v12 * (1 - k2) + v34 * k2;
}

template<class T>
scalar heightfield<T>::max_y_in_cell(int col, int row, scalar default_value) const
{
	if (col < 0 || row < 0 || col >= int(ncols_) - 1 || row >= int(nrows_) - 1)
	{
		return default_value;
	}

	scalar result = scalar(local_y_at(col + 0, row + 0));
	result = std::max(result, scalar(local_y_at(col + 0, row + 1)));
	result = std::max(result, scalar(local_y_at(col + 1, row + 0)));
	result = std::max(result, scalar(local_y_at(col + 1, row + 1)));

	return result;
}

template<class T>
scalar heightfield<T>::max_y_in_cell(vec<3> const &position, scalar default_value) const
{
	vec<3> local_position = world_to_local_ * position;

	int col = int(local_position.x);
	int row = int(local_position.z);

	return max_y_in_cell(col, row, default_value);
}

template<class T>
void heightfield<T>::set_local_to_world(rove::matrix<4, 4> const &tf)
{
	world_to_local_ = inverse(tf.check_invertible("heightfield::set_local_to_world: a matrix"));
	local_to_world_ = tf;
}

template<class T> typename heightfield<T>::cell_type
heightfield<T>::world_position_to_cell(vec<3> const &position) const
{
	vec<3> p = world_to_local_ * position;
	return cell_type(int(p.x), int(p.z));
}

template<class T>
vec<3> heightfield<T>::cell_to_world_position(cell_type const &cell) const
{
	return local_to_world_ * vec<3>(scalar(cell.x) + 0.5f, 0, scalar(cell.y) + 0.5f);
}

template<class T>
contact_info<3> heightfield<T>::trace(ray<3> const &r, scalar max_distance) const
{
	vec<3> local_p0 = world_to_local_ * r.r0;
	vec<3> local_p1 = world_to_local_ * (r.r0 + normalize_or_zero(r.a) * max_distance);

	ray<3> local_ray(local_p0, local_p1 - local_p0);

	contact_info<3> result;
	result.happened = false;

	scalar local_ray_t0, local_ray_t1;

	if (!local_aabb_.trace(local_ray, &local_ray_t0, &local_ray_t1)) return result;

	vec<3> local_p0_adjusted = local_ray.apply(local_ray_t0);
	vec<3> local_p1_adjusted = local_ray.apply(local_ray_t1);

	bresenham_supercover<> cells_iterator(cell_type(int (local_p0_adjusted.x), int(local_p0_adjusted.z)),
		cell_type(int(local_p1_adjusted.x), int(local_p1_adjusted.z)));

	while (true)
	{
		cell_type cell;
		bool flag;

		std::tie(cell, flag) = cells_iterator.get();
		if (flag) break;

		if (cell.x == static_cast<int>(ncols_) - 1) --cell.x;
		if (cell.y == static_cast<int>(nrows_) - 1) --cell.y;

		vec<3> v1 = local_vertex_at(cell.x + 0, cell.y + 0);
		vec<3> v2 = local_vertex_at(cell.x + 0, cell.y + 1);
		vec<3> v3 = local_vertex_at(cell.x + 1, cell.y + 1);
		vec<3> v4 = local_vertex_at(cell.x + 1, cell.y + 0);

		triangle<3> t1(v1, v2, v3);
		triangle<3> t2(v1, v3, v4);

		bool trace_h1, trace_h2;
		scalar trace_t1 = 0, trace_t2 = 0;     // (read only after a hit sets them)
		vec<3> trace_p1, trace_p2;

		trace_h1 = t1.trace(local_ray, trace_t1, trace_p1);
		trace_h2 = t2.trace(local_ray, trace_t2, trace_p2);

		if (trace_h1 || trace_h2)
		{
			contact_info<3> ci;

			ci.happened = true;
			ci.penetrated = false;

			if (trace_h1 && ((trace_h2 && trace_t1 < trace_t2) || trace_h2 == false))
			{
				ci.position = trace_p1;
				ci.normal = t1.normal();
				ci.time = trace_t1;
			}
			else
			{
				ci.position = trace_p2;
				ci.normal = t2.normal();
				ci.time = trace_t2;
			}

			if (result.worse_than(ci))
			{
				result = ci;
			}
		}
	}

	result.position = local_to_world_ * result.position;

	return result;
}

template<class T>
void heightfield<T>::load_from_raw_buffer(value_type const *heights)
{
	heights_.assign(heights, heights + nsamples());

	post_load();
}

template<class T>
void heightfield<T>::resize(size_t ncols, size_t nrows)
{
	std::vector<value_type> heights;
	heights.resize(nrows * ncols);

	for (size_t i = 0, ni = std::min(nrows, nrows_); i < ni; ++i)
	{
		for (size_t j = 0, nj = std::min(ncols, ncols_); j < nj; ++j)
		{
			heights[i * ncols + j] = heights_[i * ncols_ + j];
		}
	}

	heights_.swap(heights);
	ncols_ = ncols;
	nrows_ = nrows;

	post_load();
}

namespace
{
template<class T>
class scorer_closure {
public:
	using cell_type = typename heightfield<T>::cell_type;

	scorer_closure(heightfield<T> const *hf, scalar min_value, std::vector<cell_type> const &obstacles):
		hf_(hf),
		min_value_(min_value)
	{
		obstacles_.insert(obstacles.begin(), obstacles.end());
	}

	scalar operator()(cell_type const &cell) const
	{
		if (cell.x < 0) return -1;
		if (cell.y < 0) return -1;
		if (cell.x >= int(hf_->nrows())) return -1;
		if (cell.y >= int(hf_->ncols())) return -1;

		typename obstacles_type::const_iterator it = obstacles_.find(cell);
		if (it != obstacles_.end()) return -1;

		if (hf_->max_y_in_cell(cell.x, cell.y) < min_value_)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

private:
	struct cell_hash {
		std::size_t operator ()(cell_type const &cell) const {
			return (cell.x * 1011) ^ cell.y;
		}
	};

	struct cell_eq {
		std::size_t operator ()(cell_type const &lhs, cell_type const &rhs) const {
			return lhs == rhs;
		}
	};

	using obstacles_type = std::unordered_set<cell_type, cell_hash, cell_eq>;

	heightfield<T> const *hf_;
	scalar min_value_;
	obstacles_type obstacles_;
};
}

template<class T> std::vector<typename heightfield<T>::cell_type>
heightfield<T>::build_path(vec<3> const &from, vec<3> const &to, value_type min_value,
	std::vector<cell_type> const &obstacles) const
{
	cell_type start = world_position_to_cell(from);
	cell_type goal = world_position_to_cell(to);

	scorer_closure<T> scorer(this, min_value, obstacles);
	a_star<scorer_closure<T>> pathfinder(start, goal, scorer);

	pathfinder.calculate_path();

	return pathfinder.build_path();
}

template<class T>
void heightfield<T>::post_load()
{
	local_aabb_.null();

	for (typename std::vector<value_type>::const_iterator it = heights_.begin();
		 it != heights_.end(); ++it)
	{
		if (local_aabb_.lo.y > *it) local_aabb_.lo.y = *it;
		if (local_aabb_.hi.y < *it) local_aabb_.hi.y = *it;
	}

	local_aabb_.lo.y -= 1;
	local_aabb_.hi.y += 1;

	local_aabb_.lo.x = 0;
	local_aabb_.lo.z = 0;

	local_aabb_.hi.x = scalar(ncols_ - 1);
	local_aabb_.hi.z = scalar(nrows_ - 1);
}

template class heightfield<std::uint8_t>;
template class heightfield<std::uint16_t>;
template class heightfield<scalar>;

}
