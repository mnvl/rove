/**
 * @file bind_ray.cc
 * @brief Python bindings for ray classes
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <sstream>

#include "vec.h"
#include "ray.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_ray2(nb::module_ &m, const char *name) {
    using Ray = rove::ray<2, T>;
    using Vec2 = rove::vec<2, T>;

    nb::class_<Ray>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Vec2&, const Vec2&>(),
             nb::arg("origin"), nb::arg("direction"),
             "Construct ray from origin and direction")
        .def_rw("r0", &Ray::r0, "Ray origin point")
        .def_rw("a", &Ray::a, "Ray direction vector")
        .def("set", &Ray::set,
             nb::arg("origin"), nb::arg("direction"),
             "Set ray origin and direction")
        .def("apply", &Ray::apply,
             nb::arg("t"),
             "Get point on ray at parameter t: r0 + a * t")
        .def("contains", &Ray::contains,
             nb::arg("point"), nb::arg("epsilon") = rove::EPSILON,
             "Test if point lies on the ray")
        .def("distance_sq",
             nb::overload_cast<const Vec2&>(&Ray::distance_sq),
             nb::arg("point"),
             "Get squared distance from ray to point")
        .def("distance",
             nb::overload_cast<const Vec2&>(&Ray::distance),
             nb::arg("point"),
             "Get distance from ray to point")
        .def("test_intersection",
             &Ray::test_intersection,
             nb::arg("other"),
             "Test if this ray intersects with another ray")
        .def("__repr__", [](const Ray &r) {
            std::ostringstream ss;
            ss << "ray2(r0=" << r.r0 << ", a=" << r.a << ")";
            return ss.str();
        });
}

template<typename T>
void bind_ray3(nb::module_ &m, const char *name) {
    using Ray = rove::ray<3, T>;
    using Vec3 = rove::vec<3, T>;

    nb::class_<Ray>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Vec3&, const Vec3&>(),
             nb::arg("origin"), nb::arg("direction"),
             "Construct ray from origin and direction")
        .def_rw("r0", &Ray::r0, "Ray origin point")
        .def_rw("a", &Ray::a, "Ray direction vector")
        .def("set", &Ray::set,
             nb::arg("origin"), nb::arg("direction"),
             "Set ray origin and direction")
        .def("apply", &Ray::apply,
             nb::arg("t"),
             "Get point on ray at parameter t: r0 + a * t")
        .def("contains", &Ray::contains,
             nb::arg("point"), nb::arg("epsilon") = rove::EPSILON,
             "Test if point lies on the ray")
        .def("distance_sq",
             nb::overload_cast<const Vec3&>(&Ray::distance_sq),
             nb::arg("point"),
             "Get squared distance from ray to point")
        .def("distance",
             nb::overload_cast<const Vec3&>(&Ray::distance),
             nb::arg("point"),
             "Get distance from ray to point")
        .def("test_intersection",
             &Ray::test_intersection,
             nb::arg("other"),
             "Test if this ray intersects with another ray")
        .def("__repr__", [](const Ray &r) {
            std::ostringstream ss;
            ss << "ray3(r0=" << r.r0 << ", a=" << r.a << ")";
            return ss.str();
        });
}

// Explicit template instantiations
template void bind_ray2<float>(nb::module_ &m, const char *name);
template void bind_ray2<double>(nb::module_ &m, const char *name);
template void bind_ray3<float>(nb::module_ &m, const char *name);
template void bind_ray3<double>(nb::module_ &m, const char *name);
