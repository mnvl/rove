/**
 * @file bind_plane.cc
 * @brief Python bindings for plane class
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include "vec.h"
#include "plane.h"
#include "ray.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_plane(nb::module_ &m, const char *name) {
    using Plane = rove::plane<T>;
    using Vec3 = rove::vec<3, T>;
    using Ray3 = rove::ray<3, T>;

    nb::enum_<typename Plane::classification_t>(m, (std::string(name) + "_classification").c_str())
        .value("POSITIVE", Plane::POSITIVE, "Point/object is on the positive side of the plane")
        .value("INTERSECTS", Plane::INTERSECTS, "Point/object intersects the plane")
        .value("NEGATIVE", Plane::NEGATIVE, "Point/object is on the negative side of the plane")
        .export_values();

    nb::class_<Plane>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Vec3&, const Vec3&>(),
             nb::arg("origin"), nb::arg("normal"),
             "Construct plane from a point on the plane and normal vector")
        .def(nb::init<T, const Vec3&>(),
             nb::arg("d"), nb::arg("normal"),
             "Construct plane from distance D and normal vector")
        .def_rw("A", &Plane::A, "X coefficient in plane equation Ax + By + Cz + D = 0")
        .def_rw("B", &Plane::B, "Y coefficient in plane equation Ax + By + Cz + D = 0")
        .def_rw("C", &Plane::C, "Z coefficient in plane equation Ax + By + Cz + D = 0")
        .def_rw("D", &Plane::D, "Distance coefficient in plane equation Ax + By + Cz + D = 0")
        .def("set",
             nb::overload_cast<const Vec3&, const Vec3&>(&Plane::set),
             nb::arg("origin"), nb::arg("normal"),
             "Set plane from a point on the plane and normal vector")
        .def("set",
             nb::overload_cast<T, const Vec3&>(&Plane::set),
             nb::arg("d"), nb::arg("normal"),
             "Set plane from distance D and normal vector")
        .def("apply", &Plane::apply,
             nb::arg("point"),
             "Evaluate plane equation at point: Ax + By + Cz + D")
        .def("test_intersection", &Plane::test_intersection,
             nb::arg("ray"),
             "Test if ray intersects with the plane")
        .def("contains",
             nb::overload_cast<const Vec3&>(&Plane::contains, nb::const_),
             nb::arg("point"),
             "Test if point lies on the plane")
        .def("contains",
             nb::overload_cast<const Ray3&>(&Plane::contains, nb::const_),
             nb::arg("ray"),
             "Test if ray lies on the plane")
        .def("parallel", &Plane::parallel,
             nb::arg("ray"),
             "Test if ray is parallel to the plane")
        .def("trace",
             nb::overload_cast<const Ray3&>(&Plane::trace, nb::const_),
             nb::arg("ray"),
             "Get parameter t where ray intersects plane")
        .def("get_origin", &Plane::get_origin,
             "Get a point on the plane")
        .def("get_normal", &Plane::get_normal,
             "Get plane normal vector")
        .def("is_correct", &Plane::is_correct,
             "Check if plane equation is valid (at least one coefficient is non-zero)")
        .def("classify",
             nb::overload_cast<const Vec3&>(&Plane::classify, nb::const_),
             nb::arg("point"),
             "Classify point position relative to plane")
        .def("__repr__", [](const Plane &p) {
            std::ostringstream ss;
            ss << "plane(A=" << p.A << ", B=" << p.B << ", C=" << p.C << ", D=" << p.D << ")";
            return ss.str();
        });
}

// Explicit template instantiations
template void bind_plane<float>(nb::module_ &m, const char *name);
template void bind_plane<double>(nb::module_ &m, const char *name);
