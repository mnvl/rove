// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

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

#include <cstddef>
#include <string>

namespace nb = nanobind;

/// A pickled state of the size a __setstate__ reads, or ValueError: a tuple is not checked when indexed, and a malformed pickle would
/// read past it
inline void check_state(nb::tuple const &state, size_t size, char const *what)
{
	if (state.size() != size)
		throw nb::value_error((std::string(what) + ": a state of " + std::to_string(state.size()) + " items, not " + std::to_string(size)).c_str());
}

/// The value protocol on a class with a __getstate__, by that state: == and != (NotImplemented for another type, so Python says False),
/// a hash that goes with them, and pickling and copying at any protocol
void install_value_protocol(nb::handle cls);

/// Pickling and copying at any protocol by __getstate__: what a class with an == of its own still needs
void install_pickling(nb::handle cls);

/// The type of SingularMatrixError
PyObject *singular_matrix_error();

/// to_numpy as numpy's own conversion, and numbers on the left leaving the operator to the vector or matrix
void install_numpy_protocol(nb::handle cls);

/// @brief Bind vec2 class to Python module
template<class T>
void bind_vec2(nb::module_ &m, char const *name);

/// @brief Bind vec3 class to Python module
template<class T>
void bind_vec3(nb::module_ &m, char const *name);

/// @brief Bind vec4 class to Python module
template<class T>
void bind_vec4(nb::module_ &m, char const *name);

/// @brief Bind matrix<3,3> class to Python module
template<class T>
void bind_matrix3(nb::module_ &m, char const *name);

/// @brief Bind matrix<4,4> class to Python module
template<class T>
void bind_matrix4(nb::module_ &m, char const *name);

/// @brief Bind quaternion class to Python module
template<class T>
void bind_quaternion(nb::module_ &m, char const *name);

/// @brief Bind quaternion_slerper class to Python module
template<class T>
void bind_quaternion_slerper(nb::module_ &m, char const *name);

/// @brief Bind vector-related free functions to Python module
void bind_vec_functions(nb::module_ &m);

/// @brief Bind quaternion-related free functions to Python module
void bind_quaternion_functions(nb::module_ &m);

/// @brief Bind sphere<2> class to Python module
template<class T>
void bind_sphere2(nb::module_ &m, char const *name);

/// @brief Bind sphere<3> class to Python module
template<class T>
void bind_sphere3(nb::module_ &m, char const *name);

/// @brief Bind aabb<2> class to Python module
template<class T>
void bind_aabb2(nb::module_ &m, char const *name);

/// @brief Bind aabb<3> class to Python module
template<class T>
void bind_aabb3(nb::module_ &m, char const *name);

/// @brief Bind obb<2> class to Python module
template<class T>
void bind_obb2(nb::module_ &m, char const *name);

/// @brief Bind obb<3> class to Python module
template<class T>
void bind_obb3(nb::module_ &m, char const *name);

/// @brief Bind ray<2> class to Python module
template<class T>
void bind_ray2(nb::module_ &m, char const *name);

/// @brief Bind ray<3> class to Python module
template<class T>
void bind_ray3(nb::module_ &m, char const *name);

/// @brief Bind segment<2> (line2 in Python) to Python module
template<class T>
void bind_line2(nb::module_ &m, char const *name);

/// @brief Bind segment<3> (line3 in Python) to Python module
template<class T>
void bind_line3(nb::module_ &m, char const *name);

/// @brief Bind plane class to Python module
template<class T>
void bind_plane(nb::module_ &m, char const *name);

/// @brief Bind triangle<3> class to Python module
template<class T>
void bind_triangle3(nb::module_ &m, char const *name);

/// @brief Bind capsule<2> class to Python module
template<class T>
void bind_capsule2(nb::module_ &m, char const *name);

/// @brief Bind capsule<3> class to Python module
template<class T>
void bind_capsule3(nb::module_ &m, char const *name);

/// @brief Bind the collision tests (collision.h): first_contact
void bind_collision_functions(nb::module_ &m);

/**
 * @brief Bind frustum class to Python module
 * @tparam T Scalar type (float only - double not supported due to C++ implementation)
 */
template<class T>
void bind_frustum(nb::module_ &m, char const *name);

/// @brief Bind octree<triangle<3,T>, int> class to Python module
template<class T>
void bind_octree_triangle3(nb::module_ &m, char const *name);

void bind_a_star(nb::module_ &m);

/// scalar.h: angles, wraps, remaps, and the smoothing steps (on numbers and vectors)
void bind_scalar_functions(nb::module_ &m);

/// matrix.h's matrices by value and points through them by name
void bind_matrix_functions(nb::module_ &m);

/// aabb.h's bounds_of and transform_aabb
void bind_aabb_functions(nb::module_ &m);

/// transform.h: trs (`name`), to_trs, inverse, lerp, unproject_ray
template<class T>
void bind_trs(nb::module_ &m, char const *name);

/// random.h, hash.h, noise.h
void bind_random(nb::module_ &m);
