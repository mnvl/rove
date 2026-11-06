// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file obb.h
 * @brief Oriented bounding box (OBB) classes for 2D and 3D
 *
 * Provides oriented bounding boxes that can be rotated arbitrarily in space.
 * OBBs offer tighter bounds than AABBs for rotated objects at the cost of
 * more expensive intersection tests.
 */

#pragma once

#include <cassert>
#include "scalar.h"
#include "vec.h"
#include "aabb.h"
#include "ray.h"
#include "segment.h"
#include "triangle.h"
#include "matrix.h"

namespace rove
{

template<class T> class plane;
template<int N,class T> class triangle;
template<int N,class T> class capsule;

template<int N,class T=scalar> class obb;

/**
 * @brief 2D Oriented bounding box
 *
 * A 2D box defined by an origin point and two edge vectors (tangent and normal).
 * The box vertices are:
 * - origin
 * - origin + tangent
 * - origin + tangent + normal
 * - origin + normal
 */
template<class T>
class [[nodiscard]] obb<2, T> {
public:
	static constexpr size_t dimensions = 2;              ///< Number of dimensions
	using vec_type = vec<dimensions,T>;          ///< Associated vector type
	using matrix_type = matrix<dimensions+1,dimensions+1,T>;  ///< Transformation matrix type
	using aabb_type = aabb<dimensions,T>;        ///< Associated AABB type
	static constexpr size_t sides_count = 4;        ///< Number of sides
	static constexpr size_t vertices_count = 4;     ///< Number of vertices

	vec_type origin;   ///< Origin corner of the OBB
	vec_type tangent;  ///< First edge vector (width direction)
	vec_type normal;   ///< Second edge vector (height direction)

	/// Default constructor (uninitialized)
	obb() {
	}

	/**
	 * @brief Construct OBB from axis-aligned bounding box
	 *
	 * Creates an axis-aligned OBB that matches the given AABB.
	 */
	explicit obb(aabb_type const &rhs) {
		origin = rhs.lo;
		tangent.set(rhs.hi.x - rhs.lo.x, 0);
		normal.set(0, rhs.hi.y - rhs.lo.y);
	}

	/**
	 * @brief Transform vector from world to local OBB coordinates
	 *
	 * Converts a direction vector to local coordinates where tangent and
	 * normal are unit basis vectors.
	 *
	 * @param[out] v Output vector in local coordinates
	 * @param[in] v0 Input vector in world coordinates
	 */
	void world_to_local_vector(vec_type &v, vec_type const &v0) const {
		v.x = dot(v0, tangent) / tangent.length_sq();
		v.y = dot(v0, normal)	 / normal.length_sq();
	}

	/**
	 * @brief Transform vector from world to local OBB coordinates
	 *
	 * @param v0 Input vector in world coordinates
	 * @return Vector in local coordinates
	 */
	vec_type world_to_local_vector(vec_type const &v0) const {
		vec_type v;
		world_to_local_vector(v, v0);
		return v;
	}

	/**
	 * @brief Transform point from world to local OBB coordinates
	 *
	 * Converts a point to local coordinates where origin maps to (0,0)
	 * and opposite corner maps to (1,1).
	 *
	 * @param[out] v Output point in local coordinates
	 * @param[in] v0 Input point in world coordinates
	 */
	void world_to_local_point(vec_type &v,vec_type const &v0) const {
		world_to_local_vector(v, v0 - origin);
	}

	/**
	 * @brief Transform point from world to local OBB coordinates
	 *
	 * @param v0 Input point in world coordinates
	 * @return Point in local coordinates
	 */
	vec_type world_to_local_point(vec_type const &v0) const {
		vec_type v;
		world_to_local_point(v, v0);
		return v;
	}

	/**
	 * @brief Test if point is inside OBB
	 *
	 * @return true if point is inside or on the boundary
	 */
	bool contains(vec_type const &v) const {
		vec_type v1;
		world_to_local_point(v1, v);
		return v1.x>=0 && v1.x<=1
		    && v1.y>=0 && v1.y<=1;
	}

	/**
	 * @brief Transform OBB by a matrix
	 *
	 * Applies a transformation matrix to the OBB.
	 */
	void transform(matrix_type const &tf) {
		vec_type O = origin, Ox = tangent + origin, Oy = normal + origin;
		origin = project_point(tf, O);           // (the 3x3 product's own divide by z, by name)
		tangent = project_point(tf, Ox);
		normal = project_point(tf, Oy);
		tangent -= origin;
		normal -= origin;
	}

	/**
	 * @brief Get vertex position by index
	 *
	 * @param vertex_number Vertex index (0-3)
	 */
	vec_type vertex(size_t vertex_number) const {
		switch (vertex_number) {
			case 0: return origin;
			case 1: return origin + tangent;
			case 2: return origin + tangent + normal;
			case 3: return origin + normal;
			default: assert(0); return vec_type(0, 0);
		}
	}

	/**
	 * @brief Get side as line segment
	 *
	 * @param side_number Side index (0-3)
	 */
	segment<dimensions, T> side(size_t side_number) const {
		segment<dimensions, T> side;
		side.A = vertex(side_number);
		side.B = vertex((1 + side_number) % sides_count);
		return side;
	}

	/**
	 * @brief Get outward normal for a side
	 *
	 * @param side_number Side index (0-3)
	 * @return Normalized outward-facing normal vector
	 */
	vec_type side_normal(size_t side_number) const {
		switch (side_number) {
			case 0: return rove::normalize_or_zero(-normal);
			case 1: return rove::normalize_or_zero(tangent);
			case 2: return rove::normalize_or_zero(normal);
			case 3: return rove::normalize_or_zero(-tangent);
			default: assert(0); return vec_type(0, 0);
		}
	}

	/**
	 * @brief Get axis-aligned bounding box containing this OBB
	 *
	 * @param[out] b Output AABB that bounds this OBB
	 */
	aabb_type bounds() const {
		aabb_type b;
		b.lo = b.hi = origin;
		b.extend(origin + tangent);
		b.extend(origin + normal);
		b.extend(origin + tangent + normal);
		return b;
	}
};

/**
 * @brief 3D Oriented bounding box
 *
 * A 3D box defined by an origin point and three edge vectors (tangent, normal, binormal).
 * The eight vertices are all combinations of origin + {0|tangent} + {0|normal} + {0|binormal}.
 */
template<class T>
class [[nodiscard]] obb<3,T> {
public:
	static constexpr size_t dimensions = 3;              ///< Number of dimensions
	using vec_type = vec<dimensions,T>;          ///< Associated vector type
	using matrix3_t = matrix<dimensions, dimensions, T>;    ///< 3x3 matrix type
	using matrix_type = matrix<dimensions+1, dimensions+1, T>; ///< 4x4 transformation matrix type
	using aabb_type = aabb<dimensions,T>;        ///< Associated AABB type
	static constexpr size_t sides_count = 6;        ///< Number of faces
	static constexpr size_t vertices_count = 8;     ///< Number of vertices
	static constexpr size_t octants_count = 8;      ///< How many octant(i) there are

	vec_type origin;    ///< Origin corner of the OBB
	vec_type tangent;   ///< First edge vector (X-like direction)
	vec_type normal;    ///< Second edge vector (Y-like direction)
	vec_type binormal;  ///< Third edge vector (Z-like direction)

	/// Default constructor (uninitialized)
	obb() {
	}

	/**
	 * @brief Construct OBB from axis-aligned bounding box
	 *
	 * Creates an axis-aligned OBB that matches the given AABB.
	 */
	explicit obb(aabb_type const &a)
	{
		operator =(a);
	}

	/**
	 * @brief Construct OBB from origin and three edge vectors
	 *
	 * @param o Origin point
	 * @param i Tangent (first edge vector)
	 * @param j Normal (second edge vector)
	 * @param k Binormal (third edge vector)
	 */
	obb(vec_type const &o, vec_type const &i, vec_type const &j, vec_type const &k):
		origin(o),
		tangent(i),
		normal(j),
		binormal(k)
	{
	}

	/// @brief Assignment from AABB
	obb &operator =(aabb_type const &a) {
		origin = a.lo;
		tangent.set(a.hi.x - a.lo.x, 0, 0);
		normal.set(0, a.hi.y - a.lo.y, 0);
		binormal.set(0, 0, a.hi.z - a.lo.z);
		return *this;
	}

	/**
	 * @brief Get center point of the OBB
	 *
	 * @return Center point
	 */
	vec_type centre() const {
		return origin + (tangent + normal + binormal) / T(2);
	}

	/**
	 * @brief Get bounding sphere radius
	 *
	 * @return Distance from center to furthest corner
	 */
	T radius() const;

	/**
	 * @brief Get volume of the OBB
	 *
	 * @return Volume (product of edge lengths)
	 */
	T volume() const {
		return tangent.length() * normal.length() * binormal.length();
	}

	/**
	 * @brief Get axis-aligned bounding box containing this OBB
	 *
	 * @param[out] b Output AABB that bounds this OBB
	 */
	aabb_type bounds() const {
		aabb_type b;
		b.lo = b.hi = origin;
		b.extend(origin + tangent);
		b.extend(origin + normal);
		b.extend(origin + binormal);
		b.extend(origin + tangent + normal);
		b.extend(origin + tangent + binormal);
		b.extend(origin + normal + binormal);
		b.extend(origin + tangent + normal + binormal);
		return b;
	}

	/**
	 * @brief Transform OBB by a 4x4 matrix
	 *
	 * Applies a transformation matrix to the OBB.
	 */
	void transform(matrix_type const &tf) {
		// get vertex point coordinates
		vec_type O = origin,
			 Ox = tangent + origin,
			 Oy = normal + origin,
			 Oz = binormal + origin;

		// transform these coordinates
		detail::mul(origin, tf, O);
		detail::mul(tangent, tf, Ox);
		detail::mul(normal, tf, Oy);
		detail::mul(binormal, tf, Oz);

		// compute tangent, normal, binormal values relative to origin
		tangent -= origin;
		normal -= origin;
		binormal -= origin;
	}

	/**
	 * @brief Get vertex position by index
	 *
	 * Vertices are numbered 0-7 using binary encoding:
	 * bit 0 = include tangent, bit 1 = include normal, bit 2 = include binormal
	 *
	 * @param vertex_number Vertex index (0-7)
	 */
	vec_type vertex(size_t vertex_number) const {
		vec_type position = origin + T(vertex_number & 1) * tangent;
		if (vertex_number & 2) position += normal;
		if (vertex_number & 4) position += binormal;
		return position;
	}

	/**
	 * @brief Get an octant (sub-box) of this OBB
	 *
	 * Subdivides the OBB into 8 equal sub-boxes (octants).
	 *
	 * @param index Octant index (0-7, octants_count of them)
	 */
	obb octant(size_t index) const {
		obb part;
		part.tangent = tangent / T(2);
		part.normal = normal / T(2);
		part.binormal = binormal / T(2);
		part.origin = origin;

		if (index & 1) part.origin += part.tangent;
		if (index & 2) part.origin += part.normal;
		if (index & 4) part.origin += part.binormal;
		return part;
	}

	/**
	 * @brief Transform vector from world to local OBB coordinates
	 *
	 * Converts a direction vector to local coordinates where tangent,
	 * normal, and binormal form the basis.
	 *
	 * @param[out] v Output vector in local coordinates
	 * @param[in] v0 Input vector in world coordinates
	 */
	void world_to_local_vector(vec_type &v, vec_type const &v0) const;

	/**
	 * @brief Transform vector from local to world OBB coordinates
	 *
	 * @param[out] v Output vector in world coordinates
	 * @param[in] v0 Input vector in local coordinates
	 */
	void local_to_world_vector(vec_type &v, vec_type const &v0) const {
		v = v0.x * tangent + v0.y * normal + v0.z * binormal;
	}

	/**
	 * @brief Transform point from world to local OBB coordinates
	 *
	 * @param[out] v Output point in local coordinates [0,1]^3 for interior
	 * @param[in] v0 Input point in world coordinates
	 */
	void world_to_local_point(vec_type &v,vec_type const &v0) const {
		world_to_local_vector(v, v0 - origin);
	}

	/**
	 * @brief Transform ray from world to local OBB coordinates
	 *
	 * @param[out] r Output ray in local coordinates
	 * @param[in] r0 Input ray in world coordinates
	 */
	void world_to_local_ray(ray<dimensions, T> &r,ray<dimensions, T> const &r0) const {
		world_to_local_point(r.r0,r0.r0);
		world_to_local_vector(r.a,r0.a);
	}

	/**
	 * @brief Transform plane from world to local OBB coordinates
	 *
	 * @param[out] p Output plane in local coordinates
	 * @param[in] p0 Input plane in world coordinates
	 */
	void world_to_local_plane(plane<T> &p, plane<T> const &p0) const;

	/**
	 * @brief Test if point is inside OBB
	 *
	 * @return true if point is inside or on the boundary
	 */
	bool contains(vec_type const &v) const {
		vec_type v1;
		world_to_local_point(v1, v);
		return v1.x>=0 && v1.x<=1
		    && v1.y>=0 && v1.y<=1
		    && v1.z>=0 && v1.z<=1;
	}

	/**
	 * @brief Test if triangle is fully contained in OBB
	 *
	 * @return true if all triangle vertices are inside the OBB
	 */
	bool contains(triangle<dimensions, T> const &tri) const {
		return contains(tri.A) && contains(tri.B) && contains(tri.C);
	}

	/**
	 * @brief Test ray intersection with OBB
	 *
	 * Whether the ray meets the box anywhere for t in [t_min, t_max]: the slab test of trace(r, &t0, &t1, t_min, t_max), so a ray
	 * that starts or ends inside meets it.  (It tested the faces one by one, took the far face of each slab at the wrong t, and so
	 * missed a ray from inside; its defaults were [1, 1], a single point.)
	 *
	 * @param t_min Minimum t value (default 0)
	 * @param t_max Maximum t value (default 1)
	 * @return true if ray intersects OBB
	 */
	 [[nodiscard]] bool trace(ray<dimensions, T> const &r, T t_min = 0,
			 T t_max = 1) const;

	/**
	 * @brief Ray trace against OBB with intersection times
	 *
	 * Finds where a ray enters and exits the OBB.  The ray is
	 * transformed into OBB-local space (where the OBB is the unit
	 * cube [0,1]^3) and a slab test is run on it.  Assumes the
	 * tangent / normal / binormal axes are mutually orthogonal —
	 * true for every OBB constructed in this codebase.
	 *
	 * @param[out] t0 Time of entry (where ray enters OBB)
	 * @param[out] t1 Time of exit (where ray leaves OBB)
	 * @param t_min Minimum t value (default 0)
	 * @param t_max Maximum t value (default 1)
	 * @return true if ray intersects OBB in [t_min, t_max]
	 */
	[[nodiscard]] bool trace(ray<dimensions, T> const &r, T *t0, T *t1, T t_min = 0,
			T t_max = 1) const;

	/**
	 * @brief Test if another OBB is fully contained
	 *
	 * @return true if r is completely inside this OBB
	 */
	bool contains(obb const &r) const;

	/**
	 * @brief Test intersection with triangle
	 *
	 * Uses separating axis theorem (SAT) for accurate intersection test.
	 *
	 * @return true if OBB and triangle intersect
	 */
	bool test_intersection(triangle<dimensions, T> const &tri) const {
		return !test_nonintersection(tri);
	}

	/**
	 * @brief Test intersection with another OBB
	 *
	 * The separating axis theorem over its 15 axes: the 3 face axes of each box and the 9 cross products of their edges.
	 *
	 * @return true if the OBBs intersect
	 */
	bool test_intersection(obb const &box) const {
		return !test_nonintersection(box);
	}

	/**
	 * @brief Test intersection with a capsule
	 *
	 * Exact. A capsule is the Minkowski sum of its axis segment with a
	 * sphere, so the two shapes meet exactly when the distance from the
	 * axis to the box is at most the radius.
	 *
	 * @return true if OBB and capsule intersect
	 */
	bool test_intersection(capsule<dimensions, T> const &cap) const;

	/**
	 * @brief Test collision with moving triangle
	 *
	 * Whether they meet at some time in the interval, the triangle being at tri + vel * t at time t.  The separating axis theorem
	 * for moving shapes: each axis says when the two overlap on it, and they meet if those times have one in common.
	 *
	 * @param vel Relative velocity of the triangle
	 * @param t_min Start of time interval (default 0)
	 * @param t_max End of time interval (default 1)
	 * @return true if collision occurs in [t_min, t_max]
	 */
	bool test_collision(triangle<dimensions, T> const &tri, vec_type const &vel, T t_min = 0,
		T t_max = 1) const
	{
		return !test_noncollision(tri, vel, t_min, t_max);
	}

	/**
	 * @brief Test collision with moving OBB
	 *
	 * Whether they meet at some time in the interval, the other box being at box + vel * t at time t: see the triangle's overload.
	 *
	 * @param vel Relative velocity of the other box
	 * @param t_min Start of time interval (default 0)
	 * @param t_max End of time interval (default 1)
	 * @return true if collision occurs in [t_min, t_max]
	 */
	bool test_collision(obb const &box, vec_type const &vel, T t_min = 0,
		T t_max = 1) const
	{
		return !test_noncollision(box, vel, t_min, t_max);
	}

	/**
	 * @brief Decompose the swept volume of a moving OBB into 6 face-sweep OBBs
	 *
	 * Each face of the original box is extruded along the displacement vector
	 * to produce a thin OBB that covers the region swept by that face.
	 * Together the 6 OBBs cover what the box's surface passes over, which is what stops it tunnelling through thin geometry.  They do
	 * not cover all of the swept volume: the inside of the box at its start and at its end is in none of them, unless a face passes
	 * through it.  Test those two boxes as well for something that may lie wholly inside.
	 *
	 * @param box      The OBB at its starting position
	 * @param displacement  Motion vector (end position minus start position)
	 * @param[out] out Array of 6 OBBs to fill
	 */
	static void make_sweep_volume(obb const &box, vec_type const &displacement, obb *out) {
		vec_type const &o = box.origin;
		vec_type const &t = box.tangent;
		vec_type const &n = box.normal;
		vec_type const &b = box.binormal;
		vec_type const &v = displacement;

		// bottom face (binormal = 0)
		out[0] = obb(o,     t, n, v);
		// top face    (binormal = 1)
		out[1] = obb(o + b, t, n, v);
		// left face   (normal = 0)
		out[2] = obb(o,     t, b, v);
		// right face  (normal = 1)
		out[3] = obb(o + n, t, b, v);
		// back face   (tangent = 0)
		out[4] = obb(o,     n, b, v);
		// front face  (tangent = 1)
		out[5] = obb(o + t, n, b, v);
	}

	/**
	 * @brief Check if OBB edge vectors are linearly independent
	 *
	 * Uses the scalar triple product det(tangent, normal, binormal) to test
	 * whether the three edge vectors span a non-degenerate volume.  The
	 * result is compared against the product of edge lengths to make the
	 * test scale-independent.
	 *
	 * @param tolerance  Minimum ratio of |volume| to product of edge lengths.
	 *                   Default 1e-2 corresponds to ~0.6° maximum parallelism.
	 * @return true if the edge vectors are linearly independent
	 */
	bool is_basis_linearly_independent(T tolerance = T(1e-2)) const;

	/**
	 * @brief Test whether this OBB is degenerate (has near-zero volume)
	 *
	 * An OBB is degenerate when its three edge vectors are nearly coplanar,
	 * i.e. when any two edges are nearly parallel.  This is detected by
	 * comparing the scalar triple product |det(tangent, normal, binormal)|
	 * against the product of edge magnitudes.
	 *
	 * Degenerate OBBs arise from make_sweep_volume() when the displacement
	 * is parallel to one of the original OBB's face edges.  The SAT
	 * intersection test is unreliable for degenerate OBBs because the
	 * cross-product separating axes collapse to zero vectors, potentially
	 * producing false positives.
	 *
	 * @param tolerance  Minimum ratio of |volume| to product of edge lengths.
	 *                   Default 1e-2 corresponds to ~0.6° maximum parallelism.
	 * @return true if the OBB is degenerate
	 */
	bool is_degenerate(T tolerance = T(1e-2)) const;

private:
	// check for non-intersection with triangle
	bool test_nonintersection(triangle<dimensions, T> const &r) const;

	// check for non-intersection with obb
	bool test_nonintersection(obb const &r) const;

	// check for one-sided non-collision with triangle
	bool test_noncollision(triangle<dimensions, T> const &r, vec_type const &vel, T t_min = 0,
		T t_max = 1) const;

	// check for non-collision with obb
	bool test_noncollision(obb const &r, vec_type const &vel, T t_min = 0,
		T t_max = 1) const;
};

}
