// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_vec.cc
 * @brief Python bindings for vector classes (vec2, vec3, vec4)
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <nanobind/make_iterator.h>
#include <sstream>
#include <functional>

#include <rove/vec.h>

namespace nb = nanobind;
using namespace nb::literals;

template<typename T, size_t N>
static std::size_t hash_components(T const (&i)[N]) {
	std::size_t seed = 0;
	for (size_t k = 0; k < N; ++k) {
		seed ^= std::hash<T>{}(i[k]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}

template<class T>
void bind_vec2(nb::module_ &m, char const *name) {
	using Vec = rove::vec<2, T>;

	nb::class_<Vec>(m, name)
		.def(nb::init<>())
		.def(nb::init<T, T>(), nb::arg("x"), nb::arg("y"))
		.def_rw("x", &Vec::x)
		.def_rw("y", &Vec::y)
		.def_prop_ro("xy", [](Vec const &v) { return nb::make_tuple(v.x, v.y); })
		.def_static("from_numpy", [](nb::ndarray<T, nb::shape<2>> arr) {
			return Vec(arr(0), arr(1));
		}, nb::arg("array"), "Create vec2 from numpy array")
		.def("to_numpy", [](Vec const &v) {
			T* data = new T[2]{v.x, v.y};
			size_t shape[1] = {2};
			nb::capsule deleter(data, [](void *p) noexcept {
				delete[] static_cast<T*>(p);
			});
			return nb::ndarray<nb::numpy, T, nb::shape<2>>(data, 1, shape, deleter);
		}, "Convert vec2 to numpy array")
		.def("set", &Vec::set, nb::arg("x"), nb::arg("y"))
		.def("set_all", &Vec::set_all, nb::arg("a"))
		.def("length", &Vec::length)
		.def("length_sq", &Vec::length_sq)
		.def("manhattan_length", &Vec::manhattan_length)
		.def("normalize", &Vec::normalize, "Unit length in place; ValueError for a vector of no length or not finite")
		.def("negate", &Vec::negate)
		.def("perpendicular", &Vec::perpendicular)
		.def("is_collinear", &Vec::is_collinear, nb::arg("v"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon)
		.def("__neg__", [](Vec const &v) { return -v; })
		.def("__add__", [](Vec const &a, Vec const &b) { return a + b; })
		.def("__sub__", [](Vec const &a, Vec const &b) { return a - b; })
		.def("__mul__", [](Vec const &a, Vec const &b) { return a * b; })
		.def("__mul__", [](Vec const &a, T k) { return a * k; })
		.def("__rmul__", [](Vec const &a, T k) { return a * k; })
		.def("__truediv__", [](Vec const &a, Vec const &b) { return a / b; })
		.def("__truediv__", [](Vec const &a, T k) { return a / k; })
		.def("__iadd__", &Vec::operator+=, nb::arg("v"), nb::rv_policy::none)
		.def("__isub__", nb::overload_cast<Vec const &>(&Vec::operator-=), nb::arg("v"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<Vec const &>(&Vec::operator*=), nb::arg("v"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<const T>(&Vec::operator*=), nb::arg("k"), nb::rv_policy::none)
		.def("__itruediv__", nb::overload_cast<Vec const &>(&Vec::operator/=), nb::arg("v"), nb::rv_policy::none)
		.def("__itruediv__", nb::overload_cast<const T>(&Vec::operator/=), nb::arg("k"), nb::rv_policy::none)
		.def("__eq__", [](Vec const &a, Vec const &b) { return a == b; })
		.def("__ne__", [](Vec const &a, Vec const &b) { return a != b; })
		.def("__hash__", [](Vec const &v) { return hash_components(v.i); })
		.def("__len__", [](Vec const &) { return 2; })
		.def("__getitem__", [](Vec const &v, Py_ssize_t k) -> T {
			if (k < 0) k += 2;
			if (k < 0 || k >= 2) throw nb::index_error();
			return v.i[k];
		})
		.def("__iter__", [](Vec &v) {
			return nb::make_iterator(nb::type<Vec>(), "vec2_iterator", &v.i[0], &v.i[2]);
		}, nb::keep_alive<0, 1>())
		.def("__repr__", [](Vec const &v) {
			std::ostringstream ss;
			ss << "vec2(" << v.x << ", " << v.y << ")";
			return ss.str();
		})
		.def("dot", [](Vec const &a, Vec const &b) { return rove::dot(a, b); })
		.def("__getstate__", [](Vec const &v) {
			return nb::make_tuple(v.x, v.y);
		})
		.def("__setstate__", [](Vec &v, nb::tuple t) {
			v.x = nb::cast<T>(t[0]);
			v.y = nb::cast<T>(t[1]);
		});
}

template<class T>
void bind_vec3(nb::module_ &m, char const *name) {
	using Vec = rove::vec<3, T>;

	nb::class_<Vec>(m, name)
		.def(nb::init<>())
		.def(nb::init<T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"))
		.def_rw("x", &Vec::x)
		.def_rw("y", &Vec::y)
		.def_rw("z", &Vec::z)
		.def_prop_ro("xy", [](Vec const &v) { return nb::make_tuple(v.x, v.y); })
		.def_prop_ro("xyz", [](Vec const &v) { return nb::make_tuple(v.x, v.y, v.z); })
		.def_static("from_numpy", [](nb::ndarray<T, nb::shape<3>> arr) {
			return Vec(arr(0), arr(1), arr(2));
		}, nb::arg("array"), "Create vec3 from numpy array")
		.def("to_numpy", [](Vec const &v) {
			T* data = new T[3]{v.x, v.y, v.z};
			size_t shape[1] = {3};
			nb::capsule deleter(data, [](void *p) noexcept {
				delete[] static_cast<T*>(p);
			});
			return nb::ndarray<nb::numpy, T, nb::shape<3>>(data, 1, shape, deleter);
		}, "Convert vec3 to numpy array")
		.def("set", &Vec::set, nb::arg("x"), nb::arg("y"), nb::arg("z"))
		.def("set_all", &Vec::set_all, nb::arg("a"))
		.def("length", &Vec::length)
		.def("length_sq", &Vec::length_sq)
		.def("manhattan_length", &Vec::manhattan_length)
		.def("normalize", &Vec::normalize, "Unit length in place; ValueError for a vector of no length or not finite")
		.def("negate", &Vec::negate)
		.def("is_collinear", &Vec::is_collinear, nb::arg("v"), nb::arg("epsilon") = rove::scalar_traits<T>::default_epsilon)
		.def("__neg__", [](Vec const &v) { return -v; })
		.def("__add__", [](Vec const &a, Vec const &b) { return a + b; })
		.def("__sub__", [](Vec const &a, Vec const &b) { return a - b; })
		.def("__mul__", [](Vec const &a, Vec const &b) { return a * b; })
		.def("__mul__", [](Vec const &a, T k) { return a * k; })
		.def("__rmul__", [](Vec const &a, T k) { return a * k; })
		.def("__truediv__", [](Vec const &a, Vec const &b) { return a / b; })
		.def("__truediv__", [](Vec const &a, T k) { return a / k; })
		.def("__iadd__", &Vec::operator+=, nb::arg("v"), nb::rv_policy::none)
		.def("__isub__", nb::overload_cast<Vec const &>(&Vec::operator-=), nb::arg("v"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<Vec const &>(&Vec::operator*=), nb::arg("v"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<const T>(&Vec::operator*=), nb::arg("k"), nb::rv_policy::none)
		.def("__itruediv__", nb::overload_cast<Vec const &>(&Vec::operator/=), nb::arg("v"), nb::rv_policy::none)
		.def("__itruediv__", nb::overload_cast<const T>(&Vec::operator/=), nb::arg("k"), nb::rv_policy::none)
		.def("__eq__", [](Vec const &a, Vec const &b) { return a == b; })
		.def("__ne__", [](Vec const &a, Vec const &b) { return a != b; })
		.def("__hash__", [](Vec const &v) { return hash_components(v.i); })
		.def("__len__", [](Vec const &) { return 3; })
		.def("__getitem__", [](Vec const &v, Py_ssize_t k) -> T {
			if (k < 0) k += 3;
			if (k < 0 || k >= 3) throw nb::index_error();
			return v.i[k];
		})
		.def("__iter__", [](Vec &v) {
			return nb::make_iterator(nb::type<Vec>(), "vec3_iterator", &v.i[0], &v.i[3]);
		}, nb::keep_alive<0, 1>())
		.def("__repr__", [](Vec const &v) {
			std::ostringstream ss;
			ss << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
			return ss.str();
		})
		.def("dot", [](Vec const &a, Vec const &b) { return rove::dot(a, b); })
		.def("cross", [](Vec const &a, Vec const &b) {
			Vec result;
			result = rove::cross(a, b);
			return result;
		})
		.def("__getstate__", [](Vec const &v) {
			return nb::make_tuple(v.x, v.y, v.z);
		})
		.def("__setstate__", [](Vec &v, nb::tuple t) {
			v.x = nb::cast<T>(t[0]);
			v.y = nb::cast<T>(t[1]);
			v.z = nb::cast<T>(t[2]);
		});
}

template<class T>
void bind_vec4(nb::module_ &m, char const *name) {
	using Vec = rove::vec<4, T>;

	nb::class_<Vec>(m, name)
		.def(nb::init<>())
		.def(nb::init<T, T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("w"))
		.def(nb::init<rove::vec<3, T> const &, T>(), nb::arg("xyz"), nb::arg("w"))
		.def_rw("x", &Vec::x)
		.def_rw("y", &Vec::y)
		.def_rw("z", &Vec::z)
		.def_rw("w", &Vec::w)
		.def_prop_ro("xy", [](Vec const &v) { return nb::make_tuple(v.x, v.y); })
		.def_prop_ro("xyz", [](Vec const &v) { return nb::make_tuple(v.x, v.y, v.z); })
		.def_prop_ro("xyzw", [](Vec const &v) { return nb::make_tuple(v.x, v.y, v.z, v.w); })
		.def_static("from_numpy", [](nb::ndarray<T, nb::shape<4>> arr) {
			return Vec(arr(0), arr(1), arr(2), arr(3));
		}, nb::arg("array"), "Create vec4 from numpy array")
		.def("to_numpy", [](Vec const &v) {
			T* data = new T[4]{v.x, v.y, v.z, v.w};
			size_t shape[1] = {4};
			nb::capsule deleter(data, [](void *p) noexcept {
				delete[] static_cast<T*>(p);
			});
			return nb::ndarray<nb::numpy, T, nb::shape<4>>(data, 1, shape, deleter);
		}, "Convert vec4 to numpy array")
		.def("set", &Vec::set, nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("w"))
		.def("set_all", &Vec::set_all, nb::arg("a"))
		.def("length", &Vec::length)
		.def("length_sq", &Vec::length_sq)
		.def("manhattan_length", &Vec::manhattan_length)
		.def("normalize", &Vec::normalize, "Unit length in place; ValueError for a vector of no length or not finite")
		.def("negate", &Vec::negate)
		.def("__neg__", [](Vec const &v) { return -v; })
		.def("__add__", [](Vec const &a, Vec const &b) { return a + b; })
		.def("__sub__", [](Vec const &a, Vec const &b) { return a - b; })
		.def("__mul__", [](Vec const &a, Vec const &b) { return a * b; })
		.def("__mul__", [](Vec const &a, T k) { return a * k; })
		.def("__rmul__", [](Vec const &a, T k) { return a * k; })
		.def("__truediv__", [](Vec const &a, Vec const &b) { return a / b; })
		.def("__truediv__", [](Vec const &a, T k) { return a / k; })
		.def("__iadd__", &Vec::operator+=, nb::arg("v"), nb::rv_policy::none)
		.def("__isub__", nb::overload_cast<Vec const &>(&Vec::operator-=), nb::arg("v"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<Vec const &>(&Vec::operator*=), nb::arg("v"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<const T>(&Vec::operator*=), nb::arg("k"), nb::rv_policy::none)
		.def("__itruediv__", nb::overload_cast<Vec const &>(&Vec::operator/=), nb::arg("v"), nb::rv_policy::none)
		.def("__itruediv__", nb::overload_cast<const T>(&Vec::operator/=), nb::arg("k"), nb::rv_policy::none)
		.def("__eq__", [](Vec const &a, Vec const &b) { return a == b; })
		.def("__ne__", [](Vec const &a, Vec const &b) { return a != b; })
		.def("__hash__", [](Vec const &v) { return hash_components(v.i); })
		.def("__len__", [](Vec const &) { return 4; })
		.def("__getitem__", [](Vec const &v, Py_ssize_t k) -> T {
			if (k < 0) k += 4;
			if (k < 0 || k >= 4) throw nb::index_error();
			return v.i[k];
		})
		.def("__iter__", [](Vec &v) {
			return nb::make_iterator(nb::type<Vec>(), "vec4_iterator", &v.i[0], &v.i[4]);
		}, nb::keep_alive<0, 1>())
		.def("__repr__", [](Vec const &v) {
			std::ostringstream ss;
			ss << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
			return ss.str();
		})
		.def("dot", [](Vec const &a, Vec const &b) { return rove::dot(a, b); })
		.def("__getstate__", [](Vec const &v) {
			return nb::make_tuple(v.x, v.y, v.z, v.w);
		})
		.def("__setstate__", [](Vec &v, nb::tuple t) {
			v.x = nb::cast<T>(t[0]);
			v.y = nb::cast<T>(t[1]);
			v.z = nb::cast<T>(t[2]);
			v.w = nb::cast<T>(t[3]);
		});
}

// The operations of vec.h on vectors of one type; each answer is of its arguments' type
template<size_t N, typename T>
static void bind_vec_operations(nb::module_ &m) {
	using Vec = rove::vec<N, T>;
	m.def("dot", [](Vec const &a, Vec const &b) { return rove::dot(a, b); }, nb::arg("a"), nb::arg("b"), "The dot product");
	m.def("normalize", [](Vec const &v) { return rove::normalize(v); }, nb::arg("v"),
		"v at unit length, as a new vector; ValueError for one of no length or not finite");
	m.def("normalize_or_zero", [](Vec const &v) { return rove::normalize_or_zero(v); }, nb::arg("v"),
		"v at unit length, as a new vector, or v as it is where it has no length");
	m.def("all_less", [](Vec const &a, Vec const &b) { return rove::all_less(a, b); }, nb::arg("a"), nb::arg("b"),
		"Whether every component of a is less than b's");
	m.def("all_less_equal", [](Vec const &a, Vec const &b) { return rove::all_less_equal(a, b); }, nb::arg("a"), nb::arg("b"));
	m.def("all_greater", [](Vec const &a, Vec const &b) { return rove::all_greater(a, b); }, nb::arg("a"), nb::arg("b"));
	m.def("all_greater_equal", [](Vec const &a, Vec const &b) { return rove::all_greater_equal(a, b); }, nb::arg("a"), nb::arg("b"));
	m.def("min", [](Vec const &a, Vec const &b) { return rove::min(a, b); }, nb::arg("a"), nb::arg("b"), "Component by component");
	m.def("max", [](Vec const &a, Vec const &b) { return rove::max(a, b); }, nb::arg("a"), nb::arg("b"), "Component by component");
	m.def("abs", [](Vec const &v) { return rove::abs(v); }, nb::arg("v"), "Component by component");
	m.def("clamp", [](Vec const &v, Vec const &lo, Vec const &hi) { return rove::clamp(v, lo, hi); },
		nb::arg("v"), nb::arg("lo"), nb::arg("hi"), "Each component between lo's and hi's");
	m.def("clamp", [](Vec const &v, T lo, T hi) { return rove::clamp(v, lo, hi); }, nb::arg("v"), nb::arg("lo"), nb::arg("hi"),
		"Each component between lo and hi");
	m.def("is_finite", [](Vec const &v) { return rove::is_finite(v); }, nb::arg("v"));
	m.def("project", [](Vec const &v, Vec const &onto) { return rove::project(v, onto); }, nb::arg("v"), nb::arg("onto"),
		"The part of v along onto (any length but none: ValueError)");
	m.def("reject", [](Vec const &v, Vec const &onto) { return rove::reject(v, onto); }, nb::arg("v"), nb::arg("onto"),
		"The part of v square to onto (any length but none: ValueError)");
	m.def("reflect", [](Vec const &v, Vec const &n) { return rove::reflect(v, n); }, nb::arg("v"), nb::arg("n"),
		"v bounced off a surface of unit normal n");
	if constexpr (N == 2 || N == 3) {
		m.def("angle_between", [](Vec const &a, Vec const &b) { return rove::angle_between(a, b); }, nb::arg("a"), nb::arg("b"),
			"The angle between two directions, 0 to pi, exact for tiny ones; ValueError for a vector of no length");
	}
	if constexpr (N == 2) {
		m.def("signed_angle", [](Vec const &a, Vec const &b) { return rove::signed_angle(a, b); }, nb::arg("a"), nb::arg("b"),
			"The turn (-pi to pi) from a's direction to b's, positive from x towards y");
	}
	if constexpr (N == 3) {
		m.def("cross", [](Vec const &a, Vec const &b) { return Vec(rove::cross(a, b)); }, nb::arg("a"), nb::arg("b"),
			"The cross product");
		m.def("signed_angle", [](Vec const &a, Vec const &b, Vec const &axis) { return rove::signed_angle(a, b, axis); },
			nb::arg("a"), nb::arg("b"), nb::arg("axis"),
			"The turn about axis (right-handed, -pi to pi) taking a's direction to b's, seen across the axis; "
			"ValueError for an axis of no length or a or b along it");
	}
}

void bind_vec_functions(nb::module_ &m) {
	bind_vec_operations<2, float>(m);
	bind_vec_operations<3, float>(m);
	bind_vec_operations<4, float>(m);
	bind_vec_operations<2, double>(m);
	bind_vec_operations<3, double>(m);
	bind_vec_operations<4, double>(m);
}

// Explicit template instantiations
template void bind_vec2<float>(nb::module_ &m, char const *name);
template void bind_vec2<double>(nb::module_ &m, char const *name);
template void bind_vec3<float>(nb::module_ &m, char const *name);
template void bind_vec3<double>(nb::module_ &m, char const *name);
template void bind_vec4<float>(nb::module_ &m, char const *name);
template void bind_vec4<double>(nb::module_ &m, char const *name);
