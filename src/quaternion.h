// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file quaternion.h
 * @brief Quaternion class for 3D rotations
 *
 * This file provides a quaternion class for representing and interpolating
 * 3D rotations. Quaternions avoid gimbal lock and provide smooth interpolation
 * via spherical linear interpolation (SLERP).
 *
 * A quaternion represents a rotation as q = xi + yj + zk + w, where w is the
 * scalar (real) part and (x,y,z) is the vector (imaginary) part.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include "scalar.h"
#include "vec.h"
#include "matrix.h"

namespace rove
{

/**
 * @brief Quaternion class for 3D rotations
 *
 * Represents a rotation in 3D space. A unit quaternion (norm = 1) represents
 * a pure rotation. Non-unit quaternions can represent scaled rotations.
 *
 * Components: (x, y, z) = vector part, w = scalar part
 */
template<class T = scalar>
class [[nodiscard]] quaternion {
public:
	T x, y, z, w;  ///< Quaternion components (vector part x,y,z and scalar part w)

	/// The identity: no rotation (it was left uninitialised, whatever the memory held)
	quaternion():
		x(0), y(0), z(0), w(1)
	{
	}

	quaternion(T x0, T y0, T z0, T w0) {
		x = x0; y = y0;	z = z0;	w = w0;
	}

	/// @brief Set to identity quaternion (no rotation)
	/// Identity quaternion is (0, 0, 0, 1)
	void identity() {
		x = 0; y = 0; z = 0; w = 1;
	}

	/// @brief Negate all components
	quaternion operator -() const {
		return quaternion<T>(-x, -y, -z, -w);
	}

	/// @brief Scale all components by scalar
	void scale(T s) {
		x *= s; y *= s; z *= s; w *= s;
	}

	/// @brief Multiply quaternion by scalar
	quaternion operator *(T s) const {
		quaternion<T> result(*this);
		result.scale(s);
		return result;
	}

	/// @brief Compute quaternion magnitude (length)
	/// @return √(x² + y² + z² + w²)
	T norm() const {
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		return sqrt(x*x + y*y + z*z + w*w);
	}

	/// @brief Normalize quaternion to unit length in place
	/// A quaternion of no length (norm at most scalar_traits<T>::zero_length) or not a number is std::invalid_argument: it has no
	/// rotation.  (It became the identity, so a lerp of opposite quaternions or a damaged key turned into no rotation at all.)
	void normalize() {
		T n = norm();
		using std::isfinite, std::to_string;
		if (!(n > scalar_traits<T>::zero_length) || !isfinite(n))
			throw std::invalid_argument("rove: normalize a quaternion of norm " + to_string(n));
		scale(1/n);
	}

	/// @brief Return normalized copy of quaternion
	/// @return Unit quaternion in same orientation
	quaternion normalized() const {
		quaternion q(*this);
		q.normalize();
		return q;
	}

	/**
	 * @brief Extract unit quaternion from rotation matrix
	 *
	 * Converts a rotation matrix to its quaternion representation, w >= 0.
	 *
	 * Shepperd's method: the largest of w, x, y, z comes from a square
	 * root of at least 1, the other three from off-diagonal sums and
	 * differences divided by it.  A square root per component would leave
	 * a near-zero one off by sqrt(epsilon), and reading the signs off the
	 * differences 4wx, 4wy, 4wz fails at a half turn, where w = 0 and
	 * every one of them is zero.
	 *
	 * @param m Rotation matrix (3x3 or 4x4)
	 */
	template<int N, int M, class X>
	void set_unit(matrix<N, M, X> const &m) {
		T const m00 = T(m.ij[0][0]), m01 = T(m.ij[0][1]), m02 = T(m.ij[0][2]);
		T const m10 = T(m.ij[1][0]), m11 = T(m.ij[1][1]), m12 = T(m.ij[1][2]);
		T const m20 = T(m.ij[2][0]), m21 = T(m.ij[2][1]), m22 = T(m.ij[2][2]);
		T const trace = m00 + m11 + m22;

		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		if (trace > 0) {
			T const s = 2 * sqrt(1 + trace);  // 4w
			w = s / 4;
			x = (m12 - m21) / s;
			y = (m20 - m02) / s;
			z = (m01 - m10) / s;
		} else if (m00 >= m11 && m00 >= m22) {
			T const s = 2 * sqrt(1 + m00 - m11 - m22);  // 4x
			x = s / 4;
			y = (m01 + m10) / s;
			z = (m02 + m20) / s;
			w = (m12 - m21) / s;
		} else if (m11 >= m22) {
			T const s = 2 * sqrt(1 - m00 + m11 - m22);  // 4y
			x = (m01 + m10) / s;
			y = s / 4;
			z = (m12 + m21) / s;
			w = (m20 - m02) / s;
		} else {
			T const s = 2 * sqrt(1 - m00 - m11 + m22);  // 4z
			x = (m02 + m20) / s;
			y = (m12 + m21) / s;
			z = s / 4;
			w = (m01 - m10) / s;
		}

		if (w < 0) {
			x = -x;
			y = -y;
			z = -z;
			w = -w;
		}
	}
};

/**
 * @brief Compute quaternion dot product
 * @return Scalar dot product (p·q)
 */
template<class T> inline T
dot(quaternion<T> const &p, quaternion<T> const &q) {
	return p.x*q.x + p.y*q.y + p.z*q.z + p.w*q.w;
}

/// @brief Add two quaternions component-wise
template<class T> inline quaternion<T>
operator +(quaternion<T> const &p, quaternion<T> const &q) {
	return quaternion<T>(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}

/// @brief Subtract two quaternions component-wise
template<class T> inline quaternion<T>
operator -(quaternion<T> const &p, quaternion<T> const &q) {
	return quaternion<T>(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}

/**
 * @brief Hamilton product: the rotation q, then the rotation p
 *
 * For unit quaternions, matrix(p * q) is matrix(p) times matrix(q).
 */
template<class T> inline quaternion<T>
operator *(quaternion<T> const &p, quaternion<T> const &q) {
	return quaternion<T>(
		p.w*q.x + p.x*q.w + p.y*q.z - p.z*q.y,
		p.w*q.y - p.x*q.z + p.y*q.w + p.z*q.x,
		p.w*q.z + p.x*q.y - p.y*q.x + p.z*q.w,
		p.w*q.w - p.x*q.x - p.y*q.y - p.z*q.z);
}

/// @brief The conjugate: for a unit quaternion, the opposite rotation
template<class T> inline quaternion<T>
conjugate(quaternion<T> const &q) {
	return quaternion<T>(-q.x, -q.y, -q.z, q.w);
}

/**
 * @brief The unit quaternion turning by `angle` (radians) about `axis`
 * @param axis Of any length, as rotation_matrix takes it; one of no length is std::invalid_argument
 */
template<class T, class S> inline quaternion<T>
axis_angle(vec<3, T, S> const &axis, T angle) {
	using std::sin, std::cos;
	T const s = sin(angle / 2);
	// (unit to within rounding is left as it is: what was right keeps its bits)
	vec<3, T, S> const unit = abs(axis.length_sq() - T(1)) <= T(1) / T(100000) ? axis : normalize(axis);
	return quaternion<T>(unit.x * s, unit.y * s, unit.z * s, cos(angle / 2));
}

/**
 * @brief The angle (0..pi) of the rotation that takes unit quaternion p to q
 *
 * q and -q are the same rotation: the angle is the same for either.  Taken from the relative rotation's parts (2 atan2(|xyz|, |w|)),
 * which holds its precision for small angles where 2 acos(p.q) does not.
 */
template<class T> inline T
angle_between(quaternion<T> const &p, quaternion<T> const &q) {
	quaternion<T> const r = q * conjugate(p);
	using std::atan2, std::sqrt, std::abs;
	return 2 * atan2(sqrt(r.x*r.x + r.y*r.y + r.z*r.z), abs(r.w));
}

namespace detail
{

/// q's squared norm, or std::invalid_argument naming `what` for a quaternion of no length or not a number: no rotation can be read
/// from it
template<class T> inline T
norm_sq_of_a_rotation(quaternion<T> const &q, char const *what) {
	T const n2 = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
	using std::isfinite, std::to_string;
	if (!(n2 > scalar_traits<T>::zero_length_sq) || !isfinite(n2))
		throw std::invalid_argument(std::string("rove: ") + what + " of a quaternion of squared norm " + to_string(n2));
	return n2;
}

/// The s of matrix::rotation(q)'s x2 = x s ...: 2 / |q|^2, the one that makes a rotation of a quaternion of any length.  Within a
/// thousandth of unit length it is the 2 / |q| it always was, so the matrices of unit quaternions keep their bits (a lockstep
/// simulation's recordings play back); further off, 2 / |q| made a matrix that was no rotation.
template<class T> inline T
rotation_scale(quaternion<T> const &q) {
	T const n2 = norm_sq_of_a_rotation(q, "the rotation matrix");
	if (abs(n2 - 1) <= T(1) / T(1000)) {
		using std::sqrt;
		return 2 / sqrt(n2);
	}
	return 2 / n2;
}

}

/**
 * @brief v turned by q (a quaternion of any length but none: q / |q|): q v q*
 *
 * As matrix<3,3>::rotation(q) * v.  std::invalid_argument for a quaternion of no length.
 */
template<class T, class S> inline vec<3, T, S>
rotate(quaternion<T> const &q, vec<3, T, S> const &v) {
	T const n2 = detail::norm_sq_of_a_rotation(q, "rotate a vector by");
	vec<3, T, S> const u(q.x, q.y, q.z);
	vec<3, T, S> const t = cross(u, v) * (2 / n2);
	return v + t * q.w + cross(u, t);
}

/// @brief rotate(q, v)
template<class T, class S> inline vec<3, T, S>
operator *(quaternion<T> const &q, vec<3, T, S> const &v) {
	return rotate(q, v);
}

/// @brief The opposite rotation of a quaternion of any length but none: conjugate / |q|^2
template<class T> inline quaternion<T>
inverse(quaternion<T> const &q) {
	T const n2 = detail::norm_sq_of_a_rotation(q, "the inverse");
	return conjugate(q) * (1 / n2);
}

/// @brief The 4x4 rotation of a quaternion of any length but none: matrix::rotation(q), returned
template<class T> inline matrix<4, 4, T>
rotation_matrix(quaternion<T> const &q) {
	matrix<4, 4, T> m;
	m.rotation(q);
	return m;
}

/// @brief The unit quaternion of a rotation matrix (3x3 or 4x4), w >= 0: set_unit(m), returned
template<int N, int M, class T> inline quaternion<T>
to_quaternion(matrix<N, M, T> const &m) {
	quaternion<T> q;
	q.set_unit(m);
	return q;
}

/**
 * @brief The shortest rotation turning direction `from` onto direction `to` (neither need be unit length)
 *
 * By the angle between them about their common perpendicular; opposite directions are half a turn about a perpendicular of `from`.
 * std::invalid_argument for a vector of no length.
 */
template<class T> inline quaternion<T>
rotation_between(vec<3, T> const &from, vec<3, T> const &to) {
	vec<3, T> const a = direction(from), b = direction(to);
	vec<3, T> axis = cross(a, b);
	T const across = axis.length();
	using std::atan2, std::abs;
	T const angle = atan2(across, dot(a, b));
	if (across > scalar_traits<T>::zero_length) {
		axis /= across;
	} else if (dot(a, b) > 0) {
		return quaternion<T>();
	} else {
		// opposite: any axis square to a will do; cross it with the basis vector it is least along
		axis = direction(cross(a, abs(a.x) < T(9) / T(10) ? vec<3, T>(1, 0, 0) : vec<3, T>(0, 1, 0)));
	}
	return axis_angle(axis, angle);
}

/**
 * @brief The axis (unit) and angle (0 to pi) of q's rotation, q of any length but none
 *
 * No rotation has every axis: it gives (1, 0, 0) and 0.  std::invalid_argument for a quaternion of no length.
 */
template<class T> inline void
to_axis_angle(quaternion<T> const &q, vec<3, T> &axis, T &angle) {
	detail::norm_sq_of_a_rotation(q, "the axis and angle");
	quaternion<T> u = q.normalized();
	if (u.w < 0) u = -u;
	using std::atan2, std::sqrt;
	T const s = sqrt(u.x*u.x + u.y*u.y + u.z*u.z);
	angle = 2 * atan2(s, u.w);
	axis = s > 0 ? vec<3, T>(u.x / s, u.y / s, u.z / s) : vec<3, T>(1, 0, 0);
}

/**
 * @brief The rotation vector of q (its log map): the axis times the angle, 0 to pi; the zero vector for no rotation
 *
 * It keeps its size for tiny turns (atan2, not acos).  std::invalid_argument for a quaternion of no length.
 */
template<class T> inline vec<3, T>
rotation_vector(quaternion<T> const &q) {
	detail::norm_sq_of_a_rotation(q, "the rotation vector");
	quaternion<T> u = q.normalized();
	if (u.w < 0) u = -u;
	using std::atan2, std::sqrt;
	T const s = sqrt(u.x*u.x + u.y*u.y + u.z*u.z);
	if (!(s > 0)) return vec<3, T>(0, 0, 0);
	T const k = 2 * atan2(s, u.w) / s;
	return vec<3, T>(u.x * k, u.y * k, u.z * k);
}

/// @brief The unit quaternion of a rotation vector (the exp map): a turn of |r| about r; no turn for the zero vector
template<class T> inline quaternion<T>
from_rotation_vector(vec<3, T> const &r) {
	T const angle = r.length();
	if (!(angle > 0)) return quaternion<T>();
	return axis_angle(r / angle, angle);
}

/**
 * @brief Spherical linear interpolation (SLERP) helper class
 *
 * Precomputes values for efficient repeated SLERP interpolation between
 * two quaternions. SLERP provides smooth, constant-velocity rotation
 * interpolation along the shortest arc on the 4D unit hypersphere.
 *
 * Usage:
 * @code
 * quaternion_slerper<float> slerper;
 * slerper.setup(start_rotation, end_rotation);
 * quaternion<float> halfway = slerper.interpolate(0.5f);
 * @endcode
 */
template<class T = scalar>
class quaternion_slerper {
public:
	/**
	 * @brief Setup interpolation between two quaternions
	 *
	 * Normalizes both quaternions and precomputes interpolation parameters.
	 * Automatically chooses shorter rotation path.
	 */
	void setup(quaternion<T> const &p, quaternion<T> const &q) {
		(q1_ = p).normalize();
		(q2_ = q).normalize();

		T cosine = dot(q1_, q2_);

		// If the dot product is negative, negate one quaternion to take the shorter path
		if (cosine < 0) {
			q2_ = -q2_;
			cosine = -cosine;
		}

		cosine = (std::min)(T(1), (std::max)(T(-1), cosine));
		if (cosine >= T(1) - scalar_traits<T>::zero_angle) {
			linear_ = true;
			omega_ = 0;
			return;
		}

		omega_ = T(acos(cosine));
		T const sin_omega = T(sin(omega_));
		using std::abs;
		if (abs(sin_omega) <= scalar_traits<T>::zero_angle) {
			linear_ = true;
			omega_ = 0;
			return;
		}

		linear_ = false;
		T const inv_sin_omega = T(1 / sin_omega);
		q1_.scale(inv_sin_omega);
		q2_.scale(inv_sin_omega);
	}

	/**
	 * @brief Interpolate between quaternions
	 * @param t Interpolation parameter [0,1], where 0=start, 1=end
	 * @param[out] result Interpolated quaternion
	 */
	void interpolate(T t, quaternion<T> &result) const {
		if (linear_) {
			result = q1_ * T(1 - t) + q2_ * t;
			result.normalize();
			return;
		}
		result = q1_ * T(sin((1 - t) * omega_)) + q2_ * T(sin(t*omega_));
	}

	/**
	 * @brief Interpolate between quaternions (returns value)
	 * @param t Interpolation parameter [0,1], where 0=start, 1=end
	 */
	quaternion<T> interpolate(T t) const {
		quaternion<T> result;
		interpolate(t, result);
		return result;
	}

private:
	quaternion<T> q1_, q2_;  ///< Normalized and scaled quaternions
	T omega_;                ///< Angle between quaternions
	bool linear_ = false;    ///< Fall back to normalized lerp for tiny angles
};

/// @brief The unit quaternion q's rotation taken `t` of the way (0: none, 1: all of it), about its own axis, along the shorter arc
template<class T> inline quaternion<T>
power(quaternion<T> const &q, T t) {
	quaternion<T> none;
	none.identity();
	quaternion_slerper<T> slerper;
	slerper.setup(none, q);
	return slerper.interpolate(t);
}

/// @brief Spherical interpolation from p (t = 0) to q (t = 1) along the shorter arc: quaternion_slerper's, in one call
template<class T> inline quaternion<T>
slerp(quaternion<T> const &p, quaternion<T> const &q, T t) {
	quaternion_slerper<T> slerper;
	slerper.setup(p, q);
	return slerper.interpolate(t);
}

/**
 * @brief Normalised linear interpolation from p to q along the shorter arc: q's sign turned to p's side, lerped, normalised
 *
 * Cheaper than slerp and the same arc, not at constant speed.  p and q of any length but none (each normalised first), so
 * halfway between q and -q, the same rotation, is that rotation and not zero.
 */
template<class T> inline quaternion<T>
nlerp(quaternion<T> const &p, quaternion<T> const &q, T t) {
	quaternion<T> const a = p.normalized();
	quaternion<T> b = q.normalized();
	if (dot(a, b) < 0) b = -b;
	return (a * (1 - t) + b * t).normalized();
}

}
