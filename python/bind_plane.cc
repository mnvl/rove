// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_plane.cc
 * @brief Python bindings for plane class
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/plane.h>
#include <rove/ray.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_plane(nb::module_ &m, char const *name) {
	using Plane = rove::plane<T>;
	using Vec3 = rove::vec<3, T>;
	using Ray3 = rove::ray<3, T>;

	nb::class_<Plane>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec3 const &, Vec3 const &>(),
			 nb::arg("origin"), nb::arg("normal"),
			 "Construct plane from a point on the plane and normal vector")
		.def(nb::init<T, Vec3 const &>(),
			 nb::arg("d"), nb::arg("normal"),
			 "Construct plane from distance D and normal vector")
		.def_rw("A", &Plane::A, "X coefficient in plane equation Ax + By + Cz + D = 0")
		.def_rw("B", &Plane::B, "Y coefficient in plane equation Ax + By + Cz + D = 0")
		.def_rw("C", &Plane::C, "Z coefficient in plane equation Ax + By + Cz + D = 0")
		.def_rw("D", &Plane::D, "Distance coefficient in plane equation Ax + By + Cz + D = 0")
		.def("set",
			 nb::overload_cast<Vec3 const &, Vec3 const &>(&Plane::set),
			 nb::arg("origin"), nb::arg("normal"),
			 "Set plane from a point on the plane and normal vector")
		.def("set",
			 nb::overload_cast<T, Vec3 const &>(&Plane::set),
			 nb::arg("d"), nb::arg("normal"),
			 "Set plane from distance D and normal vector")
		.def("apply", &Plane::apply,
			 nb::arg("point"),
			 "Evaluate plane equation at point: Ax + By + Cz + D")
		.def("test_intersection", &Plane::test_intersection,
			 nb::arg("ray"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Test if ray intersects with the plane")
		.def("contains",
			 nb::overload_cast<Vec3 const &, T>(&Plane::contains, nb::const_),
			 nb::arg("point"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Test if point lies on the plane")
		.def("contains",
			 nb::overload_cast<Ray3 const &, T>(&Plane::contains, nb::const_),
			 nb::arg("ray"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Test if ray lies on the plane")
		.def("parallel", &Plane::parallel,
			 nb::arg("ray"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Test if ray is parallel to the plane")
		.def("trace",
			 nb::overload_cast<Ray3 const &>(&Plane::trace, nb::const_),
			 nb::arg("ray"),
			 "Get parameter t where ray intersects plane")
		.def("origin", &Plane::origin,
			 "Get a point on the plane")
		.def("normal", &Plane::normal,
			 "Get plane normal vector")
		.def("is_correct", &Plane::is_correct,
			 "Check if plane equation is valid (at least one coefficient is non-zero)")
		.def("classify",
			 [](Plane const &plane, Vec3 const &point, T epsilon) {
				 return static_cast<int>(plane.classify(point, epsilon));
			 },
			 nb::arg("point"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Classify point position relative to plane (returns int: 1=positive, 2=intersects, 3=negative)")
		.def("__repr__", [](Plane const &p) {
			std::ostringstream ss;
			ss << "plane(A=" << p.A << ", B=" << p.B << ", C=" << p.C << ", D=" << p.D << ")";
			return ss.str();
		})
		.def("__getstate__", [](Plane const &p) {
			return nb::make_tuple(p.A, p.B, p.C, p.D);
		})
		.def("__setstate__", [](Plane &p, nb::tuple t) {
			p.A = nb::cast<T>(t[0]);
			p.B = nb::cast<T>(t[1]);
			p.C = nb::cast<T>(t[2]);
			p.D = nb::cast<T>(t[3]);
		});
}

// Explicit template instantiations
template void bind_plane<float>(nb::module_ &m, char const *name);
template void bind_plane<double>(nb::module_ &m, char const *name);
