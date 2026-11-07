// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_aabb.cc
 * @brief Python bindings for AABB classes
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/operators.h>
#include <sstream>
#include <vector>

#include <rove/vec.h>
#include <rove/aabb.h>
#include <rove/ray.h>
#include <rove/matrix.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_aabb2(nb::module_ &m, char const *name) {
	using AABB = rove::aabb<2, T>;
	using Vec2 = rove::vec<2, T>;

	nb::class_<AABB>(m, name)
		.def(nb::init<>(),
			 "An empty box: it contains nothing, and the first extend() makes it that point")
		.def("is_empty", &AABB::is_empty,
			 "Whether the box holds no point: lo past hi on some axis (a box of a single point is not empty)")
		.def("grown", &AABB::grown, nb::arg("margin"),
			 "The box with every side moved out by margin (in by a negative one); an empty box stays empty, and one shrunk "
			 "past nothing is empty")
		.def(nb::init<Vec2 const &>(),
			 nb::arg("centre"),
			 "Construct degenerate AABB from single point")
		.def(nb::init<Vec2 const &, Vec2 const &>(),
			 nb::arg("lo"), nb::arg("hi"),
			 "Construct AABB from two corner points")
		.def(nb::init<Vec2 const &, T>(),
			 nb::arg("centre"), nb::arg("side_length"),
			 "Construct a cube AABB from its centre and full side length "
			 "(centre +- side_length / 2 on every axis)")
		.def_rw("lo", &AABB::lo, "Minimum corner (lower bounds)")
		.def_rw("hi", &AABB::hi, "Maximum corner (upper bounds)")
		.def("null", &AABB::null,
			 "Reset AABB to null state for accumulation")
		.def("normalize", &AABB::normalize,
			 "Ensure lo <= hi in all dimensions")
		.def("diagonal", &AABB::diagonal,
			 "Get AABB diagonal vector (hi - lo)")
		.def("centre", &AABB::centre,
			 "Get AABB center point")
		.def("extend", nb::overload_cast<Vec2 const &>(&AABB::extend),
			 nb::arg("point"),
			 "Extend AABB to include a point")
		.def("extend", nb::overload_cast<AABB const &>(&AABB::extend),
			 nb::arg("other"),
			 "Extend AABB to include another AABB")
		.def("contains", nb::overload_cast<Vec2 const &>(&AABB::contains, nb::const_),
			 nb::arg("point"),
			 "Test if point is inside AABB")
		.def("contains", nb::overload_cast<AABB const &>(&AABB::contains, nb::const_),
			 nb::arg("other"),
			 "Test if another AABB is fully contained")
		.def("test_intersection", &AABB::test_intersection,
			 nb::arg("other"),
			 "Test intersection with another AABB")
		.def("__repr__", [name](AABB const &a) {
			bind_text::repr_stream ss;
			ss << name << "(lo=" << a.lo << ", hi=" << a.hi << ")";
			return ss.str();
		})
		.def("__getstate__", [](AABB const &a) {
			return nb::make_tuple(a.lo, a.hi);
		})
		.def("__setstate__", [](AABB &a, nb::tuple t) {
			check_state(t, 2, "AABB.__setstate__");
			a.lo = nb::cast<Vec2>(t[0]);
			a.hi = nb::cast<Vec2>(t[1]);
		});
}

template<class T>
void bind_aabb3(nb::module_ &m, char const *name) {
	using AABB = rove::aabb<3, T>;
	using Vec3 = rove::vec<3, T>;
	using Ray3 = rove::ray<3, T>;

	nb::class_<AABB>(m, name)
		.def(nb::init<>(),
			 "An empty box: it contains nothing, and the first extend() makes it that point")
		.def("is_empty", &AABB::is_empty,
			 "Whether the box holds no point: lo past hi on some axis (a box of a single point is not empty)")
		.def("grown", &AABB::grown, nb::arg("margin"),
			 "The box with every side moved out by margin (in by a negative one); an empty box stays empty, and one shrunk "
			 "past nothing is empty")
		.def(nb::init<Vec3 const &>(),
			 nb::arg("centre"),
			 "Construct degenerate AABB from single point")
		.def(nb::init<Vec3 const &, Vec3 const &>(),
			 nb::arg("lo"), nb::arg("hi"),
			 "Construct AABB from two corner points")
		.def(nb::init<Vec3 const &, T>(),
			 nb::arg("centre"), nb::arg("side_length"),
			 "Construct a cube AABB from its centre and full side length "
			 "(centre +- side_length / 2 on every axis)")
		.def_rw("lo", &AABB::lo, "Minimum corner (lower bounds)")
		.def_rw("hi", &AABB::hi, "Maximum corner (upper bounds)")
		.def("null", &AABB::null,
			 "Reset AABB to null state for accumulation")
		.def("normalize", &AABB::normalize,
			 "Ensure lo <= hi in all dimensions")
		.def("diagonal", &AABB::diagonal,
			 "Get AABB diagonal vector (hi - lo)")
		.def("centre", &AABB::centre,
			 "Get AABB center point")
		.def("extend", nb::overload_cast<Vec3 const &>(&AABB::extend),
			 nb::arg("point"),
			 "Extend AABB to include a point")
		.def("extend", nb::overload_cast<AABB const &>(&AABB::extend),
			 nb::arg("other"),
			 "Extend AABB to include another AABB")
		.def("contains", nb::overload_cast<Vec3 const &>(&AABB::contains, nb::const_),
			 nb::arg("point"),
			 "Test if point is inside AABB")
		.def("contains", nb::overload_cast<AABB const &>(&AABB::contains, nb::const_),
			 nb::arg("other"),
			 "Test if another AABB is fully contained")
		.def("test_intersection", &AABB::test_intersection,
			 nb::arg("other"),
			 "Test intersection with another AABB")
		.def("trace", [](AABB const &b, Ray3 const &r, T t_min, T t_max) -> nb::object {
			T t0, t1;
			if (!b.trace(r, &t0, &t1, t_min, t_max)) return nb::none();
			return nb::make_tuple(t0, t1);
		}, nb::arg("ray"), nb::arg("t_min") = T(0), nb::arg("t_max") = T(1),
		   "Ray-AABB intersection. Returns (t_enter, t_exit) tuple of "
		   "ray-parameter times, or None if the ray misses the AABB in "
		   "[t_min, t_max].")
		.def("__repr__", [name](AABB const &a) {
			bind_text::repr_stream ss;
			ss << name << "(lo=" << a.lo << ", hi=" << a.hi << ")";
			return ss.str();
		})
		.def("__getstate__", [](AABB const &a) {
			return nb::make_tuple(a.lo, a.hi);
		})
		.def("__setstate__", [](AABB &a, nb::tuple t) {
			check_state(t, 2, "AABB.__setstate__");
			a.lo = nb::cast<Vec3>(t[0]);
			a.hi = nb::cast<Vec3>(t[1]);
		});
}

// bounds_of a list of points and transform_aabb, for boxes of one type
template<class T>
static void bind_aabb_operations(nb::module_ &m) {
	m.def("bounds_of", [](std::vector<rove::vec<3, T>> const &points) { return rove::bounds_of(points.begin(), points.end()); },
		nb::arg("points"), "The box of the points; empty for none");
	m.def("bounds_of", [](std::vector<rove::vec<2, T>> const &points) { return rove::bounds_of(points.begin(), points.end()); },
		nb::arg("points"), "The box of the points; empty for none");
	m.def("transform_aabb", [](rove::aabb<3, T> const &box, rove::matrix<4, 4, T> const &mat) { return rove::transform_aabb(box, mat); },
		nb::arg("box"), nb::arg("m"),
		"The box of the box's corners through an affine matrix; an empty box stays empty; ValueError for a projection");
}

void bind_aabb_functions(nb::module_ &m) {
	bind_aabb_operations<float>(m);
	bind_aabb_operations<double>(m);
}

// Explicit template instantiations
template void bind_aabb2<float>(nb::module_ &m, char const *name);
template void bind_aabb2<double>(nb::module_ &m, char const *name);
template void bind_aabb3<float>(nb::module_ &m, char const *name);
template void bind_aabb3<double>(nb::module_ &m, char const *name);
