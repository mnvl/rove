// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif
#include "aabb.h"
#include "obb.h"
#include "frustum.h"
#include "sphere.h"

namespace rove
{

template<class T>
frustum<T>::frustum()
{
}

template<class T>
frustum<T>::frustum(matrix_type const &tf)
{
	load(tf);
}

template<class T>
frustum<T>::~frustum()
{
}

template<class T>
void frustum<T>::load(matrix_type const &tf)
{
	// http://zach.in.tu-clausthal.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html

	// left clipping plane
	planes[left_plane].A = tf._14 + tf._11;
	planes[left_plane].B = tf._24 + tf._21;
	planes[left_plane].C = tf._34 + tf._31;
	planes[left_plane].D = tf._44 + tf._41;

	// right clipping plane
	planes[right_plane].A = tf._14 - tf._11;
	planes[right_plane].B = tf._24 - tf._21;
	planes[right_plane].C = tf._34 - tf._31;
	planes[right_plane].D = tf._44 - tf._41;

	// top clipping plane
	planes[top_plane].A = tf._14 - tf._12;
	planes[top_plane].B = tf._24 - tf._22;
	planes[top_plane].C = tf._34 - tf._32;
	planes[top_plane].D = tf._44 - tf._42;

	// bottom clipping plane
	planes[bottom_plane].A = tf._14 + tf._12;
	planes[bottom_plane].B = tf._24 + tf._22;
	planes[bottom_plane].C = tf._34 + tf._32;
	planes[bottom_plane].D = tf._44 + tf._42;

	// near clipping plane: z >= 0, where rove's projections put depth 0 (it was OpenGL's -w <= z, about half the near distance off)
	planes[near_plane].A = tf._13;
	planes[near_plane].B = tf._23;
	planes[near_plane].C = tf._33;
	planes[near_plane].D = tf._43;

	// far clipping plane
	planes[far_plane].A = tf._14 - tf._13;
	planes[far_plane].B = tf._24 - tf._23;
	planes[far_plane].C = tf._34 - tf._33;
	planes[far_plane].D = tf._44 - tf._43;
}

template<class T>
bool frustum<T>::contains(vec_type const &point) const
{
	for (size_t i = 0; i < planes_count; i++)
	{
		// Note: point is vec<3> (float) but planes[i] is plane<T>
		// For T=double, we need to convert the point
		vec<3, T> point_type(point.x, point.y, point.z);
		typename plane_type::classification cl = planes[i].classify(point_type);
		if (cl == plane_type::negative) return false;
	}

	return true;
}

template<class T>
bool frustum<T>::test_intersection(aabb_type const &bounds) const
{
	for (size_t i = 0; i < planes_count; ++i)
	{
		if (planes[i].classify(bounds) == plane_type::negative) return false;
	}

	return true;
}

template<class T>
bool frustum<T>::test_intersection(obb_type const &bounds) const
{
	for (size_t i = 0; i < planes_count; ++i)
	{
		if (planes[i].classify(bounds) == plane_type::negative) return false;
	}

	return true;
}

template<class T>
bool frustum<T>::test_intersection(sphere<dimensions, T> const &bounds) const
{
	for (size_t i = 0; i < planes_count; ++i)
	{
		// the planes are not of unit length: the signed distance is apply() over the normal's length
		T const length = planes[i].normal().length();
		if (planes[i].apply(bounds.centre) < -bounds.radius * length) return false;
	}

	return true;
}

template class frustum<>;
template class frustum<double>;
#ifdef ROVE_DFLOAT
template class frustum<dfloat>;
#endif

}
