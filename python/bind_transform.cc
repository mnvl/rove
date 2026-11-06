// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_transform.cc
 * @brief Python bindings for transform.h: trs, and the ray under a point of the screen
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include <rove/transform.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_trs(nb::module_ &m, char const *name) {
	using Trs = rove::trs<T>;
	using Vec3 = rove::vec<3, T>;
	using Quat = rove::quaternion<T>;
	using Mat4 = rove::matrix<4, 4, T>;

	nb::class_<Trs>(m, name,
		"A transform kept as its translation, rotation and scale, whose matrix scales, then turns, then moves (T R S, as glTF's "
		"nodes and a skeleton's joints)")
		.def(nb::init<>(), "No transform: no move, no turn, a scale of 1")
		.def(nb::init<Vec3 const &, Quat const &, Vec3 const &>(), "translation"_a, "rotation"_a, "scale"_a)
		.def_rw("translation", &Trs::translation)
		.def_rw("rotation", &Trs::rotation)
		.def_rw("scale", &Trs::scale)
		.def("to_matrix", &Trs::to_matrix, "T R S")
		.def("apply_point", &Trs::apply_point, "p"_a, "The point scaled, turned and moved")
		.def("apply_vector", &Trs::apply_vector, "v"_a, "The vector scaled and turned, not moved")
		.def("__mul__", [](Trs const &a, Trs const &b) { return a * b; },
			"a then b's parent: the matrices' product, where a's scale is uniform or b does not turn (a shear otherwise: "
			"ValueError)")
		.def("__getstate__", [](Trs const &t) { return nb::make_tuple(t.translation, t.rotation, t.scale); })
		.def("__setstate__", [](Trs &t, nb::tuple state) {
			check_state(state, 3, "trs.__setstate__");
			new (&t) Trs(nb::cast<Vec3>(state[0]), nb::cast<Quat>(state[1]), nb::cast<Vec3>(state[2]));
		})
		.def("__repr__", [name](Trs const &t) {
			bind_text::repr_stream ss;
			ss << name << "(translation=" << t.translation << ", rotation=(" << t.rotation.x << ", " << t.rotation.y << ", "
			   << t.rotation.z << ", " << t.rotation.w << "), scale=" << t.scale << ")";
			return ss.str();
		});

	m.def("to_trs", [](Mat4 const &mat) { return rove::to_trs(mat); }, "m"_a,
		"The trs whose matrix is m (x's scale negative for a mirror); ValueError for a projection, a shear or a scale of no length");
	m.def("inverse", [](Trs const &t) { return rove::inverse(t); }, "t"_a,
		"The opposite transform, where t's scale is uniform or t does not turn; ValueError otherwise and for a scale of zero");
	m.def("lerp", [](Trs const &a, Trs const &b, T u) { return rove::lerp(a, b, u); }, "a"_a, "b"_a, "u"_a,
		"Translation and scale lerped, rotation slerped");
	m.def("unproject_ray", [](Mat4 const &inverse_view_projection, T ndc_x, T ndc_y) {
		return rove::unproject_ray(inverse_view_projection, ndc_x, ndc_y);
	}, "inverse_view_projection"_a, "ndc_x"_a, "ndc_y"_a,
		"The ray under a point of the screen (-1 to 1, +y up), from the near plane (r0) to the far (r0 + a); ValueError for a "
		"matrix that takes the point to no finite place");
}

template void bind_trs<float>(nb::module_ &m, char const *name);
template void bind_trs<double>(nb::module_ &m, char const *name);
