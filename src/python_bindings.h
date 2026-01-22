/**
 * @file python_bindings.h
 * @brief Header for Python binding functions
 *
 * This header declares the binding functions that expose C++ classes
 * to Python via nanobind. Each class type has its own binding function
 * implemented in separate .cc files.
 */

#pragma once

#include <nanobind/nanobind.h>

namespace nb = nanobind;

/**
 * @brief Bind vec2 class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_vec2(nb::module_ &m, const char *name);

/**
 * @brief Bind vec3 class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_vec3(nb::module_ &m, const char *name);

/**
 * @brief Bind vec4 class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_vec4(nb::module_ &m, const char *name);

/**
 * @brief Bind matrix<3,3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_matrix3(nb::module_ &m, const char *name);

/**
 * @brief Bind matrix<4,4> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_matrix4(nb::module_ &m, const char *name);

/**
 * @brief Bind quaternion class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_quaternion(nb::module_ &m, const char *name);

/**
 * @brief Bind quaternion_slerper class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_quaternion_slerper(nb::module_ &m, const char *name);

/**
 * @brief Bind vector-related free functions to Python module
 * @param m Python module to bind to
 */
void bind_vec_functions(nb::module_ &m);

/**
 * @brief Bind quaternion-related free functions to Python module
 * @param m Python module to bind to
 */
void bind_quaternion_functions(nb::module_ &m);

/**
 * @brief Bind sphere<2> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_sphere2(nb::module_ &m, const char *name);

/**
 * @brief Bind sphere<3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_sphere3(nb::module_ &m, const char *name);

/**
 * @brief Bind aabb<2> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_aabb2(nb::module_ &m, const char *name);

/**
 * @brief Bind aabb<3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_aabb3(nb::module_ &m, const char *name);

/**
 * @brief Bind obb<2> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_obb2(nb::module_ &m, const char *name);

/**
 * @brief Bind obb<3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_obb3(nb::module_ &m, const char *name);

/**
 * @brief Bind ray<2> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_ray2(nb::module_ &m, const char *name);

/**
 * @brief Bind ray<3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_ray3(nb::module_ &m, const char *name);

/**
 * @brief Bind line<2> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_line2(nb::module_ &m, const char *name);

/**
 * @brief Bind line<3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_line3(nb::module_ &m, const char *name);

/**
 * @brief Bind plane class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_plane(nb::module_ &m, const char *name);

/**
 * @brief Bind triangle<3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_triangle3(nb::module_ &m, const char *name);

/**
 * @brief Bind capsule<2> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_capsule2(nb::module_ &m, const char *name);

/**
 * @brief Bind capsule<3> class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_capsule3(nb::module_ &m, const char *name);

/**
 * @brief Bind frustum class to Python module
 * @tparam T Scalar type (float or double)
 * @param m Python module to bind to
 * @param name Python class name
 */
template<typename T>
void bind_frustum(nb::module_ &m, const char *name);
