/**
 * @file bind_capsule.cc
 * @brief Python bindings for capsule classes
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include "vec.h"
#include "capsule.h"
#include "line.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_capsule2(nb::module_ &m, const char *name) {
    using Capsule = rove::capsule<2, T>;
    using Vec2 = rove::vec<2, T>;
    using Line2 = rove::line<2, T>;

    nb::class_<Capsule>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Vec2&, const Vec2&, T>(),
             nb::arg("a"), nb::arg("b"), nb::arg("radius"),
             "Construct capsule from two endpoints and radius")
        .def_rw("axe", &Capsule::axe, "Central axis line segment of the capsule")
        .def_rw("radius", &Capsule::radius, "Radius of the capsule")
        .def("contains", &Capsule::contains,
             nb::arg("point"),
             "Test if point is inside the capsule")
        .def("distance", &Capsule::distance,
             nb::arg("point"),
             "Get distance from capsule surface to point (0 if inside)")
        .def("__repr__", [](const Capsule &c) {
            std::ostringstream ss;
            ss << "capsule2(A=" << c.axe.A << ", B=" << c.axe.B << ", radius=" << c.radius << ")";
            return ss.str();
        });
}

template<typename T>
void bind_capsule3(nb::module_ &m, const char *name) {
    using Capsule = rove::capsule<3, T>;
    using Vec3 = rove::vec<3, T>;
    using Line3 = rove::line<3, T>;

    nb::class_<Capsule>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Vec3&, const Vec3&, T>(),
             nb::arg("a"), nb::arg("b"), nb::arg("radius"),
             "Construct capsule from two endpoints and radius")
        .def_rw("axe", &Capsule::axe, "Central axis line segment of the capsule")
        .def_rw("radius", &Capsule::radius, "Radius of the capsule")
        .def("contains", &Capsule::contains,
             nb::arg("point"),
             "Test if point is inside the capsule")
        .def("distance", &Capsule::distance,
             nb::arg("point"),
             "Get distance from capsule surface to point (0 if inside)")
        .def("__repr__", [](const Capsule &c) {
            std::ostringstream ss;
            ss << "capsule3(A=" << c.axe.A << ", B=" << c.axe.B << ", radius=" << c.radius << ")";
            return ss.str();
        });
}

// Explicit template instantiations
template void bind_capsule2<float>(nb::module_ &m, const char *name);
template void bind_capsule2<double>(nb::module_ &m, const char *name);
template void bind_capsule3<float>(nb::module_ &m, const char *name);
template void bind_capsule3<double>(nb::module_ &m, const char *name);
