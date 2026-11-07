// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file python_bindings.cc
 * @brief Main Python module definition for pyrove
 *
 * This file defines the pyrove Python module and registers all bindings
 * from separate binding files (bind_vec.cc, bind_matrix.cc,
 * bind_quaternion.cc).
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>

#include <utility>

namespace nb = nanobind;

namespace
{

nb::object not_implemented() { return nb::borrow(Py_NotImplemented); }

}

PyObject *singular_matrix_error()
{
	// (immortal: it lives as long as the module does, and a module of this kind is never unloaded)
	static PyObject *const type = [] {
		PyObject *const bases = PyTuple_Pack(2, PyExc_ValueError, PyExc_RuntimeError);
		PyObject *const made = PyErr_NewExceptionWithDoc("pyrove_bind.SingularMatrixError",
			"A matrix with no inverse: a ValueError, as every other refusal, and a RuntimeError, as Mat.inverse() raised before", bases, nullptr);
		Py_DECREF(bases);
		return made;
	}();
	return type;
}

void install_value_protocol(nb::handle cls)
{
	cls.attr("__eq__") = nb::cpp_function([](nb::handle a, nb::handle b) -> nb::object {
		if (!a.type().is(b.type())) return not_implemented();
		return nb::bool_(a.attr("__getstate__")().equal(b.attr("__getstate__")()));
	}, nb::is_method(), nb::is_operator());
	cls.attr("__ne__") = nb::cpp_function([](nb::handle a, nb::handle b) -> nb::object {
		if (!a.type().is(b.type())) return not_implemented();
		return nb::bool_(a.attr("__getstate__")().not_equal(b.attr("__getstate__")()));
	}, nb::is_method(), nb::is_operator());
	cls.attr("__hash__") = nb::cpp_function([](nb::handle a) { return nb::hash(a.attr("__getstate__")()); }, nb::is_method());
	install_pickling(cls);
}

void install_pickling(nb::handle cls)
{
	// (copyreg.__newobj__ pickles by name at every protocol, where object's own reduction takes 2 at least)
	cls.attr("__reduce_ex__") = nb::cpp_function([](nb::handle self, int) {
		return nb::make_tuple(nb::module_::import_("copyreg").attr("__newobj__"), nb::make_tuple(self.type()), self.attr("__getstate__")());
	}, nb::is_method(), nb::arg("protocol"));
}

void install_numpy_protocol(nb::handle cls)
{
	cls.attr("__array__") = nb::cpp_function([](nb::handle self, nb::object dtype, nb::object) -> nb::object {
		nb::object array = self.attr("to_numpy")();
		return dtype.is_none() ? array : array.attr("astype")(dtype);
	}, nb::is_method(), nb::arg("dtype") = nb::none(), nb::arg("copy") = nb::none());
	// numpy leaves `np.float64(2) * v` to the vector's __rmul__
	cls.attr("__array_ufunc__") = nb::none();
}

/**
 * @brief Python module entry point
 *
 * Defines the pyrove module and registers all class bindings and
 * free functions. The actual binding implementations are in separate
 * files for better organization.
 */
NB_MODULE(pyrove_bind, m) {
	m.doc() = "Python bindings for rove vector math library";

	// Bind vector classes (float versions)
	bind_vec2<float>(m, "vec2");
	bind_vec3<float>(m, "vec3");
	bind_vec4<float>(m, "vec4");

	// Bind vector classes (double versions)
	bind_vec2<double>(m, "vec2d");
	bind_vec3<double>(m, "vec3d");
	bind_vec4<double>(m, "vec4d");

	// Bind vector free functions
	bind_vec_functions(m);

	// Bind matrix classes (float versions)
	bind_matrix3<float>(m, "mat3");
	bind_matrix4<float>(m, "mat4");

	// Bind matrix classes (double versions)
	bind_matrix3<double>(m, "mat3d");
	bind_matrix4<double>(m, "mat4d");

	// Bind quaternion classes (float versions)
	bind_quaternion<float>(m, "quat");
	bind_quaternion_slerper<float>(m, "quat_slerper");

	// Bind quaternion classes (double versions)
	bind_quaternion<double>(m, "quatd");
	bind_quaternion_slerper<double>(m, "quat_slerperd");

	// Bind quaternion free functions
	bind_quaternion_functions(m);

	// Bind sphere classes (float versions)
	bind_sphere2<float>(m, "sphere2");
	bind_sphere3<float>(m, "sphere3");

	// Bind sphere classes (double versions)
	bind_sphere2<double>(m, "sphere2d");
	bind_sphere3<double>(m, "sphere3d");

	// Bind AABB classes (float versions)
	bind_aabb2<float>(m, "aabb2");
	bind_aabb3<float>(m, "aabb3");

	// Bind AABB classes (double versions)
	bind_aabb2<double>(m, "aabb2d");
	bind_aabb3<double>(m, "aabb3d");

	// Bind OBB classes (float versions)
	bind_obb2<float>(m, "obb2");
	bind_obb3<float>(m, "obb3");

	// Bind OBB classes (double versions)
	bind_obb2<double>(m, "obb2d");
	bind_obb3<double>(m, "obb3d");

	// Test ray bindings
	bind_ray2<float>(m, "ray2");
	bind_ray3<float>(m, "ray3");
	bind_ray2<double>(m, "ray2d");
	bind_ray3<double>(m, "ray3d");
	bind_line2<float>(m, "segment2");
	bind_line3<float>(m, "segment3");
	bind_line2<double>(m, "segment2d");
	bind_line3<double>(m, "segment3d");
	bind_plane<float>(m, "plane");
	bind_plane<double>(m, "planed");
	bind_triangle3<float>(m, "triangle3");
	bind_triangle3<double>(m, "triangle3d");
	bind_capsule2<float>(m, "capsule2");
	bind_capsule3<float>(m, "capsule3");
	bind_capsule2<double>(m, "capsule2d");
	bind_capsule3<double>(m, "capsule3d");

	// Bind the collision tests
	bind_collision_functions(m);
	bind_frustum<float>(m, "frustum");
	bind_frustum<double>(m, "frustumd");
	bind_octree_triangle3<float>(m, "OctreeTriangle3");

	bind_a_star(m);

	bind_scalar_functions(m);
	bind_matrix_functions(m);
	bind_aabb_functions(m);
	bind_trs<float>(m, "trs");
	bind_trs<double>(m, "trsd");
	bind_random(m);

	m.attr("SingularMatrixError") = nb::handle(singular_matrix_error());

	// What every value type promises alike, in one place
	for (char const *const name : {"mat3", "mat3d", "mat4", "mat4d", "quat", "quatd", "quat_slerper", "quat_slerperd", "trs", "trsd",
		"sphere2", "sphere2d", "sphere3", "sphere3d", "aabb2", "aabb2d", "aabb3", "aabb3d", "obb2", "obb2d", "obb3", "obb3d",
		"ray2", "ray2d", "ray3", "ray3d", "segment2", "segment2d", "segment3", "segment3d", "plane", "planed", "triangle3", "triangle3d",
		"capsule2", "capsule2d", "capsule3", "capsule3d", "frustum", "frustumd", "pcg32"})
		install_value_protocol(m.attr(name));
	for (char const *const name : {"vec2", "vec2d", "vec3", "vec3d", "vec4", "vec4d"}) {
		install_pickling(m.attr(name));
		install_numpy_protocol(m.attr(name));
	}
	for (char const *const name : {"mat3", "mat3d", "mat4", "mat4d"})
		install_numpy_protocol(m.attr(name));

	static char const *const CLASS_DOCS[][2] = {
		{"vec2", "A 2D vector of float32"}, {"vec2d", "A 2D vector of float64"},
		{"vec3", "A 3D vector of float32"}, {"vec3d", "A 3D vector of float64"},
		{"vec4", "A 4D vector of float32"}, {"vec4d", "A 4D vector of float64"},
		{"mat3", "A 3 x 3 matrix of float32, column-major"}, {"mat3d", "A 3 x 3 matrix of float64, column-major"},
		{"mat4", "A 4 x 4 matrix of float32, column-major"}, {"mat4d", "A 4 x 4 matrix of float64, column-major"},
		{"quat", "A quaternion (x, y, z, w) of float32"}, {"quatd", "A quaternion (x, y, z, w) of float64"},
		{"quat_slerper", "Spherical interpolation between two quaternions of float32, set up once for many steps"},
		{"quat_slerperd", "Spherical interpolation between two quaternions of float64, set up once for many steps"},
		{"sphere2", "A circle: a centre and a radius, of float32"}, {"sphere2d", "A circle: a centre and a radius, of float64"},
		{"sphere3", "A sphere: a centre and a radius, of float32"}, {"sphere3d", "A sphere: a centre and a radius, of float64"},
		{"aabb2", "An axis-aligned rectangle of float32"}, {"aabb2d", "An axis-aligned rectangle of float64"},
		{"aabb3", "An axis-aligned box of float32"}, {"aabb3d", "An axis-aligned box of float64"},
		{"obb2", "An oriented rectangle of float32: a corner and two edges"}, {"obb2d", "An oriented rectangle of float64: a corner and two edges"},
		{"obb3", "An oriented box of float32: a corner and three edges"}, {"obb3d", "An oriented box of float64: a corner and three edges"},
		{"ray2", "A 2D ray of float32: an origin r0 and a direction a; apply(t) is r0 + a t"},
		{"ray2d", "A 2D ray of float64: an origin r0 and a direction a; apply(t) is r0 + a t"},
		{"ray3", "A 3D ray of float32: an origin r0 and a direction a; apply(t) is r0 + a t"},
		{"ray3d", "A 3D ray of float64: an origin r0 and a direction a; apply(t) is r0 + a t"},
		{"segment2", "A 2D segment of float32, from A to B"}, {"segment2d", "A 2D segment of float64, from A to B"},
		{"segment3", "A 3D segment of float32, from A to B"}, {"segment3d", "A 3D segment of float64, from A to B"},
		{"plane", "A plane of float32: A x + B y + C z + D = 0"}, {"planed", "A plane of float64: A x + B y + C z + D = 0"},
		{"triangle3", "A 3D triangle of float32, of corners A, B and C"}, {"triangle3d", "A 3D triangle of float64, of corners A, B and C"},
		{"capsule2", "A 2D capsule of float32: a segment and a radius around it"}, {"capsule2d", "A 2D capsule of float64: a segment and a radius around it"},
		{"capsule3", "A 3D capsule of float32: a segment and a radius around it"}, {"capsule3d", "A 3D capsule of float64: a segment and a radius around it"},
		{"frustum", "A view frustum of float32, from a view-projection matrix"},
		{"frustumd", "A view frustum of float64, from a view-projection matrix"},
		{"OctreeTriangle3", "A static octree of triangles of float32, each with an int payload: insert, then build, then query"},
		{"a_star", "A* over a grid whose cells a Python function scores"},
	};
	for (auto const &doc : CLASS_DOCS)
		if (nb::hasattr(m, doc[0]))
			m.attr(doc[0]).attr("__doc__") = doc[1];
}
