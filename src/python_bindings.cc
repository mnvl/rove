#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include "vec.h"

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
