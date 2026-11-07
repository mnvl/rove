// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cmath>
#include <random>
#include <boost/test/unit_test.hpp>
#include <rove/obb.h>
#include <rove/aabb.h>
#include <rove/capsule.h>
#include <rove/ray.h>
#include <rove/matrix.h>
#include <rove/triangle.h>
#include "test_tolerance.h"

BOOST_AUTO_TEST_SUITE(obb)

// 3D OBB Construction tests
BOOST_AUTO_TEST_CASE(construct_3d_from_aabb)
{
	rove::aabb<3> a(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 20, 30));
	rove::obb<3> b(a);

	BOOST_REQUIRE((b.origin - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.tangent - rove::vec<3>(10, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.normal - rove::vec<3>(0, 20, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.binormal - rove::vec<3>(0, 0, 30)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(construct_3d_from_vectors)
{
	rove::vec<3> o(1, 2, 3);
	rove::vec<3> i(4, 0, 0);
	rove::vec<3> j(0, 5, 0);
	rove::vec<3> k(0, 0, 6);

	rove::obb<3> b(o, i, j, k);

	BOOST_REQUIRE((b.origin - o).length() < TOLERANCE);
	BOOST_REQUIRE((b.tangent - i).length() < TOLERANCE);
	BOOST_REQUIRE((b.normal - j).length() < TOLERANCE);
	BOOST_REQUIRE((b.binormal - k).length() < TOLERANCE);
}

// 2D OBB Construction tests
BOOST_AUTO_TEST_CASE(construct_2d_from_aabb)
{
	rove::aabb<2> a(rove::vec<2>(0, 0), rove::vec<2>(10, 20));
	rove::obb<2> b(a);

	BOOST_REQUIRE((b.origin - rove::vec<2>(0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.tangent - rove::vec<2>(10, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.normal - rove::vec<2>(0, 20)).length() < TOLERANCE);
}

// 3D Geometric property tests
BOOST_AUTO_TEST_CASE(get_centre_3d)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(10, 0, 0);
	b.normal.set(0, 20, 0);
	b.binormal.set(0, 0, 30);

	rove::vec<3> c = b.centre();

	BOOST_REQUIRE((c - rove::vec<3>(5, 10, 15)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(get_volume_3d)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(2, 0, 0);
	b.normal.set(0, 3, 0);
	b.binormal.set(0, 0, 4);

	rove::scalar volume = b.volume();

	BOOST_REQUIRE(rove::abs(volume - 24.0f) < TOLERANCE);
}

// 3D Contains tests
BOOST_AUTO_TEST_CASE(contains_point_3d_inside)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(10, 0, 0);
	b.normal.set(0, 10, 0);
	b.binormal.set(0, 0, 10);

	BOOST_REQUIRE(b.contains(rove::vec<3>(5, 5, 5)));
	BOOST_REQUIRE(b.contains(rove::vec<3>(0, 0, 0))); // boundary
	BOOST_REQUIRE(b.contains(rove::vec<3>(10, 10, 10))); // boundary
}

BOOST_AUTO_TEST_CASE(contains_point_3d_outside)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(10, 0, 0);
	b.normal.set(0, 10, 0);
	b.binormal.set(0, 0, 10);

	BOOST_REQUIRE(!b.contains(rove::vec<3>(15, 5, 5)));
	BOOST_REQUIRE(!b.contains(rove::vec<3>(-1, 5, 5)));
	BOOST_REQUIRE(!b.contains(rove::vec<3>(5, 11, 5)));
}

// 2D Contains tests
BOOST_AUTO_TEST_CASE(contains_point_2d_inside)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	BOOST_REQUIRE(b.contains(rove::vec<2>(5, 5)));
	BOOST_REQUIRE(b.contains(rove::vec<2>(0, 0)));
}

BOOST_AUTO_TEST_CASE(contains_point_2d_outside)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	BOOST_REQUIRE(!b.contains(rove::vec<2>(15, 5)));
	BOOST_REQUIRE(!b.contains(rove::vec<2>(-1, 5)));
}

// Rotated OBB contains test
BOOST_AUTO_TEST_CASE(contains_rotated_obb_3d)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(1, 1, 0); // rotated 45 degrees in XY plane
	b.normal.set(-1, 1, 0);
	b.binormal.set(0, 0, 1);

	// Point at center of the rotated box
	rove::vec<3> center = b.origin + b.tangent * 0.5f + b.normal * 0.5f + b.binormal * 0.5f;
	BOOST_REQUIRE(b.contains(center));
}

// Get vertex tests
BOOST_AUTO_TEST_CASE(get_vertex_3d)
{
	rove::obb<3> b;
	b.origin.set(1, 2, 3);
	b.tangent.set(4, 0, 0);
	b.normal.set(0, 5, 0);
	b.binormal.set(0, 0, 6);

	rove::vec<3> v0, v7;
	v0 = b.vertex(0);
	v7 = b.vertex(7);

	BOOST_REQUIRE((v0 - b.origin).length() < TOLERANCE);
	BOOST_REQUIRE((v7 - (b.origin + b.tangent + b.normal + b.binormal)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(get_vertex_2d)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	rove::vec<2> v0 = b.vertex(0);
	rove::vec<2> v1 = b.vertex(1);
	rove::vec<2> v2 = b.vertex(2);
	rove::vec<2> v3 = b.vertex(3);

	BOOST_REQUIRE((v0 - rove::vec<2>(0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((v1 - rove::vec<2>(10, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((v2 - rove::vec<2>(10, 10)).length() < TOLERANCE);
	BOOST_REQUIRE((v3 - rove::vec<2>(0, 10)).length() < TOLERANCE);
}

// Get side tests for 2D
BOOST_AUTO_TEST_CASE(get_side_2d)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	rove::segment<2> side0 = b.side(0);
	BOOST_REQUIRE((side0.A - rove::vec<2>(0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((side0.B - rove::vec<2>(10, 0)).length() < TOLERANCE);
}

// Get normal tests for 2D
BOOST_AUTO_TEST_CASE(get_normal_2d)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	rove::vec<2> n0 = b.side_normal(0);
	rove::vec<2> n1 = b.side_normal(1);
	rove::vec<2> n2 = b.side_normal(2);
	rove::vec<2> n3 = b.side_normal(3);

	// Normals should be unit vectors
	BOOST_REQUIRE(rove::abs(n0.length() - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(n1.length() - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(n2.length() - 1.0f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(n3.length() - 1.0f) < TOLERANCE);
}

// Get segment tests (octants)
BOOST_AUTO_TEST_CASE(get_segment_3d)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(10, 0, 0);
	b.normal.set(0, 10, 0);
	b.binormal.set(0, 0, 10);

	rove::obb<3> segment;
	segment = b.octant(0);

	// Segment 0 should have origin at (0,0,0) and half dimensions
	BOOST_REQUIRE((segment.origin - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((segment.tangent - rove::vec<3>(5, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((segment.normal - rove::vec<3>(0, 5, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((segment.binormal - rove::vec<3>(0, 0, 5)).length() < TOLERANCE);
}

// Get AABB tests
BOOST_AUTO_TEST_CASE(get_aabb_3d_axis_aligned)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(10, 0, 0);
	b.normal.set(0, 20, 0);
	b.binormal.set(0, 0, 30);

	rove::aabb<3> aabb;
	aabb = b.bounds();

	BOOST_REQUIRE((aabb.lo - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((aabb.hi - rove::vec<3>(10, 20, 30)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(get_aabb_2d)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	rove::aabb<2> aabb;
	aabb = b.bounds();

	BOOST_REQUIRE((aabb.lo - rove::vec<2>(0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((aabb.hi - rove::vec<2>(10, 10)).length() < TOLERANCE);
}

// World to local transformation tests
BOOST_AUTO_TEST_CASE(world_to_local_vector)
{
	rove::obb<3> bounds;
	bounds.origin.set(0, 0, 0);
	bounds.tangent.set(1, 2, 3);
	bounds.normal.set(3, 1, 2);
	bounds.binormal.set(2, 3, 1);

	rove::vec<3> point, point_local, point_world;

	point.set(100, 50, -2);

	bounds.world_to_local_vector(point_local, point);
	bounds.local_to_world_vector(point_world, point_local);

	BOOST_REQUIRE((point - point_world).length_sq() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(world_to_local_point_3d)
{
	rove::obb<3> b;
	b.origin.set(10, 10, 10);
	b.tangent.set(20, 0, 0);
	b.normal.set(0, 20, 0);
	b.binormal.set(0, 0, 20);

	rove::vec<3> world_point(20, 20, 20); // center of the box
	rove::vec<3> local_point;

	b.world_to_local_point(local_point, world_point);

	// Should be at (0.5, 0.5, 0.5) in local space
	BOOST_REQUIRE(rove::abs(local_point.x - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_point.y - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_point.z - 0.5f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(world_to_local_vector_2d)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 20);

	rove::vec<2> world_vec(5, 10);
	rove::vec<2> local_vec = b.world_to_local_vector(world_vec);

	BOOST_REQUIRE(rove::abs(local_vec.x - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_vec.y - 0.5f) < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(world_to_local_point_2d)
{
	rove::obb<2> b;
	b.origin.set(5, 5);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	rove::vec<2> world_point(10, 10); // center
	rove::vec<2> local_point = b.world_to_local_point(world_point);

	BOOST_REQUIRE(rove::abs(local_point.x - 0.5f) < TOLERANCE);
	BOOST_REQUIRE(rove::abs(local_point.y - 0.5f) < TOLERANCE);
}

// Transformation matrix tests
BOOST_AUTO_TEST_CASE(transform_3d_identity)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(10, 0, 0);
	b.normal.set(0, 10, 0);
	b.binormal.set(0, 0, 10);

	rove::matrix<4, 4> identity;
	identity.identity();

	b.transform(identity);

	BOOST_REQUIRE((b.origin - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.tangent - rove::vec<3>(10, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.normal - rove::vec<3>(0, 10, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.binormal - rove::vec<3>(0, 0, 10)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(transform_3d_translation)
{
	rove::obb<3> b;
	b.origin.set(0, 0, 0);
	b.tangent.set(10, 0, 0);
	b.normal.set(0, 10, 0);
	b.binormal.set(0, 0, 10);

	rove::matrix<4, 4> translation;
	translation.identity();
	translation.translation(5, 5, 5);

	b.transform(translation);

	BOOST_REQUIRE((b.origin - rove::vec<3>(5, 5, 5)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(transform_2d_identity)
{
	rove::obb<2> b;
	b.origin.set(0, 0);
	b.tangent.set(10, 0);
	b.normal.set(0, 10);

	rove::matrix<3, 3> identity;
	identity.identity();

	b.transform(identity);

	BOOST_REQUIRE((b.origin - rove::vec<2>(0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.tangent - rove::vec<2>(10, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((b.normal - rove::vec<2>(0, 10)).length() < TOLERANCE);
}

// make_sweep_volume tests
BOOST_AUTO_TEST_CASE(sweep_volume_axis_aligned)
{
	// Axis-aligned box at origin, moving along +X
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),  // tangent
		rove::vec<3>(0, 3, 0),  // normal
		rove::vec<3>(0, 0, 4)   // binormal
	);
	rove::vec<3> disp(10, 0, 0);
	rove::obb<3> out[6];
	rove::obb<3>::make_sweep_volume(box, disp, out);

	// bottom face (binormal=0): origin=box.origin, edges=tangent, normal, disp
	BOOST_REQUIRE((out[0].origin - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[0].tangent - rove::vec<3>(2, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[0].normal - rove::vec<3>(0, 3, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[0].binormal - rove::vec<3>(10, 0, 0)).length() < TOLERANCE);

	// top face (binormal=1): origin=box.origin + binormal
	BOOST_REQUIRE((out[1].origin - rove::vec<3>(0, 0, 4)).length() < TOLERANCE);
	BOOST_REQUIRE((out[1].tangent - rove::vec<3>(2, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[1].normal - rove::vec<3>(0, 3, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[1].binormal - rove::vec<3>(10, 0, 0)).length() < TOLERANCE);

	// left face (normal=0): origin=box.origin, edges=tangent, binormal, disp
	BOOST_REQUIRE((out[2].origin - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[2].tangent - rove::vec<3>(2, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[2].normal - rove::vec<3>(0, 0, 4)).length() < TOLERANCE);
	BOOST_REQUIRE((out[2].binormal - rove::vec<3>(10, 0, 0)).length() < TOLERANCE);

	// right face (normal=1): origin=box.origin + normal
	BOOST_REQUIRE((out[3].origin - rove::vec<3>(0, 3, 0)).length() < TOLERANCE);

	// back face (tangent=0): origin=box.origin, edges=normal, binormal, disp
	BOOST_REQUIRE((out[4].origin - rove::vec<3>(0, 0, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[4].tangent - rove::vec<3>(0, 3, 0)).length() < TOLERANCE);
	BOOST_REQUIRE((out[4].normal - rove::vec<3>(0, 0, 4)).length() < TOLERANCE);
	BOOST_REQUIRE((out[4].binormal - rove::vec<3>(10, 0, 0)).length() < TOLERANCE);

	// front face (tangent=1): origin=box.origin + tangent
	BOOST_REQUIRE((out[5].origin - rove::vec<3>(2, 0, 0)).length() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(sweep_volume_detects_thin_wall)
{
	// Small box at X=-5, moving to X=+5 (displacement 10).
	// A thin wall at X=0 (YZ plane) should be missed by the final-position
	// OBB but caught by the front face sweep OBB.
	rove::obb<3> box(
		rove::vec<3>(-5, -0.5f, -0.5f),
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(0, 1, 0),
		rove::vec<3>(0, 0, 1)
	);
	rove::vec<3> disp(10, 0, 0);

	// Triangle representing a thin wall at X=0
	rove::triangle<3> wall(
		rove::vec<3>(0, -2, -2),
		rove::vec<3>(0, 2, -2),
		rove::vec<3>(0, 0, 2)
	);

	// Final-position OBB (at X=5) should NOT intersect the wall
	rove::obb<3> final_box(
		rove::vec<3>(5, -0.5f, -0.5f),
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(0, 1, 0),
		rove::vec<3>(0, 0, 1)
	);
	BOOST_REQUIRE(!final_box.test_intersection(wall));

	// But sweep OBBs should detect the wall
	rove::obb<3> sweep[6];
	rove::obb<3>::make_sweep_volume(box, disp, sweep);

	bool any_hit = false;
	for (int i = 0; i < 6; ++i) {
		if (sweep[i].test_intersection(wall)) {
			any_hit = true;
			break;
		}
	}
	BOOST_REQUIRE(any_hit);
}

BOOST_AUTO_TEST_CASE(sweep_volume_rotated_box)
{
	// 45-degree rotated box in XY plane
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 1, 0),   // tangent rotated 45°
		rove::vec<3>(-1, 1, 0),  // normal rotated 45°
		rove::vec<3>(0, 0, 2)    // binormal along Z
	);
	rove::vec<3> disp(5, 0, 0);
	rove::obb<3> sweep[6];
	rove::obb<3>::make_sweep_volume(box, disp, sweep);

	// Verify origins match the expected face offsets
	BOOST_REQUIRE((sweep[0].origin - box.origin).length() < TOLERANCE);
	BOOST_REQUIRE((sweep[1].origin - (box.origin + box.binormal)).length() < TOLERANCE);
	BOOST_REQUIRE((sweep[2].origin - box.origin).length() < TOLERANCE);
	BOOST_REQUIRE((sweep[3].origin - (box.origin + box.normal)).length() < TOLERANCE);
	BOOST_REQUIRE((sweep[4].origin - box.origin).length() < TOLERANCE);
	BOOST_REQUIRE((sweep[5].origin - (box.origin + box.tangent)).length() < TOLERANCE);

	// All sweep OBBs should have displacement as their last edge vector
	for (int i = 0; i < 6; ++i) {
		BOOST_REQUIRE((sweep[i].binormal - disp).length() < TOLERANCE);
	}
}

// is_degenerate tests
BOOST_AUTO_TEST_CASE(degenerate_axis_aligned_box)
{
	// Well-formed axis-aligned box is not degenerate.
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 3, 0),
		rove::vec<3>(0, 0, 4)
	);
	BOOST_REQUIRE(!box.is_degenerate());
	BOOST_REQUIRE(box.is_basis_linearly_independent());
}

BOOST_AUTO_TEST_CASE(degenerate_parallel_tangent_binormal)
{
	// Tangent and binormal both along X → degenerate.
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),   // tangent along X
		rove::vec<3>(0, 3, 0),   // normal along Y
		rove::vec<3>(0.5, 0, 0)  // binormal along X (parallel to tangent)
	);
	BOOST_REQUIRE(box.is_degenerate());
	BOOST_REQUIRE(!box.is_basis_linearly_independent());
}

BOOST_AUTO_TEST_CASE(degenerate_parallel_normal_binormal)
{
	// Normal and binormal both along Y → degenerate.
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 3, 0),
		rove::vec<3>(0, 1, 0)
	);
	BOOST_REQUIRE(box.is_degenerate());
}

BOOST_AUTO_TEST_CASE(degenerate_zero_length_edge)
{
	// Zero-length binormal → degenerate.
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(2, 0, 0),
		rove::vec<3>(0, 3, 0),
		rove::vec<3>(0, 0, 0)
	);
	BOOST_REQUIRE(box.is_degenerate());
}

BOOST_AUTO_TEST_CASE(degenerate_sweep_obb_forward_displacement)
{
	// Van-like OBB moving forward: displacement parallel to tangent.
	// Sweep OBBs 0-3 should be degenerate, 4-5 should not.
	rove::obb<3> box(
		rove::vec<3>(-0.8f, 0.15f, 0.2f),
		rove::vec<3>(-0.4f, 0, 0),
		rove::vec<3>(0, -0.3f, 0),
		rove::vec<3>(0, 0, 0.4f)
	);
	rove::vec<3> disp(-0.03f, 0, 0);
	rove::obb<3> sweep[6];
	rove::obb<3>::make_sweep_volume(box, disp, sweep);

	// Bottom/top faces: tangent=(-0.4,0,0), binormal=disp=(-0.03,0,0) → parallel.
	BOOST_REQUIRE(sweep[0].is_degenerate());
	BOOST_REQUIRE(sweep[1].is_degenerate());
	// Left/right faces: tangent=(-0.4,0,0), binormal=disp=(-0.03,0,0) → parallel.
	BOOST_REQUIRE(sweep[2].is_degenerate());
	BOOST_REQUIRE(sweep[3].is_degenerate());
	// Back/front faces: tangent=(0,-0.3,0), binormal=disp=(-0.03,0,0) → not parallel.
	BOOST_REQUIRE(!sweep[4].is_degenerate());
	BOOST_REQUIRE(!sweep[5].is_degenerate());
}

BOOST_AUTO_TEST_CASE(degenerate_rotated_box_not_degenerate)
{
	// 45-degree rotated box: all edges are independent.
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 1, 0),
		rove::vec<3>(-1, 1, 0),
		rove::vec<3>(0, 0, 2)
	);
	BOOST_REQUIRE(!box.is_degenerate());
}

// ── False-positive regression tests ──────────────────────────────────────────

BOOST_AUTO_TEST_CASE(triangle_near_cube_corner_no_intersection)
{
	// Triangle on the z=0.5 plane, just outside the unit cube's corner at
	// (1,1).  Face-normal and triangle-normal projections all overlap, but
	// the triangle does not actually touch the cube.  Only the edge-edge
	// cross-product axis (1,1,0) detects the separation.
	rove::obb<3> cube(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(0, 1, 0),
		rove::vec<3>(0, 0, 1)
	);
	rove::triangle<3> tri(
		rove::vec<3>(2.0f, 0.5f, 0.5f),
		rove::vec<3>(0.5f, 2.0f, 0.5f),
		rove::vec<3>(2.0f, 2.0f, 0.5f)
	);
	BOOST_CHECK(!cube.test_intersection(tri));
}

BOOST_AUTO_TEST_CASE(swept_obb_false_positive_regression)
{
	// A thin swept face OBB (from make_sweep_volume) and a triangle that
	// is clearly separated.  Without edge-edge cross-product axes the
	// incomplete SAT reports a false intersection.
	rove::obb<3> van(
		rove::vec<3>(-0.2f, -0.15f, 0.2f),
		rove::vec<3>(0.4f, 0, 0),
		rove::vec<3>(0, 0.3f, 0),
		rove::vec<3>(0, 0, 0.4f)
	);
	rove::vec<3> disp(0, 0.05f, 0);
	rove::obb<3> sweep[6];
	rove::obb<3>::make_sweep_volume(van, disp, sweep);

	// Triangle well away from the van, near x=5, y=5.
	rove::triangle<3> far_tri(
		rove::vec<3>(5.5f, 4.5f, 0.4f),
		rove::vec<3>(4.5f, 5.5f, 0.4f),
		rove::vec<3>(5.5f, 5.5f, 0.4f)
	);
	for (int i = 0; i < 6; ++i) {
		if (!sweep[i].is_degenerate()) {
			BOOST_CHECK_MESSAGE(
				!sweep[i].test_intersection(far_tri),
				"sweep OBB " << i << " false positive with distant triangle"
			);
		}
	}
}

BOOST_AUTO_TEST_CASE(obb_triangle_typo_v0_z_regression)
{
	// Triangle with vertex A inside the OBB and B,C above it.  The old
	// code checked C.z>1 twice (never A.z), incorrectly finding a
	// z-face separating axis and missing this real intersection.
	rove::obb<3> box(
		rove::vec<3>(0, 0, 0),
		rove::vec<3>(1, 0, 0),
		rove::vec<3>(0, 1, 0),
		rove::vec<3>(0, 0, 1)
	);
	rove::triangle<3> tri(
		rove::vec<3>(0.5f, 0.5f, 0.5f),  // A — inside the OBB
		rove::vec<3>(0.5f, 0.0f, 2.0f),  // B — above
		rove::vec<3>(0.0f, 0.5f, 2.0f)   // C — above
	);
	BOOST_CHECK(box.test_intersection(tri));
}

// ── Brute-force false-positive search ────────────────────────────────────────
// Generate 1000 non-intersecting cases in easy positions (primitives outside
// bounding sphere), transform with random rotation+translation matrices,
// and verify that no false positive is reported.

namespace {

// Deterministic LCG so tests are reproducible across platforms.
struct TestRNG {
	uint32_t state;
	explicit TestRNG(uint32_t seed = 42) : state(seed) {}
	uint32_t next() { state = state * 1664525u + 1013904223u; return state; }
	float uniform(float lo = 0.f, float hi = 1.f) {
		return lo + (hi - lo) * float(next() & 0xFFFF) / 65535.f;
	}
};

// Build a random rigid-body transform (rotation + translation).
rove::matrix<4,4> random_transform(TestRNG &rng) {
	rove::matrix<4,4> m;
	m.rotation(rng.uniform(0.f, 6.2832f), rng.uniform(0.f, 6.2832f), rng.uniform(0.f, 6.2832f));
	m.ij[3][0] = rng.uniform(-50.f, 50.f);
	m.ij[3][1] = rng.uniform(-50.f, 50.f);
	m.ij[3][2] = rng.uniform(-50.f, 50.f);
	return m;
}

// Transform a vec3 by a 4×4 matrix (homogeneous, w=1).
rove::vec<3> xform_point(rove::matrix<4,4> const &m, rove::vec<3> const &p) {
	return m * p;
}

// Transform a direction (w=0) by the upper-left 3×3 of a 4×4 matrix.
rove::vec<3> xform_dir(rove::matrix<4,4> const &m, rove::vec<3> const &d) {
	return rove::vec<3>(
		m.ij[0][0]*d.x + m.ij[1][0]*d.y + m.ij[2][0]*d.z,
		m.ij[0][1]*d.x + m.ij[1][1]*d.y + m.ij[2][1]*d.z,
		m.ij[0][2]*d.x + m.ij[1][2]*d.y + m.ij[2][2]*d.z
	);
}

// Random unit vector on the sphere.
rove::vec<3> random_direction(TestRNG &rng) {
	// Marsaglia's method
	float x, y, s;
	do {
		x = rng.uniform(-1.f, 1.f);
		y = rng.uniform(-1.f, 1.f);
		s = x*x + y*y;
	} while (s >= 1.f || s < 1e-6f);
	float r = 2.f * std::sqrt(1.f - s);
	return rove::vec<3>(x * r, y * r, 1.f - 2.f * s);
}

} // namespace

BOOST_AUTO_TEST_CASE(bruteforce_obb_triangle_no_false_positives)
{
	// Unit OBB at origin.  Bounding sphere radius = sqrt(3) ≈ 1.73.
	// Place triangle centroid at distance ≥ 4 from OBB centre (0.5,0.5,0.5),
	// with triangle side length ≤ 1.  Guaranteed no intersection.
	TestRNG rng(12345);
	int false_positives = 0;

	for (int i = 0; i < 1000; ++i) {
		rove::matrix<4,4> tf = random_transform(rng);

		// Triangle in easy position: centroid at distance 4+ from (0.5,0.5,0.5)
		rove::vec<3> dir = random_direction(rng);
		float dist = rng.uniform(4.f, 20.f);
		rove::vec<3> centre = rove::vec<3>(0.5f, 0.5f, 0.5f) + dir * dist;
		// Small triangle (side ≤ 0.5) around centroid
		rove::vec<3> u = random_direction(rng);
		rove::vec<3> v = rove::normalize(cross(u, dir));
		if (v.length() < 0.01f) v = rove::vec<3>(0.f, 0.f, 1.f);
		float sz = rng.uniform(0.1f, 0.5f);
		rove::vec<3> A = centre + u * sz;
		rove::vec<3> B = centre + v * sz;
		rove::vec<3> C = centre - (u + v) * (sz * 0.5f);

		// Transform both OBB and triangle by the same rigid transform
		rove::obb<3> box(
			xform_point(tf, rove::vec<3>(0,0,0)),
			xform_dir(tf, rove::vec<3>(1,0,0)),
			xform_dir(tf, rove::vec<3>(0,1,0)),
			xform_dir(tf, rove::vec<3>(0,0,1))
		);
		rove::triangle<3> tri(
			xform_point(tf, A),
			xform_point(tf, B),
			xform_point(tf, C)
		);

		if (box.test_intersection(tri)) {
			++false_positives;
		}
	}
	BOOST_CHECK_EQUAL(false_positives, 0);
}

BOOST_AUTO_TEST_CASE(bruteforce_obb_obb_no_false_positives)
{
	// Two unit OBBs.  Place second OBB centre at distance ≥ 6 from first
	// centre.  Bounding sphere of each is sqrt(3) ≈ 1.73, so centres
	// separated by 6 guarantees no overlap.
	TestRNG rng(67890);
	int false_positives = 0;

	for (int i = 0; i < 1000; ++i) {
		rove::matrix<4,4> tf = random_transform(rng);

		// Second OBB: random orientation, centre at distance ≥ 6
		rove::vec<3> dir = random_direction(rng);
		float dist = rng.uniform(6.f, 30.f);
		rove::vec<3> offset = dir * dist;

		// Random orientation for second box
		rove::matrix<4,4> rot2;
		rot2.identity();
		rot2.rotation(rng.uniform(0.f, 6.28f), rng.uniform(0.f, 6.28f), rng.uniform(0.f, 6.28f));

		// Extract rotation columns as OBB edge directions
		rove::vec<3> t2(rot2.ij[0][0], rot2.ij[0][1], rot2.ij[0][2]);
		rove::vec<3> n2(rot2.ij[1][0], rot2.ij[1][1], rot2.ij[1][2]);
		rove::vec<3> b2(rot2.ij[2][0], rot2.ij[2][1], rot2.ij[2][2]);

		// Scale second box to have edges of length ≤ 1
		float s = rng.uniform(0.3f, 1.0f);
		t2 = t2 * s; n2 = n2 * s; b2 = b2 * s;
		rove::vec<3> o2 = offset - (t2 + n2 + b2) * 0.5f; // centre at offset

		// Transform both boxes by the global rigid transform
		rove::obb<3> box1(
			xform_point(tf, rove::vec<3>(0,0,0)),
			xform_dir(tf, rove::vec<3>(1,0,0)),
			xform_dir(tf, rove::vec<3>(0,1,0)),
			xform_dir(tf, rove::vec<3>(0,0,1))
		);
		rove::obb<3> box2(
			xform_point(tf, o2),
			xform_dir(tf, t2),
			xform_dir(tf, n2),
			xform_dir(tf, b2)
		);

		if (box1.test_intersection(box2)) {
			++false_positives;
		}
	}
	BOOST_CHECK_EQUAL(false_positives, 0);
}

// Ray trace with intersection times
BOOST_AUTO_TEST_CASE(trace_t_axis_aligned_box)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10)));
	rove::ray<3> r(rove::vec<3>(-5, 5, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 100));
	BOOST_CHECK(rove::abs(t0 - 5.0f) < 0.01f);
	BOOST_CHECK(rove::abs(t1 - 15.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(trace_t_rotated_box)
{
	// Unit cube centred at the world origin, rotated 45° about Z.
	// Origin = centre - (tangent + normal + binormal) / 2.
	float const s = 0.7071068f;
	rove::obb<3> b(
		rove::vec<3>(0, -s, -0.5f),
		rove::vec<3>(s, s, 0),
		rove::vec<3>(-s, s, 0),
		rove::vec<3>(0, 0, 1)
	);
	// Ray along the world X axis passes through the centre.  In the XY
	// plane the box is a diamond whose left vertex sits at x = -s; the
	// ray enters there and exits at the right vertex x = +s.
	rove::ray<3> r(rove::vec<3>(-2, 0, 0), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 10));
	BOOST_CHECK(rove::abs(t0 - (2.0f - s)) < 0.01f);
	BOOST_CHECK(rove::abs(t1 - (2.0f + s)) < 0.01f);
}

BOOST_AUTO_TEST_CASE(trace_t_miss)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10)));
	rove::ray<3> r(rove::vec<3>(-5, 20, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(!b.trace(r, &t0, &t1, 0, 100));
}

BOOST_AUTO_TEST_CASE(trace_t_from_inside)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10)));
	rove::ray<3> r(rove::vec<3>(5, 5, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 100));
	// Ray starts inside → entry is clamped to t_min.
	BOOST_CHECK(rove::abs(t0 - 0.0f) < 0.01f);
	BOOST_CHECK(rove::abs(t1 - 5.0f) < 0.01f);
}

BOOST_AUTO_TEST_CASE(trace_t_parallel_ray_outside_slab)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10)));
	// Ray heading +X but starting at y=20 (outside the box's y slab).
	rove::ray<3> r(rove::vec<3>(-5, 20, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(!b.trace(r, &t0, &t1, 0, 100));
}

BOOST_AUTO_TEST_CASE(trace_t_with_t_range)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10)));
	rove::ray<3> r(rove::vec<3>(-5, 5, 5), rove::vec<3>(1, 0, 0));

	rove::scalar t0, t1;
	BOOST_REQUIRE(!b.trace(r, &t0, &t1, 0, 1));
	BOOST_REQUIRE(b.trace(r, &t0, &t1, 0, 20));
}

// ── OBB vs capsule ──────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(capsule_far_away_does_not_intersect)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)));
	rove::capsule<3> c(rove::vec<3>(10, 10, 0), rove::vec<3>(10, 10, 5), 0.5f);

	BOOST_CHECK(!b.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(capsule_crossing_a_face_intersects)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)));
	rove::capsule<3> c(rove::vec<3>(0.5f, 0.5f, -5), rove::vec<3>(0.5f, 0.5f, 5), 0.1f);

	BOOST_CHECK(b.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(capsule_reaching_by_radius_alone_intersects)
{
	// Axis passes 1.4 away from the box's +x face at x=1, radius 0.5 closes
	// the gap; radius 0.3 does not.
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)));
	rove::capsule<3> near(rove::vec<3>(1.4f, 0.5f, -5), rove::vec<3>(1.4f, 0.5f, 5), 0.5f);
	rove::capsule<3> far(rove::vec<3>(1.4f, 0.5f, -5), rove::vec<3>(1.4f, 0.5f, 5), 0.3f);

	BOOST_CHECK(b.test_intersection(near));
	BOOST_CHECK(!b.test_intersection(far));
}

BOOST_AUTO_TEST_CASE(capsule_wholly_inside_the_box_intersects)
{
	// Touches no face, so a pure face-sweep would miss it.
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(10, 10, 10)));
	rove::capsule<3> c(rove::vec<3>(5, 5, 4), rove::vec<3>(5, 5, 6), 0.5f);

	BOOST_CHECK(b.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(box_wholly_inside_the_capsule_intersects)
{
	// The reverse containment: every face triangle lies inside the capsule.
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)));
	rove::capsule<3> c(rove::vec<3>(0.5f, 0.5f, -5), rove::vec<3>(0.5f, 0.5f, 5), 10.0f);

	BOOST_CHECK(b.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(capsule_beyond_the_axis_end_does_not_intersect)
{
	// The axis is a segment, not an infinite line: a capsule that ends
	// short of the box must not reach it.
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)));
	rove::capsule<3> c(rove::vec<3>(0.5f, 0.5f, 5), rove::vec<3>(0.5f, 0.5f, 10), 0.5f);

	BOOST_CHECK(!b.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(capsule_intersection_respects_rotation)
{
	// Long thin box rotated 90 degrees about z: a capsule that clears the
	// unrotated box is caught by the rotated one.
	rove::obb<3> upright(rove::vec<3>(-0.5f, -2.0f, 0),
		rove::vec<3>(1, 0, 0), rove::vec<3>(0, 4, 0), rove::vec<3>(0, 0, 1));
	rove::obb<3> turned(rove::vec<3>(-2.0f, -0.5f, 0),
		rove::vec<3>(4, 0, 0), rove::vec<3>(0, 1, 0), rove::vec<3>(0, 0, 1));
	rove::capsule<3> c(rove::vec<3>(1.5f, 0, -1), rove::vec<3>(1.5f, 0, 2), 0.2f);

	BOOST_CHECK(!upright.test_intersection(c));
	BOOST_CHECK(turned.test_intersection(c));
}

BOOST_AUTO_TEST_CASE(capsule_with_negative_radius_does_not_intersect)
{
	rove::obb<3> b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)));
	rove::capsule<3> c(rove::vec<3>(0.5f, 0.5f, 0.5f), rove::vec<3>(0.5f, 0.5f, 0.6f), -1.0f);

	BOOST_CHECK(!b.test_intersection(c));
}

// obb::trace(ray, t_min, t_max): whether the ray meets the box anywhere for t in [t_min, t_max], as the slab test that also
// gives the times (trace(r, &t0, &t1, ...)) says.  A ray that starts inside meets it; so does one that ends inside.
BOOST_AUTO_TEST_CASE(trace_from_inside_the_box_meets_it)
{
	rove::obb<3> const b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(2, 2, 2)));
	for (int axis = 0; axis < 3; ++axis) {
		rove::vec<3> d(0, 0, 0);
		d.i[axis] = 1;
		BOOST_REQUIRE(b.trace(rove::ray<3>(rove::vec<3>(1, 1, 1), d), 0, 1));           // leaves through a face at t = 1
		BOOST_REQUIRE(b.trace(rove::ray<3>(rove::vec<3>(1, 1, 1), d * 0.25f), 0, 1));    // never leaves within [0, 1]
		BOOST_REQUIRE(b.trace(rove::ray<3>(rove::vec<3>(1, 1, 1), -d), 0, 1));
	}
}

BOOST_AUTO_TEST_CASE(trace_by_default_looks_from_t_0_to_1_as_the_timed_trace_does)
{
	rove::obb<3> const b(rove::aabb<3>(rove::vec<3>(0, 0, 0), rove::vec<3>(1, 1, 1)));
	BOOST_REQUIRE(b.trace(rove::ray<3>(rove::vec<3>(-0.5f, 0.5f, 0.5f), rove::vec<3>(1, 0, 0))));     // enters at t = 0.5
	BOOST_REQUIRE(!b.trace(rove::ray<3>(rove::vec<3>(-2, 0.5f, 0.5f), rove::vec<3>(1, 0, 0))));       // would enter at t = 2
	BOOST_REQUIRE(!b.trace(rove::ray<3>(rove::vec<3>(2, 0.5f, 0.5f), rove::vec<3>(1, 0, 0))));        // behind it
}

BOOST_AUTO_TEST_CASE(trace_agrees_with_the_timed_slab_test_on_random_rays)
{
	std::mt19937 rng(7);
	std::uniform_real_distribution<float> u(-3, 3);
	rove::obb<3> const b(rove::vec<3>(0.5f, -0.25f, 0), rove::vec<3>(1, 1, 0), rove::vec<3>(-1, 1, 0), rove::vec<3>(0, 0, 1.5f));
	int hits = 0;
	for (int k = 0; k < 20000; ++k) {
		// from anywhere around towards a point near the box, reaching short of it, into it or past it
		rove::vec<3> const from(u(rng), u(rng), u(rng)), near(0.5f + u(rng) / 2, 0.5f + u(rng) / 2, 0.75f + u(rng) / 2);
		rove::ray<3> const r(from, (near - from) * (0.5f + (u(rng) + 3) / 4));
		float t0, t1;
		bool const timed = b.trace(r, &t0, &t1, 0, 1);
		BOOST_REQUIRE_EQUAL(b.trace(r, 0, 1), timed);
		hits += timed;
	}
	BOOST_REQUIRE(hits > 1000);       // the comparison saw both answers often
	BOOST_REQUIRE(hits < 19000);
}

namespace
{
using v3 = rove::vec<3>;
rove::obb<3> unit_box(v3 const &o = v3(0, 0, 0))
{
	return rove::obb<3>(o, v3(1, 0, 0), v3(0, 1, 0), v3(0, 0, 1));
}
}

// The triangle comes in face on, so its plane separates it from the box at the start and only there
BOOST_AUTO_TEST_CASE(a_triangle_swept_face_on_into_the_box_collides)
{
	rove::triangle<3> const wall(v3(-10, -10, 3), v3(10, -10, 3), v3(0, 10, 3));
	BOOST_TEST(unit_box().test_collision(wall, v3(0, 0, -3)));
	BOOST_TEST(unit_box().test_collision(wall, v3(0, 0, -2.5f)));
	BOOST_TEST(!unit_box().test_collision(wall, v3(0, 0, -1.5f)));
	BOOST_TEST(!unit_box().test_collision(wall, v3(0, 0, 3)));
}

BOOST_AUTO_TEST_CASE(a_triangle_swept_past_a_corner_does_not_collide)
{
	// standing still by the corner: the box's axes and the triangle's plane all overlap, an edge's axis separates
	rove::triangle<3> const by_the_corner(v3(1.6f, 1, 0.2f), v3(1, 1.6f, 0.2f), v3(1.3f, 1.3f, 0.8f));
	BOOST_TEST(!unit_box().test_intersection(by_the_corner));
	BOOST_TEST(!unit_box().test_collision(by_the_corner, v3(0, 0, 0)));
	// moving on the far side of the corner along (1, -1, 0): the box's axes each overlap at some time
	rove::triangle<3> const passing(v3(-1, 3.2f, 0.2f), v3(-1, 3.2f, 0.8f), v3(-0.5f, 3.7f, 0.5f));
	BOOST_TEST(!unit_box().test_collision(passing, v3(4, -4, 0)));
}

// The overlap has to fall in [t_min, t_max]: the time counts forward from the positions given
BOOST_AUTO_TEST_CASE(a_collision_counts_only_in_its_time_interval)
{
	// the other box reaches the unit box at t = 4 (x = 5 - t) and has left at t = 6
	rove::obb<3> const other = unit_box(v3(5, 0, 0));
	v3 const vel(-1, 0, 0);
	BOOST_TEST(unit_box().test_collision(other, vel, 4, 6));
	BOOST_TEST(unit_box().test_collision(other, vel, 0, 6));
	BOOST_TEST(unit_box().test_collision(other, vel, 5, 5));
	BOOST_TEST(!unit_box().test_collision(other, vel, 0, 3.9f));
	BOOST_TEST(!unit_box().test_collision(other, vel, 6.1f, 20));
	rove::triangle<3> const t(v3(5, 0.2f, 0.2f), v3(5, 0.8f, 0.2f), v3(5, 0.2f, 0.8f));
	BOOST_TEST(unit_box().test_collision(t, vel, 4, 6));
	BOOST_TEST(!unit_box().test_collision(t, vel, 0, 3.9f));
	BOOST_TEST(!unit_box().test_collision(t, vel, 5.1f, 20));
}

BOOST_AUTO_TEST_CASE(boxes_moving_together_collide_only_as_they_overlap)
{
	// separated at the start and the end of the interval and on no one axis all the way through
	rove::obb<3> const passing = unit_box(v3(-3, 0, 0));
	BOOST_TEST(unit_box().test_collision(passing, v3(6, 0, 0)));
	rove::obb<3> const miss = unit_box(v3(-3, 2, 0));
	BOOST_TEST(!unit_box().test_collision(miss, v3(6, 0, 0)));
	// diagonal motion: its box overlaps on x during [4, 6] and on y during [0, 1], never on both
	BOOST_TEST(!unit_box().test_collision(unit_box(v3(-5, 0, 0)), v3(1, 1, 0), 0, 10));
	// the same motion that does reach: x during [4, 6], y during [3, 5]
	BOOST_TEST(unit_box().test_collision(unit_box(v3(-5, -4, 0)), v3(1, 1, 0), 0, 10));
}

// Two boxes whose faces do not separate them and an edge-edge axis does
BOOST_AUTO_TEST_CASE(boxes_separated_only_by_an_edge_axis_do_not_intersect)
{
	// a rotated unit cube: on all 6 face axes it overlaps the unit cube at (0, 0, 0), and on a cross product of edges it is 0.35 away
	v3 const t(1 / 3.0f, 2 / 3.0f, -2 / 3.0f), n(2 / 3.0f, 1 / 3.0f, 2 / 3.0f), b(2 / 3.0f, -2 / 3.0f, -1 / 3.0f);
	rove::obb<3> const beside(v3(-1.6f, 1.6f, -0.6f), t, n, b);
	BOOST_TEST(!unit_box().test_intersection(beside));
	BOOST_TEST(!beside.test_intersection(unit_box()));
	BOOST_TEST(!unit_box().test_collision(beside, v3(0, 0, 0)));
	// the same box with its centre in the cube's
	rove::obb<3> const inside(v3(-1 / 3.0f, 1 / 3.0f, 2 / 3.0f), t, n, b);
	BOOST_TEST(unit_box().test_intersection(inside));
	BOOST_TEST(inside.test_intersection(unit_box()));
}

BOOST_AUTO_TEST_CASE(a_random_pair_of_boxes_agrees_with_sampling_the_one_inside_the_other)
{
	std::mt19937 gen(11);
	std::uniform_real_distribution<float> u(-1, 1);
	int intersecting = 0, separate = 0;
	for (int i = 0; i < 4000; ++i) {
		v3 const t = normalize(v3(u(gen), u(gen), u(gen)));
		v3 const n = normalize(cross(t, v3(u(gen), u(gen), u(gen))));
		v3 const b = cross(t, n);
		rove::obb<3> const a = unit_box();
		rove::obb<3> const o(v3(u(gen), u(gen), u(gen)) * 2.5f, t, n, b);
		bool const reported = a.test_intersection(o);
		BOOST_REQUIRE(reported == o.test_intersection(a));
		(reported ? intersecting : separate)++;
		if (!reported) {
			// separated boxes hold no point in common
			for (int s = 0; s < 400; ++s) {
				v3 const p = o.origin + t * ((u(gen) + 1) / 2) + n * ((u(gen) + 1) / 2) + b * ((u(gen) + 1) / 2);
				bool const in_a = p.x >= 0 && p.x <= 1 && p.y >= 0 && p.y <= 1 && p.z >= 0 && p.z <= 1;
				BOOST_REQUIRE(!in_a);
			}
		}
	}
	BOOST_TEST(intersecting > 100);
	BOOST_TEST(separate > 100);
}

BOOST_AUTO_TEST_SUITE_END()

