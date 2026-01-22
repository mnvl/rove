/**
 * @file bind_line.cc
 * @brief Python bindings for line classes
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include "vec.h"
#include "line.h"
#include "ray.h"
#include "aabb.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_line2(nb::module_ &m, const char *name) {
    using Line = rove::line<2, T>;
    using Vec2 = rove::vec<2, T>;
    using Ray2 = rove::ray<2, T>;
    using AABB2 = rove::aabb<2, T>;

    nb::class_<Line>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Vec2&, const Vec2&>(),
             nb::arg("a"), nb::arg("b"),
             "Construct line segment from two endpoints")
        .def_rw("A", &Line::A, "First endpoint of line segment")
        .def_rw("B", &Line::B, "Second endpoint of line segment")
        .def("construct", &Line::construct,
             nb::arg("a"), nb::arg("b"),
             "Set line segment endpoints")
        .def("direction", &Line::direction,
             "Get direction vector (B - A)")
        .def("get_ray", &Line::get_ray,
             "Get ray representation of this line segment")
        .def("get_aabb", &Line::get_aabb,
             "Get axis-aligned bounding box containing this line segment")
        .def("length_sq", &Line::length_sq,
             "Get squared length of line segment")
        .def("contains", &Line::contains,
             nb::arg("point"),
             "Test if point lies on the line segment")
        .def("distance_sq", &Line::distance_sq,
             nb::arg("point"),
             "Get squared distance from line segment to point")
        .def("distance", &Line::distance,
             nb::arg("point"),
             "Get distance from line segment to point")
        .def("__repr__", [](const Line &l) {
            std::ostringstream ss;
            ss << "line2(A=" << l.A << ", B=" << l.B << ")";
            return ss.str();
        });
}

template<typename T>
void bind_line3(nb::module_ &m, const char *name) {
    using Line = rove::line<3, T>;
    using Vec3 = rove::vec<3, T>;
    using Ray3 = rove::ray<3, T>;
    using AABB3 = rove::aabb<3, T>;

    nb::class_<Line>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Vec3&, const Vec3&>(),
             nb::arg("a"), nb::arg("b"),
             "Construct line segment from two endpoints")
        .def_rw("A", &Line::A, "First endpoint of line segment")
        .def_rw("B", &Line::B, "Second endpoint of line segment")
        .def("construct", &Line::construct,
             nb::arg("a"), nb::arg("b"),
             "Set line segment endpoints")
        .def("direction", &Line::direction,
             "Get direction vector (B - A)")
        .def("get_ray", &Line::get_ray,
             "Get ray representation of this line segment")
        .def("get_aabb", &Line::get_aabb,
             "Get axis-aligned bounding box containing this line segment")
        .def("length_sq", &Line::length_sq,
             "Get squared length of line segment")
        .def("contains", &Line::contains,
             nb::arg("point"),
             "Test if point lies on the line segment")
        .def("distance_sq", &Line::distance_sq,
             nb::arg("point"),
             "Get squared distance from line segment to point")
        .def("distance", &Line::distance,
             nb::arg("point"),
             "Get distance from line segment to point")
        .def("__repr__", [](const Line &l) {
            std::ostringstream ss;
            ss << "line3(A=" << l.A << ", B=" << l.B << ")";
            return ss.str();
        });
}

// Explicit template instantiations
template void bind_line2<float>(nb::module_ &m, const char *name);
template void bind_line2<double>(nb::module_ &m, const char *name);
template void bind_line3<float>(nb::module_ &m, const char *name);
template void bind_line3<double>(nb::module_ &m, const char *name);
