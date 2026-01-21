/**
 * @file vec_bindings.cc
 * @brief Python bindings for vector classes (vec2, vec3, vec4)
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <sstream>

#include "vec.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_vec2(nb::module_ &m, const char *name) {
    using Vec = rove::vec<2, T>;

    nb::class_<Vec>(m, name)
        .def(nb::init<>())
        .def(nb::init<T, T>(), nb::arg("x"), nb::arg("y"))
        .def_rw("x", &Vec::x)
        .def_rw("y", &Vec::y)
        .def_static("from_numpy", [](nb::ndarray<T, nb::shape<2>> arr) {
            return Vec(arr(0), arr(1));
        }, nb::arg("array"), "Create vec2 from numpy array")
        .def("to_numpy", [](const Vec &v) {
            T* data = new T[2]{v.x, v.y};
            size_t shape[1] = {2};
            nb::capsule deleter(data, [](void *p) noexcept {
                delete[] (T*)p;
            });
            return nb::ndarray<nb::numpy, T, nb::shape<2>>(data, 1, shape, deleter);
        }, "Convert vec2 to numpy array")
        .def("set", &Vec::set, nb::arg("x"), nb::arg("y"))
        .def("set_all", &Vec::set_all, nb::arg("a"))
        .def("length", &Vec::length)
        .def("length_sq", &Vec::length_sq)
        .def("length_manhattan", &Vec::length_manhattan)
        .def("normalize", &Vec::normalize)
        .def("negate", &Vec::negate)
        .def("perpendicular", &Vec::perpendicular)
        .def("is_collinear", &Vec::is_collinear, nb::arg("v"), nb::arg("epsilon") = rove::EPSILON)
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
        .def("dot", [](const Vec &a, const Vec &b) { return rove::dot_product(a, b); });
}

template<typename T>
void bind_vec3(nb::module_ &m, const char *name) {
    using Vec = rove::vec<3, T>;

    nb::class_<Vec>(m, name)
        .def(nb::init<>())
        .def(nb::init<T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"))
        .def_rw("x", &Vec::x)
        .def_rw("y", &Vec::y)
        .def_rw("z", &Vec::z)
        .def_static("from_numpy", [](nb::ndarray<T, nb::shape<3>> arr) {
            return Vec(arr(0), arr(1), arr(2));
        }, nb::arg("array"), "Create vec3 from numpy array")
        .def("to_numpy", [](const Vec &v) {
            T* data = new T[3]{v.x, v.y, v.z};
            size_t shape[1] = {3};
            nb::capsule deleter(data, [](void *p) noexcept {
                delete[] (T*)p;
            });
            return nb::ndarray<nb::numpy, T, nb::shape<3>>(data, 1, shape, deleter);
        }, "Convert vec3 to numpy array")
        .def("set", &Vec::set, nb::arg("x"), nb::arg("y"), nb::arg("z"))
        .def("set_all", &Vec::set_all, nb::arg("a"))
        .def("length", &Vec::length)
        .def("length_sq", &Vec::length_sq)
        .def("normalize", &Vec::normalize)
        .def("negate", &Vec::negate)
        .def("is_collinear", &Vec::is_collinear, nb::arg("v"), nb::arg("epsilon") = rove::EPSILON)
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
        .def("dot", [](const Vec &a, const Vec &b) { return rove::dot_product(a, b); })
        .def("cross", [](const Vec &a, const Vec &b) {
            Vec result;
            rove::cross_product(result, a, b);
            return result;
        });
}

template<typename T>
void bind_vec4(nb::module_ &m, const char *name) {
    using Vec = rove::vec<4, T>;

    nb::class_<Vec>(m, name)
        .def(nb::init<>())
        .def(nb::init<T, T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("w"))
        .def_rw("x", &Vec::x)
        .def_rw("y", &Vec::y)
        .def_rw("z", &Vec::z)
        .def_rw("w", &Vec::w)
        .def_static("from_numpy", [](nb::ndarray<T, nb::shape<4>> arr) {
            return Vec(arr(0), arr(1), arr(2), arr(3));
        }, nb::arg("array"), "Create vec4 from numpy array")
        .def("to_numpy", [](const Vec &v) {
            T* data = new T[4]{v.x, v.y, v.z, v.w};
            size_t shape[1] = {4};
            nb::capsule deleter(data, [](void *p) noexcept {
                delete[] (T*)p;
            });
            return nb::ndarray<nb::numpy, T, nb::shape<4>>(data, 1, shape, deleter);
        }, "Convert vec4 to numpy array")
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
        .def("dot", [](const Vec &a, const Vec &b) { return rove::dot_product(a, b); });
}

void bind_vec_functions(nb::module_ &m) {
    // Free functions for float vectors
    m.def("dot", [](const rove::vec<2, float> &a, const rove::vec<2, float> &b) {
        return rove::dot_product(a, b);
    }, nb::arg("a"), nb::arg("b"), "Compute dot product of two vec2");

    m.def("dot", [](const rove::vec<3, float> &a, const rove::vec<3, float> &b) {
        return rove::dot_product(a, b);
    }, nb::arg("a"), nb::arg("b"), "Compute dot product of two vec3");

    m.def("dot", [](const rove::vec<4, float> &a, const rove::vec<4, float> &b) {
        return rove::dot_product(a, b);
    }, nb::arg("a"), nb::arg("b"), "Compute dot product of two vec4");

    m.def("cross", [](const rove::vec<3, float> &a, const rove::vec<3, float> &b) {
        rove::vec<3, float> result;
        rove::cross_product(result, a, b);
        return result;
    }, nb::arg("a"), nb::arg("b"), "Compute cross product of two vec3");

    m.def("normalize", [](const rove::vec<2, float> &v) {
        return rove::normalize(v);
    }, nb::arg("v"), "Return normalized vec2");

    m.def("normalize", [](const rove::vec<3, float> &v) {
        return rove::normalize(v);
    }, nb::arg("v"), "Return normalized vec3");

    m.def("normalize", [](const rove::vec<4, float> &v) {
        return rove::normalize(v);
    }, nb::arg("v"), "Return normalized vec4");
}

// Explicit template instantiations
template void bind_vec2<float>(nb::module_ &m, const char *name);
template void bind_vec2<double>(nb::module_ &m, const char *name);
template void bind_vec3<float>(nb::module_ &m, const char *name);
template void bind_vec3<double>(nb::module_ &m, const char *name);
template void bind_vec4<float>(nb::module_ &m, const char *name);
template void bind_vec4<double>(nb::module_ &m, const char *name);
