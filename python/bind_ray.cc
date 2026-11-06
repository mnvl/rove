// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_ray.cc
 * @brief Python bindings for ray classes
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/ray.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_ray2(nb::module_ &m, char const *name) {
	using Ray = rove::ray<2, T>;
	using Vec2 = rove::vec<2, T>;

	nb::class_<Ray>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec2 const &, Vec2 const &>(),
			 nb::arg("origin"), nb::arg("direction"),
			 "Construct ray from origin and direction")
		.def_rw("r0", &Ray::r0, "Ray origin point")
		.def_rw("a", &Ray::a, "Ray direction vector")
		.def("set", &Ray::set,
			 nb::arg("origin"), nb::arg("direction"),
			 "Set ray origin and direction")
		.def("apply", &Ray::apply,
			 nb::arg("t"),
			 "Get point on ray at parameter t: r0 + a * t")
		.def("contains", &Ray::contains,
			 nb::arg("point"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Test if point lies on the ray")
		.def("distance_sq",
			 nb::overload_cast<Vec2 const &>(&Ray::distance_sq, nb::const_),
			 nb::arg("point"),
			 "Get squared distance from ray to point")
		.def("distance",
			 nb::overload_cast<Vec2 const &>(&Ray::distance, nb::const_),
			 nb::arg("point"),
			 "Get distance from ray to point")
		.def("distance",
			 nb::overload_cast<Ray const &>(&Ray::distance, nb::const_),
			 nb::arg("other"),
			 "Distance between the two lines: zero where they cross, the closest approach where they are skew, "
			 "how far apart they are where they are parallel")
		.def("test_intersection", &Ray::test_intersection,
			 nb::arg("other"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Whether the two lines meet: distance(other) < epsilon")
		.def("__repr__", [name](Ray const &r) {
			bind_text::repr_stream ss;
			ss << name << "(r0=" << r.r0 << ", a=" << r.a << ")";
			return ss.str();
		})
		.def("__getstate__", [](Ray const &r) {
			return nb::make_tuple(r.r0, r.a);
		})
		.def("__setstate__", [](Ray &r, nb::tuple t) {
			check_state(t, 2, "Ray.__setstate__");
			r.r0 = nb::cast<Vec2>(t[0]);
			r.a  = nb::cast<Vec2>(t[1]);
		});
}

template<class T>
void bind_ray3(nb::module_ &m, char const *name) {
	using Ray = rove::ray<3, T>;
	using Vec3 = rove::vec<3, T>;

	nb::class_<Ray>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec3 const &, Vec3 const &>(),
			 nb::arg("origin"), nb::arg("direction"),
			 "Construct ray from origin and direction")
		.def_rw("r0", &Ray::r0, "Ray origin point")
		.def_rw("a", &Ray::a, "Ray direction vector")
		.def("set", &Ray::set,
			 nb::arg("origin"), nb::arg("direction"),
			 "Set ray origin and direction")
		.def("apply", &Ray::apply,
			 nb::arg("t"),
			 "Get point on ray at parameter t: r0 + a * t")
		.def("contains", &Ray::contains,
			 nb::arg("point"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Test if point lies on the ray")
		.def("distance_sq",
			 nb::overload_cast<Vec3 const &>(&Ray::distance_sq, nb::const_),
			 nb::arg("point"),
			 "Get squared distance from ray to point")
		.def("distance",
			 nb::overload_cast<Vec3 const &>(&Ray::distance, nb::const_),
			 nb::arg("point"),
			 "Get distance from ray to point")
		.def("distance",
			 nb::overload_cast<Ray const &>(&Ray::distance, nb::const_),
			 nb::arg("other"),
			 "Distance between the two lines: zero where they cross, the closest approach where they are skew, "
			 "how far apart they are where they are parallel")
		.def("test_intersection", &Ray::test_intersection,
			 nb::arg("other"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Whether the two lines meet: distance(other) < epsilon")
		.def("__repr__", [name](Ray const &r) {
			bind_text::repr_stream ss;
			ss << name << "(r0=" << r.r0 << ", a=" << r.a << ")";
			return ss.str();
		})
		.def("__getstate__", [](Ray const &r) {
			return nb::make_tuple(r.r0, r.a);
		})
		.def("__setstate__", [](Ray &r, nb::tuple t) {
			check_state(t, 2, "Ray.__setstate__");
			r.r0 = nb::cast<Vec3>(t[0]);
			r.a  = nb::cast<Vec3>(t[1]);
		});
}

// Explicit template instantiations
template void bind_ray2<float>(nb::module_ &m, char const *name);
template void bind_ray2<double>(nb::module_ &m, char const *name);
template void bind_ray3<float>(nb::module_ &m, char const *name);
template void bind_ray3<double>(nb::module_ &m, char const *name);
