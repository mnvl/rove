// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file matrix.h
 * @brief Template matrix classes for 3x3 and 4x4 transformations
 *
 * This file provides optimized matrix classes specialized for 3x3 and 4x4 matrices.
 * Includes transformation operations (translation, rotation, scaling), matrix algebra
 * (inverse, transpose, determinant), and matrix-vector multiplication.
 *
 * Storage is column-major, ij[column][row]: the layout of GLSL's mat4 and of
 * glTF's 16 numbers, so a matrix goes to a shader as it is.  Vectors are
 * columns: m * v transforms v, and a * b applies b first, then a.
 */

#pragma once

#include <cassert>
#include <cmath>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include "scalar.h"
#include "vec.h"

namespace rove
{

/// @brief Template matrix class, specialized for 3x3 and 4x4
template<int N,int M,class T = scalar> class matrix;
template<class T> class matrix<3,3,T>;
template<class T> class matrix<4,4,T>;
template<class T> class quaternion;

// The products themselves, out of the way: m * v, transform_point and project_point are what a caller writes.
namespace detail
{

template<class T> inline void mul(vec<3,T> &r,matrix<3,3,T> const &m,vec<2,T> const &v);
template<class T> inline void mul(vec<3,T> &r,matrix<3,3,T> const &m,vec<3,T> const &v);
template<class T> inline void mul(vec<3,T> &r,matrix<4,4,T> const &m,vec<3,T> const &v);
template<class T> inline void mul(vec<4,T> &r,matrix<4,4,T> const &m,vec<3,T> const &v);
template<class T> inline void mul(vec<4,T> &r,matrix<4,4,T> const &m,vec<4,T> const &v);
template<int M,int N,int K,class T> inline void
mul(matrix<M,K,T> &result,matrix<M,N,T> const &left,matrix<N,K,T> const &right);

}

/**
 * @brief 3x3 matrix class for 2D transformations and 3D rotations
 *
 * Provides transformation operations (translation, rotation, scaling) for 2D graphics
 * and 3D rotation matrices. Includes matrix algebra operations like inverse, transpose,
 * and determinant.
 *
 * Matrix elements can be accessed via:
 * - Named members: _11, _12, ..., _33, column then row (_12 is ij[0][1])
 * - 2D array: ij[col][row]
 * - Index: m[col][row], as in GLSL; a column past the last one throws
 *
 * Storage is column-major: ij[column][row]
 */
template<class T>
class [[nodiscard]] matrix<3,3,T> {
public:
	// types and constants
	static constexpr size_t columns = 3, rows = 3;  ///< Matrix dimensions
	using point_type = vec<columns-1,T>; ///< 2D point type
	using vec_type = vec<columns,T>;     ///< 3D vector type
	using quaternion_type = quaternion<T>;  ///< Associated quaternion type

	/// Matrix element storage
	union {
		struct {
			T _11, _12, _13;  ///< Matrix elements, _<column><row>
			T _21, _22, _23;
			T _31, _32, _33;
		};

		T ij[columns][rows];  ///< 2D array access [column][row]
	};

	/// @brief Column c, so that m[c][r] is ij[c][r]; c past the last one throws std::out_of_range
	T (&operator [](size_t c))[rows] { return ij[detail::checked_index(c, columns)]; }
	T const (&operator [](size_t c) const)[rows] { return ij[detail::checked_index(c, columns)]; }

	/// Default constructor (identity)
	matrix() {
		ij[0][0]=1;	ij[0][1]=0;	ij[0][2]=0;
		ij[1][0]=0;	ij[1][1]=1;	ij[1][2]=0;
		ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=1;
	}

	/// Element by element from another scalar type (float <-> double)
	template<class U>
	explicit matrix(matrix<columns,rows,U> const &m) {
		for (size_t c = 0; c < columns; ++c)
			for (size_t r = 0; r < rows; ++r)
				ij[c][r] = T(m.ij[c][r]);
	}

	/// From columns * rows numbers column by column, the storage order (glTF's
	/// node.matrix, GLSL's mat3/mat4)
	static matrix from_column_major(T const *v) {
		matrix m;
		for (size_t c = 0; c < columns; ++c)
			for (size_t r = 0; r < rows; ++r)
				m.ij[c][r] = v[c * rows + r];
		return m;
	}

	// Initialization methods
	/// @brief Assign matrix from three row/column vectors
	void assign(vec_type const &l1, vec_type const &l2, vec_type const &l3);

	/// @brief Set all elements to zero
	void zero();

	/// @brief Set to identity matrix
	void identity();

	// Translation operations
	/// @brief Set to translation matrix
	void translation(T x,T y);

	/// @brief Set to translation matrix from point
	void translation(point_type const &p);

	/// @brief Apply translation (multiply current matrix by translation)
	void translate(T x,T y);

	/// @brief Apply translation from point
	void translate(point_type const &p);

	// Scaling operations
	/// @brief Set to scaling matrix
	void scaling(T x, T y);

	/// @brief Set to scaling matrix from point
	void scaling(point_type const &p);

	/// @brief Apply scaling (multiply current matrix by scale)
	void scale(T x, T y);

	/// @brief Apply scaling from point
	void scale(point_type const &p);

	// Rotation operations
	/// @brief Set to rotation matrix from Euler angles: Rz(gamma) Ry(beta) Rx(alpha), a turn about x first, then y, then z (the fixed
	/// axes)
	void rotation(T alpha,T beta,T gamma);

	/// @brief Set to rotation matrix from quaternion
	void rotation(quaternion_type const &q);

	/// @brief Set to rotation matrix from axis and precomputed sin/cos
	void rotation(vec_type const &axis, T sine, T cosine);

	/// @brief Set to rotation matrix from axis and angle
	void rotation(vec_type const &axis, T angle);

	/// @brief Apply rotation from Euler angles
	void rotate(T alpha,T beta,T gamma);

	/// @brief Apply rotation from rotation vector
	void rotate(vec_type const &v);

	/// @brief Apply rotation from quaternion
	void rotate(quaternion_type const &q);

	/// @brief Apply rotation from axis and precomputed sin/cos
	void rotate(vec_type const &axis, T sine, T cosine);

	/// @brief Apply rotation from axis and angle
	void rotate(vec_type const &axis, T angle);

	/// @brief Set to rotation matrix that aligns with direction
	void direction(vec_type const &dir);

	/// @brief Apply direction alignment
	void direct(vec_type const &dir);

	// Coordinate system transformations
	/// @brief Transform from world coordinates to local tangent space
	bool world_to_axes(vec_type const &tangent, vec_type const &normal, vec_type const &binormal);

	/// @brief Transform from local tangent space to world coordinates
	bool axes_to_world(vec_type const &tangent, vec_type const &normal, vec_type const &binormal);

	/// @brief The determinant of the 2x2 submatrix of columns c0, c1 and rows r0, r1 (a minor)
	T submatrix_determinant(int c0,int c1,int r0,int r1) const {
		return ij[c0][r0] * ij[c1][r1] - ij[c1][r0] * ij[c0][r1];
	}

	/// @brief Compute adjoint (adjugate) matrix
	void adjoint(matrix &M) const {
		M.ij[0][0] =  submatrix_determinant( 1, 2, 1, 2);
		M.ij[0][1] = -submatrix_determinant( 0, 2, 1, 2);
		M.ij[0][2] =  submatrix_determinant( 0, 1, 1, 2);
		M.ij[1][0] = -submatrix_determinant( 1, 2, 0, 2);
		M.ij[1][1] =  submatrix_determinant( 0, 2, 0, 2);
		M.ij[1][2] = -submatrix_determinant( 0, 1, 0, 2);
		M.ij[2][0] =  submatrix_determinant( 1, 2, 0, 1);
		M.ij[2][1] = -submatrix_determinant( 0, 2, 0, 1);
		M.ij[2][2] =  submatrix_determinant( 0, 1, 0, 1);
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where an entry is NaN or infinite
	matrix const &check_finite(char const *what) const {
		for (size_t c = 0; c < columns; ++c)
			for (size_t r = 0; r < rows; ++r)
				if (!rove::isfinite(ij[c][r]))
					throw std::invalid_argument(std::string(what) + " that is not finite");
		return *this;
	}

	/// @brief *this, or std::invalid_argument where it has no inverse: `what`, " of determinant " and the determinant where that
	/// is zero or not a number
	matrix const &check_invertible(char const *what) const {
		T const det = determinant();
		using std::to_string;
		if (!(det != 0) || !rove::isfinite(det) || !rove::isfinite(T(1) / det))
			throw std::invalid_argument(std::string(what) + " of determinant " + to_string(det));
		return *this;
	}

	/**
	 * @brief Compute matrix determinant
	 * @return Determinant value (zero for singular matrix)
	 */
	T determinant() const {
		return 	ij[0][0] * submatrix_determinant(1, 2, 1, 2) -
			ij[0][1] * submatrix_determinant(1, 2, 0, 2) +
			ij[0][2] * submatrix_determinant(1, 2, 0, 1);
	}

	/**
	 * @brief Compute matrix trace (sum of diagonal elements)
	 * @return Sum of diagonal elements
	 */
	T trace() const {
		return ij[0][0] + ij[1][1] + ij[2][2];
	}

	/**
	 * @brief Compute inverse matrix
	 * @return false if matrix is singular (determinant is zero)
	 */
	[[nodiscard]] bool inverse(matrix &M) const {
		T det = determinant();
		if (det == 0 || !rove::isfinite(T(1) / det)) return false;
		matrix r;          // (M may be *this)
		adjoint(r);
		r /= det;
		M = r;
		return true;
	}

	/**
	 * @brief Invert matrix in place
	 * @return false if matrix is singular
	 */
	bool invert() {
		return matrix(*this).inverse(*this);
	}

	/// @brief Compute transpose matrix (tp may be *this)
	void transpose(matrix &tp) const {
		matrix const m(*this);
		tp.ij[0][0] = m.ij[0][0];		tp.ij[0][1] = m.ij[1][0];		tp.ij[0][2] = m.ij[2][0];
		tp.ij[1][0] = m.ij[0][1];		tp.ij[1][1] = m.ij[1][1];		tp.ij[1][2] = m.ij[2][1];
		tp.ij[2][0] = m.ij[0][2];		tp.ij[2][1] = m.ij[1][2];		tp.ij[2][2] = m.ij[2][2];
	}

	matrix &operator *=(T k) {
		ij[0][0]*=k;	ij[0][1]*=k;	ij[0][2]*=k;
		ij[1][0]*=k;	ij[1][1]*=k;	ij[1][2]*=k;
		ij[2][0]*=k;	ij[2][1]*=k;	ij[2][2]*=k;
		return *this;
	}

	matrix &operator *=(matrix const &m) {
		// (into another matrix: m may be *this, and would be read as it was written)
		matrix n;
		detail::mul(n,*this,m);
		return *this = n;
	}

	matrix &operator /=(T k) {
		return *this *= 1/k;
	}

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & ij;
	}
};

/**
 * @brief 4x4 matrix class for 3D transformations
 *
 * Provides full 3D transformation capabilities including translation, rotation,
 * scaling, perspective projection, and view transformations.
 *
 * Matrix elements can be accessed via:
 * - Named members: _11, _12, ..., _44, column then row (_41, _42, _43 is
 *   the translation, where Direct3D's row-vector D3DMATRIX keeps it too)
 * - 2D array: ij[col][row]
 * - Index: m[col][row], as in GLSL; a column past the last one throws
 *
 * Storage is column-major: ij[column][row]
 */
template<class T>
class [[nodiscard]] matrix<4,4,T> {
public:
	// types and constants
	static constexpr size_t columns = 4, rows = 4;  ///< Matrix dimensions
	using vec_type = vec<columns,T>;     ///< 4D vector type
	using point_type = vec<columns-1,T>; ///< 3D point type
	using quaternion_type = quaternion<T>;  ///< Associated quaternion type

	/// Matrix element storage
	union {
		struct {
			T _11, _12, _13, _14;  ///< Matrix elements, _<column><row>
			T _21, _22, _23, _24;
			T _31, _32, _33, _34;
			T _41, _42, _43, _44;
		};

		T ij[columns][rows];  ///< 2D array access [column][row]
	};

	/// @brief Column c, so that m[c][r] is ij[c][r]; c past the last one throws std::out_of_range
	T (&operator [](size_t c))[rows] { return ij[detail::checked_index(c, columns)]; }
	T const (&operator [](size_t c) const)[rows] { return ij[detail::checked_index(c, columns)]; }

	/// Default constructor (identity)
	matrix() {
		ij[0][0]=1;	ij[0][1]=0;	ij[0][2]=0;	ij[0][3]=0;
		ij[1][0]=0;	ij[1][1]=1;	ij[1][2]=0;	ij[1][3]=0;
		ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=1;	ij[2][3]=0;
		ij[3][0]=0;	ij[3][1]=0;	ij[3][2]=0;	ij[3][3]=1;
	}

	/// Element by element from another scalar type (float <-> double)
	template<class U>
	explicit matrix(matrix<columns,rows,U> const &m) {
		for (size_t c = 0; c < columns; ++c)
			for (size_t r = 0; r < rows; ++r)
				ij[c][r] = T(m.ij[c][r]);
	}

	/// From columns * rows numbers column by column, the storage order (glTF's
	/// node.matrix, GLSL's mat3/mat4)
	static matrix from_column_major(T const *v) {
		matrix m;
		for (size_t c = 0; c < columns; ++c)
			for (size_t r = 0; r < rows; ++r)
				m.ij[c][r] = v[c * rows + r];
		return m;
	}

	/// @brief Set to identity matrix
	void identity();

	/// @brief Set all elements to zero
	void zero();

	// Translation operations
	/// @brief Set to translation matrix
	void translation(T x,T y,T z);

	/// @brief Set to translation matrix from 3D point
	void translation(point_type const &p);

	/// @brief Apply translation (multiply current matrix by translation)
	void translate(T x,T y,T z);

	/// @brief Apply translation from 3D point
	void translate(point_type const &p);

	// Scaling operations
	/// @brief Set to scaling matrix
	void scaling(T sx,T sy,T sz);

	/// @brief Set to scaling matrix from 3D point
	void scaling(point_type const &s);

	/// @brief Apply scaling (multiply current matrix by scale)
	void scale(T sx,T sy,T sz);

	/// @brief Apply scaling from 3D point
	void scale(point_type const &s);

	// Rotation operations
	/// @brief Set to rotation matrix from Euler angles: Rz(gamma) Ry(beta) Rx(alpha), a turn about x first, then y, then z (the fixed
	/// axes)
	void rotation(T alpha,T beta,T gamma);

	/// @brief Set to rotation matrix from quaternion
	void rotation(quaternion_type const &q);

	/// @brief Set to rotation matrix from axis and precomputed sin/cos
	void rotation(point_type const &axis, T sine, T cosine);

	/// @brief Set to rotation matrix from axis and angle
	void rotation(point_type const &axis, T angle);

	/// @brief Apply rotation from Euler angles
	void rotate(T alpha,T beta,T gamma);

	/// @brief Apply rotation from rotation vector
	void rotate(point_type const &v);

	/// @brief Apply rotation from quaternion
	void rotate(quaternion_type const &q);

	/// @brief Apply rotation from axis and precomputed sin/cos
	void rotate(point_type const &axis, T sine, T cosine);

	/// @brief Apply rotation from axis and angle
	void rotate(point_type const &axis, T angle);

	/// @brief Set to rotation matrix that aligns with direction
	void direction(point_type const &dir);

	/// @brief Apply direction alignment
	void direct(point_type const &dir);

	/// @brief Transform a vector (w = 0): rotation, scale and shear without the
	/// translation, unlike m * p.  Not normalised.
	point_type transform_vector(point_type const &d) const {
		return point_type(ij[0][0]*d.x+ij[1][0]*d.y+ij[2][0]*d.z,
		               ij[0][1]*d.x+ij[1][1]*d.y+ij[2][1]*d.z,
		               ij[0][2]*d.x+ij[1][2]*d.y+ij[2][2]*d.z);
	}

	/**
	 * @brief Create view (camera) matrix using look-at parameters
	 * @param eye Camera position
	 * @param at Target position to look at
	 * @param up Up direction vector
	 *
	 * Basis: forward = normalize(at - eye), right = forward × up,
	 * up_adjusted = right × forward. Camera looks along +Z in view space.
	 * X = right, Y = up, Z = forward.
	 */
	void lookat(point_type const &eye,point_type const &at,point_type const &up);

	/**
	 * @brief Create orthographic projection matrix
	 * @param width View width
	 * @param height View height
	 * @param z_near Near clipping plane
	 * @param z_far Far clipping plane
	 */
	void ortho(T width,T height,T z_near,T z_far);

	/**
	 * @brief Create perspective projection matrix
	 * @param fovy Field of view angle (Y axis) in radians
	 * @param aspect Aspect ratio (width/height)
	 * @param z_near Near clipping plane
	 * @param z_far Far clipping plane
	 */
	void perspective(T fovy,T aspect,T z_near,T z_far);

	/// @brief The determinant of the 3x3 submatrix of columns c0, c1, c2 and rows r0, r1, r2 (a minor)
	T submatrix_determinant(int c0,int c1,int c2,int r0,int r1,int r2) const {
		return
			ij[c0][r0] * (ij[c1][r1] * ij[c2][r2] - ij[c2][r1] * ij[c1][r2]) -
			ij[c0][r1] * (ij[c1][r0] * ij[c2][r2] - ij[c2][r0] * ij[c1][r2]) +
			ij[c0][r2] * (ij[c1][r0] * ij[c2][r1] - ij[c2][r0] * ij[c1][r1]);
	}

	/// @brief Compute adjoint (adjugate) matrix
	void adjoint(matrix &M) const {
		M.ij[0][0] =  submatrix_determinant( 1, 2, 3, 1, 2, 3);
		M.ij[0][1] = -submatrix_determinant( 0, 2, 3, 1, 2, 3);
		M.ij[0][2] =  submatrix_determinant( 0, 1, 3, 1, 2, 3);
		M.ij[0][3] = -submatrix_determinant( 0, 1, 2, 1, 2, 3);
		M.ij[1][0] = -submatrix_determinant( 1, 2, 3, 0, 2, 3);
		M.ij[1][1] =  submatrix_determinant( 0, 2, 3, 0, 2, 3);
		M.ij[1][2] = -submatrix_determinant( 0, 1, 3, 0, 2, 3);
		M.ij[1][3] =  submatrix_determinant( 0, 1, 2, 0, 2, 3);
		M.ij[2][0] =  submatrix_determinant( 1, 2, 3, 0, 1, 3);
		M.ij[2][1] = -submatrix_determinant( 0, 2, 3, 0, 1, 3);
		M.ij[2][2] =  submatrix_determinant( 0, 1, 3, 0, 1, 3);
		M.ij[2][3] = -submatrix_determinant( 0, 1, 2, 0, 1, 3);
		M.ij[3][0] = -submatrix_determinant( 1, 2, 3, 0, 1, 2);
		M.ij[3][1] =  submatrix_determinant( 0, 2, 3, 0, 1, 2);
		M.ij[3][2] = -submatrix_determinant( 0, 1, 3, 0, 1, 2);
		M.ij[3][3] =  submatrix_determinant( 0, 1, 2, 0, 1, 2);
	}

	/// @brief *this, or std::invalid_argument: `what` and " that is not finite" where an entry is NaN or infinite
	matrix const &check_finite(char const *what) const {
		for (size_t c = 0; c < columns; ++c)
			for (size_t r = 0; r < rows; ++r)
				if (!rove::isfinite(ij[c][r]))
					throw std::invalid_argument(std::string(what) + " that is not finite");
		return *this;
	}

	/// @brief *this, or std::invalid_argument where it has no inverse: `what`, " of determinant " and the determinant where that
	/// is zero or not a number
	matrix const &check_invertible(char const *what) const {
		T const det = determinant();
		using std::to_string;
		if (!(det != 0) || !rove::isfinite(det) || !rove::isfinite(T(1) / det))
			throw std::invalid_argument(std::string(what) + " of determinant " + to_string(det));
		return *this;
	}

	/// @brief *this, or std::invalid_argument: `what` and " whose last row is not (0, 0, 0, 1)" where it is not, to within 1e-5, an
	/// affine transform (a projection, or a matrix built the other way round); NaN there is not
	matrix const &check_affine(char const *what) const {
		using std::abs;
		T const loose = T(1) / T(100000);
		if (!(abs(ij[0][3]) <= loose && abs(ij[1][3]) <= loose && abs(ij[2][3]) <= loose && abs(ij[3][3] - 1) <= loose))
			throw std::invalid_argument(std::string(what) + " whose last row is not (0, 0, 0, 1)");
		return *this;
	}

	/**
	 * @brief Compute matrix determinant
	 * @return Determinant value (zero for singular matrix)
	 */
	T determinant() const {
		return
			ij[0][0] * submatrix_determinant(1, 2, 3, 1, 2, 3) -
			ij[0][1] * submatrix_determinant(1, 2, 3, 0, 2, 3) +
			ij[0][2] * submatrix_determinant(1, 2, 3, 0, 1, 3) -
			ij[0][3] * submatrix_determinant(1, 2, 3, 0, 1, 2);
	}

	/**
	 * @brief Compute inverse matrix
	 * @return false if matrix is singular (determinant is zero)
	 */
	[[nodiscard]] bool inverse(matrix &M) const {
		T det = determinant();
		if (det == 0 || !rove::isfinite(T(1) / det)) return false;
		matrix r;          // (M may be *this)
		adjoint(r);
		r /= det;
		M = r;
		return true;
	}

	/**
	 * @brief Invert matrix in place
	 * @return false if matrix is singular
	 */
	bool invert() {
		return matrix(*this).inverse(*this);
	}

	/// @brief Compute transpose matrix
	void transpose(matrix &tp) const {
		matrix const m(*this);          // (tp may be *this)
		tp.ij[0][0] = m.ij[0][0];	tp.ij[0][1] = m.ij[1][0];	tp.ij[0][2] = m.ij[2][0]; tp.ij[0][3] = m.ij[3][0];
		tp.ij[1][0] = m.ij[0][1];	tp.ij[1][1] = m.ij[1][1];	tp.ij[1][2] = m.ij[2][1]; tp.ij[1][3] = m.ij[3][1];
		tp.ij[2][0] = m.ij[0][2];	tp.ij[2][1] = m.ij[1][2];	tp.ij[2][2] = m.ij[2][2]; tp.ij[2][3] = m.ij[3][2];
		tp.ij[3][0] = m.ij[0][3];	tp.ij[3][1] = m.ij[1][3];	tp.ij[3][2] = m.ij[2][3]; tp.ij[3][3] = m.ij[3][3];
	}

	/// @brief Transpose matrix in place
	void transpose_inplace() {
		matrix tp;
		tp = *this;
		tp.transpose(*this);
	}

	matrix &operator *=(T k) {
		ij[0][0]*=k;	ij[0][1]*=k;	ij[0][2]*=k;	ij[0][3]*=k;
		ij[1][0]*=k;	ij[1][1]*=k;	ij[1][2]*=k;	ij[1][3]*=k;
		ij[2][0]*=k;	ij[2][1]*=k;	ij[2][2]*=k;	ij[2][3]*=k;
		ij[3][0]*=k;	ij[3][1]*=k;	ij[3][2]*=k;	ij[3][3]*=k;
		return *this;
	}

	matrix &operator /=(T k) {
		return *this *= 1/k;
	}

	matrix &operator *=(matrix const &m) {
		// (into another matrix: m may be *this, and would be read as it was written)
		matrix n;
		detail::mul(n,*this,m);
		return *this = n;
	}

	/**
	 * @brief Extract 3D column vector (drops w component)
	 * @param column_index Column index (0-3)
	 * @return 3D vector from column
	 */
	point_type projected_column(size_t column_index) const {
		return point_type(ij[column_index][0], ij[column_index][1], ij[column_index][2]);
	}

	/**
	 * @brief Extract 4D column vector
	 * @param column_index Column index (0-3)
	 * @return 4D vector from column
	 */
	vec_type column(size_t column_index) const {
		return vec_type(ij[column_index][0], ij[column_index][1], ij[column_index][2],
			ij[column_index][3]);
	}

	/**
	 * @brief Extract 3D row vector (drops w component)
	 * @param row_index Row index (0-3)
	 * @return 3D vector from row
	 */
	point_type projected_row(size_t row_index) const {
		return point_type(ij[0][row_index], ij[1][row_index], ij[2][row_index]);
	}

	/**
	 * @brief Extract 4D row vector
	 * @param row_index Row index (0-3)
	 * @return 4D vector from row
	 */
	vec_type row(size_t row_index) const {
		return vec_type(ij[0][row_index], ij[1][row_index], ij[2][row_index],
			ij[3][row_index]);
	}

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive & ij;
	}
};

namespace detail
{

template<class T> void
mul(vec<3,T> &result,matrix<3,3,T> const &m,vec<2,T> const &v) {
	assert((void*)&result != (void*)&v);

	result.x=m.ij[0][0]*v.x+m.ij[1][0]*v.y+m.ij[2][0];
	result.y=m.ij[0][1]*v.x+m.ij[1][1]*v.y+m.ij[2][1];
	result.z=m.ij[0][2]*v.x+m.ij[1][2]*v.y+m.ij[2][2];
}

template<class T> void
mul(vec<3,T> &result,matrix<3,3,T> const &m,vec<3,T> const &v) {
	assert((void*)&result != (void*)&v);

	result.x=m.ij[0][0]*v.x+m.ij[1][0]*v.y+m.ij[2][0]*v.z;
	result.y=m.ij[0][1]*v.x+m.ij[1][1]*v.y+m.ij[2][1]*v.z;
	result.z=m.ij[0][2]*v.x+m.ij[1][2]*v.y+m.ij[2][2]*v.z;
}

template<class T> void
mul(vec<3,T> &result,matrix<4,4,T> const &m,vec<3,T> const &v) {
	assert((void*)&result != (void*)&v);

	result.x=m.ij[0][0]*v.x+m.ij[1][0]*v.y+m.ij[2][0]*v.z+m.ij[3][0];
	result.y=m.ij[0][1]*v.x+m.ij[1][1]*v.y+m.ij[2][1]*v.z+m.ij[3][1];
	result.z=m.ij[0][2]*v.x+m.ij[1][2]*v.y+m.ij[2][2]*v.z+m.ij[3][2];
}

template<class T> void
mul(vec<4,T> &result,matrix<4,4,T> const &m,vec<3,T> const &v) {
	assert((void*)&result != (void*)&v);

	result.x=m.ij[0][0]*v.x+m.ij[1][0]*v.y+m.ij[2][0]*v.z+m.ij[3][0];
	result.y=m.ij[0][1]*v.x+m.ij[1][1]*v.y+m.ij[2][1]*v.z+m.ij[3][1];
	result.z=m.ij[0][2]*v.x+m.ij[1][2]*v.y+m.ij[2][2]*v.z+m.ij[3][2];
	result.w=m.ij[0][3]*v.x+m.ij[1][3]*v.y+m.ij[2][3]*v.z+m.ij[3][3];
}

template<class T> void
mul(vec<4,T> &result,matrix<4,4,T> const &m,vec<4,T> const &v) {
	assert((void*)&result != (void*)&v);

	result.x=m.ij[0][0]*v.x+m.ij[1][0]*v.y+m.ij[2][0]*v.z+m.ij[3][0]*v.w;
	result.y=m.ij[0][1]*v.x+m.ij[1][1]*v.y+m.ij[2][1]*v.z+m.ij[3][1]*v.w;
	result.z=m.ij[0][2]*v.x+m.ij[1][2]*v.y+m.ij[2][2]*v.z+m.ij[3][2]*v.w;
	result.w=m.ij[0][3]*v.x+m.ij[1][3]*v.y+m.ij[2][3]*v.z+m.ij[3][3]*v.w;
}

/**
 * @brief Multiply two matrices: result = left × right
 * @param[out] result Result matrix (must not alias left or right)
 * @tparam M Number of rows in left matrix
 * @tparam N Number of columns in left / rows in right
 * @tparam K Number of columns in right matrix
 */
template<int M,int N,int K,class T> void
mul(matrix<M,K,T> &result,matrix<M,N,T> const &left,matrix<N,K,T> const &right) {
	assert(&result != &left);
	assert(&result != &right);

	int i,j,k;
	for (i=0; i<K; i++) {
		for (j=0; j<M; j++) {
			result.ij[i][j]=left.ij[0][j]*right.ij[i][0];
			for (k=1; k<N; k++) result.ij[i][j]+=left.ij[k][j]*right.ij[i][k];
		}
	}
}

}

/**
 * @brief Test matrix equality within epsilon tolerance
 * @param epsilon Tolerance for floating point comparison
 * @return true if all elements differ by less than epsilon
 */
template <int M, int N, class T> inline bool
equal(matrix<M, N, T> const &lhs, matrix<M, N, T> const &rhs, T epsilon = scalar_traits<T>::default_epsilon) {
	for (size_t i = 0; i < M; i++) {
		for (size_t j = 0; j < N; j++) {
			if (abs(lhs.ij[i][j] - rhs.ij[i][j]) > epsilon) return false;
		}
	}

	return true;
}

template<int M,int N,class T> inline rove::vec<M-1,T>
operator *(rove::matrix<M,N,T> const &m, rove::vec<N-1,T> const &v) {
	rove::vec<M-1,T> result;
	detail::mul(result,m,v);
	return result;
}

/// A 3x3 matrix times a 2D point does not compile: transform_point (no divide) or project_point (divide by z) says which
template<class T> rove::vec<2,T> operator *(rove::matrix<3,3,T> const &m, rove::vec<2,T> const &v) = delete;

template<int M,int N,class T> inline rove::vec<M,T>
operator *(rove::matrix<M,N,T> const &m, rove::vec<N,T> const &v) {
	rove::vec<M,T> result;
	detail::mul(result,m,v);
	return result;
}

template<int M,int N,int K,class T> inline rove::matrix<M,K,T>
operator *(rove::matrix<M,N,T> const &left,rove::matrix<N,K,T> const &right) {
	rove::matrix<M,K,T> result;
	detail::mul(result,left,right);
	return result;
}

/// @name Points and vectors through a matrix, by name
/// m * p is the affine transform: w taken as 1 and never divided by.  That is right for any matrix whose last row is (0, 0, 0, 1) and
/// wrong for a projection, whose w must be divided by: project_point.  (A 3x3 matrix times a 2D point divided by z, so the operator
/// meant different things in 2D and 3D; that product is gone.)
/// @{

/// @brief m p with w = 1, no divide: as m * p
template<class T> inline vec<3,T>
transform_point(matrix<4,4,T> const &m,vec<3,T> const &p) {
	vec<3,T> r;
	detail::mul(r, m, p);
	return r;
}

/// @brief m d with w = 0: rotation, scale and shear without the translation
template<class T> inline vec<3,T>
transform_vector(matrix<4,4,T> const &m,vec<3,T> const &d) {
	return m.transform_vector(d);
}

/// @brief m p with w = 1, then divided by the w that comes out: a point through a projection.  w of zero (a point on the plane
/// through the eye) or not a number is std::invalid_argument.
template<class T> inline vec<3,T>
project_point(matrix<4,4,T> const &m,vec<3,T> const &p) {
	vec<4,T> h;
	detail::mul(h, m, p);
	using std::to_string;
	if (!(h.w != 0) || !rove::isfinite(h.w))
		throw std::invalid_argument("rove: project_point: w is " + to_string(h.w));
	return vec<3,T>(h.x / h.w, h.y / h.w, h.z / h.w);
}

/// @brief The 2D affine transform: (x, y, 1) through the first two rows, no divide
template<class T> inline vec<2,T>
transform_point(matrix<3,3,T> const &m,vec<2,T> const &p) {
	return vec<2,T>(m.ij[0][0]*p.x + m.ij[1][0]*p.y + m.ij[2][0],
	                m.ij[0][1]*p.x + m.ij[1][1]*p.y + m.ij[2][1]);
}

/// @brief (x, y, 1) through the matrix, divided by the third component; zero or not a number there is std::invalid_argument
template<class T> inline vec<2,T>
project_point(matrix<3,3,T> const &m,vec<2,T> const &p) {
	vec<3,T> h;
	detail::mul(h, m, p);
	using std::to_string;
	if (!(h.z != 0) || !rove::isfinite(h.z))
		throw std::invalid_argument("rove: project_point: z is " + to_string(h.z));
	return vec<2,T>(h.x / h.z, h.y / h.z);
}
/// @}

/// @brief The inverse, returned; a singular matrix (determinant zero or not a number) is std::invalid_argument.  The member's
/// arithmetic, bit for bit.
template<int N,class T> inline matrix<N,N,T>
inverse(matrix<N,N,T> const &m) {
	m.check_invertible("rove: inverse of a matrix");
	matrix<N,N,T> r;
	(void)m.inverse(r);   // the determinant is checked above
	return r;
}

/// @brief The transpose, returned
template<int N,class T> inline matrix<N,N,T>
transpose(matrix<N,N,T> const &m) {
	matrix<N,N,T> r;
	m.transpose(r);
	return r;
}

/// @name The 4x4 transforms by value: the setters of the same name (translation, scaling, rotation, lookat, perspective), returned
/// @{
template<class T> inline matrix<4,4,T>
translation_matrix(vec<3,T> const &offset) {
	matrix<4,4,T> m;
	m.translation(offset);
	return m;
}

template<class T> inline matrix<4,4,T>
scaling_matrix(vec<3,T> const &factors) {
	matrix<4,4,T> m;
	m.scaling(factors);
	return m;
}

/// @brief A turn of `angle` radians about `axis` (any length but none), right-handed
template<class T> inline matrix<4,4,T>
rotation_matrix(vec<3,T> const &axis,T angle) {
	matrix<4,4,T> m;
	m.rotation(axis, angle);
	return m;
}

/// @brief matrix::lookat's view: forward = at - eye, right = forward x up (a mirror; see lookat)
template<class T> inline matrix<4,4,T>
look_at_matrix(vec<3,T> const &eye,vec<3,T> const &at,vec<3,T> const &up) {
	matrix<4,4,T> m;
	m.lookat(eye, at, up);
	return m;
}

/// @brief matrix::perspective's projection: view +Z forward, depth 0 at z_near to 1 at z_far
template<class T> inline matrix<4,4,T>
perspective_matrix(T fovy,T aspect,T z_near,T z_far) {
	matrix<4,4,T> m;
	m.perspective(fovy, aspect, z_near, z_far);
	return m;
}

/**
 * @brief An orthographic projection of the box [left, right] x [bottom, top] x [z_near, z_far] of view space
 *
 * x and y onto -1..1, depth 0 at z_near to 1 at z_far, as matrix::ortho (which is this box centred on the axis): an off-centre one
 * fits a shadow map to what a camera sees.  A box of no width, height or depth is std::invalid_argument.
 */
template<class T> inline matrix<4,4,T>
ortho_matrix(T left,T right,T bottom,T top,T z_near,T z_far) {
	if (!(left != right) || !(bottom != top) || !(z_near != z_far))
		throw std::invalid_argument("rove: ortho_matrix of a box with no width, height or depth");
	matrix<4,4,T> m;
	m.zero();
	m.ij[0][0] = 2 / (right - left);
	m.ij[1][1] = 2 / (top - bottom);
	m.ij[2][2] = 1 / (z_far - z_near);
	m.ij[3][0] = -(right + left) / (right - left);
	m.ij[3][1] = -(top + bottom) / (top - bottom);
	m.ij[3][2] = -z_near / (z_far - z_near);
	m.ij[3][3] = 1;
	return m;
}
/// @}

/// @brief The upper left 3x3 of a 4x4: its rotation, scale and shear without the translation
template<class T> inline matrix<3,3,T>
linear_part(matrix<4,4,T> const &m) {
	matrix<3,3,T> r;
	for (int c = 0; c < 3; ++c)
		for (int row = 0; row < 3; ++row)
			r.ij[c][row] = m.ij[c][row];
	return r;
}

/// @brief What turns normals as m turns the surface: the inverse transpose of its linear part (the same as it for a rotation, not
/// for a stretch).  A singular linear part is std::invalid_argument.
template<class T> inline matrix<3,3,T>
normal_matrix(matrix<4,4,T> const &m) {
	return transpose(inverse(linear_part(m)));
}

}
