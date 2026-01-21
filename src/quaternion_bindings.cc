/**
 * @file quaternion_bindings.cc
 * @brief Python bindings for quaternion classes
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <sstream>

#include "vec.h"
#include "quaternion.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_quaternion(nb::module_ &m, const char *name) {
    using Quat = rove::quaternion<T>;
    using Vec3 = rove::vec<3, T>;

    nb::class_<Quat>(m, name)
        .def(nb::init<>())
        .def(nb::init<T, T, T, T>(), nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("w"))
        .def_static("from_numpy", [](nb::ndarray<T, nb::shape<4>> arr) {
            return Quat(arr(0), arr(1), arr(2), arr(3));
        }, nb::arg("array"), "Create quaternion from numpy array")
        .def("to_numpy", [](const Quat &q) {
            T* data = new T[4]{q.x, q.y, q.z, q.w};
            size_t shape[1] = {4};
            nb::capsule deleter(data, [](void *p) noexcept {
                delete[] (T*)p;
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
        .def("__neg__", [](const Quat &q) { return -q; })
        .def("__add__", [](const Quat &p, const Quat &q) { return p + q; })
        .def("__sub__", [](const Quat &p, const Quat &q) { return p - q; })
        .def("__mul__", [](const Quat &q, T s) { return q * s; })
        .def("__rmul__", [](const Quat &q, T s) { return q * s; })
        .def("__repr__", [](const Quat &q) {
            std::ostringstream ss;
            ss << "quat(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
            return ss.str();
        })
        .def("dot", [](const Quat &p, const Quat &q) {
            return rove::dot_product(p, q);
        }, nb::arg("other"), "Compute dot product with another quaternion");
}

template<typename T>
void bind_quaternion_slerper(nb::module_ &m, const char *name) {
    using Slerper = rove::quaternion_slerper<T>;
    using Quat = rove::quaternion<T>;

    nb::class_<Slerper>(m, name)
        .def(nb::init<>())
        .def("setup", &Slerper::setup, nb::arg("p"), nb::arg("q"),
             "Setup SLERP between two quaternions")
        .def("interpolate", nb::overload_cast<T>(&Slerper::interpolate, nb::const_),
             nb::arg("t"), "Interpolate between quaternions at parameter t [0,1]");
}

void bind_quaternion_functions(nb::module_ &m) {
    // Quaternion free functions (float)
    m.def("dot", [](const rove::quaternion<float> &p, const rove::quaternion<float> &q) {
        return rove::dot_product(p, q);
    }, nb::arg("p"), nb::arg("q"), "Compute dot product of two quaternions");

    m.def("slerp", [](const rove::quaternion<float> &p, const rove::quaternion<float> &q, float t) {
        rove::quaternion_slerper<float> slerper;
        slerper.setup(p, q);
        return slerper.interpolate(t);
    }, nb::arg("p"), nb::arg("q"), nb::arg("t"), "Spherical linear interpolation between quaternions");
}

// Explicit template instantiations
template void bind_quaternion<float>(nb::module_ &m, const char *name);
template void bind_quaternion<double>(nb::module_ &m, const char *name);
template void bind_quaternion_slerper<float>(nb::module_ &m, const char *name);
template void bind_quaternion_slerper<double>(nb::module_ &m, const char *name);
