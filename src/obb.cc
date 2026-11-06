// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "obb.h"

#include "capsule.h"
#include "triangle.h"
#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif

namespace rove
{

namespace
{

/// A thing moving past a still one, as seen along one axis in the still box's space, where that box is the unit cube.  Every point of
/// the thing's shape projects into [lo, hi] at time 0, and it moves by `speed` per unit of time.  Keeps in [enter, leave] the times
/// when the two overlap along this axis; false when none are left.
template<class T>
bool overlap_times_on_axis(vec<3, T> const &axis, T lo, T hi, T speed, T &enter, T &leave)
{
	T box_lo = 0, box_hi = 0;
	for (size_t i = 0; i < 3; ++i) {
		if (axis.i[i] < T(0)) box_lo += axis.i[i];
		else box_hi += axis.i[i];
	}
	if (speed == T(0)) return !(hi < box_lo || lo > box_hi);
	T const t_a = (box_lo - hi) / speed, t_b = (box_hi - lo) / speed;
	T const t_enter = (speed > T(0) ? t_a : t_b), t_leave = (speed > T(0) ? t_b : t_a);
	if (t_enter > enter) enter = t_enter;
	if (t_leave < leave) leave = t_leave;
	return enter <= leave;
}

/// Whether an axis is a cross product of edges that were (nearly) parallel: it has lost its direction to rounding and separates nothing
template<class T>
bool lost_its_direction(vec<3, T> const &axis, vec<3, T> const &edge)
{
	return axis.length_sq() <= scalar_traits<T>::zero_length_sq * edge.length_sq();
}

/// The times of [t_min, t_max] when a convex shape meets the unit cube, by every axis that can separate the two.  The vertices are its
/// position at time 0 in the cube's space; `normals` are the shape's face normals, `edges` its edge directions.
template<class T, size_t V, size_t N, size_t E>
bool never_meets_the_unit_cube(vec<3, T> const (&vertices)[V], vec<3, T> const (&normals)[N], vec<3, T> const (&edges)[E],
	vec<3, T> const &vel, T t_min, T t_max)
{
	T enter = t_min, leave = t_max;
	if (!(enter <= leave)) return true;
	auto const on_axis = [&](vec<3, T> const &axis) {
		T lo = dot(vertices[0], axis), hi = lo;
		for (size_t i = 1; i < V; ++i) {
			T const p = dot(vertices[i], axis);
			if (p < lo) lo = p;
			if (p > hi) hi = p;
		}
		return overlap_times_on_axis(axis, lo, hi, dot(vel, axis), enter, leave);
	};

	vec<3, T> const cube[3] = { vec<3, T>(1, 0, 0), vec<3, T>(0, 1, 0), vec<3, T>(0, 0, 1) };
	for (vec<3, T> const &axis : cube) if (!on_axis(axis)) return true;
	for (vec<3, T> const &axis : normals) if (!on_axis(axis)) return true;
	for (vec<3, T> const &edge : edges) {
		for (vec<3, T> const &c : cube) {
			vec<3, T> const axis = cross(c, edge);
			if (!lost_its_direction(axis, edge) && !on_axis(axis)) return true;
		}
	}
	return false;
}

/// An OBB's corners and edge axes in another box's space (origin and three edges already there)
template<class T>
void corners_of(vec<3, T> const &o, vec<3, T> const &t, vec<3, T> const &n, vec<3, T> const &b, vec<3, T> (&out)[8])
{
	out[0] = o;
	out[1] = o + t;
	out[2] = o + t + n;
	out[3] = o + n;
	out[4] = o + b;
	out[5] = o + t + b;
	out[6] = o + t + n + b;
	out[7] = o + n + b;
}

}

template <class T> T
obb<3,T>::radius() const {
	vec_type const c = centre();
	T radius_sq = 0;
	for (size_t i = 0; i < vertices_count; ++i) {
		vec_type pos;
		pos = vertex(i);
		T dist_sq = (pos - c).length_sq();
		if (dist_sq > radius_sq) radius_sq = dist_sq;
	}
	return sqrt(radius_sq);
}

template <class T> void
obb<3,T>::world_to_local_vector(vec_type &v,vec_type const &v0) const
{
	matrix3_t world_to_local;
	world_to_local.world_to_axes(tangent, normal, binormal);

	v = world_to_local * v0;
}

template<class T> void
obb<3,T>::world_to_local_plane(plane<T> &p, plane<T> const &p0) const
{
	vec_type local_origin, local_normal;
	world_to_local_point(local_origin, p0.origin());
	world_to_local_vector(local_normal, p0.normal());
	p.set(local_origin, local_normal);
}

template<class T> bool
obb<3,T>::trace(ray<dimensions, T> const &r, T *t0, T *t1, T t_min, T t_max) const {
	// Transform ray into OBB-local space, where the OBB is the unit
	// cube [0,1]^3.  Then run a textbook slab test against the cube.
	ray<dimensions, T> r1;
	world_to_local_ray(r1, r);

	T t_enter = t_min;
	T t_exit = t_max;
	for (size_t axis = 0; axis < dimensions; ++axis) {
		T o = r1.r0.i[axis];
		T d = r1.a.i[axis];
		if (abs(d) < scalar_traits<T>::zero_length) {
			// Ray parallel to this slab — must already lie inside it.
			if (o < T(0) || o > T(1)) return false;
			continue;
		}
		T inv = T(1) / d;
		T ta = (T(0) - o) * inv;
		T tb = (T(1) - o) * inv;
		if (ta > tb) { T tmp = ta; ta = tb; tb = tmp; }
		if (ta > t_enter) t_enter = ta;
		if (tb < t_exit) t_exit = tb;
		if (t_enter > t_exit) return false;
	}
	*t0 = t_enter;
	*t1 = t_exit;
	return true;
}

template<class T> bool
obb<3,T>::trace(ray<dimensions, T> const &r, T t_min, T t_max) const {
	T t0, t1;
	return trace(r, &t0, &t1, t_min, t_max);
}

template<class T> bool obb<3,T>::contains(obb const &r) const {
	// get r coordinates in local coordinate system
	vec_type r_origin, r_tangent, r_normal, r_binormal;
	world_to_local_point(r_origin,r.origin);
	world_to_local_vector(r_tangent,r.tangent);
	world_to_local_vector(r_normal,r.normal);
	world_to_local_vector(r_binormal,r.binormal);

	// get obb vertex coordinates
	vec_type v0 = r_origin;
	vec_type v1 = v0 + r_tangent;
	vec_type v2 = v1 + r_normal;
	vec_type v3 = v0 + r_normal;
	vec_type v4 = v0 + r_binormal;
	vec_type v5 = v1 + r_binormal;
	vec_type v6 = v2 + r_binormal;
	vec_type v7 = v3 + r_binormal;

	// one obb contains another if it contains all vertices of the other
	return (v0.x>=0 && v1.x>=0 && v2.x>=0 && v3.x>=0 && v4.x>=0 && v5.x>=0 && v6.x>=0 && v7.x>=0)
		&& (v0.y>=0 && v1.y>=0 && v2.y>=0 && v3.y>=0 && v4.y>=0 && v5.y>=0 && v6.y>=0 && v7.y>=0)
		&& (v0.z>=0 && v1.z>=0 && v2.z>=0 && v3.z>=0 && v4.z>=0 && v5.z>=0 && v6.z>=0 && v7.z>=0)
		&& (v0.x<=1 && v1.x<=1 && v2.x<=1 && v3.x<=1 && v4.x<=1 && v5.x<=1 && v6.x<=1 && v7.x<=1)
		&& (v0.y<=1 && v1.y<=1 && v2.y<=1 && v3.y<=1 && v4.y<=1 && v5.y<=1 && v6.y<=1 && v7.y<=1)
		&& (v0.z<=1 && v1.z<=1 && v2.z<=1 && v3.z<=1 && v4.z<=1 && v5.z<=1 && v6.z<=1 && v7.z<=1);
}

template<class T> bool
obb<3,T>::test_nonintersection(triangle<dimensions, T> const &r) const
{
	// get triangle vertex coordinates in local coordinate system
	vec_type A, B, C;
	world_to_local_point(A, r.A);
	world_to_local_point(B, r.B);
	world_to_local_point(C, r.C);

	// OBB face normals as separating axes (in local space the OBB is [0,1]^3)
	if ((A.x<0 && B.x<0 && C.x<0) || (A.y<0 && B.y<0 && C.y<0) || (A.z<0 && B.z<0 && C.z<0)
	|| (A.x>1 && B.x>1 && C.x>1) || (A.y>1 && B.y>1 && C.y>1) || (A.z>1 && B.z>1 && C.z>1))
	{
		return true;
	}

	// Triangle normal as separating axis
	vec_type N = cross(B - A, C - B);

	T k1 = dot(vec_type(0,0,0) - A, N);
	T k2 = dot(vec_type(0,0,1) - A, N);
	T k3 = dot(vec_type(0,1,0) - A, N);
	T k4 = dot(vec_type(0,1,1) - A, N);
	T k5 = dot(vec_type(1,0,0) - A, N);
	T k6 = dot(vec_type(1,0,1) - A, N);
	T k7 = dot(vec_type(1,1,0) - A, N);
	T k8 = dot(vec_type(1,1,1) - A, N);

	if ((k1>0 && k2>0 && k3>0 && k4>0 && k5>0 && k6>0 && k7>0 && k8>0)
	    || (k1<0 && k2<0 && k3<0 && k4<0 && k5<0 && k6<0 && k7<0 && k8<0))
	{
		return true;
	}

	// Edge-edge cross-product separating axes.
	// OBB local edges are (1,0,0), (0,1,0), (0,0,1).
	// For each of the 3 triangle edges, compute cross products with the
	// 3 OBB edges and test for separation along the resulting axis.
	// In local space the OBB projects to [min(0,a)+min(0,b)+min(0,c),
	// max(0,a)+max(0,b)+max(0,c)] on axis (a,b,c).
	vec_type edges[3] = { B - A, C - B, A - C };
	for (int i = 0; i < 3; ++i) {
		vec_type const &e = edges[i];

		// (1,0,0) x e = (0, -e.z, e.y)
		{
			T b = -e.z, c = e.y;
			if (rove::abs(b) > scalar_traits<T>::zero_length || rove::abs(c) > scalar_traits<T>::zero_length) {
				T lo = std::min(T(0), b) + std::min(T(0), c);
				T hi = std::max(T(0), b) + std::max(T(0), c);
				T pA = A.y * b + A.z * c;
				T pB = B.y * b + B.z * c;
				T pC = C.y * b + C.z * c;
				T tri_lo = std::min(pA, std::min(pB, pC));
				T tri_hi = std::max(pA, std::max(pB, pC));
				if (hi < tri_lo || lo > tri_hi) return true;
			}
		}
		// (0,1,0) x e = (e.z, 0, -e.x)
		{
			T a = e.z, c = -e.x;
			if (rove::abs(a) > scalar_traits<T>::zero_length || rove::abs(c) > scalar_traits<T>::zero_length) {
				T lo = std::min(T(0), a) + std::min(T(0), c);
				T hi = std::max(T(0), a) + std::max(T(0), c);
				T pA = A.x * a + A.z * c;
				T pB = B.x * a + B.z * c;
				T pC = C.x * a + C.z * c;
				T tri_lo = std::min(pA, std::min(pB, pC));
				T tri_hi = std::max(pA, std::max(pB, pC));
				if (hi < tri_lo || lo > tri_hi) return true;
			}
		}
		// (0,0,1) x e = (-e.y, e.x, 0)
		{
			T a = -e.y, b = e.x;
			if (rove::abs(a) > scalar_traits<T>::zero_length || rove::abs(b) > scalar_traits<T>::zero_length) {
				T lo = std::min(T(0), a) + std::min(T(0), b);
				T hi = std::max(T(0), a) + std::max(T(0), b);
				T pA = A.x * a + A.y * b;
				T pB = B.x * a + B.y * b;
				T pC = C.x * a + C.y * b;
				T tri_lo = std::min(pA, std::min(pB, pC));
				T tri_hi = std::max(pA, std::max(pB, pC));
				if (hi < tri_lo || lo > tri_hi) return true;
			}
		}
	}

	return false;
}

template<class T> bool obb<3,T>::test_nonintersection(obb const &r) const {
	return test_noncollision(r, vec_type(0, 0, 0), T(0), T(0));
}

template<class T> bool
obb<3,T>::test_noncollision(triangle<dimensions, T> const &tri, vec_type const &vel, T t_min, T t_max) const
{
	vec_type vertices[3], tri_vel;
	world_to_local_point(vertices[0], tri.A);
	world_to_local_point(vertices[1], tri.B);
	world_to_local_point(vertices[2], tri.C);
	world_to_local_vector(tri_vel, vel);

	vec_type const edges[3] = { vertices[1] - vertices[0], vertices[2] - vertices[1], vertices[0] - vertices[2] };
	vec_type const normals[1] = { cross(edges[0], edges[1]) };
	return never_meets_the_unit_cube(vertices, normals, edges, tri_vel, t_min, t_max);
}

template<class T> bool
obb<3,T>::test_noncollision(obb const &box, vec_type const &vel, T t_min,
	T t_max) const
{
	// get box coordinates in local coordinate system
	vec_type box_origin, box_tangent, box_normal, box_binormal, box_vel;
	world_to_local_point(box_origin,box.origin);
	world_to_local_vector(box_tangent,box.tangent);
	world_to_local_vector(box_normal,box.normal);
	world_to_local_vector(box_binormal,box.binormal);
	world_to_local_vector(box_vel, vel);

	vec_type vertices[8];
	corners_of(box_origin, box_tangent, box_normal, box_binormal, vertices);

	vec_type const edges[3] = { box_tangent, box_normal, box_binormal };
	vec_type const normals[3] = { cross(box_normal, box_binormal), cross(box_binormal, box_tangent), cross(box_tangent, box_normal) };
	return never_meets_the_unit_cube(vertices, normals, edges, box_vel, t_min, t_max);
}

template<class T> bool
obb<3,T>::is_basis_linearly_independent(T tolerance) const
{
	return !is_degenerate(tolerance);
}

template<class T> bool
obb<3,T>::is_degenerate(T tolerance) const
{
	// Scalar triple product: vol = dot(tangent, cross(normal, binormal))
	vec_type cross_nb = cross(normal, binormal);
	T vol = dot(tangent, cross_nb);

	T t_sq = tangent.length_sq();
	T n_sq = normal.length_sq();
	T b_sq = binormal.length_sq();

	// Any zero-length edge makes the OBB degenerate.
	if (t_sq < scalar_traits<T>::zero_length_sq || n_sq < scalar_traits<T>::zero_length_sq || b_sq < scalar_traits<T>::zero_length_sq)
		return true;

	// |vol|^2 / (|t|^2 * |n|^2 * |b|^2) < tolerance^2
	return vol * vol < tolerance * tolerance * t_sq * n_sq * b_sq;
}

template<class T> bool
obb<3,T>::test_intersection(capsule<dimensions, T> const &cap) const
{
	if (cap.radius < T(0)) return false;

	// A capsule wholly inside the box touches none of its faces, so the
	// face sweep below would miss it. One endpoint is enough to decide:
	// if it is inside, the shapes intersect.
	if (contains(cap.axe.A)) return true;

	// Otherwise any intersection reaches the surface, and the surface is
	// exactly the union of the 12 triangles of the box's 6 faces.
	// capsule/triangle already handles the reverse containment (a face
	// triangle lying wholly inside the capsule), so this is exact rather
	// than a sampled approximation.
	static int const FACE_TRIANGLES[12][3] = {
		{0, 1, 3}, {0, 3, 2},   // -binormal
		{4, 5, 7}, {4, 7, 6},   // +binormal
		{0, 1, 5}, {0, 5, 4},   // -normal
		{2, 3, 7}, {2, 7, 6},   // +normal
		{0, 2, 6}, {0, 6, 4},   // -tangent
		{1, 3, 7}, {1, 7, 5},   // +tangent
	};

	vec_type vertices[vertices_count];
	for (size_t i = 0; i < vertices_count; ++i) vertices[i] = vertex(i);

	for (size_t i = 0; i < 12; ++i) {
		triangle<dimensions, T> const face(
			vertices[FACE_TRIANGLES[i][0]],
			vertices[FACE_TRIANGLES[i][1]],
			vertices[FACE_TRIANGLES[i][2]]);
		if (cap.test_intersection(face)) return true;
	}
	return false;
}

template class obb<3>;
template class obb<3, double>;

#ifdef ROVE_DFLOAT
template class obb<3, dfloat>;
#endif

}
