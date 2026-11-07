// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_capsule.cc
 * @brief Python bindings for capsule classes
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/capsule.h>
#include <rove/segment.h>
#include <rove/triangle.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_capsule2(nb::module_ &m, char const *name) {
	using Capsule = rove::capsule<2, T>;
	using Vec2 = rove::vec<2, T>;
	using Line2 = rove::segment<2, T>;

	nb::class_<Capsule>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec2 const &, Vec2 const &, T>(),
			 nb::arg("a"), nb::arg("b"), nb::arg("radius"),
			 "Construct capsule from two endpoints and radius")
		.def_rw("axe", &Capsule::axe, "Central axis line segment of the capsule")
		.def_rw("radius", &Capsule::radius, "Radius of the capsule")
		.def("contains", &Capsule::contains,
			 nb::arg("point"),
			 "Test if point is inside the capsule")
		.def("distance", &Capsule::distance,
			 nb::arg("point"),
			 "Get distance from capsule surface to point (0 if inside)")
		.def("__repr__", [name](Capsule const &c) {
			bind_text::repr_stream ss;
			ss << name << "(A=" << c.axe.A << ", B=" << c.axe.B << ", radius=" << c.radius << ")";
			return ss.str();
		})
		.def("__getstate__", [](Capsule const &c) {
			return nb::make_tuple(c.axe.A, c.axe.B, c.radius);
		})
		.def("__setstate__", [](Capsule &c, nb::tuple t) {
			check_state(t, 3, "Capsule.__setstate__");
			c.axe.A = nb::cast<Vec2>(t[0]);
			c.axe.B = nb::cast<Vec2>(t[1]);
			c.radius = nb::cast<T>(t[2]);
		});
}

template<class T>
void bind_capsule3(nb::module_ &m, char const *name) {
	using Capsule = rove::capsule<3, T>;
	using Vec3 = rove::vec<3, T>;
	using Line3 = rove::segment<3, T>;
	using Triangle3 = rove::triangle<3, T>;

	nb::class_<Capsule>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec3 const &, Vec3 const &, T>(),
			 nb::arg("a"), nb::arg("b"), nb::arg("radius"),
			 "Construct capsule from two endpoints and radius")
		.def_rw("axe", &Capsule::axe, "Central axis line segment of the capsule")
		.def_rw("radius", &Capsule::radius, "Radius of the capsule")
		.def("contains", &Capsule::contains,
			 nb::arg("point"),
			 "Test if point is inside the capsule")
		.def("distance", &Capsule::distance,
			 nb::arg("point"),
			 "Get distance from capsule surface to point (0 if inside)")
		.def("test_intersection",
			 static_cast<bool (Capsule::*)(Triangle3 const &) const>(&Capsule::test_intersection),
			 nb::arg("triangle"),
			 "Test if capsule intersects a triangle")
		.def("test_intersection",
			 static_cast<bool (Capsule::*)(Capsule const &) const>(&Capsule::test_intersection),
			 nb::arg("other"),
			 "Test if capsule intersects another capsule")
		.def("__repr__", [name](Capsule const &c) {
			bind_text::repr_stream ss;
			ss << name << "(A=" << c.axe.A << ", B=" << c.axe.B << ", radius=" << c.radius << ")";
			return ss.str();
		})
		.def("__getstate__", [](Capsule const &c) {
			return nb::make_tuple(c.axe.A, c.axe.B, c.radius);
		})
		.def("__setstate__", [](Capsule &c, nb::tuple t) {
			check_state(t, 3, "Capsule.__setstate__");
			c.axe.A = nb::cast<Vec3>(t[0]);
			c.axe.B = nb::cast<Vec3>(t[1]);
			c.radius = nb::cast<T>(t[2]);
		});
}

// Explicit template instantiations
template void bind_capsule2<float>(nb::module_ &m, char const *name);
template void bind_capsule2<double>(nb::module_ &m, char const *name);
template void bind_capsule3<float>(nb::module_ &m, char const *name);
template void bind_capsule3<double>(nb::module_ &m, char const *name);
