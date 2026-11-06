// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_matrix.cc
 * @brief Python bindings for matrix classes (mat3, mat4)
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <sstream>
#include <stdexcept>

#include <rove/vec.h>
#include <rove/matrix.h>
#include <rove/quaternion.h>

namespace nb = nanobind;
using namespace nb::literals;

template<class T>
void bind_matrix3(nb::module_ &m, char const *name) {
	using Mat = rove::matrix<3, 3, T>;
	using Vec3 = rove::vec<3, T>;
	using Vec2 = rove::vec<2, T>;
	using Quat = rove::quaternion<T>;

	nb::class_<Mat>(m, name)
		.def(nb::init<>())
		.def_static("from_numpy", [](nb::ndarray<T, nb::shape<3, 3>> arr) {
			Mat result;
			for (int row = 0; row < 3; row++) {
				for (int col = 0; col < 3; col++) {
					result.ij[col][row] = arr(row, col);
				}
			}
			return result;
		}, nb::arg("array"), "Create mat3 from numpy array")
		.def("to_numpy", [](Mat const &m) {
			T* data = new T[9];
			for (int row = 0; row < 3; row++) {
				for (int col = 0; col < 3; col++) {
					data[row * 3 + col] = m.ij[col][row];
				}
			}
			size_t shape[2] = {3, 3};
			nb::capsule deleter(data, [](void *p) noexcept {
				delete[] static_cast<T*>(p);
			});
			return nb::ndarray<nb::numpy, T, nb::shape<3, 3>>(data, 2, shape, deleter);
		}, "Convert mat3 to numpy array")
		.def("identity", &Mat::identity, "Set to identity matrix")
		.def("zero", &Mat::zero, "Set to zero matrix")
		.def("translation", nb::overload_cast<T, T>(&Mat::translation),
			 nb::arg("x"), nb::arg("y"), "Set to translation matrix")
		.def("translation", nb::overload_cast<Vec2 const &>(&Mat::translation),
			 nb::arg("p"), "Set to translation matrix")
		.def("translate", nb::overload_cast<T, T>(&Mat::translate),
			 nb::arg("x"), nb::arg("y"), "Apply translation")
		.def("translate", nb::overload_cast<Vec2 const &>(&Mat::translate),
			 nb::arg("p"), "Apply translation")
		.def("scaling", nb::overload_cast<T, T>(&Mat::scaling),
			 nb::arg("x"), nb::arg("y"), "Set to scaling matrix")
		.def("scaling", nb::overload_cast<Vec2 const &>(&Mat::scaling),
			 nb::arg("p"), "Set to scaling matrix")
		.def("scale", nb::overload_cast<T, T>(&Mat::scale),
			 nb::arg("x"), nb::arg("y"), "Apply scaling")
		.def("scale", nb::overload_cast<Vec2 const &>(&Mat::scale),
			 nb::arg("p"), "Apply scaling")
		.def("rotation", nb::overload_cast<Vec3 const &, T>(&Mat::rotation),
			 nb::arg("axis"), nb::arg("angle"), "Set to rotation matrix")
		.def("rotation", nb::overload_cast<Quat const &>(&Mat::rotation),
			 nb::arg("q"), "Set to rotation matrix from quaternion")
		.def("rotate", nb::overload_cast<Vec3 const &, T>(&Mat::rotate),
			 nb::arg("axis"), nb::arg("angle"), "Apply rotation")
		.def("rotate", nb::overload_cast<Quat const &>(&Mat::rotate),
			 nb::arg("q"), "Apply rotation from quaternion")
		.def("determinant", &Mat::determinant, "Compute determinant")
		.def("trace", &Mat::trace, "Compute trace")
		.def("invert", &Mat::invert, "Invert matrix in place")
		.def("inverse", [](Mat const &m) {
			Mat result;
			if (!m.inverse(result)) {
				throw std::runtime_error("Matrix is singular");
			}
			return result;
		}, "Return inverse matrix")
		.def("transpose", [](Mat const &m) {
			Mat result;
			m.transpose(result);
			return result;
		}, "Return transposed matrix")
		.def("__imul__", nb::overload_cast<T>(&Mat::operator*=), nb::arg("k"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<Mat const &>(&Mat::operator*=), nb::arg("m"), nb::rv_policy::none)
		.def("__itruediv__", &Mat::operator/=, nb::arg("k"), nb::rv_policy::none)
		.def("__mul__", [](Mat const &m, T k) {
			Mat result = m;
			result *= k;
			return result;
		})
		.def("__mul__", [](Mat const &a, Mat const &b) {
			return Mat(a * b);
		})
		.def("__mul__", [](Mat const &m, Vec2 const &v) {
			return rove::project_point(m, v);
		}, "Transform vec2 by mat3 as a homogeneous point, divided by z, returning vec2")
		.def("__mul__", [](Mat const &m, Vec3 const &v) {
			return m * v;
		}, "Transform vec3 by mat3")
		.def("__rmul__", [](Mat const &m, T k) {
			Mat result = m;
			result *= k;
			return result;
		})
		.def("__truediv__", [](Mat const &m, T k) {
			Mat result = m;
			result /= k;
			return result;
		})
		.def("__repr__", [](Mat const &m) {
			std::ostringstream ss;
			ss << "mat3([";
			for (int i = 0; i < 3; i++) {
				ss << "[";
				for (int j = 0; j < 3; j++) {
					ss << m.ij[j][i];
					if (j < 2) ss << ", ";
				}
				ss << "]";
				if (i < 2) ss << ", ";
			}
			ss << "])";
			return ss.str();
		})
		.def("get", [](Mat const &m, int i, int j) {
			if (i < 0 || i >= 3 || j < 0 || j >= 3) {
				throw std::out_of_range("Matrix index out of range");
			}
			return m.ij[i][j];
		}, nb::arg("i"), nb::arg("j"), "Get matrix element at [i][j]")
		.def("set", [](Mat &m, int i, int j, T value) {
			if (i < 0 || i >= 3 || j < 0 || j >= 3) {
				throw std::out_of_range("Matrix index out of range");
			}
			m.ij[i][j] = value;
		}, nb::arg("i"), nb::arg("j"), nb::arg("value"), "Set matrix element at [i][j]")
		.def("__getstate__", [](Mat const &m) {
			return nb::make_tuple(
				m.ij[0][0], m.ij[0][1], m.ij[0][2],
				m.ij[1][0], m.ij[1][1], m.ij[1][2],
				m.ij[2][0], m.ij[2][1], m.ij[2][2]);
		})
		.def("__setstate__", [](Mat &m, nb::tuple t) {
			m.ij[0][0]=nb::cast<T>(t[0]); m.ij[0][1]=nb::cast<T>(t[1]); m.ij[0][2]=nb::cast<T>(t[2]);
			m.ij[1][0]=nb::cast<T>(t[3]); m.ij[1][1]=nb::cast<T>(t[4]); m.ij[1][2]=nb::cast<T>(t[5]);
			m.ij[2][0]=nb::cast<T>(t[6]); m.ij[2][1]=nb::cast<T>(t[7]); m.ij[2][2]=nb::cast<T>(t[8]);
		});
}

template<class T>
void bind_matrix4(nb::module_ &m, char const *name) {
	using Mat = rove::matrix<4, 4, T>;
	using Vec3 = rove::vec<3, T>;
	using Vec4 = rove::vec<4, T>;
	using Quat = rove::quaternion<T>;

	nb::class_<Mat>(m, name)
		.def(nb::init<>())
		.def_static("from_numpy", [](nb::ndarray<T, nb::shape<4, 4>> arr) {
			Mat result;
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					result.ij[col][row] = arr(row, col);
				}
			}
			return result;
		}, nb::arg("array"), "Create mat4 from numpy array")
		.def("to_numpy", [](Mat const &m) {
			T* data = new T[16];
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					data[row * 4 + col] = m.ij[col][row];
				}
			}
			size_t shape[2] = {4, 4};
			nb::capsule deleter(data, [](void *p) noexcept {
				delete[] static_cast<T*>(p);
			});
			return nb::ndarray<nb::numpy, T, nb::shape<4, 4>>(data, 2, shape, deleter);
		}, "Convert mat4 to numpy array")
		.def("identity", &Mat::identity, "Set to identity matrix")
		.def("zero", &Mat::zero, "Set to zero matrix")
		.def("translation", nb::overload_cast<T, T, T>(&Mat::translation),
			 nb::arg("x"), nb::arg("y"), nb::arg("z"), "Set to translation matrix")
		.def("translation", nb::overload_cast<Vec3 const &>(&Mat::translation),
			 nb::arg("p"), "Set to translation matrix")
		.def("translate", nb::overload_cast<T, T, T>(&Mat::translate),
			 nb::arg("x"), nb::arg("y"), nb::arg("z"), "Apply translation")
		.def("translate", nb::overload_cast<Vec3 const &>(&Mat::translate),
			 nb::arg("p"), "Apply translation")
		.def("scaling", nb::overload_cast<T, T, T>(&Mat::scaling),
			 nb::arg("x"), nb::arg("y"), nb::arg("z"), "Set to scaling matrix")
		.def("scaling", nb::overload_cast<Vec3 const &>(&Mat::scaling),
			 nb::arg("p"), "Set to scaling matrix")
		.def("scale", nb::overload_cast<T, T, T>(&Mat::scale),
			 nb::arg("x"), nb::arg("y"), nb::arg("z"), "Apply scaling")
		.def("scale", nb::overload_cast<Vec3 const &>(&Mat::scale),
			 nb::arg("p"), "Apply scaling")
		.def("rotation", nb::overload_cast<Vec3 const &, T>(&Mat::rotation),
			 nb::arg("axis"), nb::arg("angle"), "Set to rotation matrix")
		.def("rotation", nb::overload_cast<Quat const &>(&Mat::rotation),
			 nb::arg("q"), "Set to rotation matrix from quaternion")
		.def("rotate", nb::overload_cast<Vec3 const &, T>(&Mat::rotate),
			 nb::arg("axis"), nb::arg("angle"), "Apply rotation")
		.def("rotate", nb::overload_cast<Quat const &>(&Mat::rotate),
			 nb::arg("q"), "Apply rotation from quaternion")
		.def("lookat", &Mat::lookat,
			 nb::arg("eye"), nb::arg("at"), nb::arg("up"), "Set to lookat matrix")
		.def("ortho", &Mat::ortho,
			 nb::arg("width"), nb::arg("height"), nb::arg("z_near"), nb::arg("z_far"),
			 "Set to orthographic projection matrix")
		.def("perspective", &Mat::perspective,
			 nb::arg("fovy"), nb::arg("aspect"), nb::arg("z_near"), nb::arg("z_far"),
			 "Set to perspective projection matrix")
		.def("determinant", &Mat::determinant, "Compute determinant")
		.def("invert", &Mat::invert, "Invert matrix in place")
		.def("inverse", [](Mat const &m) {
			Mat result;
			if (!m.inverse(result)) {
				throw std::runtime_error("Matrix is singular");
			}
			return result;
		}, "Return inverse matrix")
		.def("transpose", [](Mat const &m) {
			Mat result;
			m.transpose(result);
			return result;
		}, "Return transposed matrix")
		.def("transpose_inplace", &Mat::transpose_inplace, "Transpose matrix in place")
		.def("column", &Mat::column, nb::arg("index"), "Get column as vec4")
		.def("projected_column", &Mat::projected_column, nb::arg("index"), "Get column as vec3")
		.def("projected_row", &Mat::projected_row, nb::arg("index"), "Get row as vec3")
		.def("__imul__", nb::overload_cast<T>(&Mat::operator*=), nb::arg("k"), nb::rv_policy::none)
		.def("__imul__", nb::overload_cast<Mat const &>(&Mat::operator*=), nb::arg("m"), nb::rv_policy::none)
		.def("__itruediv__", &Mat::operator/=, nb::arg("k"), nb::rv_policy::none)
		.def("__mul__", [](Mat const &m, T k) {
			Mat result = m;
			result *= k;
			return result;
		})
		.def("__mul__", [](Mat const &a, Mat const &b) {
			return Mat(a * b);
		})
		.def("__mul__", [](Mat const &m, Vec3 const &v) {
			return m * v;
		}, "Transform vec3 by mat4, returning vec3")
		.def("__mul__", [](Mat const &m, Vec4 const &v) {
			return m * v;
		}, "Transform vec4 by mat4")
		.def("__rmul__", [](Mat const &m, T k) {
			Mat result = m;
			result *= k;
			return result;
		})
		.def("__truediv__", [](Mat const &m, T k) {
			Mat result = m;
			result /= k;
			return result;
		})
		.def("__repr__", [](Mat const &m) {
			std::ostringstream ss;
			ss << "mat4([";
			for (int i = 0; i < 4; i++) {
				ss << "[";
				for (int j = 0; j < 4; j++) {
					ss << m.ij[j][i];
					if (j < 3) ss << ", ";
				}
				ss << "]";
				if (i < 3) ss << ", ";
			}
			ss << "])";
			return ss.str();
		})
		.def("get", [](Mat const &m, int i, int j) {
			if (i < 0 || i >= 4 || j < 0 || j >= 4) {
				throw std::out_of_range("Matrix index out of range");
			}
			return m.ij[i][j];
		}, nb::arg("i"), nb::arg("j"), "Get matrix element at [i][j]")
		.def("set", [](Mat &m, int i, int j, T value) {
			if (i < 0 || i >= 4 || j < 0 || j >= 4) {
				throw std::out_of_range("Matrix index out of range");
			}
			m.ij[i][j] = value;
		}, nb::arg("i"), nb::arg("j"), nb::arg("value"), "Set matrix element at [i][j]")
		.def("__getstate__", [](Mat const &m) {
			return nb::make_tuple(
				m.ij[0][0], m.ij[0][1], m.ij[0][2], m.ij[0][3],
				m.ij[1][0], m.ij[1][1], m.ij[1][2], m.ij[1][3],
				m.ij[2][0], m.ij[2][1], m.ij[2][2], m.ij[2][3],
				m.ij[3][0], m.ij[3][1], m.ij[3][2], m.ij[3][3]);
		})
		.def("__setstate__", [](Mat &m, nb::tuple t) {
			m.ij[0][0]=nb::cast<T>(t[0]);  m.ij[0][1]=nb::cast<T>(t[1]);  m.ij[0][2]=nb::cast<T>(t[2]);  m.ij[0][3]=nb::cast<T>(t[3]);
			m.ij[1][0]=nb::cast<T>(t[4]);  m.ij[1][1]=nb::cast<T>(t[5]);  m.ij[1][2]=nb::cast<T>(t[6]);  m.ij[1][3]=nb::cast<T>(t[7]);
			m.ij[2][0]=nb::cast<T>(t[8]);  m.ij[2][1]=nb::cast<T>(t[9]);  m.ij[2][2]=nb::cast<T>(t[10]); m.ij[2][3]=nb::cast<T>(t[11]);
			m.ij[3][0]=nb::cast<T>(t[12]); m.ij[3][1]=nb::cast<T>(t[13]); m.ij[3][2]=nb::cast<T>(t[14]); m.ij[3][3]=nb::cast<T>(t[15]);
		});
}

// The matrices by value and points through them by name (matrix.h), for matrices of one type
template<class T>
static void bind_matrix_operations(nb::module_ &m) {
	using Vec2 = rove::vec<2, T>;
	using Vec3 = rove::vec<3, T>;
	using Mat3 = rove::matrix<3, 3, T>;
	using Mat4 = rove::matrix<4, 4, T>;
	m.def("transform_point", [](Mat4 const &mat, Vec3 const &p) { return rove::transform_point(mat, p); }, nb::arg("m"), nb::arg("p"),
		"m p with w = 1, not divided (an affine transform's point)");
	m.def("transform_point", [](Mat3 const &mat, Vec2 const &p) { return rove::transform_point(mat, p); }, nb::arg("m"), nb::arg("p"),
		"(x, y, 1) through m, not divided");
	m.def("transform_vector", [](Mat4 const &mat, Vec3 const &v) { return rove::transform_vector(mat, v); }, nb::arg("m"), nb::arg("v"),
		"m v with w = 0: turned and scaled, not moved");
	m.def("project_point", [](Mat4 const &mat, Vec3 const &p) { return rove::project_point(mat, p); }, nb::arg("m"), nb::arg("p"),
		"m p with w = 1, divided by the w that comes out; ValueError for w of zero");
	m.def("project_point", [](Mat3 const &mat, Vec2 const &p) { return rove::project_point(mat, p); }, nb::arg("m"), nb::arg("p"),
		"(x, y, 1) through m, divided by the third component; ValueError for zero there");
	m.def("inverse", [](Mat4 const &mat) { return rove::inverse(mat); }, nb::arg("m"), "The inverse; ValueError for a singular matrix");
	m.def("inverse", [](Mat3 const &mat) { return rove::inverse(mat); }, nb::arg("m"), "The inverse; ValueError for a singular matrix");
	m.def("transpose", [](Mat4 const &mat) { return rove::transpose(mat); }, nb::arg("m"));
	m.def("transpose", [](Mat3 const &mat) { return rove::transpose(mat); }, nb::arg("m"));
	m.def("translation_matrix", [](Vec3 const &offset) { return rove::translation_matrix(offset); }, nb::arg("offset"));
	m.def("scaling_matrix", [](Vec3 const &factors) { return rove::scaling_matrix(factors); }, nb::arg("factors"));
	m.def("rotation_matrix", [](Vec3 const &axis, T angle) { return rove::rotation_matrix(axis, angle); }, nb::arg("axis"),
		nb::arg("angle"), "A right-handed turn of angle radians about axis (any length but none: ValueError)");
	m.def("look_at_matrix", [](Vec3 const &eye, Vec3 const &at, Vec3 const &up) { return rove::look_at_matrix(eye, at, up); },
		nb::arg("eye"), nb::arg("at"), nb::arg("up"), "mat4.lookat, returned");
	m.def("linear_part", [](Mat4 const &mat) { return rove::linear_part(mat); }, nb::arg("m"), "The upper-left 3x3");
	m.def("normal_matrix", [](Mat4 const &mat) { return rove::normal_matrix(mat); }, nb::arg("m"),
		"The inverse transpose of the linear part: what turns normals as m turns the surface; ValueError for a singular one");
}

void bind_matrix_functions(nb::module_ &m) {
	bind_matrix_operations<float>(m);
	bind_matrix_operations<double>(m);
	// of numbers alone: a mat4, pyrove's default (Python's floats would take the first overload of two in any case)
	m.def("perspective_matrix", &rove::perspective_matrix<float>, nb::arg("fovy"), nb::arg("aspect"), nb::arg("z_near"),
		nb::arg("z_far"), "mat4.perspective, returned: depth 0 at z_near to 1 at z_far");
	m.def("ortho_matrix", &rove::ortho_matrix<float>, nb::arg("left"), nb::arg("right"), nb::arg("bottom"), nb::arg("top"),
		nb::arg("z_near"), nb::arg("z_far"),
		"An orthographic projection off centre: [left, right] and [bottom, top] to -1..1, depth 0 at z_near to 1 at z_far; "
		"ValueError for a box of no width, height or depth");
}

// Explicit template instantiations
template void bind_matrix3<float>(nb::module_ &m, char const *name);
template void bind_matrix3<double>(nb::module_ &m, char const *name);
template void bind_matrix4<float>(nb::module_ &m, char const *name);
template void bind_matrix4<double>(nb::module_ &m, char const *name);
