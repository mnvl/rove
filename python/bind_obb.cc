// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_obb.cc
 * @brief Python bindings for OBB classes
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/operators.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/aabb.h>
#include <rove/capsule.h>
#include <rove/obb.h>
#include <rove/ray.h>
#include <rove/triangle.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_obb2(nb::module_ &m, char const *name) {
	using OBB = rove::obb<2, T>;
	using Vec2 = rove::vec<2, T>;
	using AABB2 = rove::aabb<2, T>;

	nb::class_<OBB>(m, name)
		.def(nb::init<>(),
			 "Default constructor (uninitialized)")
		.def(nb::init<AABB2 const &>(),
			 nb::arg("aabb"),
			 "Construct OBB from axis-aligned bounding box")
		.def(nb::init<OBB const &>(),
			 nb::arg("other"),
			 "Copy constructor")
		.def_rw("origin", &OBB::origin, "Origin corner of the OBB")
		.def_rw("tangent", &OBB::tangent, "First edge vector (width direction)")
		.def_rw("normal", &OBB::normal, "Second edge vector (height direction)")
		.def("world_to_local_point",
			 static_cast<Vec2 (OBB::*)(Vec2 const &) const>(&OBB::world_to_local_point),
			 nb::arg("point"),
			 "Transform point from world to local OBB coordinates")
		.def("world_to_local_vector",
			 static_cast<Vec2 (OBB::*)(Vec2 const &) const>(&OBB::world_to_local_vector),
			 nb::arg("vector"),
			 "Transform vector from world to local OBB coordinates")
		.def("contains", &OBB::contains,
			 nb::arg("point"),
			 "Test if point is inside OBB")
		.def("vertex", &OBB::vertex,
			 nb::arg("vertex_number"),
			 "Get vertex position by index (0-3)")
		.def("side_normal", &OBB::side_normal,
			 nb::arg("side_number"),
			 "Get outward normal for a side (0-3)")
		.def("bounds", [](OBB const &obb) {
			AABB2 result;
			result = obb.bounds();
			return result;
		}, "Get axis-aligned bounding box containing this OBB")
		.def("__repr__", [name](OBB const &o) {
			std::ostringstream ss;
			ss << name << "(origin=" << o.origin
			   << ", tangent=" << o.tangent
			   << ", normal=" << o.normal << ")";
			return ss.str();
		})
		.def("__getstate__", [](OBB const &o) {
			return nb::make_tuple(o.origin, o.tangent, o.normal);
		})
		.def("__setstate__", [](OBB &o, nb::tuple t) {
			o.origin  = nb::cast<Vec2>(t[0]);
			o.tangent = nb::cast<Vec2>(t[1]);
			o.normal  = nb::cast<Vec2>(t[2]);
		});
}

template<class T>
void bind_obb3(nb::module_ &m, char const *name) {
	using OBB = rove::obb<3, T>;
	using Vec3 = rove::vec<3, T>;
	using AABB3 = rove::aabb<3, T>;
	using Ray3 = rove::ray<3, T>;

	nb::class_<OBB>(m, name)
		.def(nb::init<>(),
			 "Default constructor (uninitialized)")
		.def(nb::init<AABB3 const &>(),
			 nb::arg("aabb"),
			 "Construct OBB from axis-aligned bounding box")
		.def(nb::init<Vec3 const &, Vec3 const &, Vec3 const &, Vec3 const &>(),
			 nb::arg("origin"), nb::arg("tangent"), nb::arg("normal"), nb::arg("binormal"),
			 "Construct OBB from origin and three edge vectors")
		.def(nb::init<OBB const &>(),
			 nb::arg("other"),
			 "Copy constructor")
		.def_rw("origin", &OBB::origin, "Origin corner of the OBB")
		.def_rw("tangent", &OBB::tangent, "First edge vector (X-like direction)")
		.def_rw("normal", &OBB::normal, "Second edge vector (Y-like direction)")
		.def_rw("binormal", &OBB::binormal, "Third edge vector (Z-like direction)")
		.def("transform", &OBB::transform,
			 nb::arg("matrix"),
			 "Apply a 4x4 transformation matrix to the OBB")
		.def("centre", &OBB::centre,
			 "Get center point of the OBB")
		.def("volume", &OBB::volume,
			 "Get volume of the OBB")
		.def("bounds", [](OBB const &obb) {
			AABB3 result;
			result = obb.bounds();
			return result;
		}, "Get axis-aligned bounding box containing this OBB")
		.def("vertex", [](OBB const &obb, size_t vertex_number) {
			Vec3 position;
			position = obb.vertex(vertex_number);
			return position;
		}, nb::arg("vertex_number"),
		   "Get vertex position by index (0-7)")
		.def("contains", nb::overload_cast<Vec3 const &>(&OBB::contains, nb::const_),
			 nb::arg("point"),
			 "Test if point is inside OBB")
		.def("test_intersection", nb::overload_cast<OBB const &>(&OBB::test_intersection, nb::const_),
			 nb::arg("other"),
			 "Test intersection with another OBB")
		.def("test_intersection", nb::overload_cast<rove::triangle<3, T> const &>(&OBB::test_intersection, nb::const_),
			 nb::arg("tri"),
			 "Test intersection with a triangle (SAT)")
		.def("test_intersection", nb::overload_cast<rove::capsule<3, T> const &>(&OBB::test_intersection, nb::const_),
			 nb::arg("cap"),
			 "Test intersection with a capsule (exact)")
		.def("trace", [](OBB const &b, Ray3 const &r, T t_min, T t_max) -> nb::object {
			T t0, t1;
			if (!b.trace(r, &t0, &t1, t_min, t_max)) return nb::none();
			return nb::make_tuple(t0, t1);
		}, nb::arg("ray"), nb::arg("t_min") = T(0), nb::arg("t_max") = T(1),
		   "Ray-OBB intersection. Returns (t_enter, t_exit) tuple of "
		   "ray-parameter times, or None if the ray misses the OBB in "
		   "[t_min, t_max].  When the ray origin is inside the OBB, "
		   "t_enter == t_min.")
		.def("is_degenerate", &OBB::is_degenerate,
			 nb::arg("tolerance") = T(1e-2),
			 "Test whether this OBB has near-zero volume (parallel edges)")
		.def("is_basis_linearly_independent", &OBB::is_basis_linearly_independent,
			 nb::arg("tolerance") = T(1e-2),
			 "Test whether edge vectors are linearly independent")
		.def_static("make_sweep_volume", [](OBB const &box, Vec3 const &displacement) {
			OBB out[6];
			OBB::make_sweep_volume(box, displacement, out);
			return std::vector<OBB>(out, out + 6);
		}, nb::arg("box"), nb::arg("displacement"),
		   "Decompose swept volume into 6 face-sweep OBBs for tunneling prevention")
		.def("__repr__", [name](OBB const &o) {
			std::ostringstream ss;
			ss << name << "(origin=" << o.origin
			   << ", tangent=" << o.tangent
			   << ", normal=" << o.normal
			   << ", binormal=" << o.binormal << ")";
			return ss.str();
		})
		.def("__getstate__", [](OBB const &o) {
			return nb::make_tuple(o.origin, o.tangent, o.normal, o.binormal);
		})
		.def("__setstate__", [](OBB &o, nb::tuple t) {
			o.origin   = nb::cast<Vec3>(t[0]);
			o.tangent  = nb::cast<Vec3>(t[1]);
			o.normal   = nb::cast<Vec3>(t[2]);
			o.binormal = nb::cast<Vec3>(t[3]);
		});
}

// Explicit template instantiations
template void bind_obb2<float>(nb::module_ &m, char const *name);
template void bind_obb2<double>(nb::module_ &m, char const *name);
template void bind_obb3<float>(nb::module_ &m, char const *name);
template void bind_obb3<double>(nb::module_ &m, char const *name);
