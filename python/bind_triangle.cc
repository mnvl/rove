// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_triangle.cc
 * @brief Python bindings for triangle class
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/triangle.h>
#include <rove/ray.h>
#include <rove/plane.h>
#include <rove/sphere.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_triangle3(nb::module_ &m, char const *name) {
	using Triangle = rove::triangle<3, T>;
	using Vec3 = rove::vec<3, T>;
	using Ray3 = rove::ray<3, T>;
	using Plane = rove::plane<T>;
	using Sphere3 = rove::sphere<3, T>;

	nb::class_<Triangle>(m, name)
		.def(nb::init<>())
		.def(nb::init<Vec3 const &, Vec3 const &, Vec3 const &>(),
			 nb::arg("a"), nb::arg("b"), nb::arg("c"),
			 "Construct triangle from three vertices")
		.def_rw("A", &Triangle::A, "First vertex of triangle")
		.def_rw("B", &Triangle::B, "Second vertex of triangle")
		.def_rw("C", &Triangle::C, "Third vertex of triangle")
		.def("construct", &Triangle::construct,
			 nb::arg("a"), nb::arg("b"), nb::arg("c"),
			 "Set triangle vertices")
		.def("normal", &Triangle::normal,
			 "Get normalized normal vector of the triangle")
		.def("to_plane", &Triangle::to_plane,
			 "Get the plane containing this triangle")
		.def("perimeter", &Triangle::perimeter,
			 "Get perimeter of the triangle")
		.def("area", &Triangle::area,
			 "Get area of the triangle")
		.def("cog", &Triangle::cog,
			 "Get center of gravity (centroid)")
		.def("orthocenter", &Triangle::orthocenter,
			 "Get orthocenter (concurrence of altitudes)")
		.def("circumcenter", &Triangle::circumcenter,
			 "Get circumcenter (center of circumscribed circle)")
		.def("circumcircle_radius", &Triangle::circumcircle_radius,
			 "Get radius of circumscribed circle")
		.def("circumcircle", &Triangle::circumcircle,
			 "Get circumscribed sphere (passes through all three vertices)")
		.def("incenter", &Triangle::incenter,
			 "Get incenter (center of inscribed circle)")
		.def("incircle_radius", &Triangle::incircle_radius,
			 "Get radius of inscribed circle")
		.def("incircle", &Triangle::incircle,
			 "Get inscribed sphere (tangent to all three edges)")
		.def("excenter_A", &Triangle::excenter_A,
			 "Get excenter opposite to vertex A")
		.def("excircle_A_radius", &Triangle::excircle_A_radius,
			 "Get radius of excircle opposite to vertex A")
		.def("excenter_B", &Triangle::excenter_B,
			 "Get excenter opposite to vertex B")
		.def("excircle_B_radius", &Triangle::excircle_B_radius,
			 "Get radius of excircle opposite to vertex B")
		.def("excenter_C", &Triangle::excenter_C,
			 "Get excenter opposite to vertex C")
		.def("excircle_C_radius", &Triangle::excircle_C_radius,
			 "Get radius of excircle opposite to vertex C")
		.def("feuerbach_center", &Triangle::feuerbach_center,
			 "Get Feuerbach circle center (nine-point circle)")
		.def("feuerbach_radius", &Triangle::feuerbach_radius,
			 "Get Feuerbach circle radius")
		.def("contains_on_plane", &Triangle::contains_on_plane,
			 nb::arg("point"),
			 "Test if point on triangle's plane is inside the triangle")
		.def("contains", &Triangle::contains,
			 nb::arg("point"),
			 "Test if 3D point is inside the triangle")
		.def("trace",
			 nb::overload_cast<Ray3 const &, T, T, T>(&Triangle::trace, nb::const_),
			 nb::arg("ray"), nb::arg("t_min") = 0, nb::arg("t_max") = 1, nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon,
			 "Test if ray intersects triangle within parameter range; a determinant under epsilon counts as parallel")
		// Note: test_intersection and query_intersection are not bound because
		// they depend on line::query_intersection which is not implemented in C++
		.def("__repr__", [](Triangle const &t) {
			std::ostringstream ss;
			ss << "triangle3(A=" << t.A << ", B=" << t.B << ", C=" << t.C << ")";
			return ss.str();
		})
		.def("__getstate__", [](Triangle const &t) {
			return nb::make_tuple(t.A, t.B, t.C);
		})
		.def("__setstate__", [](Triangle &t, nb::tuple state) {
			t.A = nb::cast<Vec3>(state[0]);
			t.B = nb::cast<Vec3>(state[1]);
			t.C = nb::cast<Vec3>(state[2]);
		});
}

// Explicit template instantiations
template void bind_triangle3<float>(nb::module_ &m, char const *name);
template void bind_triangle3<double>(nb::module_ &m, char const *name);
