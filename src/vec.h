// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file vec.h
 * @brief Template vector classes for 2D, 3D, 4D and N-dimensional vectors
 *
 * This file provides highly optimized vector math classes with specializations
 * for common dimensions (2D, 3D, 4D). Vectors support flexible component access
 * through unions (xyz/ABC) and comprehensive arithmetic operations.
 *
 * What the components are called is the vector's storage, its third
 * parameter. The default is positions, so vec<3> and vec<3, double> are
 * what they have always been. A colour is a vector whose storage names its
 * components in the order they lie in memory: rgb<T>, bgr<T>, rgba<T>,
 * bgra<T>. The order is part of the type, so one order is not added to,
 * assigned to or taken for another. xz<T> is the ground plane of a world
 * that is Y up: a 2D vector named x and z, which is not taken for an xy one.
 *
 * v[n] is the component at position n in memory, whatever it is called; n
 * past the last component throws std::out_of_range.
 */

#pragma once

#include <iosfwd>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>
#include "scalar.h"

namespace rove
{

/**
 * @brief The components of a vector and their names
 *
 * A storage is a union of the components under each of its names and as the
 * array i[]. Every storage of 3 or 4 components has the positional names
 * x, y, z, w, which the vector's own members are written in; a colour
 * storage adds the colour names in the order of its memory, and has no
 * others. The 2D members are written in i[], so that xz, the ground plane,
 * can name its second component z and have no y. rebind<U> is the same
 * storage of another scalar.
 */
namespace storage
{

template<class T>
struct xy {
	template<class U> using rebind = xy<U>;

	union {
		struct {
			T x,y;  ///< Cartesian coordinates
		};

		struct {
			T u,v;  ///< Texture coordinates (alternative 1)
		};

		struct {
			T s,t;  ///< Texture coordinates (alternative 2)
		};

		struct {
			T A,B;  ///< Generic components
		};

		T i[2];  ///< Array access
	};
};

template<class T>
struct xz {
	template<class U> using rebind = xz<U>;

	union {
		struct {
			T x,z;  ///< The ground plane of a world that is Y up
		};

		T i[2];  ///< Array access
	};
};

template<class T>
struct xyz {
	template<class U> using rebind = xyz<U>;

	union {
		struct {
			T x,y,z;  ///< Cartesian coordinates
		};

		struct {
			T A,B,C;  ///< Generic components
		};

		T i[3];  ///< Array access
	};
};

template<class T>
struct rgb {
	template<class U> using rebind = rgb<U>;

	union {
		struct {
			T x,y,z;  ///< By position
		};

		struct {
			T r,g,b;  ///< Red first in memory
		};

		T i[3];  ///< Array access
	};
};

template<class T>
struct bgr {
	template<class U> using rebind = bgr<U>;

	union {
		struct {
			T x,y,z;  ///< By position
		};

		struct {
			T b,g,r;  ///< Blue first in memory
		};

		T i[3];  ///< Array access
	};
};

template<class T>
struct xyzw {
	template<class U> using rebind = xyzw<U>;

	union {
		struct {
			T x,y,z,w;  ///< Homogeneous coordinates
		};

		struct {
			T A,B,C,D;  ///< Generic components
		};

		T i[4];  ///< Array access
	};
};

template<class T>
struct rgba {
	template<class U> using rebind = rgba<U>;

	union {
		struct {
			T x,y,z,w;  ///< By position
		};

		struct {
			T r,g,b,a;  ///< Red first in memory
		};

		T i[4];  ///< Array access
	};
};

template<class T>
struct bgra {
	template<class U> using rebind = bgra<U>;

	union {
		struct {
			T x,y,z,w;  ///< By position
		};

		struct {
			T b,g,r,a;  ///< Blue first in memory
		};

		T i[4];  ///< Array access
	};
};

template<int N,class T>
struct array {
	template<class U> using rebind = array<N,U>;

	T i[N];
};

}

/// @brief The storage of vec<N,T> when none is given: positions
template<int N,class T> struct default_storage { using type = storage::array<N,T>; };
template<class T> struct default_storage<2,T> { using type = storage::xy<T>; };
template<class T> struct default_storage<3,T> { using type = storage::xyz<T>; };
template<class T> struct default_storage<4,T> { using type = storage::xyzw<T>; };

/// @brief Template vector class, specialized for N=2,3,4
template<int N,class T=scalar,class S=typename default_storage<N,T>::type> class vec;
template<class T> class quaternion;
template<class T,class S> class vec<2,T,S>;
template<class T,class S> class vec<3,T,S>;
template<class T,class S> class vec<4,T,S>;

/// @brief Colours, their components named in the order of their memory
template<class T> using rgb = vec<3,T,storage::rgb<T>>;
template<class T> using bgr = vec<3,T,storage::bgr<T>>;
template<class T> using rgba = vec<4,T,storage::rgba<T>>;
template<class T> using bgra = vec<4,T,storage::bgra<T>>;

/// @brief The ground plane of a world that is Y up: x and z
template<class T> using xz = vec<2,T,storage::xz<T>>;

namespace detail
{

/// n if it is a position in a vector or matrix of count, or std::out_of_range
inline size_t checked_index(size_t n, size_t count)
{
	if (n >= count)
		throw std::out_of_range("rove: index " + std::to_string(n) + " of " + std::to_string(count));
	return n;
}

}

/// @brief a · b
template<class T,class S> inline T dot(vec<2,T,S> const &a,vec<2,T,S> const &b);
template<class T,class S> inline T dot(vec<3,T,S> const &a,vec<3,T,S> const &b);
template<class T,class S> inline T dot(vec<4,T,S> const &a,vec<4,T,S> const &b);
template<int N,class T,class S> inline T dot(vec<N,T,S> const &a,vec<N,T,S> const &b);

/// @brief a × b
template<class T> inline vec<3,T> cross(vec<3,T> const &a,vec<3,T> const &b);

/**
 * @brief Swap components to ensure v1 <= v2 component-wise
 * @param v1 First vector (modified to contain minimums)
 * @param v2 Second vector (modified to contain maximums)
 */
template<class T> inline void minmax(vec<2,T> &v1,vec<2,T> &v2);
template<class T> inline void minmax(vec<3,T> &v1,vec<3,T> &v2);
template<class T> inline void minmax(vec<4,T> &v1,vec<4,T> &v2);

/**
 * @brief v made unit length
 *
 * std::invalid_argument for a vector of length zero or not finite, which has no direction (it came back as it was, no error);
 * normalize_or_zero(v) is that.  A vector however short but not zero is made unit length, bit for bit as before, where
 * direction(v) throws for one of zero_length or less.
 */
template<int N,class T,class S> inline vec<N,T,S> normalize(vec<N,T,S> const &v);

/// @brief v made unit length, as normalize(v) does however long or short, or v as it is where it has no length or is not a number
template<int N,class T,class S> inline vec<N,T,S> normalize_or_zero(vec<N,T,S> const &v);

/**
 * @brief The unit vector along v
 *
 * A vector of no length (scalar_traits<T>::zero_length or less) has no direction, and nor has one
 * that is not finite: asking for it is a bug in the caller, and throws
 * std::invalid_argument where normalize(v) would return v.
 */
template<int N,class T,class S> inline vec<N,T,S> direction(vec<N,T,S> const &v);

namespace detail
{

template<int N,class T,class S> void check_finite(vec<N,T,S> const &v, char const *what);
template<int N,class T,class S> void check_nonzero(vec<N,T,S> const &v, char const *what, T epsilon);

}

/**
 * @brief 2D vector class with flexible component access
 *
 * Provides a 2-component vector with multiple naming conventions:
 * - Cartesian: x, y
 * - Texture: u, v or s, t
 * - Generic: A, B
 * - Array: i[0], i[1]
 *
 * All component aliases reference the same memory through a union, the
 * storage (storage::xy unless another is given; storage::xz names them x, z).
 * The members go by position, so on the ground z is the second component.
 *
 * @tparam S Storage: the components and their names
 */
template<class T,class S>
class [[nodiscard]] vec<2,T,S>: public S {
public:
	static constexpr size_t dimensions = 2; ///< Number of components (2)

	using S::i;

	/// Default constructor - initializes to zero vector
	vec() {
		i[0] = i[1] = 0;
	}

	/// The same vector of another scalar type, each component cast
	template<class type>
	explicit vec(vec<dimensions,type,typename S::template rebind<type>> const &v)
	{
		i[0] = T(v.i[0]);
		i[1] = T(v.i[1]);
	}

	vec(T x0,T y0)
	{
		i[0] = x0;
		i[1] = y0;
	}

	void set(T x0, T y0)
	{
		i[0] = x0;
		i[1] = y0;
	}

	/// @brief The component at position n; n past the last one throws std::out_of_range
	T &operator [](size_t n) { return i[detail::checked_index(n, dimensions)]; }
	T const &operator [](size_t n) const { return i[detail::checked_index(n, dimensions)]; }

	void set_all(T a)
	{
		set(a, a);
	}

	vec operator -() const
	{
		return vec(-i[0], -i[1]);
	}

	vec &operator *=(T const k)
	{
		i[0] *= k;
		i[1] *= k;
		return *this;
	}

	vec &operator /=(T const k)
	{
		i[0] /= k;
		i[1] /= k;
		return *this;
	}

	vec &operator +=(vec const &v)
	{
		i[0] += v.i[0];
		i[1] += v.i[1];
		return *this;
	}

	vec &operator -=(vec const &v)
	{
		i[0] -= v.i[0];
		i[1] -= v.i[1];
		return *this;
	}

	vec &operator *=(vec const &v)
	{
		i[0] *= v.i[0];
		i[1] *= v.i[1];
		return *this;
	}

	vec &operator /=(vec const &v)
	{
		i[0] /= v.i[0];
		i[1] /= v.i[1];
		return *this;
	}

	vec operator +(vec const &v) const
	{
		return vec(i[0] + v.i[0], i[1] + v.i[1]);
	}

	vec operator -(vec const &v) const
	{
		return vec(i[0] - v.i[0], i[1] - v.i[1]);
	}

	vec operator *(vec const &v) const
	{
		return vec(i[0] * v.i[0], i[1] * v.i[1]);
	}

	vec operator /(vec const &v) const
	{
		return vec(i[0] / v.i[0], i[1] / v.i[1]);
	}

	vec operator *(T const k) const
	{
		return vec(i[0] * k, i[1] * k);
	}

	vec operator /(T const k) const
	{
		return vec(i[0] / k, i[1] / k);
	}

	bool operator ==(vec const &rhs) const
	{
		return i[0] == rhs.i[0] && i[1] == rhs.i[1];
	}


	/// @brief Squared length (magnitude) of vector
	/// @return x² + y² (avoids square root for performance)
	T length_sq() const
	{
		return i[0] * i[0] + i[1] * i[1];
	}

	/// @brief Euclidean length (magnitude) of vector
	/// @return √(x² + y²)
	T length() const
	{
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		return sqrt(length_sq());
	}

	/// @brief Manhattan/taxicab length: |i[0]| + |i[1]|
	T manhattan_length() const
	{
		return abs(i[0]) + abs(i[1]);
	}

	/// @brief Normalize vector to unit length in place: *this = normalize(*this), which throws for a vector of no length
	void normalize()
	{
		*this = rove::normalize(*this);
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where a component is NaN or infinite
	vec const &check_finite(char const *what) const
	{
		detail::check_finite(*this, what);
		return *this;
	}

	/// @brief *this, or std::invalid_argument where it has no direction: `what` and " that is not finite", or `what`, " of length "
	/// and the length where that is epsilon or less
	vec const &check_nonzero(char const *what, T epsilon = scalar_traits<T>::zero_length) const
	{
		detail::check_nonzero(*this, what, epsilon);
		return *this;
	}

	/// @brief Negate all components in place
	void negate()
	{
		i[0] = -i[0];
		i[1] = -i[1];
	}

	/// @brief Test if two vectors are collinear (parallel)
	/// @param epsilon Tolerance for floating point comparison
	/// @return true if the cross product's magnitude is within epsilon
	bool is_collinear(vec const &v, T epsilon = scalar_traits<T>::default_epsilon) const
	{
		return abs(i[0]*v.i[1] - i[1]*v.i[0]) <= epsilon;
	}

	/// @brief Convert to polar coordinates
	/// @param[out] modulus Length of vector
	/// @param[out] angle Angle in radians from the first axis towards the second
	void to_polar(T &modulus, T &angle) const
	{
		modulus = length();
		angle = atan2(i[1], i[0]);
	}

	/// @brief Perpendicular vector: this one turned a quarter clockwise, (x, y) to (y, -x)
	/// @return Vector perpendicular to this one, of the same length
	vec perpendicular() const { return vec(i[1], -i[0]); }

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & i[0] & i[1];
	}
};

/**
 * @brief 3D vector class with flexible component access
 *
 * Provides a 3-component vector with multiple naming conventions:
 * - Cartesian: x, y, z
 * - Generic: A, B, C
 * - Array: i[0], i[1], i[2]
 *
 * All component aliases reference the same memory through a union, the
 * storage (storage::xyz unless another is given: rgb<T> and bgr<T> are
 * the colours).
 * Supports cross product operations and 3D transformations.
 *
 * @tparam S Storage: the components and their names
 */
template<class T,class S>
class [[nodiscard]] vec<3,T,S>: public S {
public:
	static constexpr size_t dimensions = 3; ///< Number of components (3)
	using vec2_t = vec<2, T>; ///< Associated 2D vector type

	using S::x; using S::y; using S::z; using S::i;

	vec()
	{
		x = y = z = 0;
	}

	vec(vec<dimensions - 1, T> const &v, T const z0 = 0)
	{
		x = v.x;
		y = v.y;
		z = z0;
	}

	/// The same vector of another scalar type, each component cast
	template<class type>
	explicit vec(vec<dimensions,type,typename S::template rebind<type>> const &v)
	{
		x = T(v.x);
		y = T(v.y);
		z = T(v.z);
	}

	vec(T x0,T y0,T z0)
	{
		x = x0;
		y = y0;
		z = z0;
	}

	void set(T x0,T y0,T z0)
	{
		x = x0;
		y = y0;
		z = z0;
	}

	/// @brief The component at position n; n past the last one throws std::out_of_range
	T &operator [](size_t n) { return i[detail::checked_index(n, dimensions)]; }
	T const &operator [](size_t n) const { return i[detail::checked_index(n, dimensions)]; }

	void set_all(T a)
	{
		set(a, a, a);
	}

	vec operator -() const
	{
		return vec(-x, -y, -z);
	}

	vec &operator *=(T const k)
	{
		x *= k;
		y *= k;
		z *= k;
		return *this;
	}

	vec &operator /=(T const k)
	{
		x /= k;
		y /= k;
		z /= k;
		return *this;
	}

	vec &operator +=(vec const &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	vec &operator -=(vec const &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	vec &operator *=(vec const &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	vec &operator /=(vec const &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	vec operator +(vec const &v) const
	{
		return vec(x + v.x, y + v.y, z + v.z);
	}

	vec operator -(vec const &v) const
	{
		return vec(x - v.x, y - v.y, z - v.z);
	}

	vec operator *(vec const &v) const
	{
		return vec(x * v.x, y * v.y, z * v.z);
	}

	vec operator /(vec const &v) const
	{
		return vec(x / v.x, y / v.y, z / v.z);
	}

	vec operator *(T const k) const
	{
		return vec(x * k, y * k, z * k);
	}

	vec operator /(T const k) const
	{
		return vec(x / k, y / k, z / k);
	}

	bool operator ==(vec const &rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}


	T length_sq() const
	{
		return x * x + y * y + z * z;
	}

	T length() const
	{
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		return sqrt(length_sq());
	}

	/// @brief Manhattan/taxicab length: |x| + |y| + |z|
	T manhattan_length() const
	{
		return abs(x) + abs(y) + abs(z);
	}

	/// @brief *this = normalize(*this), which throws for a vector of no length
	void normalize()
	{
		*this = rove::normalize(*this);
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where a component is NaN or infinite
	vec const &check_finite(char const *what) const
	{
		detail::check_finite(*this, what);
		return *this;
	}

	/// @brief *this, or std::invalid_argument where it has no direction: `what` and " that is not finite", or `what`, " of length "
	/// and the length where that is epsilon or less
	vec const &check_nonzero(char const *what, T epsilon = scalar_traits<T>::zero_length) const
	{
		detail::check_nonzero(*this, what, epsilon);
		return *this;
	}

	void negate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	/// @brief Convert rotation vector to axis-angle representation: the vector's direction, and its length
	/// @param[out] axis Rotation axis (normalized; (1, 0, 0) for no rotation, which has none of its own)
	/// @param[out] angle Rotation angle in radians, 0 or more
	void to_axis_angle(vec &axis, T &angle) const
	{
		angle = length();
		axis = (angle > T(0)) ? rove::normalize_or_zero(*this) : vec(T(1), T(0), T(0));
	}

	/// @brief Test if two 3D vectors are collinear (parallel)
	/// @param epsilon Tolerance for floating point comparison
	/// @return true if the cross product's magnitude is within epsilon
	bool is_collinear(vec const &v, T epsilon = scalar_traits<T>::default_epsilon) const
	{
		return rove::cross(*this, v).length_sq() <= epsilon * epsilon;
	}

	/// @brief The first two components, x and y
	vec2_t xy() const { return vec2_t(x, y); }

	/// @brief Get vector perpendicular in XY plane
	vec perpendicular_xy() const
	{
		if (abs(x) + abs(y) < scalar_traits<T>::zero_length) return vec(0, 0, 1);
		return vec(y, -x, 0);
	}

	/// @brief Get vector perpendicular in YZ plane
	vec perpendicular_yz() const
	{
		if (abs(y) + abs(z) < scalar_traits<T>::zero_length) return vec(1, 0, 0);
		return vec(0, z, -y);
	}

	/// @brief Get vector perpendicular in XZ plane
	vec perpendicular_xz() const
	{
		if (abs(x) + abs(z) < scalar_traits<T>::zero_length) return vec(0, 1, 0);
		return vec(z, 0, -x);
	}

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & x & y & z;
	}
};

/**
 * @brief 4D vector class with flexible component access
 *
 * Provides a 4-component vector with multiple naming conventions:
 * - Homogeneous coordinates: x, y, z, w
 * - Generic: A, B, C, D
 * - Array: i[0], i[1], i[2], i[3]
 *
 * All component aliases reference the same memory through a union, the
 * storage (storage::xyzw unless another is given: rgba<T> and bgra<T> are
 * the colours).
 * Commonly used for homogeneous coordinates in 3D graphics.
 *
 * @tparam S Storage: the components and their names
 */
template<class T,class S>
class [[nodiscard]] vec<4,T,S>: public S {
public:
	static constexpr size_t dimensions = 4; ///< Number of components (4)

	using S::x; using S::y; using S::z; using S::w; using S::i;

	vec() {
		x = y = z = w = 0;
	}

	/// The same vector of another scalar type, each component cast
	template<class type>
	explicit vec(vec<dimensions,type,typename S::template rebind<type>> const &v)
	{
		x = T(v.x);
		y = T(v.y);
		z = T(v.z);
		w = T(v.w);
	}

	vec(T x0,T y0,T z0,T w0)
	{
		x = x0;
		y = y0;
		z = z0;
		w = w0;
	}

	/// (v, w0): a point or direction made homogeneous, as vec<3>(vec<2>, z) does for vec<2>
	vec(vec<dimensions - 1, T> const &v, T const w0)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = w0;
	}

	/// @brief The first three components, x, y and z
	vec<dimensions - 1, T> xyz() const { return vec<dimensions - 1, T>(x, y, z); }

	vec operator -() const
	{
		return vec(-x, -y, -z, -w);
	}

	void set(T x0,T y0,T z0,T w0) {
		x = x0;
		y = y0;
		z = z0;
		w = w0;
	}

	/// @brief The component at position n; n past the last one throws std::out_of_range
	T &operator [](size_t n) { return i[detail::checked_index(n, dimensions)]; }
	T const &operator [](size_t n) const { return i[detail::checked_index(n, dimensions)]; }

	void set_all(T a)
	{
		set(a, a, a, a);
	}

	vec &operator *=(T const k)
	{
		x *= k;
		y *= k;
		z *= k;
		w *= k;
		return *this;
	}

	vec &operator /=(T const k)
	{
		x /= k;
		y /= k;
		z /= k;
		w /= k;
		return *this;
	}

	vec &operator +=(vec const &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	vec &operator -=(vec const &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	vec &operator *=(vec const &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	vec &operator /=(vec const &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	vec operator +(vec const &rhs) const
	{
		return vec(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	vec operator -(vec const &rhs) const
	{
		return vec(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	vec operator *(vec const &rhs) const
	{
		return vec(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
	}

	vec operator /(vec const &rhs) const
	{
		return vec(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
	}

	vec operator *(T const k) const
	{
		return vec(x * k, y * k, z * k, w * k);
	}

	vec operator /(T const k) const
	{
		return vec(x / k, y / k, z / k, w / k);
	}

	bool operator ==(vec const &rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}


	T length_sq() const
	{
		return x * x + y * y + z * z + w * w;
	}

	T length() const
	{
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		return sqrt(length_sq());
	}

	/// @brief Manhattan/taxicab length: |x| + |y| + |z| + |w|
	T manhattan_length() const
	{
		return abs(x) + abs(y) + abs(z) + abs(w);
	}

	/// @brief *this = normalize(*this), which throws for a vector of no length
	void normalize()
	{
		*this = rove::normalize(*this);
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where a component is NaN or infinite
	vec const &check_finite(char const *what) const
	{
		detail::check_finite(*this, what);
		return *this;
	}

	/// @brief *this, or std::invalid_argument where it has no direction: `what` and " that is not finite", or `what`, " of length "
	/// and the length where that is epsilon or less
	vec const &check_nonzero(char const *what, T epsilon = scalar_traits<T>::zero_length) const
	{
		detail::check_nonzero(*this, what, epsilon);
		return *this;
	}

	void negate()
	{
		x = -x;
		y = -y;
		z = -z;
		w = -w;
	}

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & x & y & z & w;
	}
};

template<int N,class T,class S>
class [[nodiscard]] vec: public S {
public:
	static constexpr size_t dimensions = N;

	using S::i;

	/// @brief The zero vector, as of any other length
	vec()
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] = T(0);
	}

	/// @brief Every component, in order
	template<class... A> requires (sizeof...(A) == size_t(N) && (std::is_convertible_v<A, T> && ...))
	vec(A... components)
	{
		T const values[] = {T(components)...};
		for (size_t n = 0; n < dimensions; n++)
			i[n] = values[n];
	}

	/// @brief The component at position n; n past the last one throws std::out_of_range
	T &operator [](size_t n) { return i[detail::checked_index(n, dimensions)]; }
	T const &operator [](size_t n) const { return i[detail::checked_index(n, dimensions)]; }

	void set_all(T a)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] = a;
	}

	vec operator -() const
	{
		vec r(*this);
		for (size_t n = 0; n < dimensions; n++)
			r.i[n] = -r.i[n];
		return r;
	}

	vec operator +(vec const &v) const { vec r(*this); r += v; return r; }
	vec operator -(vec const &v) const { vec r(*this); r -= v; return r; }
	vec operator *(vec const &v) const { vec r(*this); r *= v; return r; }
	vec operator /(vec const &v) const { vec r(*this); r /= v; return r; }
	vec operator *(T const k) const { vec r(*this); r *= k; return r; }
	vec operator /(T const k) const { vec r(*this); r /= k; return r; }

	/// @brief Every component equal (!= is its negation, as C++20 writes it)
	bool operator ==(vec const &rhs) const
	{
		for (size_t n = 0; n < dimensions; n++)
			if (!(i[n] == rhs.i[n])) return false;
		return true;
	}

	vec &operator *=(T const k)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] *= k;
		return *this;
	}

	vec &operator /=(T const k)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] /= k;
		return *this;
	}

	template<class X>
	vec &operator =(vec<dimensions, X> const &v)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] = T(v.i[n]);
		return *this;
	}

	vec &operator +=(vec const &v)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] += v.i[n];
		return *this;
	}

	vec &operator -=(vec const &v)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] -= v.i[n];
		return *this;
	}

	vec &operator *=(vec const &v)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] *= v.i[n];
		return *this;
	}

	vec &operator /=(vec const &v)
	{
		for (size_t n = 0; n < dimensions; n++)
			i[n] /= v.i[n];
		return *this;
	}

	/// @brief Every component within epsilon of zero
	bool is_zero(T epsilon = scalar_traits<T>::default_epsilon) const
	{
		for (size_t n = 0; n < dimensions; n++)
			if (abs(i[n]) > epsilon) return false;
		return true;
	}

	T length_sq() const
	{
		T l = i[0] * i[0];
		for (size_t n = 1; n < dimensions; n++)
			l += i[n] * i[n];
		return l;
	}

	T length() const
	{
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		return sqrt(length_sq());
	}

	/// @brief Manhattan/taxicab length: |i[0]| + |i[1]| + ...
	T manhattan_length() const
	{
		T l = 0;
		for (size_t n = 0; n < dimensions; n++)
			l += abs(i[n]);
		return l;
	}

	/// @brief *this = normalize(*this), which throws for a vector of no length
	void normalize()
	{
		*this = rove::normalize(*this);
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where a component is NaN or infinite
	vec const &check_finite(char const *what) const
	{
		detail::check_finite(*this, what);
		return *this;
	}

	/// @brief *this, or std::invalid_argument where it has no direction: `what` and " that is not finite", or `what`, " of length "
	/// and the length where that is epsilon or less
	vec const &check_nonzero(char const *what, T epsilon = scalar_traits<T>::zero_length) const
	{
		detail::check_nonzero(*this, what, epsilon);
		return *this;
	}
};

// The components and nothing else, aligned as the scalar: an array of vectors is an array of scalars, as numpy and a GPU read it.
// (They were packed to an alignment of 1, which misaligned a vec<3, double> in a struct and gained no byte.)
static_assert(sizeof(vec<2>) == 2 * sizeof(float) && alignof(vec<2>) == alignof(float));
static_assert(sizeof(vec<3>) == 3 * sizeof(float) && alignof(vec<3>) == alignof(float));
static_assert(sizeof(vec<4>) == 4 * sizeof(float) && alignof(vec<4>) == alignof(float));
static_assert(sizeof(vec<3, double>) == 3 * sizeof(double) && alignof(vec<3, double>) == alignof(double));
static_assert(sizeof(rgba<std::uint8_t>) == 4 && sizeof(vec<5>) == 5 * sizeof(float));

template<class L,class R> inline bool
lexicographical_less(vec<2,L> const &left,vec<2,R> const &right)
{
	if (left.x < right.x) return true;
	if (left.x > right.x) return false;

	return left.y < right.y;
}

template<class L,class R> inline bool
lexicographical_less(vec<3,L> const &left,vec<3,R> const &right)
{
	if (left.x < right.x) return true;
	if (left.x > right.x) return false;

	if (left.y < right.y) return true;
	if (left.y > right.y) return false;

	return left.z < right.z;
}

template<class L,class R> inline bool
lexicographical_less(vec<4,L> const &left,vec<4,R> const &right)
{
	if (left.x < right.x) return true;
	if (left.x > right.x) return false;

	if (left.y < right.y) return true;
	if (left.y > right.y) return false;

	if (left.z < right.z) return true;
	if (left.z > right.z) return false;

	return left.w < right.w;
}

template<int N,class L,class R> inline bool
lexicographical_greater(vec<N,L> const &left,vec<N,R> const &right)
{
	return lexicographical_less(right, left);
}

template<class T,class S> T
dot(vec<2,T,S> const &a,vec<2,T,S> const &b)
{
	return a.i[0]*b.i[0] + a.i[1]*b.i[1];
}

template<class T,class S> T
dot(vec<3,T,S> const &a,vec<3,T,S> const &b)
{
	return a.i[0]*b.i[0] + a.i[1]*b.i[1] + a.i[2]*b.i[2];
}

template<class T,class S> T
dot(vec<4,T,S> const &a,vec<4,T,S> const &b)
{
	return a.i[0]*b.i[0] + a.i[1]*b.i[1] + a.i[2]*b.i[2] + a.i[3]*b.i[3];
}

template<int N,class T,class S> T
dot(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	T sum = a.i[0] * b.i[0];
	for (int i=1; i<N; i++)
		sum += a.i[i] * b.i[i];
	return sum;
}

/// @brief a × b
template<class T> vec<3,T>
cross(vec<3,T> const &a,vec<3,T> const &b)
{
	vec<3,T> u;
	u.x = a.y * b.z - a.z * b.y;
	u.y = a.z * b.x - a.x * b.z;
	u.z = a.x * b.y - a.y * b.x;
	return u;
}

template<class T> inline void
minmax(vec<2,T> &v1,vec<2,T> &v2)
{
	if (v1.x>v2.x) std::swap(v1.x,v2.x);
	if (v1.y>v2.y) std::swap(v1.y,v2.y);
}

template<class T> inline void
minmax(vec<3,T> &v1,vec<3,T> &v2)
{
	if (v1.x>v2.x) std::swap(v1.x,v2.x);
	if (v1.y>v2.y) std::swap(v1.y,v2.y);
	if (v1.z>v2.z) std::swap(v1.z,v2.z);
}

template<class T> inline void
minmax(vec<4,T> &v1,vec<4,T> &v2)
{
	if (v1.x>v2.x) std::swap(v1.x,v2.x);
	if (v1.y>v2.y) std::swap(v1.y,v2.y);
	if (v1.z>v2.z) std::swap(v1.z,v2.z);
	if (v1.w>v2.w) std::swap(v1.w,v2.w);
}

namespace detail
{

/// What normalize() and direction() do past their one test of a length: std::invalid_argument for a vector of no direction (as
/// check_nonzero), or else one so long that its square overflows, scaled down by its largest component so that it does not.  Out
/// of their way, so that the common case keeps the vector in registers.
template<int N,class T,class S> vec<N,T,S>
scaled_down(vec<N,T,S> const &v, char const *what, T epsilon)
{
	using std::abs;
	T largest = abs(v.i[0]);
	for (int n = 1; n < N; ++n)
		largest = std::max(largest, abs(v.i[n]));
	// (with no epsilon only a true zero has no direction: a vector whose square underflows has one, by its largest component)
	using std::isfinite;
	if (epsilon > T(0) || !(largest > T(0) && isfinite(largest)))
		v.check_nonzero(what, epsilon);
	vec<N,T,S> t(v);
	for (int n = 0; n < N; ++n)
		t.i[n] = v.i[n] / largest;
	return t;
}

}

template<int N,class T,class S> vec<N,T,S>
normalize(vec<N,T,S> const &v)
{
	using std::isfinite;
	T const l = v.length();
	if (!(l > 0 && isfinite(l))) {       // no length, not a number, or so long that its square overflows
		vec<N,T,S> t = detail::scaled_down(v, "rove: normalize a vector", T(0));
		t /= t.length();
		return t;
	}
	vec<N,T,S> t(v);
	t /= l;
	return t;
}

template<int N,class T,class S> vec<N,T,S>
normalize_or_zero(vec<N,T,S> const &v)
{
	vec<N,T,S> t(v);
	T l = t.length();
	using std::isfinite;
	if (l > 0 && isfinite(l)) {
		t /= l;
		return t;
	}
	// its square underflowed to no length, or overflowed: by its largest component, it has a length.  Zero and NaN stay as they are.
	T largest = T(0);
	for (int n = 0; n < N; ++n)
		largest = std::max(largest, T(abs(v.i[n])));
	if (!(largest > 0 && isfinite(largest))) {
		if (abs(l) > 0) t /= l;
		return t;
	}
	for (int n = 0; n < N; ++n)
		t.i[n] = v.i[n] / largest;
	t /= t.length();
	return t;
}

/// @brief Whether every component is a number and not infinite
template<int N,class T,class S> inline bool
is_finite(vec<N,T,S> const &v)
{
	using std::isfinite;
	for (int n = 0; n < N; ++n)
		if (!isfinite(v.i[n])) return false;
	return true;
}

namespace detail
{

template<int N,class T,class S> void
check_finite(vec<N,T,S> const &v, char const *what)
{
	if (!is_finite(v))
		throw std::invalid_argument(std::string(what) + " that is not finite");
}

// A vector so long that its square overflows is not short.
template<int N,class T,class S> void
check_nonzero(vec<N,T,S> const &v, char const *what, T epsilon)
{
	check_finite(v, what);
	T const l = v.length();
	if (!(l > epsilon)) {
		using std::to_string;
		throw std::invalid_argument(std::string(what) + " of length " + to_string(l));
	}
}

}

/// @brief v made unit length; std::invalid_argument for a vector that is not finite or too short (zero_length) to have a direction
template<int N,class T,class S> vec<N,T,S>
direction(vec<N,T,S> const &v)
{
	using std::isfinite;
	T const l = v.length();
	if (!(l > scalar_traits<T>::zero_length && isfinite(l))) {
		vec<N,T,S> t = detail::scaled_down(v, "rove: direction of a vector", scalar_traits<T>::zero_length);
		t *= 1 / t.length();
		return t;
	}
	vec<N,T,S> t(v);
	t *= 1 / l;          // one division for all the components; within a rounding of normalize(v)
	return t;
}

/// @brief |v|: v.length() as a free function, like normalize(v)
template<int N,class T,class S> inline T
length(vec<N,T,S> const &v)
{
	return v.length();
}

/// @brief |v|²: v.length_sq() as a free function, like length(v)
template<int N,class T,class S> inline T
length_sq(vec<N,T,S> const &v)
{
	return v.length_sq();
}

/// @brief |a - b|²
template<int N,class T,class S> inline T
distance_sq(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	vec<N,T,S> d(a);
	d -= b;
	return d.length_sq();
}

/// @brief |a - b|
template<int N,class T,class S> inline T
distance(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	vec<N,T,S> d(a);
	d -= b;
	return d.length();
}

/// @brief a + (b - a) * t, component by component, as lerp() of scalars
template<int N,class T,class S> inline vec<N,T,S>
lerp(vec<N,T,S> const &a,vec<N,T,S> const &b,T t)
{
	vec<N,T,S> r(b);
	r -= a;
	r *= t;
	r += a;
	return r;
}

/// @name Every component compared: whether each of a's is less (and so on) than b's.  This is no order, so a vector has no
/// operator <, <=, > or >=: std::less of a vector is lexicographic (below), which a std::set or std::map takes by default and
/// std::sort is given.
/// @{
template<int N,class T,class S> inline bool
all_less(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	for (int n = 0; n < N; ++n)
		if (!(a.i[n] < b.i[n])) return false;
	return true;
}

template<int N,class T,class S> inline bool
all_less_equal(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	for (int n = 0; n < N; ++n)
		if (!(a.i[n] <= b.i[n])) return false;
	return true;
}

template<int N,class T,class S> inline bool
all_greater(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	return all_less(b, a);
}

template<int N,class T,class S> inline bool
all_greater_equal(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	return all_less_equal(b, a);
}
/// @}

/// @brief The smaller of a's and b's on each component (a's where they are equal or unordered, as std::min)
template<int N,class T,class S> inline vec<N,T,S>
min(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	vec<N,T,S> r(a);
	for (int n = 0; n < N; ++n)
		if (b.i[n] < a.i[n]) r.i[n] = b.i[n];
	return r;
}

/// @brief The larger of a's and b's on each component (a's where they are equal or unordered, as std::max)
template<int N,class T,class S> inline vec<N,T,S>
max(vec<N,T,S> const &a,vec<N,T,S> const &b)
{
	vec<N,T,S> r(a);
	for (int n = 0; n < N; ++n)
		if (a.i[n] < b.i[n]) r.i[n] = b.i[n];
	return r;
}

/// @brief |v| on each component
template<int N,class T,class S> inline vec<N,T,S>
abs(vec<N,T,S> const &v)
{
	vec<N,T,S> r(v);
	for (int n = 0; n < N; ++n)
		r.i[n] = abs(v.i[n]);
	return r;
}

/// @brief Each component of v into [lo, hi] of its own.  (clamp() of vectors took the vector for its T, and std::clamp's
/// every-component <: a vector neither all below lo nor all above hi came back as it was.)
template<int N,class T,class S> inline vec<N,T,S>
clamp(vec<N,T,S> const &v,vec<N,T,S> const &lo,vec<N,T,S> const &hi)
{
	vec<N,T,S> r(v);
	for (int n = 0; n < N; ++n)
		r.i[n] = std::clamp(v.i[n], lo.i[n], hi.i[n]);
	return r;
}

/// @brief Each component of v into [lo, hi]
template<int N,class T,class S> inline vec<N,T,S>
clamp(vec<N,T,S> const &v,T lo,T hi)
{
	vec<N,T,S> r(v);
	for (int n = 0; n < N; ++n)
		r.i[n] = std::clamp(v.i[n], lo, hi);
	return r;
}

namespace detail
{

/// v's squared length, or std::invalid_argument naming `what` where it is too short to have a direction
template<int N,class T,class S> inline T
length_sq_of_a_direction(vec<N,T,S> const &v, char const *what)
{
	T const l2 = v.length_sq();
	using std::isfinite, std::to_string;
	if (!(l2 > scalar_traits<T>::zero_length_sq) || !isfinite(l2))
		throw std::invalid_argument(std::string("rove: ") + what + " of squared length " + to_string(l2));
	return l2;
}

}

/// @brief The part of v along `onto` (any length but none); std::invalid_argument for onto of no length
template<int N,class T,class S> inline vec<N,T,S>
project(vec<N,T,S> const &v,vec<N,T,S> const &onto)
{
	T const l2 = detail::length_sq_of_a_direction(onto, "project onto a vector");
	return onto * (dot(v, onto) / l2);
}

/// @brief The part of v across `onto`: v - project(v, onto)
template<int N,class T,class S> inline vec<N,T,S>
reject(vec<N,T,S> const &v,vec<N,T,S> const &onto)
{
	return v - project(v, onto);
}

/// @brief v bounced off a surface of unit normal n: its part along n turned round
template<int N,class T,class S> inline vec<N,T,S>
reflect(vec<N,T,S> const &v,vec<N,T,S> const &n)
{
	return v - n * (2 * dot(v, n));
}

/// @brief The angle between a and b, 0 to pi, from atan2 of their cross and dot products: it holds its precision near 0 and pi, where
/// acos of the normalised dot does not.  std::invalid_argument for a vector of no length.
template<class T> inline T
angle_between(vec<3,T> const &a,vec<3,T> const &b)
{
	detail::length_sq_of_a_direction(a, "angle between vectors, the first");
	detail::length_sq_of_a_direction(b, "angle between vectors, the second");
	using std::atan2;
	return atan2(cross(a, b).length(), dot(a, b));
}

template<class T> inline T
angle_between(vec<2,T> const &a,vec<2,T> const &b)
{
	detail::length_sq_of_a_direction(a, "angle between vectors, the first");
	detail::length_sq_of_a_direction(b, "angle between vectors, the second");
	using std::atan2;
	return atan2(abs(a.x * b.y - a.y * b.x), dot(a, b));
}

/// @brief The turn about `axis` (right-handed, -pi to pi) that takes a's direction to b's, both seen across the plane square to the
/// axis (their parts along it do not count).  std::invalid_argument for an axis of no length, or a or b along it.
template<class T> inline T
signed_angle(vec<3,T> const &a,vec<3,T> const &b,vec<3,T> const &axis)
{
	T const l2 = detail::length_sq_of_a_direction(axis, "signed angle about an axis");
	vec<3,T> const across_a = a - axis * (dot(a, axis) / l2), across_b = b - axis * (dot(b, axis) / l2);
	detail::length_sq_of_a_direction(across_a, "signed angle, the first vector across the axis");
	detail::length_sq_of_a_direction(across_b, "signed angle, the second vector across the axis");
	using std::atan2, std::sqrt;
	return atan2(dot(cross(across_a, across_b), axis) / sqrt(l2), dot(across_a, across_b));
}

/// @brief The turn (-pi to pi) from a's direction to b's, positive from the first axis towards the second (to_polar's sense)
template<class T> inline T
signed_angle(vec<2,T> const &a,vec<2,T> const &b)
{
	detail::length_sq_of_a_direction(a, "signed angle, the first vector");
	detail::length_sq_of_a_direction(b, "signed angle, the second vector");
	using std::atan2;
	return atan2(a.x * b.y - a.y * b.x, dot(a, b));
}

/// @brief current moved towards target by max_delta at most, landing on it exactly (scalar.h's, for vectors)
template<int N,class T,class S> inline vec<N,T,S>
move_towards(vec<N,T,S> const &current,vec<N,T,S> const &target,T max_delta)
{
	detail::require_not_negative(max_delta, "move_towards: max_delta");
	vec<N,T,S> const d = target - current;
	T const l2 = d.length_sq();
	if (l2 <= max_delta * max_delta) return target;
	using std::sqrt;
	return current + d * (max_delta / sqrt(l2));
}

/// @name The ground of a world that is Y up (xz<T>)
/// @{
template<class T> inline xz<T>
to_xz(vec<3,T> const &v)
{
	return xz<T>(v.x, v.z);
}

template<class T> inline vec<3,T>
from_xz(xz<T> const &g,T y)
{
	return vec<3,T>(g.x, y, g.z);
}

/// @brief The length of v's part along the ground, x and z
template<class T> inline T
horizontal_length(vec<3,T> const &v)
{
	return to_xz(v).length();
}
/// @}

/// @brief Scalar-vector multiplication (commutative)
/// @param lhs Scalar value
/// @param rhs Vector
template<int N,class T,class S> inline vec<N,T,S>
operator *(T lhs, vec<N,T,S> const &rhs)
{
	return rhs * lhs;
}

template<class Ch, class ChT, class T>
std::basic_istream<Ch, ChT> &operator >>(std::basic_istream<Ch, ChT> &is, vec<2,T> &v)
{
	return is >> v.x >> v.y;
}

template<class Ch, class ChT, class T>
std::basic_ostream<Ch, ChT> &operator <<(std::basic_ostream<Ch, ChT> &os, vec<2,T> const &v)
{
	return os << v.x << ' ' << v.y;
}

template<class Ch, class ChT, class T>
std::basic_istream<Ch, ChT> &operator >>(std::basic_istream<Ch, ChT> &is, vec<3,T> &v)
{
	return is >> v.x >> v.y >> v.z;
}

template<class Ch, class ChT, class T>
std::basic_ostream<Ch, ChT> &operator <<(std::basic_ostream<Ch, ChT> &os, vec<3,T> const &v)
{
	return os << v.x << ' ' << v.y << ' ' << v.z;
}

template<class Ch, class ChT, class T>
std::basic_istream<Ch, ChT> &operator >>(std::basic_istream<Ch, ChT> &is, vec<4,T> &v)
{
	return is >> v.x >> v.y >> v.z >> v.w;
}

template<class Ch, class ChT, class T>
std::basic_ostream<Ch, ChT> &operator <<(std::basic_ostream<Ch, ChT> &os, vec<4,T> const &v)
{
	return os << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w;
}

}

/// A set or map of vectors orders them lexicographically, component by component.  NaN has no place in it.
template<int N, class T, class S>
struct std::less<rove::vec<N, T, S>>
{
	bool operator()(rove::vec<N, T, S> const &a, rove::vec<N, T, S> const &b) const
	{
		for (int n = 0; n < N; ++n) {
			if (a.i[n] < b.i[n]) return true;
			if (b.i[n] < a.i[n]) return false;
		}
		return false;
	}
};

/// The components' hashes combined; +0 and -0, equal components, hash alike.
template<int N, class T, class S>
struct std::hash<rove::vec<N, T, S>>
{
	std::size_t operator()(rove::vec<N, T, S> const &v) const
	{
		std::size_t seed = 0;
		for (int n = 0; n < N; ++n) {
			T const c = v.i[n] == T(0) ? T(0) : v.i[n];
			seed ^= std::hash<T>()(c) + std::size_t(0x9e3779b97f4a7c15ull) + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

/// auto [x, y, z] = v: a vector's components, in the order of its memory, as a structured binding (by value, or by reference with
/// auto &), for every storage and any N
template<int N, class T, class S>
struct std::tuple_size<rove::vec<N, T, S>>: std::integral_constant<std::size_t, std::size_t(N)> {};

template<std::size_t I, int N, class T, class S>
struct std::tuple_element<I, rove::vec<N, T, S>> {
	static_assert(I < std::size_t(N), "rove: a vector has no component past its last");
	using type = T;
};

namespace rove
{

/// The component at position I, for a structured binding (found by argument lookup)
template<std::size_t I, int N, class T, class S> constexpr T &get(vec<N, T, S> &v) noexcept { return v.i[I]; }
template<std::size_t I, int N, class T, class S> constexpr T const &get(vec<N, T, S> const &v) noexcept { return v.i[I]; }
template<std::size_t I, int N, class T, class S> constexpr T &&get(vec<N, T, S> &&v) noexcept { return std::move(v.i[I]); }

}
