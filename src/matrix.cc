// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "quaternion.h"
#include <stdexcept>
#include "matrix.h"
#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif

namespace rove
{

template<class T> void
matrix<3,3,T>::assign(vec_type const &l1, vec_type const &l2, vec_type const &l3)
{
	ij[0][0]=l1.x;	ij[0][1]=l1.y;	ij[0][2]=l1.z;
	ij[1][0]=l2.x;	ij[1][1]=l2.y;	ij[1][2]=l2.z;
	ij[2][0]=l3.x;	ij[2][1]=l3.y;	ij[2][2]=l3.z;
}

template<class T> void
matrix<3,3,T>::identity() {
	ij[0][0]=1;	ij[0][1]=0;	ij[0][2]=0;
	ij[1][0]=0;	ij[1][1]=1;	ij[1][2]=0;
	ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=1;
}

template<class T> void
matrix<3,3,T>::zero() {
	ij[0][0]=0;	ij[0][1]=0;	ij[0][2]=0;
	ij[1][0]=0;	ij[1][1]=0;	ij[1][2]=0;
	ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=0;
}

template<class T> void
matrix<3,3,T>::translation(T x,T y) {
	ij[0][0]=1;	ij[0][1]=0;	ij[0][2]=0;
	ij[1][0]=0;	ij[1][1]=1;	ij[1][2]=0;
	ij[2][0]=x;	ij[2][1]=y;	ij[2][2]=1;
}

template<class T> void
matrix<3,3,T>::translation(point_type const &p) {
	translation(p.x,p.y);
}

template<class T> void
matrix<3,3,T>::translate(T x,T y) {
	matrix temp, t;
	temp.translation(x,y);
	detail::mul(t, temp, *this);
	*this = t;
}

template<class T> void
matrix<3,3,T>::translate(point_type const &p) {
	translate(p.x,p.y);
}

template<class T> void
matrix<3,3,T>::scaling(T x, T y) {
	ij[0][0]=x;	ij[0][1]=0;	ij[0][2]=0;
	ij[1][0]=0;	ij[1][1]=y;	ij[1][2]=0;
	ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=1;
}

template<class T> void
matrix<3,3,T>::scaling(point_type const &p) {
	scaling(p.x, p.y);
}

template<class T> void
matrix<3,3,T>::scale(T x, T y) {
	matrix temp, t;
	temp.scaling(x,y);
	detail::mul(t, temp, *this);
	*this = t;
}

template<class T> void
matrix<3,3,T>::scale(point_type const &p) {
	scale(p.x, p.y);
}

template<class T> void
matrix<3,3,T>::rotation(T alpha,T beta,T gamma) {
	T sa = sin(alpha),
		    ca = cos(alpha),
		    sb = sin(beta),
		    cb = cos(beta),
		    sg = sin(gamma),
		    cg = cos(gamma),
		    sa_sb = sa * sb,
		    ca_sb = ca * sb;

	ij[0][0] = cb*cg;
	ij[0][1] = cb*sg;
	ij[0][2] = -sb;

	ij[1][0] = sa_sb*cg - ca*sg;
	ij[1][1] = sa_sb*sg + ca*cg;
	ij[1][2] = sa*cb;

	ij[2][0] = ca_sb*cg + sa*sg;
	ij[2][1] = ca_sb*sg - sa*cg;
	ij[2][2] = ca*cb;
}

template<class T> void
matrix<3,3,T>::rotation(quaternion<T> const &q) {
	T wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	T s  = detail::rotation_scale(q);
	x2 = q.x * s;    y2 = q.y * s;    z2 = q.z * s;
	xx = q.x * x2;   xy = q.x * y2;   xz = q.x * z2;
	yy = q.y * y2;   yz = q.y * z2;   zz = q.z * z2;
	wx = q.w * x2;   wy = q.w * y2;   wz = q.w * z2;

	ij[0][0] = 1 - (yy + zz);
	ij[1][0] = xy - wz;
	ij[2][0] = xz + wy;

	ij[0][1] = xy + wz;
	ij[1][1] = 1 - (xx + zz);
	ij[2][1] = yz - wx;

	ij[0][2] = xz - wy;
	ij[1][2] = yz + wx;
	ij[2][2] = 1 - (xx + yy);
}

template<class T> void
matrix<3,3,T>::rotation(vec_type const &axis, T angle) {
	rotation(axis, sin(angle), cos(angle));
}

template<class T> void
matrix<3,3,T>::rotation(vec_type const &axis, T sine, T cosine) {
	// an axis of no length (or not a number) has no direction: it was a matrix of NaN
	detail::length_sq_of_a_direction(axis, "a turn about an axis");
	T length = axis.length();
	T a = axis.x / length;
	T b = axis.y / length;
	T c = axis.z / length;
	T aa = a * a;
	T bb = b * b;
	T cc = c * c;
	T omcos = 1 - cosine;

	ij[0][0] = aa + (1 - aa) * cosine;
	ij[1][1] = bb + (1 - bb) * cosine;
	ij[2][2] = cc + (1 - cc) * cosine;
	ij[0][1] = a * b * omcos + c * sine;
	ij[0][2] = a * c * omcos - b * sine;
	ij[1][0] = a * b * omcos - c * sine;
	ij[1][2] = b * c * omcos + a * sine;
	ij[2][0] = a * c * omcos + b * sine;
	ij[2][1] = b * c * omcos - a * sine;
}

template<class T> void
matrix<3,3,T>::rotate(T alpha,T beta,T gamma) {
	matrix self = *this, temp;
	temp.rotation(alpha,beta,gamma);
	detail::mul(*this, temp, self);
}

template<class T> void
matrix<3,3,T>::rotate(vec_type const &v) {
	rotate(v.A,v.B,v.C);
}

template<class T> void
matrix<3,3,T>::rotate(quaternion_type const &q) {
	matrix self = *this, temp;
	temp.rotation(q);
	detail::mul(*this, temp, self);
}

template<class T> void
matrix<3,3,T>::rotate(vec_type const &axis, T angle) {
	matrix self = *this, temp;
	temp.rotation(axis, angle);
	detail::mul(*this, temp, self);
}

template<class T> void
matrix<3,3,T>::rotate(vec_type const &axis, T sine, T cosine) {
	matrix self = *this, temp;
	temp.rotation(axis, sine, cosine);
	detail::mul(*this, temp, self);
}

template<class T> void
matrix<3,3,T>::direction(vec_type const &dir) {
	vec_type x_axis = dir.check_nonzero("rove: matrix::direction along a vector", T(0));
	x_axis.normalize();

	vec_type y_axis;
	y_axis.set(0,1,0);

	// a dir straight up or down has no z: the axes are then zero, as they always were
	vec_type z_axis = normalize_or_zero(cross(x_axis, y_axis));
	y_axis = normalize_or_zero(cross(z_axis, x_axis));

	ij[0][0] = x_axis.x;	ij[0][1] = x_axis.y;	ij[0][2] = x_axis.z;
	ij[1][0] = y_axis.x;	ij[1][1] = y_axis.y;	ij[1][2] = y_axis.z;
	ij[2][0] = z_axis.x;	ij[2][1] = z_axis.y;	ij[2][2] = z_axis.z;
}

template<class T> void
matrix<3,3,T>::direct(vec_type const &dir) {
	matrix self = *this, temp;
	temp.direction(dir);
	detail::mul(*this, temp, self);
}

template<class T> bool
matrix<3,3,T>::world_to_axes(vec_type const &tangent, vec_type const &normal, vec_type const &binormal)
{
	T tt = dot(tangent, tangent);
	T nt = dot(normal, tangent);
	T bt = dot(binormal, tangent);
	T nn = dot(normal, normal);
	T bn = dot(binormal, normal);
	T bb = dot(binormal, binormal);

	matrix A;
	A.ij[0][0] = tt; A.ij[0][1] = nt; A.ij[0][2] = bt;
	A.ij[1][0] = nt; A.ij[1][1] = nn; A.ij[1][2] = bn;
	A.ij[2][0] = bt; A.ij[2][1] = bn; A.ij[2][2] = bb;

	matrix B;
	if (!A.inverse(B)) return false;

	matrix C;
	C.ij[0][0] = tangent.x; C.ij[0][1] = normal.x; C.ij[0][2] = binormal.x;
	C.ij[1][0] = tangent.y; C.ij[1][1] = normal.y; C.ij[1][2] = binormal.y;
	C.ij[2][0] = tangent.z; C.ij[2][1] = normal.z; C.ij[2][2] = binormal.z;

	*this = B * C;

	return true;
}

template<class T> bool
matrix<3,3,T>::axes_to_world(vec_type const &tangent, vec_type const &normal, vec_type const &binormal)
{
	matrix world_to_axes;
	if (!world_to_axes.world_to_axes(tangent, normal, binormal)) return false;
	return world_to_axes.inverse(*this);
}

template<class T> void
matrix<4,4,T>::identity() {
	ij[0][0]=1;	ij[0][1]=0;	ij[0][2]=0;	ij[0][3]=0;
	ij[1][0]=0;	ij[1][1]=1;	ij[1][2]=0;	ij[1][3]=0;
	ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=1;	ij[2][3]=0;
	ij[3][0]=0;	ij[3][1]=0;	ij[3][2]=0;	ij[3][3]=1;
}

template<class T> void
matrix<4,4,T>::zero() {
	ij[0][0]=0;	ij[0][1]=0;	ij[0][2]=0;	ij[0][3]=0;
	ij[1][0]=0;	ij[1][1]=0;	ij[1][2]=0;	ij[1][3]=0;
	ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=0;	ij[2][3]=0;
	ij[3][0]=0;	ij[3][1]=0;	ij[3][2]=0;	ij[3][3]=0;
}

template<class T> void
matrix<4,4,T>::translation(T x,T y,T z) {
	ij[0][0]=1;		ij[0][1]=0;		ij[0][2]=0;		ij[0][3]=0;
	ij[1][0]=0;		ij[1][1]=1;		ij[1][2]=0;		ij[1][3]=0;
	ij[2][0]=0;		ij[2][1]=0;		ij[2][2]=1;		ij[2][3]=0;
	ij[3][0]=x;		ij[3][1]=y;		ij[3][2]=z;		ij[3][3]=1;
}

template<class T> void
matrix<4,4,T>::translation(point_type const &p) {
	translation(p.x,p.y,p.z);
}

template<class T> void
matrix<4,4,T>::translate(T x,T y,T z) {
	matrix m, t;
	m.translation(x,y,z);
	detail::mul(t, m, *this);
	*this = t;
}

template<class T> void
matrix<4,4,T>::translate(point_type const &p) {
	translate(p.x,p.y,p.z);
}

template<class T> void
matrix<4,4,T>::scaling(T sx,T sy,T sz) {
	ij[0][0]=sx;	ij[0][1]=0;	ij[0][2]=0;	ij[0][3]=0;
	ij[1][0]=0;	ij[1][1]=sy;	ij[1][2]=0;	ij[1][3]=0;
	ij[2][0]=0;	ij[2][1]=0;	ij[2][2]=sz;	ij[2][3]=0;
	ij[3][0]=0;	ij[3][1]=0;	ij[3][2]=0;	ij[3][3]=1;
}

template<class T> void
matrix<4,4,T>::scaling(point_type const &s) {
	scaling(s.x,s.y,s.z);
}

template<class T> void
matrix<4,4,T>::scale(T sx,T sy,T sz) {
	matrix m, t;
	m.scaling(sx,sy,sz);
	detail::mul(t, m, *this);
	*this = t;
}

template<class T> void
matrix<4,4,T>::scale(point_type const &s) {
	scale(s.x,s.y,s.z);
}

template<class T> void
matrix<4,4,T>::rotation(T alpha,T beta,T gamma) {
	T sa = sin(alpha),
		    ca = cos(alpha),
		    sb = sin(beta),
		    cb = cos(beta),
		    sg = sin(gamma),
		    cg = cos(gamma),
		    sa_sb = sa * sb,
		    ca_sb = ca * sb;

	ij[0][0] = cb*cg;
	ij[0][1] = cb*sg;
	ij[0][2] = -sb;
	ij[0][3] = 0;

	ij[1][0] = sa_sb*cg - ca*sg;
	ij[1][1] = sa_sb*sg + ca*cg;
	ij[1][2] = sa*cb;
	ij[1][3] = 0;

	ij[2][0] = ca_sb*cg + sa*sg;
	ij[2][1] = ca_sb*sg - sa*cg;
	ij[2][2] = ca*cb;
	ij[2][3] = 0;

	ij[3][0] = 0;
	ij[3][1] = 0;
	ij[3][2] = 0;
	ij[3][3] = 1;
}

template<class T> void
matrix<4,4,T>::rotation(quaternion_type const &q) {
	T wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	T s  = detail::rotation_scale(q);

	x2 = q.x * s;    y2 = q.y * s;    z2 = q.z * s;
	xx = q.x * x2;   xy = q.x * y2;   xz = q.x * z2;
	yy = q.y * y2;   yz = q.y * z2;   zz = q.z * z2;
	wx = q.w * x2;   wy = q.w * y2;   wz = q.w * z2;

	ij[0][0] = 1 - (yy + zz);
	ij[1][0] = xy - wz;
	ij[2][0] = xz + wy;
	ij[3][0] = 0;


	ij[0][1] = xy + wz;
	ij[1][1] = 1 - (xx + zz);
	ij[2][1] = yz - wx;
	ij[3][1] = 0;

	ij[0][2] = xz - wy;
	ij[1][2] = yz + wx;
	ij[2][2] = 1 - (xx + yy);
	ij[3][2] = 0;


	ij[0][3] = 0;
	ij[1][3] = 0;
	ij[2][3] = 0;
	ij[3][3] = 1;
}

template<class T> void
matrix<4,4,T>::rotation(point_type const &axis, T sine, T cosine) {
	// an axis of no length (or not a number) has no direction: it was a matrix of NaN
	detail::length_sq_of_a_direction(axis, "a turn about an axis");
	T length = axis.length();
	T a = axis.x / length;
	T b = axis.y / length;
	T c = axis.z / length;
	T aa = a * a;
	T bb = b * b;
	T cc = c * c;
	T omcos = 1 - cosine;

	ij[0][0] = aa + (1 - aa) * cosine;
	ij[1][1] = bb + (1 - bb) * cosine;
	ij[2][2] = cc + (1 - cc) * cosine;
	ij[0][1] = a * b * omcos + c * sine;
	ij[0][2] = a * c * omcos - b * sine;
	ij[1][0] = a * b * omcos - c * sine;
	ij[1][2] = b * c * omcos + a * sine;
	ij[2][0] = a * c * omcos + b * sine;
	ij[2][1] = b * c * omcos - a * sine;
	ij[0][3] = ij[1][3] = ij[2][3] = ij[3][0] = ij[3][1] = ij[3][2] = 0;
	ij[3][3] = 1;
}

template<class T> void
matrix<4,4,T>::rotation(point_type const &axis, T angle) {
	rotation(axis, sin(angle), cos(angle));
}

template<class T> void
matrix<4,4,T>::rotate(T alpha,T beta,T gamma) {
	matrix self = *this, temp;
	temp.rotation(alpha,beta,gamma);
	detail::mul(*this,temp,self);
}

template<class T> void
matrix<4,4,T>::rotate(point_type const &v) {
	rotate(v.A,v.B,v.C);
}

template<class T> void
matrix<4,4,T>::rotate(quaternion_type const &q) {
	matrix self = *this, temp;
	temp.rotation(q);
	detail::mul(*this,temp,self);
}

template<class T> void
matrix<4,4,T>::rotate(point_type const &axis, T sine, T cosine) {
	matrix self = *this, temp;
	temp.rotation(axis, sine, cosine);
	detail::mul(*this, temp, self);
}

template<class T> void
matrix<4,4,T>::rotate(point_type const &axis, T angle) {
	matrix self = *this, temp;
	temp.rotation(axis, angle);
	detail::mul(*this, temp, self);
}

template<class T> void
matrix<4,4,T>::direction(point_type const &dir) {
	point_type x_axis = dir.check_nonzero("rove: matrix::direction along a vector", T(0));
	x_axis.normalize();

	point_type y_axis;
	y_axis.set(0,1,0);

	// a dir straight up or down has no z: the axes are then zero, as they always were
	point_type z_axis = normalize_or_zero(cross(x_axis, y_axis));
	y_axis = normalize_or_zero(cross(z_axis, x_axis));

	ij[0][0] = x_axis.x;	ij[0][1] = x_axis.y;	ij[0][2] = x_axis.z;	ij[0][3] = 0;
	ij[1][0] = y_axis.x;	ij[1][1] = y_axis.y;	ij[1][2] = y_axis.z;	ij[1][3] = 0;
	ij[2][0] = z_axis.x;	ij[2][1] = z_axis.y;	ij[2][2] = z_axis.z;	ij[2][3] = 0;
	ij[3][0] = 0;		    ij[3][1] = 0;		    ij[3][2] = 0;		    ij[3][3] = 1;
}

template<class T> void
matrix<4,4,T>::direct(point_type const &dir) {
	matrix self = *this, temp;
	temp.direction(dir);
	detail::mul(*this, temp, self);
}

template<class T> void
matrix<4,4,T>::lookat(point_type const &eye,point_type const &at,point_type const &up)
{
	point_type x_axis,y_axis,z_axis;

	z_axis = normalize((at - eye).check_nonzero("rove: lookat from an eye at the target", T(0)));
	x_axis = normalize_or_zero(cross(z_axis, up));     // zero for an up along the view, as it always was
	y_axis = cross(x_axis, z_axis);

	ij[0][0]=x_axis.x;	ij[0][1]=y_axis.x;	ij[0][2]=z_axis.x;	ij[0][3]=0;
	ij[1][0]=x_axis.y;	ij[1][1]=y_axis.y;	ij[1][2]=z_axis.y;	ij[1][3]=0;
	ij[2][0]=x_axis.z;	ij[2][1]=y_axis.z;	ij[2][2]=z_axis.z;	ij[2][3]=0;

	ij[3][0]=-dot(x_axis, eye);
	ij[3][1]=-dot(y_axis, eye);
	ij[3][2]=-dot(z_axis, eye);
	ij[3][3]=1;
}

template<class T> void
matrix<4,4,T>::ortho(T width,T height,T z_near,T z_far)
{
	if (!(width != 0 && height != 0 && z_near != z_far))
		throw std::invalid_argument("rove: ortho of no size, or with nothing between z_near and z_far");
	ij[0][0] = 2/width;
	ij[0][1] = 0;
	ij[0][2] = 0;
	ij[0][3] = 0;

	ij[1][0] = 0;
	ij[1][1] = 2/height;
	ij[1][2] = 0;
	ij[1][3] = 0;

	ij[2][0] = 0;
	ij[2][1] = 0;
	ij[2][2] = 1 / (z_far - z_near);
	ij[2][3] = 0;

	ij[3][0] = 0;
	ij[3][1] = 0;
	ij[3][2] = z_near / (z_near - z_far);
	ij[3][3] = 1;
}

template<class T> void
matrix<4,4,T>::perspective(T fovy,T aspect,T z_near,T z_far)
{
	if (!(fovy != 0 && aspect != 0 && z_near != z_far))
		throw std::invalid_argument("rove: perspective of no field of view or aspect, or with nothing between z_near and z_far");
	T h = 1 / tan(fovy/2);
	T w = h / aspect;

	ij[0][0] = w;
	ij[0][1] = 0;
	ij[0][2] = 0;
	ij[0][3] = 0;

	ij[1][0] = 0;
	ij[1][1] = h;
	ij[1][2] = 0;
	ij[1][3] = 0;

	ij[2][0] = 0;
	ij[2][1] = 0;
	ij[2][2] = z_far/(z_far-z_near);
	ij[2][3] = 1;

	ij[3][0] = 0;
	ij[3][1] = 0;
	ij[3][2] = -z_near*z_far/(z_far-z_near);
	ij[3][3] = 0;
}

template class matrix<3,3>;
template class matrix<4,4>;
template class matrix<3,3,double>;
template class matrix<4,4,double>;
#ifdef ROVE_DFLOAT
template class matrix<3,3,dfloat>;
template class matrix<4,4,dfloat>;
#endif

}
