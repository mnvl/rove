// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_quaternion.cc
 * @brief Python bindings for quaternion classes
 */

#include "python_bindings.h"
#include "bind_text.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <sstream>

#include <rove/vec.h>
#include <rove/quaternion.h>
#include <rove/matrix.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_quaternion(nb::module_ &m, char const *name) {
	using Quat = rove::quaternion<T>;
	using Vec3 = rove::vec<3, T>;

	nb::class_<Quat>(m, name)
		.def(nb::init<>())
		.def(nb::init<T, T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("w"))
		.def_static("from_numpy", [](nb::ndarray<T const, nb::shape<4>> arr) {
			return Quat(arr(0), arr(1), arr(2), arr(3));
		}, nb::arg("array"), "Create quaternion from numpy array")
		.def("to_numpy", [](Quat const &q) {
			T* data = new T[4]{q.x, q.y, q.z, q.w};
			size_t shape[1] = {4};
			nb::capsule deleter(data, [](void *p) noexcept {
				delete[] static_cast<T*>(p);
			});
			return nb::ndarray<nb::numpy, T, nb::shape<4>>(data, 1, shape, deleter);
		}, "Convert quaternion to numpy array")
		.def_rw("x", &Quat::x)
		.def_rw("y", &Quat::y)
		.def_rw("z", &Quat::z)
		.def_rw("w", &Quat::w)
		.def("identity", &Quat::identity, "Set to identity quaternion (0, 0, 0, 1)")
		.def("norm", &Quat::norm, "Compute quaternion norm")
		.def("normalize", &Quat::normalize, "Normalize quaternion in place")
		.def("normalized", &Quat::normalized, "Return normalized quaternion")
		.def("scale", &Quat::scale, nb::arg("s"), "Scale quaternion by scalar")
		.def("__neg__", [](Quat const &q) { return -q; })
		.def("__add__", [](Quat const &p, Quat const &q) { return p + q; })
		.def("__sub__", [](Quat const &p, Quat const &q) { return p - q; })
		.def("__mul__", [](Quat const &q, T s) { return q * s; })
		.def("__mul__", [](Quat const &p, Quat const &q) { return p * q; }, "The composed rotation: q first, then p")
		.def("__mul__", [](Quat const &q, Vec3 const &v) { return q * v; },
			"v turned by q (any length but none: ValueError), as rotate(q, v)")
		.def("__rmul__", [](Quat const &q, T s) { return q * s; })
		.def("__repr__", [name](Quat const &q) {
			bind_text::repr_stream ss;
			ss << name << "(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
			return ss.str();
		})
		.def("dot", [](Quat const &p, Quat const &q) {
			return rove::dot(p, q);
		}, nb::arg("other"), "Compute dot product with another quaternion")
		.def("__getstate__", [](Quat const &q) {
			return nb::make_tuple(q.x, q.y, q.z, q.w);
		})
		.def("__setstate__", [](Quat &q, nb::tuple t) {
			check_state(t, 4, "Quat.__setstate__");
			q.x = nb::cast<T>(t[0]);
			q.y = nb::cast<T>(t[1]);
			q.z = nb::cast<T>(t[2]);
			q.w = nb::cast<T>(t[3]);
		});
}

template<class T>
void bind_quaternion_slerper(nb::module_ &m, char const *name) {
	using Slerper = rove::quaternion_slerper<T>;
	using Quat = rove::quaternion<T>;

	nb::class_<Slerper>(m, name)
		.def(nb::init<>())
		.def("__getstate__", [](Slerper const &s) {
			T values[Slerper::state_size];
			s.save_state(values);
			nb::list out;
			for (T const value : values) out.append(value);
			return nb::tuple(out);
		})
		.def("__setstate__", [](Slerper &s, nb::tuple t) {
			check_state(t, Slerper::state_size, "quat_slerper.__setstate__");
			T values[Slerper::state_size];
			for (size_t k = 0; k < Slerper::state_size; ++k) values[k] = nb::cast<T>(t[k]);
			new (&s) Slerper();
			s.load_state(values);
		})
		.def("setup", &Slerper::setup, nb::arg("p"), nb::arg("q"),
			 "Setup SLERP between two quaternions")
		.def("interpolate", nb::overload_cast<T>(&Slerper::interpolate, nb::const_),
			 nb::arg("t"), "Interpolate between quaternions at parameter t [0,1]");
}

// The operations of quaternion.h on quaternions of one type
template<class T>
static void bind_quaternion_operations(nb::module_ &m) {
	using Quat = rove::quaternion<T>;
	using Vec3 = rove::vec<3, T>;
	using Mat3 = rove::matrix<3, 3, T>;
	using Mat4 = rove::matrix<4, 4, T>;
	m.def("dot", [](Quat const &p, Quat const &q) { return rove::dot(p, q); }, nb::arg("p"), nb::arg("q"), "The dot product");
	m.def("conjugate", [](Quat const &q) { return rove::conjugate(q); }, nb::arg("q"), "The quaternion with x, y and z negated");
	m.def("axis_angle", [](Vec3 const &axis, T angle) { return rove::axis_angle(axis, angle); }, nb::arg("axis"), nb::arg("angle"),
		"The unit quaternion turning by angle (radians) about an axis of any length; ValueError for an axis of no length");
	m.def("rotate", [](Quat const &q, Vec3 const &v) { return rove::rotate(q, v); }, nb::arg("q"), nb::arg("v"),
		"v turned by q, a quaternion of any length but none (ValueError)");
	m.def("inverse", [](Quat const &q) { return rove::inverse(q); }, nb::arg("q"),
		"The opposite rotation, of a quaternion of any length but none (ValueError)");
	m.def("rotation_matrix", [](Quat const &q) { return rove::rotation_matrix(q); }, nb::arg("q"),
		"The 4x4 rotation of a quaternion of any length but none (ValueError)");
	m.def("to_quaternion", [](Mat3 const &m3) { return rove::to_quaternion(m3); }, nb::arg("m"),
		"The unit quaternion (w >= 0) of a rotation matrix");
	m.def("to_quaternion", [](Mat4 const &m4) { return rove::to_quaternion(m4); }, nb::arg("m"),
		"The unit quaternion (w >= 0) of a rotation matrix's upper 3x3");
	m.def("rotation_between", [](Vec3 const &from, Vec3 const &to) { return rove::rotation_between(from, to); },
		nb::arg("from_"), nb::arg("to"),
		"The shortest rotation turning direction from_ onto direction to (any lengths but none: ValueError); half a turn for "
		"opposite ones");
	m.def("to_axis_angle", [](Quat const &q) {
		Vec3 axis;
		T angle;
		rove::to_axis_angle(q, axis, angle);
		return nb::make_tuple(axis, angle);
	}, nb::arg("q"), "(axis, angle): the unit axis and angle (0 to pi) of q's rotation; (x, 0) for none; ValueError for no quaternion");
	m.def("rotation_vector", [](Quat const &q) { return rove::rotation_vector(q); }, nb::arg("q"),
		"The axis times the angle (0 to pi): the log map; ValueError for no quaternion");
	m.def("from_rotation_vector", [](Vec3 const &r) { return rove::from_rotation_vector(r); }, nb::arg("r"),
		"The unit quaternion of a turn of |r| about r: the exp map");
	m.def("slerp", [](Quat const &p, Quat const &q, T t) { return rove::slerp(p, q, t); }, nb::arg("p"), nb::arg("q"), nb::arg("t"),
		"Spherical interpolation from p (t = 0) to q (t = 1) along the shorter arc, at constant speed");
	m.def("nlerp", [](Quat const &p, Quat const &q, T t) { return rove::nlerp(p, q, t); }, nb::arg("p"), nb::arg("q"), nb::arg("t"),
		"Normalised linear interpolation from p to q along the shorter arc; p and q of any length but none (ValueError)");
}

void bind_quaternion_functions(nb::module_ &m) {
	bind_quaternion_operations<float>(m);
	bind_quaternion_operations<double>(m);
}

// Explicit template instantiations
template void bind_quaternion<float>(nb::module_ &m, char const *name);
template void bind_quaternion<double>(nb::module_ &m, char const *name);
template void bind_quaternion_slerper<float>(nb::module_ &m, char const *name);
template void bind_quaternion_slerper<double>(nb::module_ &m, char const *name);
