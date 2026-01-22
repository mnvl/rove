/**
 * @file bind_frustum.cc
 * @brief Python bindings for frustum class
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/array.h>
#include <sstream>

#include "vec.h"
#include "frustum.h"
#include "plane.h"
#include "matrix.h"
#include "aabb.h"
#include "obb.h"

namespace nb = nanobind;
using namespace nb::literals;

template<typename T>
void bind_frustum(nb::module_ &m, const char *name) {
    using Frustum = rove::frustum<T>;
    using Plane = rove::plane<T>;
    using Vec3 = rove::vec<3, T>;
    using Matrix4 = rove::matrix<4, 4, T>;
    using AABB3 = rove::aabb<3, T>;
    using OBB3 = rove::obb<3, T>;

    nb::class_<Frustum>(m, name)
        .def(nb::init<>())
        .def(nb::init<const Matrix4&>(),
             nb::arg("transform"),
             "Construct frustum from view-projection matrix")
        .def_rw("planes", &Frustum::planes, "Array of 6 planes defining the frustum")
        .def("load", &Frustum::load,
             nb::arg("transform"),
             "Load frustum planes from view-projection matrix")
        .def("contains", &Frustum::contains,
             nb::arg("point"),
             "Test if point is inside the frustum")
        .def("test_intersection",
             nb::overload_cast<const AABB3&>(&Frustum::test_intersection, nb::const_),
             nb::arg("aabb"),
             "Test intersection with axis-aligned bounding box")
        .def("test_intersection",
             nb::overload_cast<const OBB3&>(&Frustum::test_intersection, nb::const_),
             nb::arg("obb"),
             "Test intersection with oriented bounding box")
        .def("__repr__", [](const Frustum &f) {
            std::ostringstream ss;
            ss << "frustum(planes=[...])";
            return ss.str();
        });

    // Expose plane index constants
    m.attr("PLANE_LEFT") = Frustum::PLANE_LEFT;
    m.attr("PLANE_RIGHT") = Frustum::PLANE_RIGHT;
    m.attr("PLANE_TOP") = Frustum::PLANE_TOP;
    m.attr("PLANE_BOTTOM") = Frustum::PLANE_BOTTOM;
    m.attr("PLANE_NEAR") = Frustum::PLANE_NEAR;
    m.attr("PLANE_FAR") = Frustum::PLANE_FAR;
}

// Explicit template instantiations
template void bind_frustum<float>(nb::module_ &m, const char *name);
template void bind_frustum<double>(nb::module_ &m, const char *name);
