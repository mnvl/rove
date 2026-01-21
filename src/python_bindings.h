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
