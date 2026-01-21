#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include "vec.h"
#include "matrix.h"

namespace nb = nanobind;

template<typename T>
void bind_vec2(nb::module_ &m, const char *name) {
    using Vec = vek::vec<2, T>;

    nb::class_<Vec>(m, name)
        .def(nb::init<>())
        .def(nb::init<T, T>(), nb::arg("x"), nb::arg("y"))
        .def_rw("x", &Vec::x)
        .def_rw("y", &Vec::y)
        .def("set", &Vec::set, nb::arg("x"), nb::arg("y"))
        .def("set_all", &Vec::set_all, nb::arg("a"))
        .def("length", &Vec::length)
        .def("length_sq", &Vec::length_sq)
        .def("length_manhattan", &Vec::length_manhattan)
        .def("normalize", &Vec::normalize)
        .def("negate", &Vec::negate)
        .def("perpendicular", &Vec::perpendicular)
        .def("is_collinear", &Vec::is_collinear, nb::arg("v"), nb::arg("epsilon") = vek::EPSILON)
        .def("__neg__", [](const Vec &v) { return -v; })
        .def("__add__", [](const Vec &a, const Vec &b) { return a + b; })
        .def("__sub__", [](const Vec &a, const Vec &b) { return a - b; })
        .def("__mul__", [](const Vec &a, const Vec &b) { return a * b; })
        .def("__mul__", [](const Vec &a, T k) { return a * k; })
        .def("__rmul__", [](const Vec &a, T k) { return a * k; })
        .def("__truediv__", [](const Vec &a, const Vec &b) { return a / b; })
        .def("__truediv__", [](const Vec &a, T k) { return a / k; })
        .def("__eq__", [](const Vec &a, const Vec &b) { return a == b; })
        .def("__ne__", [](const Vec &a, const Vec &b) { return a != b; })
        .def("__repr__", [](const Vec &v) {
            std::ostringstream ss;
            ss << "vec2(" << v.x << ", " << v.y << ")";
            return ss.str();
        })
        .def("dot", [](const Vec &a, const Vec &b) { return vek::dot_product(a, b); });
}

template<typename T>
void bind_vec3(nb::module_ &m, const char *name) {
    using Vec = vek::vec<3, T>;

    nb::class_<Vec>(m, name)
        .def(nb::init<>())
        .def(nb::init<T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"))
        .def_rw("x", &Vec::x)
        .def_rw("y", &Vec::y)
        .def_rw("z", &Vec::z)
        .def("set", &Vec::set, nb::arg("x"), nb::arg("y"), nb::arg("z"))
        .def("set_all", &Vec::set_all, nb::arg("a"))
        .def("length", &Vec::length)
        .def("length_sq", &Vec::length_sq)
        .def("normalize", &Vec::normalize)
        .def("negate", &Vec::negate)
        .def("is_collinear", &Vec::is_collinear, nb::arg("v"), nb::arg("epsilon") = vek::EPSILON)
        .def("__neg__", [](const Vec &v) { return -v; })
        .def("__add__", [](const Vec &a, const Vec &b) { return a + b; })
        .def("__sub__", [](const Vec &a, const Vec &b) { return a - b; })
        .def("__mul__", [](const Vec &a, const Vec &b) { return a * b; })
        .def("__mul__", [](const Vec &a, T k) { return a * k; })
        .def("__rmul__", [](const Vec &a, T k) { return a * k; })
        .def("__truediv__", [](const Vec &a, const Vec &b) { return a / b; })
        .def("__truediv__", [](const Vec &a, T k) { return a / k; })
        .def("__eq__", [](const Vec &a, const Vec &b) { return a == b; })
        .def("__ne__", [](const Vec &a, const Vec &b) { return a != b; })
        .def("__repr__", [](const Vec &v) {
            std::ostringstream ss;
            ss << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
            return ss.str();
        })
        .def("dot", [](const Vec &a, const Vec &b) { return vek::dot_product(a, b); })
        .def("cross", [](const Vec &a, const Vec &b) {
            Vec result;
            vek::cross_product(result, a, b);
            return result;
        });
}

template<typename T>
void bind_vec4(nb::module_ &m, const char *name) {
    using Vec = vek::vec<4, T>;

    nb::class_<Vec>(m, name)
        .def(nb::init<>())
        .def(nb::init<T, T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("w"))
        .def_rw("x", &Vec::x)
        .def_rw("y", &Vec::y)
        .def_rw("z", &Vec::z)
        .def_rw("w", &Vec::w)
        .def("set", &Vec::set, nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("w"))
        .def("set_all", &Vec::set_all, nb::arg("a"))
        .def("length", &Vec::length)
        .def("length_sq", &Vec::length_sq)
        .def("normalize", &Vec::normalize)
        .def("negate", &Vec::negate)
        .def("__neg__", [](const Vec &v) { return Vec(-v.x, -v.y, -v.z, -v.w); })
        .def("__add__", [](const Vec &a, const Vec &b) { return a + b; })
        .def("__sub__", [](const Vec &a, const Vec &b) { return a - b; })
        .def("__mul__", [](const Vec &a, T k) { return a * k; })
        .def("__rmul__", [](const Vec &a, T k) { return a * k; })
        .def("__truediv__", [](const Vec &a, T k) { return a / k; })
        .def("__eq__", [](const Vec &a, const Vec &b) { return a == b; })
        .def("__ne__", [](const Vec &a, const Vec &b) { return a != b; })
        .def("__repr__", [](const Vec &v) {
            std::ostringstream ss;
            ss << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
            return ss.str();
        })
        .def("dot", [](const Vec &a, const Vec &b) { return vek::dot_product(a, b); });
}

template<typename T>
void bind_matrix3(nb::module_ &m, const char *name) {
    using Mat = vek::matrix<3, 3, T>;
    using Vec3 = vek::vec<3, T>;
    using Vec2 = vek::vec<2, T>;

    nb::class_<Mat>(m, name)
        .def(nb::init<>())
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
        .def("rotate", nb::overload_cast<const Vec3&, T>(&Mat::rotate),
             nb::arg("axis"), nb::arg("angle"), "Apply rotation")
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
            vek::mul(result, a, b);
            return result;
        })
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
    using Mat = vek::matrix<4, 4, T>;
    using Vec3 = vek::vec<3, T>;
    using Vec4 = vek::vec<4, T>;

    nb::class_<Mat>(m, name)
        .def(nb::init<>())
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
        .def("rotate", nb::overload_cast<const Vec3&, T>(&Mat::rotate),
             nb::arg("axis"), nb::arg("angle"), "Apply rotation")
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
            vek::mul(result, a, b);
            return result;
        })
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

NB_MODULE(pyvek, m) {
    m.doc() = "Python bindings for vek vector math library";

    // Bind float versions (default)
    bind_vec2<float>(m, "vec2");
    bind_vec3<float>(m, "vec3");
    bind_vec4<float>(m, "vec4");

    // Bind double versions
    bind_vec2<double>(m, "dvec2");
    bind_vec3<double>(m, "dvec3");
    bind_vec4<double>(m, "dvec4");

    // Bind matrix float versions (default)
    bind_matrix3<float>(m, "mat3");
    bind_matrix4<float>(m, "mat4");

    // Bind matrix double versions
    bind_matrix3<double>(m, "dmat3");
    bind_matrix4<double>(m, "dmat4");

    // Free functions
    m.def("dot", [](const vek::vec<2, float> &a, const vek::vec<2, float> &b) {
        return vek::dot_product(a, b);
    }, nb::arg("a"), nb::arg("b"), "Compute dot product of two vec2");

    m.def("dot", [](const vek::vec<3, float> &a, const vek::vec<3, float> &b) {
        return vek::dot_product(a, b);
    }, nb::arg("a"), nb::arg("b"), "Compute dot product of two vec3");

    m.def("dot", [](const vek::vec<4, float> &a, const vek::vec<4, float> &b) {
        return vek::dot_product(a, b);
    }, nb::arg("a"), nb::arg("b"), "Compute dot product of two vec4");

    m.def("cross", [](const vek::vec<3, float> &a, const vek::vec<3, float> &b) {
        vek::vec<3, float> result;
        vek::cross_product(result, a, b);
        return result;
    }, nb::arg("a"), nb::arg("b"), "Compute cross product of two vec3");

    m.def("normalize", [](const vek::vec<2, float> &v) {
        return vek::normalize(v);
    }, nb::arg("v"), "Return normalized vec2");

    m.def("normalize", [](const vek::vec<3, float> &v) {
        return vek::normalize(v);
    }, nb::arg("v"), "Return normalized vec3");

    m.def("normalize", [](const vek::vec<4, float> &v) {
        return vek::normalize(v);
    }, nb::arg("v"), "Return normalized vec4");
}
