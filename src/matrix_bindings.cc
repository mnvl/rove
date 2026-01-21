/**
 * @file matrix_bindings.cc
 * @brief Python bindings for matrix classes (mat3, mat4)
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <sstream>
#include <stdexcept>

#include "vec.h"
#include "matrix.h"
#include "quaternion.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_matrix3(nb::module_ &m, const char *name) {
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
        .def("to_numpy", [](const Mat &m) {
            T* data = new T[9];
            for (int row = 0; row < 3; row++) {
                for (int col = 0; col < 3; col++) {
                    data[row * 3 + col] = m.ij[col][row];
                }
            }
            size_t shape[2] = {3, 3};
            nb::capsule deleter(data, [](void *p) noexcept {
                delete[] (T*)p;
            });
            return nb::ndarray<nb::numpy, T, nb::shape<3, 3>>(data, 2, shape, deleter);
        }, "Convert mat3 to numpy array")
        .def("identity", &Mat::identity, "Set to identity matrix")
        .def("zero", &Mat::zero, "Set to zero matrix")
        .def("translation", nb::overload_cast<T, T>(&Mat::translation),
             nb::arg("x"), nb::arg("y"), "Set to translation matrix")
        .def("translation", nb::overload_cast<const Vec2&>(&Mat::translation),
             nb::arg("p"), "Set to translation matrix")
        .def("translate", nb::overload_cast<T, T>(&Mat::translate),
             nb::arg("x"), nb::arg("y"), "Apply translation")
        .def("translate", nb::overload_cast<const Vec2&>(&Mat::translate),
             nb::arg("p"), "Apply translation")
        .def("scaling", nb::overload_cast<T, T>(&Mat::scaling),
             nb::arg("x"), nb::arg("y"), "Set to scaling matrix")
        .def("scaling", nb::overload_cast<const Vec2&>(&Mat::scaling),
             nb::arg("p"), "Set to scaling matrix")
        .def("scale", nb::overload_cast<T, T>(&Mat::scale),
             nb::arg("x"), nb::arg("y"), "Apply scaling")
        .def("scale", nb::overload_cast<const Vec2&>(&Mat::scale),
             nb::arg("p"), "Apply scaling")
        .def("rotation", nb::overload_cast<const Vec3&, T>(&Mat::rotation),
             nb::arg("axis"), nb::arg("angle"), "Set to rotation matrix")
        .def("rotation", nb::overload_cast<const Quat&>(&Mat::rotation),
             nb::arg("q"), "Set to rotation matrix from quaternion")
        .def("rotate", nb::overload_cast<const Vec3&, T>(&Mat::rotate),
             nb::arg("axis"), nb::arg("angle"), "Apply rotation")
        .def("rotate", nb::overload_cast<const Quat&>(&Mat::rotate),
             nb::arg("q"), "Apply rotation from quaternion")
        .def("determinant", &Mat::determinant, "Compute determinant")
        .def("trace", &Mat::trace, "Compute trace")
        .def("invert", &Mat::invert, "Invert matrix in place")
        .def("inverse", [](const Mat &m) {
            Mat result;
            if (!m.inverse(result)) {
                throw std::runtime_error("Matrix is singular");
            }
            return result;
        }, "Return inverse matrix")
        .def("transpose", [](const Mat &m) {
            Mat result;
            m.transpose(result);
            return result;
        }, "Return transposed matrix")
        .def("__imul__", nb::overload_cast<T>(&Mat::operator*=), nb::arg("k"))
        .def("__imul__", nb::overload_cast<const Mat&>(&Mat::operator*=), nb::arg("m"))
        .def("__itruediv__", &Mat::operator/=, nb::arg("k"))
        .def("__mul__", [](const Mat &m, T k) {
            Mat result = m;
            result *= k;
            return result;
        })
        .def("__mul__", [](const Mat &a, const Mat &b) {
            Mat result;
            rove::mul(result, a, b);
            return result;
        })
        .def("__mul__", [](const Mat &m, const Vec2 &v) {
            return v * m;
        }, "Transform vec2 by mat3, returning vec3")
        .def("__mul__", [](const Mat &m, const Vec3 &v) {
            return v * m;
        }, "Transform vec3 by mat3")
        .def("__rmul__", [](const Mat &m, T k) {
            Mat result = m;
            result *= k;
            return result;
        })
        .def("__truediv__", [](const Mat &m, T k) {
            Mat result = m;
            result /= k;
            return result;
        })
        .def("__repr__", [](const Mat &m) {
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
        .def("get", [](const Mat &m, int i, int j) {
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
        }, nb::arg("i"), nb::arg("j"), nb::arg("value"), "Set matrix element at [i][j]");
}

template<typename T>
void bind_matrix4(nb::module_ &m, const char *name) {
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
        .def("to_numpy", [](const Mat &m) {
            T* data = new T[16];
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    data[row * 4 + col] = m.ij[col][row];
                }
            }
            size_t shape[2] = {4, 4};
            nb::capsule deleter(data, [](void *p) noexcept {
                delete[] (T*)p;
            });
            return nb::ndarray<nb::numpy, T, nb::shape<4, 4>>(data, 2, shape, deleter);
        }, "Convert mat4 to numpy array")
        .def("identity", &Mat::identity, "Set to identity matrix")
        .def("zero", &Mat::zero, "Set to zero matrix")
        .def("translation", nb::overload_cast<T, T, T>(&Mat::translation),
             nb::arg("x"), nb::arg("y"), nb::arg("z"), "Set to translation matrix")
        .def("translation", nb::overload_cast<const Vec3&>(&Mat::translation),
             nb::arg("p"), "Set to translation matrix")
        .def("translate", nb::overload_cast<T, T, T>(&Mat::translate),
             nb::arg("x"), nb::arg("y"), nb::arg("z"), "Apply translation")
        .def("translate", nb::overload_cast<const Vec3&>(&Mat::translate),
             nb::arg("p"), "Apply translation")
        .def("scaling", nb::overload_cast<T, T, T>(&Mat::scaling),
             nb::arg("x"), nb::arg("y"), nb::arg("z"), "Set to scaling matrix")
        .def("scaling", nb::overload_cast<const Vec3&>(&Mat::scaling),
             nb::arg("p"), "Set to scaling matrix")
        .def("scale", nb::overload_cast<T, T, T>(&Mat::scale),
             nb::arg("x"), nb::arg("y"), nb::arg("z"), "Apply scaling")
        .def("scale", nb::overload_cast<const Vec3&>(&Mat::scale),
             nb::arg("p"), "Apply scaling")
        .def("rotation", nb::overload_cast<const Vec3&, T>(&Mat::rotation),
             nb::arg("axis"), nb::arg("angle"), "Set to rotation matrix")
        .def("rotation", nb::overload_cast<const Quat&>(&Mat::rotation),
             nb::arg("q"), "Set to rotation matrix from quaternion")
        .def("rotate", nb::overload_cast<const Vec3&, T>(&Mat::rotate),
             nb::arg("axis"), nb::arg("angle"), "Apply rotation")
        .def("rotate", nb::overload_cast<const Quat&>(&Mat::rotate),
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
        .def("inverse", [](const Mat &m) {
            Mat result;
            if (!m.inverse(result)) {
                throw std::runtime_error("Matrix is singular");
            }
            return result;
        }, "Return inverse matrix")
        .def("transpose", [](const Mat &m) {
            Mat result;
            m.transpose(result);
            return result;
        }, "Return transposed matrix")
        .def("transpose_inplace", &Mat::transpose_inplace, "Transpose matrix in place")
        .def("column", &Mat::column, nb::arg("index"), "Get column as vec4")
        .def("projected_column", &Mat::projected_column, nb::arg("index"), "Get column as vec3")
        .def("projected_row", &Mat::projected_row, nb::arg("index"), "Get row as vec3")
        .def("__imul__", nb::overload_cast<T>(&Mat::operator*=), nb::arg("k"))
        .def("__imul__", nb::overload_cast<const Mat&>(&Mat::operator*=), nb::arg("m"))
        .def("__itruediv__", &Mat::operator/=, nb::arg("k"))
        .def("__mul__", [](const Mat &m, T k) {
            Mat result = m;
            result *= k;
            return result;
        })
        .def("__mul__", [](const Mat &a, const Mat &b) {
            Mat result;
            rove::mul(result, a, b);
            return result;
        })
        .def("__mul__", [](const Mat &m, const Vec3 &v) {
            return v * m;
        }, "Transform vec3 by mat4, returning vec3")
        .def("__mul__", [](const Mat &m, const Vec4 &v) {
            return v * m;
        }, "Transform vec4 by mat4")
        .def("__rmul__", [](const Mat &m, T k) {
            Mat result = m;
            result *= k;
            return result;
        })
        .def("__truediv__", [](const Mat &m, T k) {
            Mat result = m;
            result /= k;
            return result;
        })
        .def("__repr__", [](const Mat &m) {
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
        .def("get", [](const Mat &m, int i, int j) {
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
        }, nb::arg("i"), nb::arg("j"), nb::arg("value"), "Set matrix element at [i][j]");
}

// Explicit template instantiations
template void bind_matrix3<float>(nb::module_ &m, const char *name);
template void bind_matrix3<double>(nb::module_ &m, const char *name);
template void bind_matrix4<float>(nb::module_ &m, const char *name);
template void bind_matrix4<double>(nb::module_ &m, const char *name);
