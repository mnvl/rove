// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_sphere.cc
 * @brief Python bindings for sphere classes
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/sphere.h>
#include <rove/aabb.h>
#include <rove/ray.h>
#include <rove/segment.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_sphere2(nb::module_ &m, char const *name) {
	using Sphere = rove::sphere<2, T>;
	using Vec2 = rove::vec<2, T>;
	using AABB2 = rove::aabb<2, T>;
	using Ray2 = rove::ray<2, T>;
	using Line2 = rove::segment<2, T>;

	nb::class_<Sphere>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec2 const &, T>(),
			 nb::arg("centre"), nb::arg("radius"),
			 "Construct sphere from center and radius")
		.def(nb::init<AABB2 const &>(),
			 nb::arg("aabb"),
			 "Construct bounding sphere from AABB")
		.def_rw("centre", &Sphere::centre, "Center point of the sphere")
		.def_rw("radius", &Sphere::radius, "Radius of the sphere")
		.def("set", &Sphere::set,
			 nb::arg("centre"), nb::arg("radius"),
			 "Set sphere center and radius")
		.def("construct", &Sphere::construct,
			 nb::arg("aabb"),
			 "Initialize sphere from AABB")
		.def("bounds", &Sphere::bounds,
			 "Get axis-aligned bounding box containing this sphere")
		.def("contains", &Sphere::contains,
			 nb::arg("point"),
			 "Test if point is inside sphere")
		.def("test_intersection",
			 nb::overload_cast<Sphere const &>(&Sphere::test_intersection, nb::const_),
			 nb::arg("other"),
			 "Test intersection with another sphere")
		.def("test_intersection",
			 nb::overload_cast<Ray2 const &>(&Sphere::test_intersection, nb::const_),
			 nb::arg("ray"),
			 "Test if a ray intersects this circle")
		.def("test_intersection",
			 nb::overload_cast<Line2 const &>(&Sphere::test_intersection, nb::const_),
			 nb::arg("line"),
			 "Test if a line segment intersects this circle")
		.def("query_intersection",
			 [](Sphere const &s, Ray2 const &r) {
				 Vec2 p1, p2;
				 int n = s.query_intersection(r, p1, p2);
				 return nb::make_tuple(n, p1, p2);
			 },
			 nb::arg("ray"),
			 "Intersect circle with ray. Returns (count, p1, p2); count is 0, 1, or 2.")
		.def("query_intersection",
			 [](Sphere const &s, Line2 const &l) {
				 Vec2 p1, p2;
				 int n = s.query_intersection(l, p1, p2);
				 return nb::make_tuple(n, p1, p2);
			 },
			 nb::arg("line"),
			 "Intersect circle with segment. Returns (count, p1, p2); count is 0, 1, or 2.")
		.def("__repr__", [name](Sphere const &s) {
			bind_text::repr_stream ss;
			ss << name << "(centre=" << s.centre << ", radius=" << s.radius << ")";
			return ss.str();
		})
		.def("__getstate__", [](Sphere const &s) {
			return nb::make_tuple(s.centre, s.radius);
		})
		.def("__setstate__", [](Sphere &s, nb::tuple t) {
			check_state(t, 2, "Sphere.__setstate__");
			s.centre = nb::cast<Vec2>(t[0]);
			s.radius = nb::cast<T>(t[1]);
		});
}

template<class T>
void bind_sphere3(nb::module_ &m, char const *name) {
	using Sphere = rove::sphere<3, T>;
	using Vec3 = rove::vec<3, T>;
	using AABB3 = rove::aabb<3, T>;

	nb::class_<Sphere>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec3 const &, T>(),
			 nb::arg("centre"), nb::arg("radius"),
			 "Construct sphere from center and radius")
		.def(nb::init<AABB3 const &>(),
			 nb::arg("aabb"),
			 "Construct bounding sphere from AABB")
		.def_rw("centre", &Sphere::centre, "Center point of the sphere")
		.def_rw("radius", &Sphere::radius, "Radius of the sphere")
		.def("set", &Sphere::set,
			 nb::arg("centre"), nb::arg("radius"),
			 "Set sphere center and radius")
		.def("construct", &Sphere::construct,
			 nb::arg("aabb"),
			 "Initialize sphere from AABB")
		.def("bounds", &Sphere::bounds,
			 "Get axis-aligned bounding box containing this sphere")
		.def("contains", &Sphere::contains,
			 nb::arg("point"),
			 "Test if point is inside sphere")
		.def("test_intersection",
			 nb::overload_cast<Sphere const &>(&Sphere::test_intersection, nb::const_),
			 nb::arg("other"),
			 "Test intersection with another sphere")
		.def("__repr__", [name](Sphere const &s) {
			bind_text::repr_stream ss;
			ss << name << "(centre=" << s.centre << ", radius=" << s.radius << ")";
			return ss.str();
		})
		.def("__getstate__", [](Sphere const &s) {
			return nb::make_tuple(s.centre, s.radius);
		})
		.def("__setstate__", [](Sphere &s, nb::tuple t) {
			check_state(t, 2, "Sphere.__setstate__");
			s.centre = nb::cast<Vec3>(t[0]);
			s.radius = nb::cast<T>(t[1]);
		});
}

// Explicit template instantiations
template void bind_sphere2<float>(nb::module_ &m, char const *name);
template void bind_sphere2<double>(nb::module_ &m, char const *name);
template void bind_sphere3<float>(nb::module_ &m, char const *name);
template void bind_sphere3<double>(nb::module_ &m, char const *name);
