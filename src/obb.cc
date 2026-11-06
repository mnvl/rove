// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "obb.h"

#include "capsule.h"
#include "triangle.h"

namespace rove
{

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

	// two obbs do not intersect if there exists a plane that divides space into
	// two half-spaces, one containing the first obb and the other containing the second
	// (sufficient condition)
	return (v0.x<0 && v1.x<0 && v2.x<0 && v3.x<0 && v4.x<0 && v5.x<0 && v6.x<0 && v7.x<0)
		|| (v0.y<0 && v1.y<0 && v2.y<0 && v3.y<0 && v4.y<0 && v5.y<0 && v6.y<0 && v7.y<0)
		|| (v0.z<0 && v1.z<0 && v2.z<0 && v3.z<0 && v4.z<0 && v5.z<0 && v6.z<0 && v7.z<0)
		|| (v0.x>1 && v1.x>1 && v2.x>1 && v3.x>1 && v4.x>1 && v5.x>1 && v6.x>1 && v7.x>1)
		|| (v0.y>1 && v1.y>1 && v2.y>1 && v3.y>1 && v4.y>1 && v5.y>1 && v6.y>1 && v7.y>1)
		|| (v0.z>1 && v1.z>1 && v2.z>1 && v3.z>1 && v4.z>1 && v5.z>1 && v6.z>1 && v7.z>1);
}

template<class T> bool
obb<3,T>::test_noncollision(triangle<dimensions, T> const &tri, vec_type const &vel, T t_min, T t_max) const
{
	vec_type tri_A, tri_B, tri_C, tri_vel;
	world_to_local_point(tri_A, tri.A);
	world_to_local_point(tri_B, tri.B);
	world_to_local_point(tri_C, tri.C);
	world_to_local_vector(tri_vel, vel);

		T t_len = t_max - t_min;
	vec_type tri_delta = tri_vel * t_len;

	if (t_min != 0) {
		vec_type tri_start = tri_vel * t_min;
		tri_A -= tri_start;
		tri_B -= tri_start;
		tri_C -= tri_start;
	}

	vec_type tri_A1 = tri_A + tri_delta;
	vec_type tri_B1 = tri_B + tri_delta;
	vec_type tri_C1 = tri_C + tri_delta;

	if ((tri_A.x<0 && tri_B.x<0 && tri_C.x<0 && tri_A1.x<0 && tri_B1.x<0 && tri_C1.x<0)
	|| (tri_A.x>1 && tri_B.x>1 && tri_C.x>1 && tri_A1.x>1 && tri_B1.x>1 && tri_C1.x>1)
	|| (tri_A.y<0 && tri_B.y<0 && tri_C.y<0 && tri_A1.y<0 && tri_B1.y<0 && tri_C1.y<0)
	|| (tri_A.y>1 && tri_B.y>1 && tri_C.y>1 && tri_A1.y>1 && tri_B1.y>1 && tri_C1.y>1)
	|| (tri_A.z<0 && tri_B.z<0 && tri_C.z<0 && tri_A1.z<0 && tri_B1.z<0 && tri_C1.z<0)
	|| (tri_A.z>1 && tri_B.z>1 && tri_C.z>1 && tri_A1.z>1 && tri_B1.z>1 && tri_C1.z>1))
	{
		return true;
	}

	vec_type tri_normal = cross(tri_B - tri_A, tri_C - tri_B);

	T k1 = dot(vec_type(0,0,0) - tri_A, tri_normal);
	T k2 = dot(vec_type(0,0,1) - tri_A, tri_normal);
	T k3 = dot(vec_type(0,1,0) - tri_A, tri_normal);
	T k4 = dot(vec_type(0,1,1) - tri_A, tri_normal);
	T k5 = dot(vec_type(1,0,0) - tri_A, tri_normal);
	T k6 = dot(vec_type(1,0,1) - tri_A, tri_normal);
	T k7 = dot(vec_type(1,1,0) - tri_A, tri_normal);
	T k8 = dot(vec_type(1,1,1) - tri_A, tri_normal);

	return (k1<0 && k2<0 && k3<0 && k4<0 && k5<0 && k6<0 && k7<0 && k8<0)
	    || (k1>0 && k2>0 && k3>0 && k4>0 && k5>0 && k6>0 && k7>0 && k8>0);
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

	// compute interval length
	T t_len = t_max - t_min;
	vec_type delta = box_vel * t_len;

	// get obb vertex coordinates
	vec_type v0_0 = box_origin - box_vel * t_min;
	vec_type v1_0 = v0_0 + box_tangent;
	vec_type v2_0 = v1_0 + box_normal;
	vec_type v3_0 = v0_0 + box_normal;
	vec_type v4_0 = v0_0 + box_binormal;
	vec_type v5_0 = v1_0 + box_binormal;
	vec_type v6_0 = v2_0 + box_binormal;
	vec_type v7_0 = v3_0 + box_binormal;
	vec_type v0_1 = v0_0 + delta;
	vec_type v1_1 = v1_0 + delta;
	vec_type v2_1 = v2_0 + delta;
	vec_type v3_1 = v3_0 + delta;
	vec_type v4_1 = v4_0 + delta;
	vec_type v5_1 = v5_0 + delta;
	vec_type v6_1 = v6_0 + delta;
	vec_type v7_1 = v7_0 + delta;

	// two obbs do not intersect if there exists a plane that divides space into
	// two half-spaces, one containing the first obb and the other containing the second
	// (sufficient condition)
	return (v0_0.x<0 && v1_0.x<0 && v2_0.x<0 && v3_0.x<0 && v4_0.x<0 && v5_0.x<0 && v6_0.x<0 && v7_0.x<0
	    &&  v0_1.x<0 && v1_1.x<0 && v2_1.x<0 && v3_1.x<0 && v4_1.x<0 && v5_1.x<0 && v6_1.x<0 && v7_1.x<0)
	    || (v0_0.y<0 && v1_0.y<0 && v2_0.y<0 && v3_0.y<0 && v4_0.y<0 && v5_0.y<0 && v6_0.y<0 && v7_0.y<0
	    &&  v0_1.y<0 && v1_1.y<0 && v2_1.y<0 && v3_1.y<0 && v4_1.y<0 && v5_1.y<0 && v6_1.y<0 && v7_1.y<0)
	    || (v0_0.z<0 && v1_0.z<0 && v2_0.z<0 && v3_0.z<0 && v4_0.z<0 && v5_0.z<0 && v6_0.z<0 && v7_0.z<0
	    &&  v0_1.z<0 && v1_1.z<0 && v2_1.z<0 && v3_1.z<0 && v4_1.z<0 && v5_1.z<0 && v6_1.z<0 && v7_1.z<0)
	    || (v0_0.x>1 && v1_0.x>1 && v2_0.x>1 && v3_0.x>1 && v4_0.x>1 && v5_0.x>1 && v6_0.x>1 && v7_0.x>1
	    &&  v0_1.x>1 && v1_1.x>1 && v2_1.x>1 && v3_1.x>1 && v4_1.x>1 && v5_1.x>1 && v6_1.x>1 && v7_1.x>1)
	    || (v0_0.y>1 && v1_0.y>1 && v2_0.y>1 && v3_0.y>1 && v4_0.y>1 && v5_0.y>1 && v6_0.y>1 && v7_0.y>1
	    &&  v0_1.y>1 && v1_1.y>1 && v2_1.y>1 && v3_1.y>1 && v4_1.y>1 && v5_1.y>1 && v6_1.y>1 && v7_1.y>1)
	    || (v0_0.z>1 && v1_0.z>1 && v2_0.z>1 && v3_0.z>1 && v4_0.z>1 && v5_0.z>1 && v6_0.z>1 && v7_0.z>1
	    &&  v0_1.z>1 && v1_1.z>1 && v2_1.z>1 && v3_1.z>1 && v4_1.z>1 && v5_1.z>1 && v6_1.z>1 && v7_1.z>1);
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

}
