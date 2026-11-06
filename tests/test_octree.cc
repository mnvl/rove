// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cmath>
#include <cstdint>
#include <limits>
#include <cstring>
#include <optional>
#include <random>
#include <utility>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/test/unit_test.hpp>
#include <rove/aabb.h>
#include <rove/segment.h>
#include <rove/octree.h>
#include "test_garbage.h"
#include <rove/sphere.h>
#include <rove/triangle.h>
#include <rove/random.h>
#include "test_tolerance.h"

// The inputs these tests draw: one generator for the file, the same numbers on every platform
namespace
{

rove::pcg32 draw(1);

rove::scalar uniform(rove::scalar lo = 0, rove::scalar hi = 1)
{
	return draw.uniform(lo, hi);
}

}

namespace
{

std::set<int> to_set(std::vector<int> const &values)
{
	return std::set<int>(values.begin(), values.end());
}

rove::sphere<3> random_sphere(rove::scalar max_radius = 0.25f)
{
	rove::scalar radius = uniform(0.01f, max_radius);
	rove::vec<3> centre(
		uniform(-1 + radius, 1 - radius),
		uniform(-1 + radius, 1 - radius),
		uniform(-1 + radius, 1 - radius));
	return rove::sphere<3>(centre, radius);
}

rove::aabb<3> random_aabb()
{
	rove::vec<3> half_size(
		uniform(0.01f, 0.2f),
		uniform(0.01f, 0.2f),
		uniform(0.01f, 0.2f));

	rove::vec<3> centre(
		uniform(-1 + half_size.x, 1 - half_size.x),
		uniform(-1 + half_size.y, 1 - half_size.y),
		uniform(-1 + half_size.z, 1 - half_size.z));

	return rove::aabb<3>(centre - half_size, centre + half_size);
}

rove::segment<3> random_line()
{
	rove::vec<3> a(
		uniform(-1.0f, 1.0f),
		uniform(-1.0f, 1.0f),
		uniform(-1.0f, 1.0f));
	rove::vec<3> b(
		uniform(-1.0f, 1.0f),
		uniform(-1.0f, 1.0f),
		uniform(-1.0f, 1.0f));
	return rove::segment<3>(a, b);
}

rove::triangle<3> random_triangle()
{
	for (size_t i = 0; i < 64; ++i) {
		rove::vec<3> a(
			uniform(-1.0f, 1.0f),
			uniform(-1.0f, 1.0f),
			uniform(-1.0f, 1.0f));
		rove::vec<3> b(
			uniform(-1.0f, 1.0f),
			uniform(-1.0f, 1.0f),
			uniform(-1.0f, 1.0f));
		rove::vec<3> c(
			uniform(-1.0f, 1.0f),
			uniform(-1.0f, 1.0f),
			uniform(-1.0f, 1.0f));

		if (rove::cross(b - a, c - a).length_sq() > TOLERANCE) {
			return rove::triangle<3>(a, b, c);
		}
	}

	return rove::triangle<3>(
		rove::vec<3>(-0.5f, -0.5f, 0.0f),
		rove::vec<3>(0.5f, -0.5f, 0.0f),
		rove::vec<3>(0.0f, 0.5f, 0.0f));
}

template<class Primitive, class QueryPrimitive>
std::set<int> brute_force_intersection_set(std::vector<Primitive> const &primitives, QueryPrimitive const &query)
{
	using query_traits_t = rove::spatial_query_traits<Primitive, QueryPrimitive>;
	std::set<int> result;

	for (size_t i = 0; i < primitives.size(); ++i) {
		if (query_traits_t::intersects(primitives[i], query)) {
			result.insert(static_cast<int>(i));
		}
	}

	return result;
}

}

BOOST_AUTO_TEST_SUITE(test_octree)

BOOST_AUTO_TEST_CASE(subcluster_bounds)
{
	rove::octree<rove::aabb<3>, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)));

	BOOST_REQUIRE((tree.subcluster_bounds(0).lo - rove::vec<3>(-1, -1, -1)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(0).hi - rove::vec<3>(0, 0, 0)).length_sq() < TOLERANCE);

	BOOST_REQUIRE((tree.subcluster_bounds(1).lo - rove::vec<3>(0, -1, -1)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(1).hi - rove::vec<3>(1, 0, 0)).length_sq() < TOLERANCE);

	BOOST_REQUIRE((tree.subcluster_bounds(2).lo - rove::vec<3>(-1, 0, -1)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(2).hi - rove::vec<3>(0, 1, 0)).length_sq() < TOLERANCE);

	BOOST_REQUIRE((tree.subcluster_bounds(3).lo - rove::vec<3>(0, 0, -1)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(3).hi - rove::vec<3>(1, 1, 0)).length_sq() < TOLERANCE);

	BOOST_REQUIRE((tree.subcluster_bounds(4).lo - rove::vec<3>(-1, -1, 0)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(4).hi - rove::vec<3>(0, 0, 1)).length_sq() < TOLERANCE);

	BOOST_REQUIRE((tree.subcluster_bounds(5).lo - rove::vec<3>(0, -1, 0)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(5).hi - rove::vec<3>(1, 0, 1)).length_sq() < TOLERANCE);

	BOOST_REQUIRE((tree.subcluster_bounds(6).lo - rove::vec<3>(-1, 0, 0)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(6).hi - rove::vec<3>(0, 1, 1)).length_sq() < TOLERANCE);

	BOOST_REQUIRE((tree.subcluster_bounds(7).lo - rove::vec<3>(0, 0, 0)).length_sq() < TOLERANCE);
	BOOST_REQUIRE((tree.subcluster_bounds(7).hi - rove::vec<3>(1, 1, 1)).length_sq() < TOLERANCE);
}

BOOST_AUTO_TEST_CASE(subcluster_by_point)
{
	rove::octree<rove::aabb<3>, int> tree(
		rove::aabb<3>(rove::vec<3>(-10, -100, -1000), rove::vec<3>(10, 100, 1000)));

	for (size_t i = 0; i < 1000; ++i) {
		rove::vec<3> point(
			uniform(-10, 10),
			uniform(-100, 100),
			uniform(-1000, 1000));

		unsigned index = tree.subcluster_by_point(point);
		rove::aabb<3> bounds = tree.subcluster_bounds(index);
		BOOST_REQUIRE(bounds.contains(point));
	}
}

BOOST_AUTO_TEST_CASE(subclusters_mask_by_aabb)
{
	rove::octree<rove::aabb<3>, int> tree(
		rove::aabb<3>(rove::vec<3>(-5, -5, -5), rove::vec<3>(5, 5, 5)));

	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(-1, -1, -1))) == 1);
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(1, -2, -2), rove::vec<3>(2, -1, -1))) == 2);
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, 1, -2), rove::vec<3>(-1, 2, -1))) == 4);
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(1, 1, -2), rove::vec<3>(2, 2, -1))) == 8);

	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, 1), rove::vec<3>(-1, -1, 2))) == 16);
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(1, -2, 1), rove::vec<3>(2, -1, 2))) == 32);
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, 1, 1), rove::vec<3>(-1, 2, 2))) == 64);
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(1, 1, 1), rove::vec<3>(2, 2, 2))) == 128);

	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(2, -1, -1))) == (1 | 2));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(-1, 2, -1))) == (1 | 4));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(1, -2, -2), rove::vec<3>(2, 2, -1))) == (2 | 8));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, 1, -2), rove::vec<3>(2, 2, -1))) == (4 | 8));

	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, 1), rove::vec<3>(2, -1, 2))) == (16 | 32));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, 1), rove::vec<3>(-1, 2, 2))) == (16 | 64));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(1, -2, 1), rove::vec<3>(2, 2, 2))) == (32 | 128));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, 1, 1), rove::vec<3>(2, 2, 2))) == (64 | 128));

	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(2, 2, -1))) == (1 | 2 | 4 | 8));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, 1), rove::vec<3>(2, 2, 2))) == (16 | 32 | 64 | 128));
	BOOST_REQUIRE(tree.subclusters_mask_by_aabb(rove::aabb<3>(rove::vec<3>(-2, -2, -2), rove::vec<3>(2, 2, 2))) == (1 | 2 | 4 | 8 | 16 | 32 | 64 | 128));
}

BOOST_AUTO_TEST_CASE(static_sphere_query_correctness_small)
{
	using primitive_type = rove::sphere<3>;

	rove::octree<primitive_type, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)),
		2);

	std::vector<primitive_type> primitives;
	primitives.reserve(8);

	primitives.push_back(primitive_type(rove::vec<3>(-0.5f, -0.5f, -0.5f), 0.1f));
	primitives.push_back(primitive_type(rove::vec<3>(0.5f, -0.5f, -0.5f), 0.1f));
	primitives.push_back(primitive_type(rove::vec<3>(-0.5f, 0.5f, -0.5f), 0.1f));
	primitives.push_back(primitive_type(rove::vec<3>(0.5f, 0.5f, -0.5f), 0.1f));
	primitives.push_back(primitive_type(rove::vec<3>(-0.5f, -0.5f, 0.5f), 0.1f));
	primitives.push_back(primitive_type(rove::vec<3>(0.5f, -0.5f, 0.5f), 0.1f));
	primitives.push_back(primitive_type(rove::vec<3>(-0.5f, 0.5f, 0.5f), 0.1f));
	primitives.push_back(primitive_type(rove::vec<3>(0.5f, 0.5f, 0.5f), 0.1f));

	for (size_t i = 0; i < primitives.size(); ++i) {
		tree.insert(primitives[i], static_cast<int>(i));
	}

	tree.build();

	for (size_t i = 0; i < 100; ++i) {
		primitive_type query = random_sphere(0.9f);
		std::set<int> expected = brute_force_intersection_set(primitives, query);
		std::set<int> actual = to_set(tree.query_intersection(query));
		BOOST_REQUIRE(actual == expected);
	}
}

BOOST_AUTO_TEST_CASE(static_sphere_query_correctness_random)
{
	using primitive_type = rove::sphere<3>;
	static size_t const N = 500;
	static size_t const M = 100;

	rove::octree<primitive_type, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)),
		8);

	std::vector<primitive_type> primitives;
	primitives.reserve(N);
	tree.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		primitive_type primitive = random_sphere(0.2f);
		primitives.push_back(primitive);
		tree.insert(primitive, static_cast<int>(i));
	}

	tree.build();

	for (size_t i = 0; i < M; ++i) {
		primitive_type query = random_sphere(0.4f);
		std::set<int> expected = brute_force_intersection_set(primitives, query);
		std::set<int> actual = to_set(tree.query_intersection(query));
		BOOST_REQUIRE(actual == expected);
	}
}

BOOST_AUTO_TEST_CASE(static_aabb_query_correctness_random)
{
	using primitive_type = rove::aabb<3>;
	static size_t const N = 500;
	static size_t const M = 100;

	rove::octree<primitive_type, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)),
		8);

	std::vector<primitive_type> primitives;
	primitives.reserve(N);
	tree.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		primitive_type primitive = random_aabb();
		primitives.push_back(primitive);
		tree.insert(primitive, static_cast<int>(i));
	}

	tree.build();

	for (size_t i = 0; i < M; ++i) {
		primitive_type query = random_aabb();
		std::set<int> expected = brute_force_intersection_set(primitives, query);
		std::set<int> actual = to_set(tree.query_intersection(query));
		BOOST_REQUIRE(actual == expected);
	}
}

BOOST_AUTO_TEST_CASE(static_triangle_query_correctness_random)
{
	using primitive_type = rove::triangle<3>;
	static size_t const N = 200;
	static size_t const M = 80;

	rove::octree<primitive_type, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)),
		8);

	std::vector<primitive_type> primitives;
	primitives.reserve(N);
	tree.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		primitive_type primitive = random_triangle();
		primitives.push_back(primitive);
		tree.insert(primitive, static_cast<int>(i));
	}

	tree.build();

	for (size_t i = 0; i < M; ++i) {
		primitive_type query = random_triangle();
		std::set<int> expected = brute_force_intersection_set(primitives, query);
		std::set<int> actual = to_set(tree.query_intersection(query));
		BOOST_REQUIRE(actual == expected);
	}
}

BOOST_AUTO_TEST_CASE(static_triangle_query_by_aabb_correctness_random)
{
	using primitive_type = rove::triangle<3>;
	using query_t = rove::aabb<3>;
	static size_t const N = 200;
	static size_t const M = 80;

	rove::octree<primitive_type, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)),
		8);

	std::vector<primitive_type> primitives;
	primitives.reserve(N);
	tree.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		primitive_type primitive = random_triangle();
		primitives.push_back(primitive);
		tree.insert(primitive, static_cast<int>(i));
	}

	tree.build();

	for (size_t i = 0; i < M; ++i) {
		query_t query = random_aabb();
		std::set<int> expected = brute_force_intersection_set(primitives, query);
		std::set<int> actual = to_set(tree.query_intersection(query));
		BOOST_REQUIRE(actual == expected);
	}
}

BOOST_AUTO_TEST_CASE(static_triangle_query_by_sphere_correctness_random)
{
	using primitive_type = rove::triangle<3>;
	using query_t = rove::sphere<3>;
	static size_t const N = 200;
	static size_t const M = 80;

	rove::octree<primitive_type, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)),
		8);

	std::vector<primitive_type> primitives;
	primitives.reserve(N);
	tree.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		primitive_type primitive = random_triangle();
		primitives.push_back(primitive);
		tree.insert(primitive, static_cast<int>(i));
	}

	tree.build();

	for (size_t i = 0; i < M; ++i) {
		query_t query = random_sphere(0.4f);
		std::set<int> expected = brute_force_intersection_set(primitives, query);
		std::set<int> actual = to_set(tree.query_intersection(query));
		BOOST_REQUIRE(actual == expected);
	}
}

BOOST_AUTO_TEST_CASE(static_triangle_query_by_line_correctness_random)
{
	using primitive_type = rove::triangle<3>;
	using query_t = rove::segment<3>;
	static size_t const N = 200;
	static size_t const M = 80;

	rove::octree<primitive_type, int> tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)),
		8);

	std::vector<primitive_type> primitives;
	primitives.reserve(N);
	tree.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		primitive_type primitive = random_triangle();
		primitives.push_back(primitive);
		tree.insert(primitive, static_cast<int>(i));
	}

	tree.build();

	for (size_t i = 0; i < M; ++i) {
		query_t query = random_line();
		std::set<int> expected = brute_force_intersection_set(primitives, query);
		std::set<int> actual = to_set(tree.query_intersection(query));
		BOOST_REQUIRE(actual == expected);
	}
}

// trace(): the nearest hit along a ray, as a scan of every item finds it --
// through items on the planes that divide the nodes, items reaching out of
// the tree's bounds, rays along an axis and from far outside -- reported as a
// contact that names the item.

namespace
{

using v3 = rove::vec<3>;
using tri_t = rove::triangle<3>;
using ray_type = rove::ray<3>;
using tri_tree_t = rove::octree<tri_t, int>;

std::vector<tri_t> trace_scene(unsigned seed)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> pos(-1, 1), small(-0.1f, 0.1f), large(-0.8f, 0.8f);
	std::uniform_int_distribution<int> plane(-2, 2);
	auto p = [&] { return v3{pos(rng), pos(rng), pos(rng)}; };
	std::vector<tri_t> out;
	for (int i = 0; i < 300; ++i) {
		v3 const c = p();
		out.push_back(tri_t{c, c + v3{small(rng), small(rng), small(rng)}, c + v3{small(rng), small(rng), small(rng)}});
	}
	for (int i = 0; i < 30; ++i) {
		v3 const c = p();
		out.push_back(tri_t{c + v3{large(rng), large(rng), large(rng)}, c + v3{large(rng), large(rng), large(rng)},
			c + v3{large(rng), large(rng), large(rng)}});
	}
	for (int i = 0; i < 60; ++i) {
		// flat on a plane that divides nodes or bounds them: x, y or z = -1, -0.5, 0, 0.5 or 1
		int const axis = i % 3;
		v3 a = p(), b = a + v3{small(rng), small(rng), small(rng)} * 3, c = a + v3{small(rng), small(rng), small(rng)} * 3;
		a.i[axis] = b.i[axis] = c.i[axis] = plane(rng) * 0.5f;
		out.emplace_back(a, b, c);
	}
	for (int i = 0; i < 20; ++i) {
		// reaching out of the bounds, and wholly outside them
		v3 const c = p() * (i % 2 ? 1.5f : 3.0f);
		out.push_back(tri_t{c, c + v3{large(rng), large(rng), large(rng)}, c + v3{large(rng), large(rng), large(rng)}});
	}
	return out;
}

tri_tree_t tree_of(std::vector<tri_t> const &tris)
{
	tri_tree_t tree(rove::aabb<3>(v3(-1, -1, -1), v3(1, 1, 1)), 4, 1e-4f);
	for (size_t i = 0; i < tris.size(); ++i) {
		tree.insert(tris[i], static_cast<int>(i) * 10);
	}
	tree.build();
	return tree;
}

std::vector<ray_type> trace_rays(unsigned seed)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> pos(-1.2f, 1.2f), dir(-1, 1), len(0.5f, 4);
	std::uniform_int_distribution<int> plane(-2, 2);
	std::vector<ray_type> out;
	for (int i = 0; i < 400; ++i) {
		v3 d{dir(rng), dir(rng), dir(rng)};
		d.normalize();
		out.push_back(ray_type{v3{pos(rng), pos(rng), pos(rng)}, d * len(rng)});
	}
	for (int i = 0; i < 90; ++i) {
		// along an axis, in the planes that divide nodes, both ways
		int const axis = i % 3;
		v3 o{plane(rng) * 0.25f, plane(rng) * 0.25f, plane(rng) * 0.25f}, d(0, 0, 0);
		o.i[axis] = pos(rng);
		d.i[axis] = (i % 2 ? 1.0f : -1.0f) * len(rng);
		out.emplace_back(o, d);
	}
	for (int i = 0; i < 40; ++i) {
		// from far outside, through the tree
		v3 const from{pos(rng) * 20, pos(rng) * 20, pos(rng) * 20}, to{pos(rng), pos(rng), pos(rng)};
		out.emplace_back(from, (to - from) * 1.5f);
	}
	return out;
}

struct nearest
{
	size_t index;
	float t;
};

template<class Accept>
std::optional<nearest> scan_trace(std::vector<tri_t> const &tris, ray_type const &r, float t_min, float t_max, Accept accept)
{
	std::optional<nearest> best;
	for (size_t i = 0; i < tris.size(); ++i) {
		float t;
		v3 at;
		if (accept(i) && tris[i].trace(r, t, at, t_min, t_max) && (!best || t < best->t)) {
			best = nearest{i, t};
		}
	}
	return best;
}

}

BOOST_AUTO_TEST_CASE(trace_finds_what_a_scan_finds)
{
	std::vector<tri_t> const tris = trace_scene(3);
	tri_tree_t const tree = tree_of(tris);
	BOOST_REQUIRE(!tree.is_leaf());
	float const inf = std::numeric_limits<float>::infinity();
	size_t hits = 0, misses = 0;
	std::vector<ray_type> const rays = trace_rays(4);
	for (size_t k = 0; k < rays.size(); ++k) {
		ray_type const &r = rays[k];
		for (auto const [t_min, t_max] : {std::pair(0.0f, 1.0f), std::pair(0.3f, 0.6f), std::pair(0.0f, inf), std::pair(-inf, inf)}) {
			auto const expected = scan_trace(tris, r, t_min, t_max, [](size_t) { return true; });
			rove::contact_info<3> const got = tree.trace(r, t_min, t_max);
			BOOST_TEST_CONTEXT("ray " << k << " t " << t_min << ".." << t_max) {
				BOOST_REQUIRE_EQUAL(got.happened, bool(expected));
				if (!got.happened) {
					BOOST_TEST(!got.index);
					++misses;
					continue;
				}
				++hits;
				BOOST_TEST(got.time == expected->t);
				BOOST_REQUIRE(got.index);
				BOOST_REQUIRE(*got.index < tris.size());
				BOOST_TEST(tree[*got.index].payload == static_cast<int>(*got.index) * 10);
				tri_t const &hit = tris[*got.index];
				float t;
				v3 at;
				BOOST_TEST(hit.trace(r, t, at, t_min, t_max));
				BOOST_TEST(t == got.time);
				BOOST_TEST((got.position - r.apply(got.time)).length() < 1e-4f);
				BOOST_TEST(!got.penetrated);
				BOOST_TEST(std::abs(got.normal.length() - 1) < 1e-5f);
				BOOST_TEST(std::abs(rove::dot(got.normal, hit.normal())) > 1 - 1e-5f);
				BOOST_TEST(rove::dot(got.normal, r.a) <= 0);
			}
		}
	}
	BOOST_TEST(hits > 800u);
	BOOST_TEST(misses > 200u);
}

BOOST_AUTO_TEST_CASE(trace_with_its_own_test)
{
	std::vector<tri_t> const tris = trace_scene(5);
	tri_tree_t const tree = tree_of(tris);
	// only items with a payload divisible by 20 count, and the contact comes
	// back as the test filled it: the payload as its normal's z
	auto const even = [](tri_tree_t::value_type const &item, ray_type const &r, float t_min, float t_max, rove::contact_info<3> &ci) {
		if (item.payload % 20 != 0 || !item.primitive.trace(r, ci.time, ci.position, t_min, t_max)) {
			return false;
		}
		ci.normal = v3(0, 0, float(item.payload));
		return true;
	};
	size_t hits = 0;
	for (ray_type const &r : trace_rays(6)) {
		auto const expected = scan_trace(tris, r, 0, 1, [](size_t i) { return i % 2 == 0; });
		rove::contact_info<3> const got = tree.trace(r, 0, 1, even);
		BOOST_REQUIRE_EQUAL(got.happened, bool(expected));
		if (got.happened) {
			++hits;
			BOOST_REQUIRE(got.index);
			BOOST_TEST(*got.index % 2 == 0u);
			BOOST_TEST(got.time == expected->t);
			BOOST_TEST(got.normal.z == float(*got.index * 10));
		}
	}
	BOOST_TEST(hits > 100u);
}

// The cases lidar leans on: one hit, the nearest of several, one past the
// end of the ray, an empty tree.
BOOST_AUTO_TEST_CASE(trace_known_answers)
{
	rove::aabb<3> const bounds(v3(-1, -1, -1), v3(11, 11, 11));
	ray_type const up(v3(0.2f, 0.2f, 0), v3(0, 0, 9));

	tri_tree_t one(bounds, 8);
	one.insert(tri_t(v3(0, 0, 2), v3(2, 0, 2), v3(0, 2, 2)), 7);
	one.build();
	rove::contact_info<3> const hit = one.trace(up);
	BOOST_REQUIRE(hit.happened);
	BOOST_TEST(hit.time * 9 == 2.0f, boost::test_tools::tolerance(1e-6f));
	BOOST_TEST(one[*hit.index].payload == 7);
	BOOST_TEST((hit.position - v3(0.2f, 0.2f, 2)).length() < 1e-6f);
	BOOST_TEST((hit.normal - v3(0, 0, -1)).length() < 1e-6f);

	tri_tree_t two(bounds, 8);
	two.insert(tri_t(v3(0, 0, 2), v3(2, 0, 2), v3(0, 2, 2)), 100);
	two.insert(tri_t(v3(0, 0, 1), v3(2, 0, 1), v3(0, 2, 1)), 200);
	two.build();
	rove::contact_info<3> const near = two.trace(up);
	BOOST_REQUIRE(near.happened);
	BOOST_TEST(near.time * 9 == 1.0f, boost::test_tools::tolerance(1e-6f));
	BOOST_TEST(two[*near.index].payload == 200);

	tri_tree_t far(bounds, 8);
	far.insert(tri_t(v3(0, 0, 10), v3(2, 0, 10), v3(0, 2, 10)), 1);
	far.build();
	BOOST_TEST(!far.trace(up).happened);
	BOOST_TEST(far.trace(up, 0, 2).happened);

	tri_tree_t empty(bounds, 8);
	empty.build();
	BOOST_TEST(!empty.trace(up).happened);
}

// An unbuilt tree is an error to ask, not an empty answer: a caller who
// forgot build() would otherwise see a world with nothing in it.
BOOST_AUTO_TEST_CASE(an_unbuilt_tree_refuses_queries)
{
	rove::aabb<3> const box(v3(-1, -1, -1), v3(1, 1, 1));
	tri_tree_t tree(box);
	tri_t const t(v3(0, 0, 0), v3(0.5f, 0, 0), v3(0, 0.5f, 0));
	tree.insert(t, 1);
	auto refuses = [&] {
		BOOST_CHECK_THROW(tree.query_intersection(box), std::logic_error);
		BOOST_CHECK_THROW(tree.query_intersection(t), std::logic_error);
		BOOST_CHECK_THROW(tree.query_intersection_items(rove::sphere<3>(v3(0, 0, 0), 1)), std::logic_error);
		BOOST_CHECK_THROW((void)tree.trace(ray_type(v3(0.1f, 0.1f, -1), v3(0, 0, 2))), std::logic_error);
	};
	refuses();
	tree.build();
	BOOST_TEST(tree.query_intersection(box) == std::vector<int>({1}));
	BOOST_TEST(tree.trace(ray_type(v3(0.1f, 0.1f, -1), v3(0, 0, 2))).happened);
	tree.insert(t, 2);              // an insert unbuilds it
	refuses();
	tree.build();
	BOOST_TEST(tree.query_intersection(box).size() == 2u);
	tree.clear();
	refuses();
}

// is_built() says what the queries need: true after build() and load(), false before, after an insert() and after a clear()
BOOST_AUTO_TEST_CASE(is_built_is_what_queries_need)
{
	rove::aabb<3> const box(v3(-1, -1, -1), v3(1, 1, 1));
	tri_tree_t tree(box);
	BOOST_TEST(!tree.is_built());
	tree.build();
	BOOST_TEST(tree.is_built());            // a built empty tree answers queries
	BOOST_TEST(tree.query_intersection(box).empty());
	tree.insert(tri_t(v3(0, 0, 0), v3(0.5f, 0, 0), v3(0, 0.5f, 0)), 1);
	BOOST_TEST(!tree.is_built());
	BOOST_CHECK_THROW(tree.query_intersection(box), std::logic_error);
	tree.build();
	BOOST_TEST(tree.is_built());
	BOOST_TEST(tri_tree_t::load(tree.save()).is_built());
	tree.clear();
	BOOST_TEST(!tree.is_built());
}

// query_indices names the items query_intersection gives the payloads of, each once, by their operator[] index
BOOST_AUTO_TEST_CASE(query_indices_names_what_query_intersection_finds)
{
	tri_tree_t tree(rove::aabb<3>(v3(-1, -1, -1), v3(1, 1, 1)), 4);
	for (int i = 0; i < 300; ++i)
		tree.insert(random_triangle(), 1000 + i);          // payload 1000 + index: an index is not mistaken for a payload
	std::vector<tri_tree_t::index_type> unbuilt;
	BOOST_CHECK_THROW(tree.query_indices(rove::aabb<3>(v3(0, 0, 0), v3(1, 1, 1)), unbuilt), std::logic_error);
	tree.build();
	for (int i = 0; i < 200; ++i) {
		rove::aabb<3> const box = random_aabb();
		rove::sphere<3> const sphere = random_sphere(0.5f);
		std::vector<tri_tree_t::index_type> by_box, by_sphere;
		tree.query_indices(box, by_box);
		tree.query_indices(sphere, by_sphere);
		std::set<int> from_box, from_sphere;
		for (auto k : by_box)
			BOOST_TEST_REQUIRE(from_box.insert(tree[k].payload).second);
		for (auto k : by_sphere)
			BOOST_TEST_REQUIRE(from_sphere.insert(tree[k].payload).second);
		BOOST_TEST_REQUIRE(from_box == to_set(tree.query_intersection(box)));
		BOOST_TEST_REQUIRE(from_sphere == to_set(tree.query_intersection(sphere)));
		for (auto k : by_box)
			BOOST_TEST_REQUIRE(tree[k].payload == int(1000 + k));
	}
	// it appends
	std::vector<tri_tree_t::index_type> twice;
	rove::aabb<3> const all(v3(-2, -2, -2), v3(2, 2, 2));
	tree.query_indices(all, twice);
	tree.query_indices(all, twice);
	BOOST_TEST(twice.size() == 600u);
}

BOOST_AUTO_TEST_CASE(bad_input_is_an_error)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	tri_tree_t const tree = tree_of(trace_scene(7));
	BOOST_CHECK_THROW((void)tree.trace(ray_type(v3(nan, 0, 0), v3(1, 0, 0))), std::invalid_argument);
	BOOST_CHECK_THROW((void)tree.trace(ray_type(v3(0, 0, 0), v3(1, inf, 0))), std::invalid_argument);
	BOOST_CHECK_THROW((void)tree.trace(ray_type(v3(0, 0, 0), v3(1, 0, 0)), 1, 0), std::invalid_argument);
	BOOST_CHECK_THROW((void)tree.trace(ray_type(v3(0, 0, 0), v3(1, 0, 0)), nan, 1), std::invalid_argument);
	BOOST_CHECK_THROW(tree[static_cast<tri_tree_t::index_type>(tree.size())], std::out_of_range);
}

// A query of the stored type is tested by the tree's own traits, as a query
// of any other type is by the traits given with it.
namespace
{

// Boxes that meet only when the query holds the stored box whole.
struct holding_traits: rove::spatial_traits<rove::aabb<3>> {
	static bool intersects(rove::aabb<3> const &stored, rove::aabb<3> const &query)
	{
		return query.contains(stored.lo) && query.contains(stored.hi);
	}
};

}

BOOST_AUTO_TEST_CASE(a_query_of_the_stored_type_uses_the_trees_own_traits)
{
	rove::octree<rove::aabb<3>, int, holding_traits> tree(rove::aabb<3>(v3(-1, -1, -1), v3(1, 1, 1)));
	tree.insert(rove::aabb<3>(v3(0, 0, 0), v3(0.5f, 0.5f, 0.5f)), 1);
	tree.insert(rove::aabb<3>(v3(0.4f, 0.4f, 0.4f), v3(0.9f, 0.9f, 0.9f)), 2);
	tree.build();
	rove::aabb<3> const query(v3(-0.1f, -0.1f, -0.1f), v3(0.6f, 0.6f, 0.6f));
	BOOST_TEST(tree.query_intersection(query) == std::vector<int>({1}));
	BOOST_TEST(tree.query_intersection_items(query).size() == 1u);
}

// save() / load(): a built tree written out and read back is the same tree --
// same items, same nodes -- so it answers every query with the same payloads
// in the same order, which only an identical structure does.

namespace
{

using triangle_octree_t = rove::octree<rove::triangle<3>, int>;

triangle_octree_t random_triangle_tree(size_t n)
{
	triangle_octree_t tree(
		rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)), 8);
	for (size_t i = 0; i < n; ++i) {
		tree.insert(random_triangle(), static_cast<int>(i) * 7 - 3);
	}
	tree.build();
	return tree;
}

std::string saved(triangle_octree_t const &tree)
{
	return tree.save();
}

triangle_octree_t loaded(std::string const &bytes)
{
	return triangle_octree_t::load(bytes);
}

}

BOOST_AUTO_TEST_CASE(save_load_round_trip_answers_every_query_identically)
{
	triangle_octree_t tree = random_triangle_tree(500);
	triangle_octree_t copy = loaded(saved(tree));

	BOOST_REQUIRE_EQUAL(copy.size(), tree.size());
	BOOST_REQUIRE(copy.bounds().lo == tree.bounds().lo);
	BOOST_REQUIRE(copy.bounds().hi == tree.bounds().hi);

	for (size_t i = 0; i < 200; ++i) {
		rove::aabb<3> box = random_aabb();
		BOOST_REQUIRE(copy.query_intersection<rove::aabb<3>>(box) ==
			tree.query_intersection<rove::aabb<3>>(box));
		rove::sphere<3> sphere = random_sphere(0.4f);
		BOOST_REQUIRE(copy.query_intersection<rove::sphere<3>>(sphere) ==
			tree.query_intersection<rove::sphere<3>>(sphere));
		rove::segment<3> line = random_line();
		BOOST_REQUIRE(copy.query_intersection<rove::segment<3>>(line) ==
			tree.query_intersection<rove::segment<3>>(line));
	}

	// The triangles themselves, not only their payloads, come back bit for bit.
	auto original_items = tree.query_intersection_items<rove::aabb<3>>(tree.bounds());
	auto copied_items = copy.query_intersection_items<rove::aabb<3>>(copy.bounds());
	BOOST_REQUIRE_EQUAL(copied_items.size(), 500u);
	BOOST_REQUIRE_EQUAL(copied_items.size(), original_items.size());
	for (size_t i = 0; i < original_items.size(); ++i) {
		BOOST_REQUIRE(std::memcmp(&copied_items[i].primitive, &original_items[i].primitive,
			sizeof(rove::triangle<3>)) == 0);
		BOOST_REQUIRE_EQUAL(copied_items[i].payload, original_items[i].payload);
	}

	// And a second save of the copy is the same bytes as the first.
	BOOST_REQUIRE(saved(copy) == saved(tree));
}

BOOST_AUTO_TEST_CASE(save_load_round_trip_of_an_empty_built_tree)
{
	triangle_octree_t tree = random_triangle_tree(0);
	triangle_octree_t copy = loaded(saved(tree));
	BOOST_REQUIRE_EQUAL(copy.size(), 0u);
	BOOST_REQUIRE(copy.query_intersection<rove::aabb<3>>(copy.bounds()).empty());
}

BOOST_AUTO_TEST_CASE(save_refuses_an_unbuilt_tree)
{
	triangle_octree_t tree(rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)), 8);
	tree.insert(random_triangle(), 1);
	BOOST_REQUIRE_THROW(tree.save(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(load_refuses_every_truncation_and_any_extra_byte)
{
	std::string bytes = saved(random_triangle_tree(50));
	for (size_t length = 0; length < bytes.size(); ++length) {
		BOOST_REQUIRE_THROW(loaded(bytes.substr(0, length)), std::runtime_error);
	}
	BOOST_REQUIRE_THROW(loaded(bytes + '\0'), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(load_refuses_a_foreign_header)
{
	std::string bytes = saved(random_triangle_tree(10));

	std::string bad_magic = bytes;
	bad_magic[0] ^= 0x5a;
	BOOST_REQUIRE_THROW(loaded(bad_magic), std::runtime_error);

	// The version follows the 4-byte magic.
	std::string bad_version = bytes;
	bad_version[4] ^= 0x01;
	BOOST_REQUIRE_THROW(loaded(bad_version), std::runtime_error);

	// Then the primitive's and the payload's sizes: a tree of another type.
	std::string bad_primitive = bytes;
	bad_primitive[8] ^= 0x01;
	BOOST_REQUIRE_THROW(loaded(bad_primitive), std::runtime_error);
	std::string bad_payload = bytes;
	bad_payload[12] ^= 0x01;
	BOOST_REQUIRE_THROW(loaded(bad_payload), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(load_refuses_a_node_naming_a_missing_item)
{
	// One triangle under a leaf threshold of 8: the root is the only node,
	// and the stream ends with it -- [u32 count = 1][u32 index = 0][u8 mask = 0].
	triangle_octree_t tree(rove::aabb<3>(rove::vec<3>(-1, -1, -1), rove::vec<3>(1, 1, 1)), 8);
	tree.insert(random_triangle(), 9);
	tree.build();
	std::string bytes = saved(tree);
	BOOST_REQUIRE_NO_THROW(loaded(bytes));

	std::string bad_index = bytes;
	std::uint32_t const missing = 1;
	std::memcpy(&bad_index[bytes.size() - 5], &missing, sizeof(missing));
	BOOST_REQUIRE_THROW(loaded(bad_index), std::runtime_error);

	// An item no node names is a tree that would never report it.
	std::string no_index = bytes;
	std::uint32_t const zero = 0;
	std::memcpy(&no_index[bytes.size() - 9], &zero, sizeof(zero));
	no_index.erase(bytes.size() - 5, 4);
	BOOST_REQUIRE_THROW(loaded(no_index), std::runtime_error);
}

// What the contact of a miss says is defined: MemorySanitizer took it as uninitialised
BOOST_AUTO_TEST_CASE(a_miss_is_all_zero)
{
	rove::contact_info<3> const fresh = constructed_over_garbage<rove::contact_info<3>>();
	BOOST_TEST(!fresh.happened);
	BOOST_TEST(!fresh.penetrated);
	BOOST_TEST(fresh.time == 0.0f);
	BOOST_TEST(fresh.penetration_depth == 0.0f);
}

BOOST_AUTO_TEST_SUITE_END()
