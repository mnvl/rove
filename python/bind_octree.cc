// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_octree.cc
 * @brief Python bindings for octree<triangle<3>, int>
 */

#include "python_bindings.h"
#include "bind_rays.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/pair.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
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

namespace
{

/// The lock of a tree: writers (insert, build) one at a time and alone, readers (queries) together.  A tree's users here block on it, so
/// another thread's use of a tree waits while it builds rather than reading what is being rebuilt; and it is held without the GIL where
/// the work is done without it, so a thread that holds the GIL and waits here never holds up the one that holds the lock.  (By the
/// tree's address, in a fixed table: a bound class of its own would stop being the C++ octree that other bindings can share.)
std::shared_mutex &lock_of(void const *tree)
{
	static std::shared_mutex locks[64];
	return locks[(reinterpret_cast<std::uintptr_t>(tree) >> 6) % 64];
}

}

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
		.def("insert", [](Octree &self, Triangle const &triangle, int payload) {
				nb::gil_scoped_release release;
				std::unique_lock lock(lock_of(&self));
				self.insert(triangle, payload);
			}, "triangle"_a, "payload"_a, "Adds an item; other threads' use of the tree waits for it")
		.def("build", [](Octree &self) {
				nb::gil_scoped_release release;
				std::unique_lock lock(lock_of(&self));
				self.build();
			},
			"Builds the tree, letting other Python threads run meanwhile: one that uses this tree waits until it returns.  ValueError for "
			"items in a box that is empty or not finite")
		.def_prop_ro("size", [](Octree const &self) {
				nb::gil_scoped_release release;
				std::shared_lock lock(lock_of(&self));
				return self.size();
			})
		.def("__repr__", [name](Octree const &self) {
				AABB bounds;
				std::size_t size;
				bool built;
				{
					nb::gil_scoped_release release;
					std::shared_lock lock(lock_of(&self));
					bounds = self.bounds();
					size = self.size();
					built = self.is_built();
				}
				bind_text::repr_stream ss;
				ss << name << "(lo=" << bounds.lo << ", hi=" << bounds.hi << ", size=" << size << ", " << (built ? "built" : "not built") << ")";
				return ss.str();
			})
		.def_prop_ro("bounds", &Octree::bounds)
		// Pickled as octree::save() bytes: the built tree, nodes and all, so
		// a tree built in one process is loaded in another without build().
		// One not built yet is built in a copy for it: it comes back built.
		.def("__getstate__",
			[](Octree const &self) {
				std::string bytes;
				{
					nb::gil_scoped_release release;
					std::shared_lock lock(lock_of(&self));
					if (self.is_built()) {
						bytes = self.save();
					} else {
						Octree built = self.copy_unbuilt();
						built.build();
						bytes = built.save();
					}
				}
				return nb::bytes(bytes.data(), bytes.size());
			})
		.def("__reduce_ex__", [](nb::handle self, int) {
				return nb::make_tuple(nb::module_::import_("copyreg").attr("__newobj__"), nb::make_tuple(self.type()),
					self.attr("__getstate__")());
			}, "protocol"_a)
		.def("__setstate__",
			[](Octree &self, nb::bytes state) {
				new (&self) Octree(Octree::load(std::string_view(state.c_str(), state.size())));
			}, "state"_a)
		.def("query_aabb",
			[](Octree const &self, AABB const &query) {
				nb::gil_scoped_release release;
				std::shared_lock lock(lock_of(&self));
				return self.template query_intersection<AABB>(query);
			}, "bounds"_a)
		.def("query_sphere",
			[](Octree const &self, Sphere const &query) {
				nb::gil_scoped_release release;
				std::shared_lock lock(lock_of(&self));
				return self.template query_intersection<Sphere>(query);
			}, "sphere"_a)
		.def("query_aabb_items",
			[](Octree const &self, AABB const &query) {
				nb::gil_scoped_release release;
				std::shared_lock lock(lock_of(&self));
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
				nb::gil_scoped_release release;
				std::shared_lock lock(lock_of(&self));
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
				nb::gil_scoped_release release;
				std::shared_lock lock(lock_of(&self));
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
			   nb::ndarray<T const, nb::ndim<2>, nb::c_contig, nb::device::cpu> directions,
			   T max_distance) {
				if (directions.shape(1) != 3) {
					throw std::invalid_argument("directions must have shape (N, 3)");
				}
				if (!(max_distance >= 0)) {
					throw std::invalid_argument("OctreeTriangle3.raycast_batch: a max_distance of " + std::to_string(max_distance));
				}
				size_t const n = directions.shape(0);
				T const *in = directions.data();
				std::unique_ptr<T[]> out(new T[n]);
				{
					// the rays traced in C++ alone: other Python threads run meanwhile
					nb::gil_scoped_release release;
					std::shared_lock lock(lock_of(&self));
					for (size_t i = 0; i < n; ++i) {
						Vec3 const dir(in[3 * i], in[3 * i + 1], in[3 * i + 2]);
						// out to max_distance as a distance: scaling the direction by it would overflow for a large one
						T const speed = dir.length();
						if (!(speed > T(0))) {
							out[i] = std::numeric_limits<T>::infinity();
							continue;
						}
						auto const hit = self.trace(rove::ray<3, T>(origin, dir), T(0), max_distance / speed);
						out[i] = hit.happened ? hit.time * speed : std::numeric_limits<T>::infinity();
					}
				}
				return bind_rays::owned(std::move(out), {n});
			}, "origin"_a, "directions"_a, "max_distance"_a,
			"Nearest-hit distance for every ray in `directions` (shape (N,3), of any length, read-only or not; "
			"numbers of another type are rounded to float32), from a shared `origin`, within max_distance (may be inf; "
			"ValueError if negative). One octree traversal per ray, all "
			"looped in C++ -- a single Python call per octree per frame. "
			"Returns an (N,) float array; inf where a ray hits nothing within "
			"max_distance or has no direction. Other Python threads run while it traces (none may "
			"change the directions meanwhile; one that changes the tree waits).")
		.def("raycast_batch_hits",
			[](Octree const &self, bind_rays::rows3<T> origins, bind_rays::rows3<T> directions, T max_distance) {
				{
					nb::gil_scoped_release release;
					std::shared_lock lock(lock_of(&self));
					if (!self.is_built()) {
						throw std::logic_error("OctreeTriangle3.raycast_batch_hits: build() the tree first");
					}
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
					}, &lock_of(&self));
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
