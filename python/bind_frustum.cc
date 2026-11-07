// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_frustum.cc
 * @brief Python bindings for frustum class
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <rove/frustum.h>
#include <rove/aabb.h>
#include <rove/obb.h>
#include <rove/sphere.h>
#include <rove/matrix.h>
#include <rove/vec.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_frustum(nb::module_ &m, char const *name) {
	using Frustum = rove::frustum<T>;
	using AABB3 = rove::aabb<3, T>;
	using OBB3 = rove::obb<3, T>;
	using Plane = rove::plane<T>;

	using Mat4 = rove::matrix<4, 4, T>;
	using Vec3 = rove::vec<3, T>;

	nb::class_<Frustum>(m, name)
		.def(nb::init<>(), "Default constructor")
		.def(nb::init<Mat4 const &>(),
			 nb::arg("matrix"),
			 "Construct frustum from view-projection matrix")
		.def("load", &Frustum::load,
			 nb::arg("matrix"),
			 "Load frustum planes from view-projection matrix")
		.def("contains", &Frustum::contains,
			 nb::arg("point"),
			 "Test if point is inside the frustum")
		.def("test_intersection",
			 nb::overload_cast<rove::sphere<3, T> const &>(&Frustum::test_intersection, nb::const_),
			 nb::arg("sphere"),
			 "Test whether the sphere is in view: False only where it lies wholly outside a plane")
		.def("test_intersection",
			 nb::overload_cast<AABB3 const &>(&Frustum::test_intersection, nb::const_),
			 nb::arg("aabb"),
			 "Test intersection with axis-aligned bounding box")
		.def("test_intersection",
			 nb::overload_cast<OBB3 const &>(&Frustum::test_intersection, nb::const_),
			 nb::arg("obb"),
			 "Test intersection with oriented bounding box")
		.def("plane", [](Frustum const &f, int index) -> Plane const & {
			if (index < 0 || index >= int(Frustum::planes_count)) {
				throw std::out_of_range("a frustum's plane index is 0 to 5");
			}
			return f.planes[index];
		}, nb::arg("index"), nb::rv_policy::reference_internal,
		   "The plane of that index: frustum.left_plane, right_plane, top_plane, bottom_plane, near_plane, far_plane (0 to 5)")
		.def_prop_rw("planes",
			// Getter: return list of 6 planes
			[](Frustum const &f) {
				std::vector<Plane> result;
				result.reserve(6);
				for (int i = 0; i < 6; i++) {
					result.push_back(f.planes[i]);
				}
				return result;
			},
			// Setter: accept list of 6 planes
			[](Frustum &f, std::vector<Plane> const &planes) {
				if (planes.size() != 6) {
					throw std::invalid_argument("planes must have exactly 6 elements");
				}
				for (int i = 0; i < 6; i++) {
					f.planes[i] = planes[i];
				}
			},
			"The 6 planes, in the order of the indices left_plane, right_plane, top_plane, bottom_plane, near_plane, far_plane")
		.def_prop_ro_static("planes_count", [](nb::handle) { return int(Frustum::planes_count); }, "6")
		.def_prop_ro_static("left_plane", [](nb::handle) { return int(Frustum::left_plane); }, "The left plane's index")
		.def_prop_ro_static("right_plane", [](nb::handle) { return int(Frustum::right_plane); }, "The right plane's index")
		.def_prop_ro_static("top_plane", [](nb::handle) { return int(Frustum::top_plane); }, "The top plane's index")
		.def_prop_ro_static("bottom_plane", [](nb::handle) { return int(Frustum::bottom_plane); }, "The bottom plane's index")
		.def_prop_ro_static("near_plane", [](nb::handle) { return int(Frustum::near_plane); }, "The near plane's index")
		.def_prop_ro_static("far_plane", [](nb::handle) { return int(Frustum::far_plane); }, "The far plane's index")
		.def("__repr__", [name](Frustum const &f) {
			bind_text::repr_stream ss;
			ss << name << "(planes=[";
			for (int i = 0; i < 6; i++) {
				ss << name << "(A=" << f.planes[i].A
				   << ", B=" << f.planes[i].B
				   << ", C=" << f.planes[i].C
				   << ", D=" << f.planes[i].D << ")";
				if (i < 5) ss << ", ";
			}
			ss << "])";
			return ss.str();
		})
		.def("__getstate__", [](Frustum const &f) {
			return nb::make_tuple(
				f.planes[0], f.planes[1], f.planes[2],
				f.planes[3], f.planes[4], f.planes[5]);
		})
		.def("__setstate__", [](Frustum &f, nb::tuple t) {
			check_state(t, 6, "frustum.__setstate__");
			for (int i = 0; i < 6; i++) {
				f.planes[i] = nb::cast<Plane>(t[i]);
			}
		});
}

// Explicit template instantiations
template void bind_frustum<float>(nb::module_ &m, char const *name);
template void bind_frustum<double>(nb::module_ &m, char const *name);
