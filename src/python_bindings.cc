/**
 * @file python_bindings.cc
 * @brief Main Python module definition for pyrove
 *
 * This file defines the pyrove Python module and registers all bindings
 * from separate binding files (vec_bindings.cc, matrix_bindings.cc,
 * quaternion_bindings.cc).
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>

namespace nb = nanobind;

/**
 * @brief Python module entry point
 *
 * Defines the pyrove module and registers all class bindings and
 * free functions. The actual binding implementations are in separate
 * files for better organization.
 */
NB_MODULE(pyrove, m) {
    m.doc() = "Python bindings for rove vector math library";

    // Bind vector classes (float versions)
    bind_vec2<float>(m, "vec2");
    bind_vec3<float>(m, "vec3");
    bind_vec4<float>(m, "vec4");

    // Bind vector classes (double versions)
    bind_vec2<double>(m, "dvec2");
    bind_vec3<double>(m, "dvec3");
    bind_vec4<double>(m, "dvec4");

    // Bind vector free functions
    bind_vec_functions(m);

    // Bind matrix classes (float versions)
    bind_matrix3<float>(m, "mat3");
    bind_matrix4<float>(m, "mat4");

    // Bind matrix classes (double versions)
    bind_matrix3<double>(m, "dmat3");
    bind_matrix4<double>(m, "dmat4");

    // Bind quaternion classes (float versions)
    bind_quaternion<float>(m, "quat");
    bind_quaternion_slerper<float>(m, "quat_slerper");

    // Bind quaternion classes (double versions)
    bind_quaternion<double>(m, "dquat");
    bind_quaternion_slerper<double>(m, "dquat_slerper");

    // Bind quaternion free functions
    bind_quaternion_functions(m);
}
