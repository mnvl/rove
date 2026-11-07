// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_line.cc
 * @brief Python bindings for line classes
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/segment.h>
#include <rove/ray.h>
#include <rove/aabb.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_line2(nb::module_ &m, char const *name) {
	using Line = rove::segment<2, T>;
	using Vec2 = rove::vec<2, T>;
	using Ray2 = rove::ray<2, T>;
	using AABB2 = rove::aabb<2, T>;

	nb::class_<Line>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec2 const &, Vec2 const &>(),
			 nb::arg("a"), nb::arg("b"),
			 "Construct line segment from two endpoints")
		.def_rw("A", &Line::A, "First endpoint of line segment")
		.def_rw("B", &Line::B, "Second endpoint of line segment")
		.def("construct", &Line::construct,
			 nb::arg("a"), nb::arg("b"),
			 "Set line segment endpoints")
		.def("direction", &Line::direction,
			 "Get direction vector (B - A)")
		.def("to_ray", &Line::to_ray,
			 "Get ray representation of this line segment")
		.def("bounds", &Line::bounds,
			 "Get axis-aligned bounding box containing this line segment")
		.def("length_sq", &Line::length_sq,
			 "Get squared length of line segment")
		.def("contains", &Line::contains,
			 nb::arg("point"),
			 "Test if point lies on the line segment")
		.def("distance_sq", &Line::distance_sq,
			 nb::arg("point"),
			 "Get squared distance from line segment to point")
		.def("distance", &Line::distance,
			 nb::arg("point"),
			 "Get distance from line segment to point")
		.def("point", &Line::point,
			 nb::arg("t"),
			 "Get the point A + (B - A) t; exactly A at t = 0, exactly B at t = 1")
		.def("parameter", &Line::parameter,
			 nb::arg("point"),
			 "Get t in [0, 1] of the nearest point on the segment; 0 when A == B")
		.def("closest_point", &Line::closest_point,
			 nb::arg("point"),
			 "Get the point on the segment nearest to point")
		.def("__repr__", [name](Line const &l) {
			bind_text::repr_stream ss;
			ss << name << "(A=" << l.A << ", B=" << l.B << ")";
			return ss.str();
		})
		.def("__getstate__", [](Line const &l) {
			return nb::make_tuple(l.A, l.B);
		})
		.def("__setstate__", [](Line &l, nb::tuple t) {
			check_state(t, 2, "Line.__setstate__");
			l.A = nb::cast<Vec2>(t[0]);
			l.B = nb::cast<Vec2>(t[1]);
		});
}

template<class T>
void bind_line3(nb::module_ &m, char const *name) {
	using Line = rove::segment<3, T>;
	using Vec3 = rove::vec<3, T>;
	using Ray3 = rove::ray<3, T>;
	using AABB3 = rove::aabb<3, T>;

	nb::class_<Line>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec3 const &, Vec3 const &>(),
			 nb::arg("a"), nb::arg("b"),
			 "Construct line segment from two endpoints")
		.def_rw("A", &Line::A, "First endpoint of line segment")
		.def_rw("B", &Line::B, "Second endpoint of line segment")
		.def("construct", &Line::construct,
			 nb::arg("a"), nb::arg("b"),
			 "Set line segment endpoints")
		.def("direction", &Line::direction,
			 "Get direction vector (B - A)")
		.def("to_ray", &Line::to_ray,
			 "Get ray representation of this line segment")
		.def("bounds", &Line::bounds,
			 "Get axis-aligned bounding box containing this line segment")
		.def("length_sq", &Line::length_sq,
			 "Get squared length of line segment")
		.def("contains", &Line::contains,
			 nb::arg("point"),
			 "Test if point lies on the line segment")
		.def("distance_sq", &Line::distance_sq,
			 nb::arg("point"),
			 "Get squared distance from line segment to point")
		.def("distance", &Line::distance,
			 nb::arg("point"),
			 "Get distance from line segment to point")
		.def("point", &Line::point,
			 nb::arg("t"),
			 "Get the point A + (B - A) t; exactly A at t = 0, exactly B at t = 1")
		.def("parameter", &Line::parameter,
			 nb::arg("point"),
			 "Get t in [0, 1] of the nearest point on the segment; 0 when A == B")
		.def("closest_point", &Line::closest_point,
			 nb::arg("point"),
			 "Get the point on the segment nearest to point")
		.def("__repr__", [name](Line const &l) {
			bind_text::repr_stream ss;
			ss << name << "(A=" << l.A << ", B=" << l.B << ")";
			return ss.str();
		})
		.def("__getstate__", [](Line const &l) {
			return nb::make_tuple(l.A, l.B);
		})
		.def("__setstate__", [](Line &l, nb::tuple t) {
			check_state(t, 2, "Line.__setstate__");
			l.A = nb::cast<Vec3>(t[0]);
			l.B = nb::cast<Vec3>(t[1]);
		});
}

// Explicit template instantiations
template void bind_line2<float>(nb::module_ &m, char const *name);
template void bind_line2<double>(nb::module_ &m, char const *name);
template void bind_line3<float>(nb::module_ &m, char const *name);
template void bind_line3<double>(nb::module_ &m, char const *name);
