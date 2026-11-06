// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <rove/matrix.h>
#include <rove/obb.h>
#include <rove/frustum.h>
#include <rove/sphere.h>

BOOST_AUTO_TEST_SUITE(frustum)

BOOST_AUTO_TEST_CASE(orthographic_projection_and_obb_1)
{
	rove::matrix<4,4> tf;
	tf.ortho(400, 300, 0, 1000);

	rove::frustum<> fr(tf);

	rove::obb<3> bound;
	bound.origin.set(-1000, 0, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == false);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_and_obb_2)
{
	rove::matrix<4,4> tf;
	tf.ortho(400, 300, 0, 1000);

	rove::frustum<> fr(tf);

	rove::obb<3> bound;
	bound.origin.set(-199.5f, 0, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == true);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_and_obb_3)
{
	rove::matrix<4,4> tf;
	tf.ortho(400, 300, 0, 1000);

	rove::frustum<> fr(tf);

	rove::obb<3> bound;
	bound.origin.set(0, 0, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == true);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_and_obb_4)
{
	rove::matrix<4,4> tf;
	tf.ortho(400, 300, 0, 1000);

	rove::frustum<> fr(tf);

	rove::obb<3> bound;
	bound.origin.set(199.5f, 0, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == true);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_and_obb_5)
{
	rove::matrix<4,4> tf;
	tf.ortho(400, 300, 0, 1000);

	rove::frustum<> fr(tf);

	rove::obb<3> bound;
	bound.origin.set(1000, 0, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == false);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_with_rotation_and_obb_1)
{
	rove::matrix<4,4> projection;
	projection.ortho(400, 300, 0, 1000);

	rove::matrix<4,4> world;
	world.rotation(-rove::PI/4, 0, 0);

	rove::frustum<> fr(projection * world);

	rove::obb<3> bound;
	bound.origin.set(0, 1000, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == false);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_with_rotation_and_obb_2)
{
	rove::matrix<4,4> projection;
	projection.ortho(400, 300, 0, 1000);

	rove::matrix<4,4> world;
	world.rotation(-rove::PI/4, 0, 0);

	rove::frustum<> fr(projection * world);

	rove::obb<3> bound;
	bound.origin.set(0, 0, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == true);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_with_rotation_and_obb_3)
{
	rove::matrix<4,4> projection;
	projection.ortho(400, 300, 0, 1000);

	rove::matrix<4,4> world;
	world.rotation(-rove::PI/4, 0, 0);

	rove::frustum<> fr(projection * world);

	rove::obb<3> bound;
	bound.origin.set(0, -1000, 0);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.test_intersection(bound) == false);
}

BOOST_AUTO_TEST_CASE(perspective_projection_with_rotation_translation_and_obb_1)
{
	rove::matrix<4,4> projection;
	projection.perspective(rove::PI/2, 1.5f, 0, 1000);

	rove::matrix<4,4> world;
	world.translation(-500, -10, 500);
	world.rotate(-rove::PI/4, 0, 0);

	rove::frustum<> fr(projection * world);

	rove::obb<3> bound;
	bound.origin.set(510, 2, 490);
	bound.tangent.set(1, 0, 0);
	bound.normal.set(0, 1, 0);
	bound.binormal.set(0, 0, 1);

	BOOST_REQUIRE (fr.contains(bound.origin) == true);
	BOOST_REQUIRE (fr.contains(bound.origin + bound.tangent) == true);
	BOOST_REQUIRE (fr.contains(bound.origin + bound.normal) == true);
	BOOST_REQUIRE (fr.contains(bound.origin + bound.binormal) == true);

	BOOST_REQUIRE (fr.test_intersection(bound) == true);
}

BOOST_AUTO_TEST_CASE(from_matrix_and_intersection_with_obb_1)
{
	rove::matrix<4,4> tf;
	tf._11 =  0.20000000f;
	tf._12 =  0.00000000f;
	tf._13 =  0.00000000f;
	tf._14 =  0.00000000f;
	tf._21 =  0.00000000f;
	tf._22 =  0.26937401f;
	tf._23 = -0.00070781459f;
	tf._24 =  0.00000000f;
	tf._31 =  0.00000000f;
	tf._32 =  0.26937401f;
	tf._33 =  0.00070781459f;
	tf._34 =  0.00000000f;
	tf._41 = -0.19687501f;
	tf._42 = -0.0061834427f;
	tf._43 =  0.012431229f;
	tf._44 =  1.0000000f;

	rove::frustum<> fr(tf);

	rove::obb<3> bound;
	bound.origin.set(-5.6245117f, 0.00000000f, 23.716305f);
	bound.tangent.set(2.0000000f, 0.00000000f, 0.00000000f);
	bound.normal.set(0.00000000f, 1.0000000f, 0.00000000f);
	bound.binormal.set(0.00000000f, 0.00000000f, 2.0000000f);

	BOOST_REQUIRE (fr.contains(bound.origin) == false);
	BOOST_REQUIRE (fr.contains(bound.origin + bound.tangent) == false);
	BOOST_REQUIRE (fr.contains(bound.origin + bound.normal) == false);
	BOOST_REQUIRE (fr.contains(bound.origin + bound.binormal) == false);

	BOOST_REQUIRE (fr.test_intersection(bound) == false);
}

BOOST_AUTO_TEST_CASE(the_near_plane_is_where_the_projection_puts_depth_0)
{
	// rove's projections put depth 0 at z_near (Vulkan's and Direct3D's 0..1); the plane was taken as OpenGL's -w <= z, which
	// for this projection lies at about half the near distance, so what was between it and the near plane was not culled
	rove::matrix<4, 4> p;
	p.perspective(1.0f, 1.0f, 1.0f, 100.0f);
	rove::frustum<> const f(p);
	BOOST_REQUIRE(!f.contains(rove::vec<3>(0, 0, 0.6f)));
	BOOST_REQUIRE(!f.contains(rove::vec<3>(0, 0, 0.99f)));
	BOOST_REQUIRE(f.contains(rove::vec<3>(0, 0, 1.01f)));
	BOOST_REQUIRE(f.contains(rove::vec<3>(0, 0, 99.0f)));
	BOOST_REQUIRE(!f.contains(rove::vec<3>(0, 0, 101.0f)));
	BOOST_REQUIRE(!f.test_intersection(rove::aabb<3>(rove::vec<3>(-0.1f, -0.1f, 0.5f), rove::vec<3>(0.1f, 0.1f, 0.9f))));
	BOOST_REQUIRE(f.test_intersection(rove::aabb<3>(rove::vec<3>(-0.1f, -0.1f, 0.5f), rove::vec<3>(0.1f, 0.1f, 1.1f))));
	// an orthographic one too
	rove::matrix<4, 4> o;
	o.ortho(10, 10, 2, 50);
	rove::frustum<> const g(o);
	BOOST_REQUIRE(!g.contains(rove::vec<3>(0, 0, 1.5f)));
	BOOST_REQUIRE(g.contains(rove::vec<3>(0, 0, 2.5f)));
}

BOOST_AUTO_TEST_CASE(a_sphere_meets_the_frustum_unless_it_is_wholly_outside_a_plane)
{
	rove::matrix<4, 4> p;
	p.perspective(rove::PI / 2, 1.0f, 1.0f, 100.0f);      // 45 degrees either side of +z
	rove::frustum<> const f(p);
	BOOST_REQUIRE(f.test_intersection(rove::sphere<3>(rove::vec<3>(0, 0, 50), 1)));
	BOOST_REQUIRE(!f.test_intersection(rove::sphere<3>(rove::vec<3>(0, 0, -5), 1)));                // behind
	BOOST_REQUIRE(!f.test_intersection(rove::sphere<3>(rove::vec<3>(0, 0, 105), 4.9f)));            // past the far plane
	BOOST_REQUIRE(f.test_intersection(rove::sphere<3>(rove::vec<3>(0, 0, 105), 5.1f)));             // reaching over it
	// beside the right side plane x = z (distance (x - z) / sqrt 2): the planes are not of unit length, the distance must be
	BOOST_REQUIRE(!f.test_intersection(rove::sphere<3>(rove::vec<3>(20, 0, 10), 7.0f)));            // 7.07 away
	BOOST_REQUIRE(f.test_intersection(rove::sphere<3>(rove::vec<3>(20, 0, 10), 7.2f)));
	BOOST_REQUIRE(!f.test_intersection(rove::sphere<3>(rove::vec<3>(-20, 0, 10), 7.0f)));
	BOOST_REQUIRE(!f.test_intersection(rove::sphere<3>(rove::vec<3>(0, 20, 10), 7.0f)));
	BOOST_REQUIRE(f.test_intersection(rove::sphere<3>(rove::vec<3>(0, -20, 10), 7.2f)));
	// a sphere is never refused where its box is met by a plane test and it reaches inside
	BOOST_REQUIRE(f.test_intersection(rove::sphere<3>(rove::vec<3>(0, 0, 0.5f), 0.6f)));
}

BOOST_AUTO_TEST_SUITE_END()
