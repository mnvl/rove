// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

// Contract of the spatial traits: a triangle's box is exact, a NaN in any
// vertex reaches the tests built on it instead of being dropped, a sphere
// meets a triangle exactly when some point of the triangle is within its
// radius (not when it meets the triangle's box), and a cone never passes over
// a triangle that has a point inside it (it may report one that has none).

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <rove/traits.h>
#include <rove/octree.h>

namespace
{

using v3 = rove::vec<3>;
using tri_t = rove::triangle<3>;

tri_t with_nan(tri_t t, int vertex, int axis)
{
	(vertex == 0 ? t.A : vertex == 1 ? t.B : t.C).i[axis] = std::numeric_limits<float>::quiet_NaN();
	return t;
}

int const GRID = 60;

// The point of the triangle at barycentric grid point (i, j) of GRID, in double.
rove::vec<3, double> grid_point(tri_t const &t, int i, int j)
{
	double const u = double(i) / GRID, v = double(j) / GRID;
	rove::vec<3, double> const a(t.A), b(t.B), c(t.C);
	return a + (b - a) * u + (c - a) * v;
}

// The least distance from p to the triangle's grid points: never under the
// true distance, and over it by at most the longest edge / GRID.
double grid_distance(tri_t const &t, v3 const &p)
{
	rove::vec<3, double> const q(p);
	double best = std::numeric_limits<double>::infinity();
	for (int i = 0; i <= GRID; ++i)
		for (int j = 0; i + j <= GRID; ++j)
			best = std::min(best, (grid_point(t, i, j) - q).length());
	return best;
}

double longest_edge(tri_t const &t)
{
	return std::max({(t.B - t.A).length(), (t.C - t.B).length(), (t.A - t.C).length()});
}

// Whether a grid point of the triangle is inside the cone.
bool grid_meets(tri_t const &t, rove::cone<3> const &c)
{
	for (int i = 0; i <= GRID; ++i)
		for (int j = 0; i + j <= GRID; ++j)
			if (c.contains(v3(grid_point(t, i, j))))
				return true;
	return false;
}

}

BOOST_AUTO_TEST_SUITE(test_traits)

// A triangle's bounds are the min and max of its vertices per axis, and a
// NaN in any vertex makes them NaN: dropping it would hand a broken triangle
// a plausible box.
BOOST_AUTO_TEST_CASE(triangle_bounds_are_exact_and_keep_nan)
{
	using traits = rove::spatial_traits<tri_t>;
	rove::triangle<3> const t(rove::vec<3>(1, -2, 5), rove::vec<3>(-3, 4, 0.5f), rove::vec<3>(2, 0, -7));
	rove::aabb<3> const b = traits::bounds(t);
	BOOST_TEST((b.lo == rove::vec<3>(-3, -2, -7)));
	BOOST_TEST((b.hi == rove::vec<3>(2, 4, 5)));

	float const nan = std::numeric_limits<float>::quiet_NaN();
	for (int vertex = 0; vertex < 3; ++vertex)
		for (int axis = 0; axis < 3; ++axis)
		{
			rove::triangle<3> broken = t;
			(vertex == 0 ? broken.A : vertex == 1 ? broken.B : broken.C).i[axis] = nan;
			rove::aabb<3> const nb = traits::bounds(broken);
			BOOST_TEST_CONTEXT("vertex " << vertex << " axis " << axis)
			{
				BOOST_TEST(std::isnan(nb.lo.i[axis]));
				BOOST_TEST(std::isnan(nb.hi.i[axis]));
			}
		}
}

// The sphere test builds the same box: a triangle with a NaN in B or C meets
// no sphere, where a box that dropped the NaN met it.
BOOST_AUTO_TEST_CASE(a_triangle_with_nan_meets_no_sphere)
{
	using sphere_traits = rove::spatial_query_traits<tri_t, rove::sphere<3>>;
	tri_t const t(v3(0, 0, 0), v3(1, 0, 0), v3(0, 0, 1));
	rove::sphere<3> const ball(v3(0.2f, 0.5f, 0.2f), 1);
	BOOST_REQUIRE(sphere_traits::intersects(t, ball));
	for (int vertex = 1; vertex < 3; ++vertex)
		for (int axis = 0; axis < 3; ++axis)
			BOOST_TEST_CONTEXT("vertex " << vertex << " axis " << axis)
				BOOST_TEST(!sphere_traits::intersects(with_nan(t, vertex, axis), ball));
}

// A sphere meets a triangle when the triangle comes within its radius, not when
// the triangle's box does: a long diagonal triangle's box holds a corner far
// from the triangle.
BOOST_AUTO_TEST_CASE(a_sphere_meets_a_triangle_exactly_not_its_box)
{
	using sphere_traits = rove::spatial_query_traits<tri_t, rove::sphere<3>>;
	tri_t const diagonal(v3(0, 0, 0), v3(10, 0, 10), v3(10, 0.01f, 10));
	rove::sphere<3> const in_the_corner(v3(9, 0, 1), 1);
	BOOST_REQUIRE(rove::spatial_traits<tri_t>::bounds(diagonal).contains(in_the_corner.centre));
	BOOST_TEST(!sphere_traits::intersects(diagonal, in_the_corner));
	BOOST_TEST(sphere_traits::intersects(diagonal, rove::sphere<3>(v3(5, 0, 6), 1.1f)));    // 0.707 from the edge

	std::mt19937 rng(17);
	std::uniform_real_distribution<float> pos(-5, 5), r(0.05f, 3);
	int box_only = 0, met = 0;
	for (int k = 0; k < 3000; ++k)
	{
		tri_t const t{v3{pos(rng), pos(rng), pos(rng)}, v3{pos(rng), pos(rng), pos(rng)}, v3{pos(rng), pos(rng), pos(rng)}};
		rove::sphere<3> const s{v3{pos(rng), pos(rng), pos(rng)}, r(rng)};
		double const d = grid_distance(t, s.centre), slack = longest_edge(t) / GRID + 1e-4;
		bool const got = sphere_traits::intersects(t, s);
		BOOST_TEST_CONTEXT("case " << k << ": grid distance " << d << ", radius " << s.radius)
		{
			if (got)
				BOOST_TEST(d <= s.radius + slack);
			else
				BOOST_TEST(d > s.radius - 1e-4);
		}
		met += got;
		box_only += !got && rove::spatial_traits<tri_t>::bounds(t).test_intersection(s.bounds());
	}
	// the cases must tell an exact test from a box test
	BOOST_TEST(met > 200);
	BOOST_TEST(box_only > 200);
}

// A cone may report a triangle it does not reach, never pass over one it
// does: a spot light that skips a lit triangle leaves a hole in the light.
BOOST_AUTO_TEST_CASE(a_cone_never_passes_over_a_triangle_it_reaches)
{
	using cone_traits = rove::spatial_query_traits<tri_t, rove::cone<3>>;
	rove::cone<3> const spot(v3(0, 0, 0), v3(0, 0, 1), std::cos(rove::PI / 4), 10);

	// a sliver across the inside of the cone, off its axis, every vertex and
	// every corner of its box outside the cone
	tri_t const sliver(v3(3, -100, 5), v3(3, 100, 5), v3(3, 0, -100));
	BOOST_REQUIRE(spot.contains(v3(3, 0, 4)));
	BOOST_REQUIRE(!spot.contains(sliver.A) && !spot.contains(sliver.B) && !spot.contains(sliver.C));
	BOOST_TEST(cone_traits::intersects(sliver, spot));

	std::mt19937 rng(23);
	std::uniform_real_distribution<float> pos(-8, 8), unit(-1, 1), angle(0.05f, 1.4f), range(1, 10);
	int reached = 0, passed = 0;
	for (int k = 0; k < 4000; ++k)
	{
		v3 axis;
		do
			axis = v3{unit(rng), unit(rng), unit(rng)};
		while (axis.length_sq() < 0.01f);
		rove::cone<3> const c{v3{pos(rng), pos(rng), pos(rng)} * 0.25f, rove::normalize(axis), std::cos(angle(rng)), range(rng)};
		tri_t const t{v3{pos(rng), pos(rng), pos(rng)}, v3{pos(rng), pos(rng), pos(rng)}, v3{pos(rng), pos(rng), pos(rng)}};
		bool const got = cone_traits::intersects(t, c);
		if (grid_meets(t, c))
		{
			++reached;
			BOOST_TEST_CONTEXT("case " << k)
				BOOST_TEST(got);
		}
		else
			passed += !got;
	}
	BOOST_TEST(reached > 500);
	BOOST_TEST(passed > 500);     // not a test that says yes to everything
}

// What a cone must pass over: a triangle behind its apex, one past its range,
// one off to the side, one beyond the rim.
BOOST_AUTO_TEST_CASE(a_cone_passes_over_what_is_clearly_outside_it)
{
	using cone_traits = rove::spatial_query_traits<tri_t, rove::cone<3>>;
	rove::cone<3> const spot(v3(0, 0, 0), v3(0, 0, 1), std::cos(rove::PI / 6), 10);
	auto small_at = [](v3 const &c) { return tri_t(c, c + v3(0.2f, 0, 0), c + v3(0, 0.2f, 0)); };
	BOOST_REQUIRE(cone_traits::intersects(small_at(v3(0, 0, 5)), spot));
	BOOST_TEST(!cone_traits::intersects(small_at(v3(0, 0, -1)), spot));                // behind
	BOOST_TEST(!cone_traits::intersects(small_at(v3(0, 0, 11)), spot));                // past the range
	BOOST_TEST(!cone_traits::intersects(small_at(v3(5, 0, 2)), spot));                 // to the side
	BOOST_TEST(!cone_traits::intersects(small_at(v3(0, 7, 9)), spot));                 // 38 degrees off the axis
	BOOST_TEST(!cone_traits::intersects(tri_t(v3(-20, -20, -1), v3(20, -20, -1), v3(0, 20, -1)), spot));    // a floor behind
}

// What the cone holds, its bounds hold: the base's radius is range * tan(half angle)
BOOST_AUTO_TEST_CASE(a_cone_is_inside_its_bounds)
{
	std::mt19937 gen(5);
	std::uniform_real_distribution<float> u(-1, 1);
	for (float const degrees : {10.0f, 45.0f, 60.0f, 80.0f}) {
		rove::cone<3> const c(v3(1, 2, 3), normalize(v3(0.3f, 1, -0.2f)), std::cos(degrees * rove::PI / 180), 2);
		auto const b = c.bounds();
		int inside = 0;
		for (int i = 0; i < 200000; ++i) {
			v3 const p = c.apex + v3(u(gen), u(gen), u(gen)) * (2.5f * std::max(1.0f, std::tan(degrees * rove::PI / 180)));
			if (c.contains(p)) {
				++inside;
				BOOST_REQUIRE(b.contains(p));
			}
		}
		BOOST_TEST(inside > 100);
	}
}

BOOST_AUTO_TEST_CASE(a_cone_of_45_degrees_bounds_its_base)
{
	rove::cone<3> const c(v3(0, 0, 0), v3(0, 0, 1), std::cos(rove::PI / 4), 1);
	BOOST_TEST(c.contains(v3(0.99f, 0, 1)));
	BOOST_TEST(c.bounds().contains(v3(0.99f, 0, 1)));
	BOOST_TEST(c.bounds().hi.x == 1.0f, boost::test_tools::tolerance(1e-5f));
	BOOST_TEST(c.bounds().lo.y == -1.0f, boost::test_tools::tolerance(1e-5f));
}

// A half angle of 90 degrees has no finite base: its bounds say so instead of cutting it off
BOOST_AUTO_TEST_CASE(a_cone_of_90_degrees_has_unbounded_sides)
{
	rove::cone<3> const c(v3(0, 0, 0), v3(0, 0, 1), 0, 1);
	BOOST_TEST(c.contains(v3(1e6f, 0, 0.5f)));
	BOOST_TEST(c.bounds().contains(v3(1e6f, 0, 0.5f)));
}

BOOST_AUTO_TEST_CASE(an_octree_finds_what_a_cone_holds_at_the_edge_of_its_base)
{
	// a small box of a tree: subdivided so that the cone's bounds decide which cells are visited
	rove::octree<tri_t, int> tree(rove::aabb<3>(v3(-4, -4, -4), v3(4, 4, 4)), 1, 0.01f);
	// a tiny triangle at radius 0.9 of a base of radius 1, outside range * sin(45 degrees)
	tree.insert(tri_t(v3(0.9f, 0, 0.99f), v3(0.91f, 0, 0.99f), v3(0.9f, 0.01f, 0.99f)), 7);
	// others, so that the tree does divide
	for (int i = 0; i < 16; ++i) {
		float const x = -3.5f + 0.1f * float(i);
		tree.insert(tri_t(v3(x, -3, -3), v3(x + 0.01f, -3, -3), v3(x, -2.99f, -3)), i);
	}
	tree.build();
	rove::cone<3> const c(v3(0, 0, 0), v3(0, 0, 1), std::cos(rove::PI / 4), 1);
	auto const found = tree.query_intersection(c);
	BOOST_REQUIRE(found.size() == 1u);
	BOOST_TEST(found[0] == 7);
}

BOOST_AUTO_TEST_SUITE_END()
