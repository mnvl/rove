// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_octree.cc
 * @brief Python bindings for octree<triangle<3>, int>
 */

#include "python_bindings.h"
#include "bind_rays.h"
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/pair.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include <rove/octree.h>
#include <rove/triangle.h>
#include <rove/aabb.h>
#include <rove/segment.h>
#include <rove/ray.h>
#include <rove/sphere.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_octree_triangle3(nb::module_ &m, char const *name)
{
	using Octree = rove::octree<rove::triangle<3, T>, int>;
	using Triangle = rove::triangle<3, T>;
	using AABB = rove::aabb<3, T>;
	using Sphere = rove::sphere<3, T>;
	using Line = rove::segment<3, T>;
	using Vec3 = rove::vec<3, T>;
	using value_type = typename Octree::value_type;

	nb::class_<Octree>(m, name)
		.def(nb::init<AABB const &, std::size_t, T>(),
			"bounds"_a, "leaf_items_threshold"_a = 16,
			"min_subcluster_diagonal_sq"_a = T(1e-5))
		.def("insert", &Octree::insert, "triangle"_a, "payload"_a)
		.def("build", &Octree::build, nb::call_guard<nb::gil_scoped_release>(),
			"Builds the tree, letting other Python threads run meanwhile (none may use this tree until it returns)")
		.def_prop_ro("size", &Octree::size)
		.def_prop_ro("bounds", &Octree::bounds)
		// Pickled as octree::save() bytes: the built tree, nodes and all, so
		// a tree built in one process is loaded in another without build().
		.def("__getstate__",
			[](Octree const &self) {
				std::string const bytes = self.save();
				return nb::bytes(bytes.data(), bytes.size());
			})
		.def("__setstate__",
			[](Octree &self, nb::bytes state) {
				new (&self) Octree(Octree::load(std::string_view(state.c_str(), state.size())));
			}, "state"_a)
		.def("query_aabb",
			[](Octree const &self, AABB const &query) {
				return self.template query_intersection<AABB>(query);
			}, "bounds"_a)
		.def("query_sphere",
			[](Octree const &self, Sphere const &query) {
				return self.template query_intersection<Sphere>(query);
			}, "sphere"_a)
		.def("query_aabb_items",
			[](Octree const &self, AABB const &query) {
				auto items = self.template query_intersection_items<AABB>(query);
				std::vector<std::pair<Triangle, int>> result;
				result.reserve(items.size());
				for (auto const &item : items) {
					result.emplace_back(item.primitive, item.payload);
				}
				return result;
			}, "bounds"_a)
		.def("query_sphere_items",
			[](Octree const &self, Sphere const &query) {
				auto items = self.template query_intersection_items<Sphere>(query);
				std::vector<std::pair<Triangle, int>> result;
				result.reserve(items.size());
				for (auto const &item : items) {
					result.emplace_back(item.primitive, item.payload);
				}
				return result;
			}, "sphere"_a)
		.def("query_line_items",
			[](Octree const &self, Line const &query) {
				auto items = self.template query_intersection_items<Line>(query);
				std::vector<std::pair<Triangle, int>> result;
				result.reserve(items.size());
				for (auto const &item : items) {
					result.emplace_back(item.primitive, item.payload);
				}
				return result;
			}, "line"_a,
			"Triangles the segment `line` (A to B) actually hits, exact "
			"(not an AABB approximation), tree-pruned.")
		.def("raycast_batch",
			[](Octree const &self, Vec3 const &origin,
			   nb::ndarray<T, nb::ndim<2>, nb::c_contig, nb::device::cpu> directions,
			   T max_distance) {
				if (directions.shape(1) != 3) {
					throw std::invalid_argument("directions must have shape (N, 3)");
				}
				size_t const n = directions.shape(0);
				T const *in = directions.data();
				T *out = new T[n];
				{
					// the rays traced in C++ alone: other Python threads run meanwhile
					nb::gil_scoped_release release;
					for (size_t i = 0; i < n; ++i) {
						Vec3 const dir(in[3 * i], in[3 * i + 1], in[3 * i + 2]);
						rove::ray<3, T> const r(origin, dir * max_distance);
						auto const hit = self.trace(r, T(0), T(1));
						// time is a share of r.a: the distance is that share of its length
						out[i] = hit.happened ? hit.time * r.a.length() : std::numeric_limits<T>::infinity();
					}
				}
				size_t shape[1] = {n};
				nb::capsule deleter(out, [](void *p) noexcept { delete[] static_cast<T*>(p); });
				return nb::ndarray<nb::numpy, T, nb::ndim<1>>(out, 1, shape, deleter);
			}, "origin"_a, "directions"_a, "max_distance"_a,
			"Nearest-hit distance for every ray in `directions` (shape (N,3), "
			"shared `origin`, expected-unit direction vectors scaled by "
			"`max_distance` internally). One octree traversal per ray, all "
			"looped in C++ -- a single Python call per octree per frame. "
			"Returns an (N,) float array; inf where a ray hits nothing within "
			"max_distance. Other Python threads run while it traces (none may "
			"change the directions or the tree meanwhile).")
		.def("raycast_batch_hits",
			[](Octree const &self, bind_rays::rows3<T> origins, bind_rays::rows3<T> directions, T max_distance) {
				if (!self.is_built()) {
					throw std::logic_error("OctreeTriangle3.raycast_batch_hits: build() the tree first");
				}
				size_t const n = origins.shape(0);
				std::unique_ptr<T[]> distance(new T[n]), normal(new T[3 * n]);
				std::unique_ptr<std::int64_t[]> item(new std::int64_t[n]);
				std::unique_ptr<std::int32_t[]> payload(new std::int32_t[n]);
				bind_rays::each_ray<T>("raycast_batch_hits", origins, directions, max_distance,
					[&](size_t i, rove::ray<3, T> const &r) {
						auto const hit = self.trace(r, T(0), max_distance);
						if (!hit.happened) {
							distance[i] = std::numeric_limits<T>::infinity();
							item[i] = payload[i] = -1;
							normal[3 * i] = normal[3 * i + 1] = normal[3 * i + 2] = std::numeric_limits<T>::quiet_NaN();
							return;
						}
						distance[i] = hit.time;          // r is unit: its time is the distance
						item[i] = std::int64_t(*hit.index);
						payload[i] = self[*hit.index].payload;
						for (int k = 0; k < 3; ++k) {
							normal[3 * i + k] = hit.normal.i[k];
						}
					});
				return nb::make_tuple(bind_rays::owned(std::move(distance), {n}), bind_rays::owned(std::move(item), {n}),
					bind_rays::owned(std::move(payload), {n}), bind_rays::owned(std::move(normal), {n, 3}));
			}, "origins"_a.noconvert(), "directions"_a.noconvert(), "max_distance"_a,
			"The nearest hit of every ray, origins[i] towards directions[i] (both (N, 3) "
			"float32 in C order; a direction of any length but none), within "
			"max_distance (may be inf), traced in C++ with other Python threads "
			"running. Returns (distance (N,) float32, item (N,) int64: the "
			"triangle's index in the tree, payload (N,) int32, normal (N, 3) "
			"float32: unit, facing against the ray). A miss is distance inf, item "
			"and payload -1 and a NaN normal. ValueError naming a ray that is no "
			"ray (NaN, infinite, no length), for a negative max_distance or arrays "
			"of different lengths; TypeError for arrays of another type, shape or "
			"order. raycast_batch, one origin and distances only, is unchanged.");
}

template void bind_octree_triangle3<float>(nb::module_ &m, char const *name);
