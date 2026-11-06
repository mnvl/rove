// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file transform.h
 * @brief trs: a transform kept as its translation, rotation and scale
 *
 * The matrix of a trs scales, then turns, then moves: T R S, as glTF's nodes and a skeleton's joints are.  Composing and inverting
 * stay a trs only where the result is one: a parent of uniform scale, or a child that does not turn (a stretch under a turn is a
 * shear).  Where it is not, they throw rather than hand back a trs that is some other transform.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include "matrix.h"
#include "quaternion.h"
#include "ray.h"
#include "scalar.h"
#include "vec.h"

namespace rove
{

template<class T = scalar>
class [[nodiscard]] trs {
public:
	vec<3, T> translation;
	quaternion<T> rotation;       ///< unit length (a turn), as the functions below keep it
	vec<3, T> scale;              ///< along the local axes, before the rotation; a negative one is a mirror

	/// No transform: no translation, no rotation, a scale of 1
	trs():
		translation(0, 0, 0),
		scale(1, 1, 1)
	{
	}

	trs(vec<3, T> const &t, quaternion<T> const &r, vec<3, T> const &s):
		translation(t),
		rotation(r),
		scale(s)
	{
	}

	/// @brief T R S as a 4x4 matrix
	rove::matrix<4, 4, T> to_matrix() const {
		rove::matrix<4, 4, T> m;
		m.rotation(rotation);
		for (int c = 0; c < 3; ++c)
			for (int r = 0; r < 3; ++r)
				m.ij[c][r] *= scale.i[c];
		m.ij[3][0] = translation.x;
		m.ij[3][1] = translation.y;
		m.ij[3][2] = translation.z;
		return m;
	}

	/// @brief The point scaled, turned and moved
	vec<3, T> apply_point(vec<3, T> const &p) const {
		return translation + rotate(rotation, scale * p);
	}

	/// @brief The vector scaled and turned, not moved
	vec<3, T> apply_vector(vec<3, T> const &v) const {
		return rotate(rotation, scale * v);
	}
};

namespace detail
{

template<class T> inline bool
uniform_scale(vec<3, T> const &s) {
	using std::abs;
	T const largest = (std::max)({abs(s.x), abs(s.y), abs(s.z)});
	T const tolerance = largest / T(10000);
	return abs(s.x - s.y) <= tolerance && abs(s.x - s.z) <= tolerance;
}

template<class T> inline bool
no_turn(quaternion<T> const &q) {
	using std::abs;
	T const tolerance = abs(q.w) / T(1000000);
	return abs(q.x) <= tolerance && abs(q.y) <= tolerance && abs(q.z) <= tolerance;
}

}

/**
 * @brief The trs whose matrix is m: translation from its last column, scale from the lengths of the others (x's negative for a mirror),
 * rotation from what is left
 *
 * std::invalid_argument for a matrix that is no T R S: a last row other than (0, 0, 0, 1) (a projection), columns not square to each
 * other (a shear), or a scale of no length.
 */
template<class T> inline trs<T>
to_trs(matrix<4, 4, T> const &m) {
	using std::abs, std::to_string;
	m.check_affine("rove: to_trs of a matrix");
	vec<3, T> column[3];
	T length[3];
	for (int c = 0; c < 3; ++c) {
		column[c] = vec<3, T>(m.ij[c][0], m.ij[c][1], m.ij[c][2]);
		length[c] = column[c].length();
		if (!(length[c] > scalar_traits<T>::zero_length))
			throw std::invalid_argument("rove: to_trs of a matrix with a scale of " + to_string(length[c]) + " on axis " + to_string(c));
	}
	T const square = T(1) / T(10000);
	for (int a = 0; a < 3; ++a)
		for (int b = a + 1; b < 3; ++b)
			if (abs(dot(column[a], column[b])) > square * length[a] * length[b])
				throw std::invalid_argument("rove: to_trs of a sheared matrix: columns " + to_string(a) + " and " + to_string(b) +
					" are not square to each other");
	if (dot(cross(column[0], column[1]), column[2]) < 0)
		length[0] = -length[0];
	matrix<3, 3, T> r;
	for (int c = 0; c < 3; ++c)
		for (int row = 0; row < 3; ++row)
			r.ij[c][row] = column[c].i[row] / length[c];
	return trs<T>(vec<3, T>(m.ij[3][0], m.ij[3][1], m.ij[3][2]), to_quaternion(r), vec<3, T>(length[0], length[1], length[2]));
}

/**
 * @brief a after b: the transform that applies b, then a (as a.to_matrix() * b.to_matrix())
 *
 * A trs where a's scale is uniform or b does not turn; otherwise the product is a shear, and std::invalid_argument.
 */
template<class T> inline trs<T>
operator *(trs<T> const &a, trs<T> const &b) {
	if (!detail::uniform_scale(a.scale) && !detail::no_turn(b.rotation))
		throw std::invalid_argument("rove: a trs of uneven scale after one that turns is a shear, no trs");
	return trs<T>(a.apply_point(b.translation), a.rotation * b.rotation, a.scale * b.scale);
}

/**
 * @brief The transform that undoes t
 *
 * A trs where t's scale is uniform or t does not turn; otherwise std::invalid_argument, as for a scale of zero.
 */
template<class T> inline trs<T>
inverse(trs<T> const &t) {
	for (int c = 0; c < 3; ++c)
		if (!(t.scale.i[c] != 0))
			throw std::invalid_argument("rove: the inverse of a trs with a scale of zero");
	if (!detail::uniform_scale(t.scale) && !detail::no_turn(t.rotation))
		throw std::invalid_argument("rove: the inverse of a trs of uneven scale that turns is a shear, no trs");
	quaternion<T> const r = inverse(t.rotation);
	vec<3, T> const s(1 / t.scale.x, 1 / t.scale.y, 1 / t.scale.z);
	return trs<T>(-rotate(r, s * t.translation), r, s);
}

/**
 * @brief The ray under a point of the screen: from the near plane (depth 0) at t = 0 to the far plane (depth 1) at t = 1
 *
 * `inverse_view_projection` is the inverse of projection * view; ndc_x and ndc_y are the point in normalised device coordinates, -1
 * to 1 (edge's: +y up).  A matrix that does not take the point back to a finite place (w of zero) is std::invalid_argument.
 */
template<class T> inline ray<3, T>
unproject_ray(matrix<4, 4, T> const &inverse_view_projection, T ndc_x, T ndc_y)
{
	vec<3, T> const at_near = project_point(inverse_view_projection, vec<3, T>(ndc_x, ndc_y, 0));
	vec<3, T> const at_far = project_point(inverse_view_projection, vec<3, T>(ndc_x, ndc_y, 1));
	return ray<3, T>(at_near, at_far - at_near);
}

/// @brief From a (u = 0) to b (u = 1): translation and scale lerped, rotation slerped
template<class T> inline trs<T>
lerp(trs<T> const &a, trs<T> const &b, T u) {
	return trs<T>(lerp(a.translation, b.translation, u), slerp(a.rotation, b.rotation, u), lerp(a.scale, b.scale, u));
}

}
