// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

// Contract of hash_grid: every query answers exactly what a brute-force scan
// of all items answers -- each item once, whatever cells it spans, whichever
// axis is up -- and trace finds the nearest hit a scan finds, over long rays,
// from outside the grid and along cell boundaries, as a contact_info that
// says which item, where, and with a normal facing the ray.  It answers what octree
// answers for the same items, const queries are safe from several threads,
// and bad input is an error, not an empty answer.

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <rove/hash_grid.h>
#include <rove/octree.h>

namespace
{

using v3 = rove::vec<3>;
using tri_t = rove::triangle<3>;
using box_t = rove::aabb<3>;
using seg_t = rove::segment<3>;
using ray_type = rove::ray<3>;
using grid_type = rove::hash_grid<tri_t, int>;
using index_type = grid_type::index_type;
using contact_type = rove::contact_info<3>;

float const CELL = 2.0f;

box_t bounds_of(tri_t const &t)
{
	box_t b(t.A);
	b.extend(t.B);
	b.extend(t.C);
	return b;
}

// A scene that is hard on a grid: small and large triangles, slivers, walls
// and floors lying exactly on cell boundaries, triangles many cells across.
std::vector<tri_t> scene(unsigned seed)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> pos(-20, 20), small(-0.5f, 0.5f), large(-12, 12);
	std::uniform_int_distribution<int> line(-9, 9);
	auto p = [&] { return v3{pos(rng), pos(rng), pos(rng)}; };
	std::vector<tri_t> out;
	for (int i = 0; i < 300; ++i)
	{
		v3 const c = p();
		out.push_back(tri_t{c, c + v3{small(rng), small(rng), small(rng)}, c + v3{small(rng), small(rng), small(rng)}});
	}
	for (int i = 0; i < 40; ++i)
	{
		v3 const c = p();
		out.push_back(tri_t{c + v3{large(rng), large(rng), large(rng)}, c + v3{large(rng), large(rng), large(rng)},
			c + v3{large(rng), large(rng), large(rng)}});
	}
	for (int i = 0; i < 30; ++i)
	{
		v3 const a = p(), b = p();
		out.emplace_back(a, b, a + (b - a) * 0.5f + v3{small(rng), small(rng), small(rng)} * 0.01f);
	}
	for (int i = 0; i < 40; ++i)
	{
		// a wall in the plane x = k * CELL, y = k * CELL or z = k * CELL
		int const axis = i % 3;
		float const k = line(rng) * CELL;
		v3 a = p(), b = p(), c = p();
		a.i[axis] = b.i[axis] = c.i[axis] = k;
		out.emplace_back(a, b, c);
	}
	return out;
}

std::vector<box_t> boxes(unsigned seed)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> pos(-24, 24), size(0, 6);
	std::uniform_int_distribution<int> line(-10, 10);
	std::vector<box_t> out;
	for (int i = 0; i < 200; ++i)
	{
		v3 const lo{pos(rng), pos(rng), pos(rng)};
		out.emplace_back(lo, lo + v3{size(rng), size(rng), size(rng)});
	}
	for (int i = 0; i < 60; ++i)
	{
		// faces exactly on cell boundaries
		v3 const lo{line(rng) * CELL, line(rng) * CELL, line(rng) * CELL};
		out.emplace_back(lo, lo + v3(CELL * (1 + i % 3), CELL, CELL * (i % 2)));
	}
	for (int i = 0; i < 20; ++i)
	{
		v3 const c{pos(rng), pos(rng), pos(rng)};
		out.emplace_back(c, c);                                       // a point
	}
	out.emplace_back(v3(-1e6f, -1e6f, -1e6f), v3(1e6f, 1e6f, 1e6f));     // everything
	out.emplace_back(v3(100, 100, 100), v3(110, 110, 110));             // nothing
	out.emplace_back(v3(-1e6f, -1, -1), v3(1e6f, 1, 1));                // a long thin slab
	return out;
}

template<class Query, class Test>
std::vector<index_type> scan(std::vector<tri_t> const &tris, Query const &q, Test test)
{
	std::vector<index_type> out;
	for (size_t i = 0; i < tris.size(); ++i)
		if (test(tris[i], q))
			out.push_back(index_type(i));
	return out;
}

grid_type grid_of(std::vector<tri_t> const &tris, int up = 1)
{
	grid_type g(CELL, up);
	g.reserve(tris.size() / 2);          // too little on purpose: insert goes on past it
	for (size_t i = 0; i < tris.size(); ++i)
		BOOST_REQUIRE_EQUAL(g.insert(tris[i], int(i)), index_type(i));
	return g;
}

std::string str(v3 const &v)
{
	return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}

std::vector<index_type> sorted(std::vector<index_type> v)
{
	std::sort(v.begin(), v.end());
	return v;
}

bool has_duplicates(std::vector<index_type> v)
{
	std::sort(v.begin(), v.end());
	return std::adjacent_find(v.begin(), v.end()) != v.end();
}

struct nearest
{
	index_type index;
	float t;
};

std::optional<nearest> scan_trace(std::vector<tri_t> const &tris, ray_type const &r, float t_min, float t_max)
{
	std::optional<nearest> best;
	for (size_t i = 0; i < tris.size(); ++i)
	{
		float t;
		v3 at;
		if (tris[i].trace(r, t, at, t_min, t_max) && (!best || t < best->t))
			best = nearest{index_type(i), t};
	}
	return best;
}

std::vector<ray_type> rays(unsigned seed)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> pos(-26, 26), dir(-1, 1), len(1, 70);
	std::uniform_int_distribution<int> line(-10, 10);
	std::vector<ray_type> out;
	for (int i = 0; i < 400; ++i)
	{
		v3 d{dir(rng), dir(rng), dir(rng)};
		d.normalize();
		out.push_back(ray_type{v3{pos(rng), pos(rng), pos(rng)}, d * len(rng)});
	}
	for (int i = 0; i < 60; ++i)
	{
		// along an axis, on cell boundary lines, both ways
		int const axis = i % 3;
		v3 o{line(rng) * CELL, line(rng) * CELL, line(rng) * CELL}, d(0, 0, 0);
		o.i[axis] = pos(rng);
		d.i[axis] = (i % 2 ? 1.0f : -1.0f) * len(rng);
		out.emplace_back(o, d);
	}
	for (int i = 0; i < 40; ++i)
	{
		// from far outside, through the scene
		v3 const from{pos(rng) * 40, pos(rng) * 40, pos(rng) * 40}, to{pos(rng), pos(rng), pos(rng)};
		out.emplace_back(from, (to - from) * 1.5f);
	}
	return out;
}

}

BOOST_AUTO_TEST_SUITE(test_hash_grid)

BOOST_AUTO_TEST_CASE(insert_numbers_items_in_order_and_keeps_them)
{
	std::vector<tri_t> const tris = scene(1);
	grid_type const g = grid_of(tris);
	BOOST_REQUIRE_EQUAL(g.size(), tris.size());
	for (size_t i = 0; i < tris.size(); ++i)
	{
		BOOST_TEST((g[index_type(i)].primitive.A == tris[i].A));
		BOOST_TEST((g[index_type(i)].primitive.C == tris[i].C));
		BOOST_TEST(g[index_type(i)].payload == int(i));
	}
}

BOOST_AUTO_TEST_CASE(bounds_are_the_union_of_the_items)
{
	std::vector<tri_t> const tris = scene(2);
	grid_type const g = grid_of(tris);
	box_t expected = bounds_of(tris[0]);
	for (tri_t const &t : tris)
		expected.extend(bounds_of(t));
	BOOST_TEST((g.bounds().lo == expected.lo));
	BOOST_TEST((g.bounds().hi == expected.hi));
}

BOOST_AUTO_TEST_CASE(box_queries_match_a_scan_for_every_up_axis)
{
	std::vector<tri_t> const tris = scene(3);
	auto const meets = [](tri_t const &t, box_t const &b) { return bounds_of(t).test_intersection(b); };
	for (int up = 0; up < 3; ++up)
	{
		grid_type const g = grid_of(tris, up);
		std::vector<box_t> const bs = boxes(30 + up);
		for (size_t k = 0; k < bs.size(); ++k)
		{
			std::vector<index_type> got;
			g.query_indices(bs[k], got);
			BOOST_TEST_CONTEXT("up " << up << " box " << k << " " << str(bs[k].lo) << " .. " << str(bs[k].hi))
			{
				BOOST_TEST(!has_duplicates(got));
				BOOST_TEST(sorted(got) == scan(tris, bs[k], meets), boost::test_tools::per_element());
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(segment_and_sphere_queries_are_exact_not_bounds)
{
	std::vector<tri_t> const tris = scene(4);
	grid_type const g = grid_of(tris);
	std::mt19937 rng(41);
	std::uniform_real_distribution<float> pos(-24, 24), r(0.1f, 4);
	size_t hits = 0, bound_only = 0;
	for (int k = 0; k < 200; ++k)
	{
		seg_t const s{v3{pos(rng), pos(rng), pos(rng)}, v3{pos(rng), pos(rng), pos(rng)}};
		std::vector<index_type> got;
		g.query_indices(s, got);
		auto const exact = scan(tris, s, [](tri_t const &t, seg_t const &q) { return t.trace(q.to_ray(), 0, 1); });
		auto const loose = scan(tris, s.bounds(), [](tri_t const &t, box_t const &b) { return bounds_of(t).test_intersection(b); });
		BOOST_TEST_CONTEXT("segment " << k)
		{
			BOOST_TEST(!has_duplicates(got));
			BOOST_TEST(sorted(got) == exact, boost::test_tools::per_element());
		}
		hits += exact.size();
		bound_only += loose.size() - exact.size();
	}
	// the scene must make the difference between exact and bounds visible
	BOOST_TEST(hits > 50u);
	BOOST_TEST(bound_only > 500u);

	// a sphere meets a triangle that comes within its radius (triangle::closest_point
	// has a contract of its own in test_triangle)
	hits = bound_only = 0;
	for (int k = 0; k < 200; ++k)
	{
		rove::sphere<3> const s{v3{pos(rng), pos(rng), pos(rng)}, r(rng)};
		std::vector<index_type> got;
		g.query_indices(s, got);
		auto const exact = scan(tris, s, [](tri_t const &t, rove::sphere<3> const &q) {
			return (t.closest_point(q.centre) - q.centre).length_sq() <= q.radius * q.radius;
		});
		auto const loose = scan(tris, s.bounds(), [](tri_t const &t, box_t const &b) { return bounds_of(t).test_intersection(b); });
		BOOST_TEST_CONTEXT("sphere " << k)
		{
			BOOST_TEST(!has_duplicates(got));
			BOOST_TEST(sorted(got) == exact, boost::test_tools::per_element());
		}
		hits += exact.size();
		bound_only += loose.size() - exact.size();
	}
	BOOST_TEST(hits > 50u);
	BOOST_TEST(bound_only > 100u);
}

BOOST_AUTO_TEST_CASE(queries_append_to_the_result)
{
	grid_type g(CELL);
	g.insert(tri_t(v3(0, 0, 0), v3(1, 0, 0), v3(0, 0, 1)), 7);
	std::vector<index_type> indices{99};
	g.query_indices(box_t(v3(-1, -1, -1), v3(1, 1, 1)), indices);
	BOOST_TEST(indices == std::vector<index_type>({99, 0}), boost::test_tools::per_element());
	std::vector<int> payloads{-1};
	g.query_intersection(box_t(v3(-1, -1, -1), v3(1, 1, 1)), payloads);
	BOOST_TEST(payloads == std::vector<int>({-1, 7}), boost::test_tools::per_element());
}

// The same items in an octree and in a hash grid give the same answers, so a
// caller can switch between them.
BOOST_AUTO_TEST_CASE(answers_what_octree_answers)
{
	std::vector<tri_t> const tris = scene(5);
	grid_type const g = grid_of(tris);
	rove::octree<tri_t, int> o(box_t(v3(-40, -40, -40), v3(40, 40, 40)));
	for (size_t i = 0; i < tris.size(); ++i)
		o.insert(tris[i], int(i));
	o.build();
	auto as_set = [](std::vector<int> v) { std::sort(v.begin(), v.end()); return v; };
	for (box_t const &b : boxes(50))
		BOOST_TEST(as_set(g.query_intersection(b)) == as_set(o.query_intersection(b)), boost::test_tools::per_element());
	std::mt19937 rng(51);
	std::uniform_real_distribution<float> pos(-24, 24);
	for (int k = 0; k < 100; ++k)
	{
		seg_t const s{v3{pos(rng), pos(rng), pos(rng)}, v3{pos(rng), pos(rng), pos(rng)}};
		BOOST_TEST(as_set(g.query_intersection(s)) == as_set(o.query_intersection(s)), boost::test_tools::per_element());
	}
	std::vector<grid_type::value_type> items;
	g.query_intersection_items(box_t(v3(-3, -3, -3), v3(3, 3, 3)), items);
	BOOST_TEST(!items.empty());
	for (auto const &item : items)
		BOOST_TEST((item.primitive.A == tris[size_t(item.payload)].A));
}

// Items thousands of cells apart: a box covering both must not walk every
// empty cell between them (this would take minutes), and must find both.
BOOST_AUTO_TEST_CASE(sparse_items_far_apart)
{
	grid_type g(0.5f);
	g.insert(tri_t(v3(-50000, 0, -50000), v3(-49999, 0, -50000), v3(-50000, 0, -49999)), 0);
	g.insert(tri_t(v3(50000, 0, 50000), v3(50001, 0, 50000), v3(50000, 0, 50001)), 1);
	std::vector<index_type> got;
	g.query_indices(box_t(v3(-1e5f, -1, -1e5f), v3(1e5f, 1, 1e5f)), got);
	BOOST_TEST(sorted(got) == std::vector<index_type>({0, 1}), boost::test_tools::per_element());
	got.clear();
	g.query_indices(box_t(v3(-40000, -1, -40000), v3(40000, 1, 40000)), got);
	BOOST_TEST(got.empty());
	// a ray that meets the near item at once, and one that crosses ~180000
	// columns of nothing to the far one
	auto const near = g.trace(ray_type(v3(-59999.8f, 10000, -59999.8f), v3(1e5f, -1e5f, 1e5f)), 0, 1);
	BOOST_REQUIRE(near.happened);
	BOOST_TEST((near.index == std::optional<size_t>(0)));
	BOOST_TEST(near.time == 0.1f, boost::test_tools::tolerance(1e-4f));
	auto const far = g.trace(ray_type(v3(-39999.8f, 9000, -39999.8f), v3(1e5f, -1e4f, 1e5f)), 0, 1);
	BOOST_REQUIRE(far.happened);
	BOOST_TEST((far.index == std::optional<size_t>(1)));
	BOOST_TEST(far.time == 0.9f, boost::test_tools::tolerance(1e-4f));
}

BOOST_AUTO_TEST_CASE(trace_finds_what_a_scan_finds_for_every_up_axis)
{
	std::vector<tri_t> const tris = scene(6);
	size_t hits = 0, misses = 0;
	for (int up = 0; up < 3; ++up)
	{
		grid_type const g = grid_of(tris, up);
		std::vector<ray_type> const rs = rays(60 + up);
		for (size_t k = 0; k < rs.size(); ++k)
			for (auto const [t_min, t_max] : {std::pair(0.0f, 1.0f), std::pair(0.2f, 0.6f), std::pair(0.0f, 0.05f)})
			{
				auto const expected = scan_trace(tris, rs[k], t_min, t_max);
				auto const got = g.trace(rs[k], t_min, t_max);
				BOOST_TEST_CONTEXT("up " << up << " ray " << k << " from " << str(rs[k].r0) << " along " << str(rs[k].a) << " t " << t_min << ".." << t_max)
				{
					BOOST_REQUIRE_EQUAL(got.happened, bool(expected));
					if (!got.happened)
					{
						BOOST_TEST(!got.index);
						++misses;
						continue;
					}
					++hits;
					BOOST_TEST(got.time == expected->t);
					BOOST_REQUIRE(got.index);
					BOOST_REQUIRE(*got.index < tris.size());
					tri_t const &hit = tris[*got.index];
					float t;
					v3 at;
					BOOST_TEST(hit.trace(rs[k], t, at, t_min, t_max));
					BOOST_TEST(t == got.time);
					BOOST_TEST((got.position - rs[k].apply(got.time)).length() < 1e-3f);
					BOOST_TEST(!got.penetrated);
					// a unit normal of the triangle hit, on the side the ray came from
					BOOST_TEST(std::abs(got.normal.length() - 1) < 1e-5f);
					BOOST_TEST(std::abs(dot(got.normal, hit.normal())) > 1 - 1e-5f);
					BOOST_TEST(dot(got.normal, rs[k].a) <= 0);
				}
			}
	}
	BOOST_TEST(hits > 500u);
	BOOST_TEST(misses > 100u);
}

BOOST_AUTO_TEST_CASE(trace_through_walls_on_cell_boundaries)
{
	// walls at x = 0, 2, 4, ... 20: each exactly on a cell boundary
	grid_type g(CELL);
	for (int k = 0; k <= 10; ++k)
	{
		float const x = k * CELL;
		g.insert(tri_t(v3(x, -5, -5), v3(x, 5, -5), v3(x, 0, 5)), k);
	}
	for (float const x0 : {-3.0f, -1.0f, 0.0f, 1.0f, 2.0f, 2.5f, 3.999f, 4.0f})
	{
		auto const forward = g.trace(ray_type(v3(x0, 0, 0), v3(100, 0, 0)), 0, 1);
		BOOST_REQUIRE(forward.happened);
		float const expected = std::max(0.0f, std::ceil(x0 / CELL) * CELL);
		BOOST_TEST_CONTEXT("+x from " << x0)
			BOOST_TEST(forward.time * 100 + x0 == expected, boost::test_tools::tolerance(1e-5f));

		auto const back = g.trace(ray_type(v3(x0 + 10, 0, 0), v3(-100, 0, 0)), 0, 1);
		BOOST_REQUIRE(back.happened);
		float const expected_back = std::min(20.0f, std::floor((x0 + 10) / CELL) * CELL);
		BOOST_TEST_CONTEXT("-x from " << x0 + 10)
			BOOST_TEST(x0 + 10 - back.time * 100 == expected_back, boost::test_tools::tolerance(1e-5f));
	}
	// along the up axis: in the plane of the wall at x = 4 (parallel, a miss)
	// down onto a floor below it; beside the floor; with no direction at all
	g.insert(tri_t(v3(3, -6, -2), v3(6, -6, -2), v3(3, -6, 4)), 100);
	auto const down = g.trace(ray_type(v3(4, 20, 0), v3(0, -40, 0)), 0, 1);
	BOOST_REQUIRE(down.happened);
	BOOST_TEST((down.index == std::optional<size_t>(11)));
	BOOST_TEST(down.time == 0.65f, boost::test_tools::tolerance(1e-6f));
	BOOST_TEST((down.position - v3(4, -6, 0)).length() < 1e-5f);
	BOOST_TEST((down.normal - v3(0, 1, 0)).length() < 1e-6f);      // wound facing down, reported facing the ray
	BOOST_TEST(!g.trace(ray_type(v3(4, 20, 0), v3(0, -40, 0)), 0, 0.6f).happened);
	BOOST_TEST(!g.trace(ray_type(v3(1, 20, 0), v3(0, -40, 0)), 0, 1).happened);
	// an unbounded ray: t_max infinite, and a direction with zero components
	float const inf = std::numeric_limits<float>::infinity();
	auto const forever = g.trace(ray_type(v3(4, 20, 0), v3(0, -1, 0)), 0, inf);
	BOOST_REQUIRE(forever.happened);
	BOOST_TEST((forever.index == std::optional<size_t>(11)));
	BOOST_TEST(forever.time == 26.0f, boost::test_tools::tolerance(1e-6f));
	auto const along = g.trace(ray_type(v3(-7, 0, 0), v3(1, 0, 0)), 0, inf);
	BOOST_REQUIRE(along.happened);
	BOOST_TEST(along.time == 7.0f, boost::test_tools::tolerance(1e-6f));
	BOOST_TEST(!g.trace(ray_type(v3(-7, 0, 0), v3(-1, 0, 0)), 0, inf).happened);
	// a whole line, both ways from the origin: the nearest t is the most negative
	auto const line = g.trace(ray_type(v3(-7, 0, 0), v3(-1, 0, 0)), -inf, inf);
	BOOST_REQUIRE(line.happened);
	BOOST_TEST(line.time == -27.0f, boost::test_tools::tolerance(1e-6f));
	BOOST_TEST(!g.trace(ray_type(v3(3, 0, 0), v3(0, 0, 0)), 0, 1).happened);
}

BOOST_AUTO_TEST_CASE(trace_with_its_own_test)
{
	std::vector<tri_t> const tris = scene(7);
	grid_type const g = grid_of(tris);
	// only items with an even payload count, and the contact is reported as
	// the test filled it: the payload as its normal's z
	auto const even = [](grid_type::value_type const &item, ray_type const &r, float t_min, float t_max, contact_type &ci) {
		if (item.payload % 2 != 0 || !item.primitive.trace(r, ci.time, ci.position, t_min, t_max))
			return false;
		ci.normal = v3(0, 0, float(item.payload));
		return true;
	};
	std::vector<tri_t> evens;
	for (size_t i = 0; i < tris.size(); i += 2)
		evens.push_back(tris[i]);
	size_t hits = 0;
	for (ray_type const &r : rays(70))
	{
		auto const expected = scan_trace(evens, r, 0, 1);
		auto const got = g.trace(r, 0, 1, even);
		BOOST_REQUIRE_EQUAL(got.happened, bool(expected));
		if (got.happened)
		{
			++hits;
			BOOST_REQUIRE(got.index);
			BOOST_TEST(*got.index % 2 == 0u);
			BOOST_TEST(got.time == expected->t);
			BOOST_TEST(got.normal.z == float(*got.index));
		}
	}
	BOOST_TEST(hits > 100u);
}

BOOST_AUTO_TEST_CASE(an_empty_grid_answers_nothing)
{
	grid_type const g(CELL);
	std::vector<index_type> got;
	g.query_indices(box_t(v3(-1e6f, -1e6f, -1e6f), v3(1e6f, 1e6f, 1e6f)), got);
	BOOST_TEST(got.empty());
	BOOST_TEST(!g.trace(ray_type(v3(0, 0, 0), v3(1, 1, 1)), 0, 1).happened);
	BOOST_TEST(g.size() == 0u);
	BOOST_CHECK_THROW(g.bounds(), std::logic_error);
}

// clear() leaves a grid that answers nothing and then fills again as a new one
// does: numbers from 0, bounds of the new items only, no trace of the old ones
// in any query (a grid rebuilt every frame from things that move).
BOOST_AUTO_TEST_CASE(clear_empties_the_grid_and_it_fills_again_as_new)
{
	std::vector<tri_t> const before = scene(11), after = scene(12);
	grid_type g = grid_of(before);
	g.clear();
	std::vector<index_type> got;
	g.query_indices(box_t(v3(-1e6f, -1e6f, -1e6f), v3(1e6f, 1e6f, 1e6f)), got);
	BOOST_TEST(got.empty());
	BOOST_TEST(!g.trace(ray_type(v3(-30, -30, -30), v3(60, 60, 60)), 0, 1).happened);
	BOOST_TEST(g.size() == 0u);
	BOOST_CHECK_THROW(g.bounds(), std::logic_error);

	for (size_t i = 0; i < after.size(); ++i)
		BOOST_REQUIRE_EQUAL(g.insert(after[i], int(i)), index_type(i));
	box_t expected = bounds_of(after[0]);
	for (tri_t const &t : after)
		expected.extend(bounds_of(t));
	BOOST_TEST((g.bounds().lo == expected.lo));
	BOOST_TEST((g.bounds().hi == expected.hi));
	auto const meets = [](tri_t const &t, box_t const &b) { return bounds_of(t).test_intersection(b); };
	for (box_t const &b : boxes(13))
	{
		got.clear();
		g.query_indices(b, got);
		BOOST_TEST(!has_duplicates(got));
		BOOST_TEST(sorted(got) == scan(after, b, meets), boost::test_tools::per_element());
	}
	for (ray_type const &r : rays(14))
	{
		auto const expected_hit = scan_trace(after, r, 0, 1);
		auto const hit = g.trace(r, 0, 1);
		BOOST_REQUIRE_EQUAL(hit.happened, bool(expected_hit));
		if (hit.happened)
			BOOST_TEST(hit.time == expected_hit->t);
	}
}

// Queries are const and touch no shared state: several threads querying at
// once get the answers a single thread gets.
BOOST_AUTO_TEST_CASE(const_queries_are_safe_from_several_threads)
{
	std::vector<tri_t> const tris = scene(8);
	grid_type const g = grid_of(tris);
	std::vector<box_t> const bs = boxes(80);
	std::vector<std::vector<index_type>> expected(bs.size());
	for (size_t k = 0; k < bs.size(); ++k)
	{
		g.query_indices(bs[k], expected[k]);
		std::sort(expected[k].begin(), expected[k].end());
	}
	std::vector<int> wrong(4, 0);
	std::vector<std::thread> threads;
	for (int n = 0; n < 4; ++n)
		threads.emplace_back([&, n] {
			for (int round = 0; round < 20; ++round)
				for (size_t k = 0; k < bs.size(); ++k)
				{
					std::vector<index_type> got;
					g.query_indices(bs[(k + n * 37) % bs.size()], got);
					std::sort(got.begin(), got.end());
					wrong[n] += got != expected[(k + n * 37) % bs.size()];
				}
		});
	for (std::thread &t : threads)
		t.join();
	BOOST_TEST(wrong == std::vector<int>(4, 0), boost::test_tools::per_element());
}

namespace
{

// Boxes that meet only when the query holds the stored box whole.
struct holding_traits: rove::spatial_traits<box_t> {
	static bool intersects(box_t const &stored, box_t const &query)
	{
		return query.contains(stored.lo) && query.contains(stored.hi);
	}
};

}

// A query of the stored type is tested by the grid's own traits, as a query
// of any other type is by the traits given with it.
BOOST_AUTO_TEST_CASE(a_query_of_the_stored_type_uses_the_grids_own_traits)
{
	rove::hash_grid<box_t, int, holding_traits> g(CELL);
	g.insert(box_t(v3(0, 0, 0), v3(0.5f, 0.5f, 0.5f)), 1);
	g.insert(box_t(v3(0.4f, 0.4f, 0.4f), v3(0.9f, 0.9f, 0.9f)), 2);
	box_t const query(v3(-0.1f, -0.1f, -0.1f), v3(0.6f, 0.6f, 0.6f));
	BOOST_TEST(g.query_intersection(query) == std::vector<int>({1}), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(bad_input_is_an_error)
{
	float const nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
	BOOST_CHECK_THROW(grid_type{0}, std::invalid_argument);
	BOOST_CHECK_THROW(grid_type{-1}, std::invalid_argument);
	BOOST_CHECK_THROW(grid_type{nan}, std::invalid_argument);
	BOOST_CHECK_THROW(grid_type{inf}, std::invalid_argument);
	BOOST_CHECK_THROW((grid_type{1, 3}), std::invalid_argument);
	BOOST_CHECK_THROW((grid_type{1, -1}), std::invalid_argument);

	grid_type g(CELL);
	BOOST_CHECK_THROW(g.insert(tri_t(v3(nan, 0, 0), v3(1, 0, 0), v3(0, 0, 1)), 0), std::invalid_argument);
	BOOST_CHECK_THROW(g.insert(tri_t(v3(0, 0, 0), v3(1, nan, 0), v3(0, 0, 1)), 0), std::invalid_argument);
	BOOST_CHECK_THROW(g.insert(tri_t(v3(0, 0, 0), v3(1, 0, 0), v3(0, 0, nan)), 0), std::invalid_argument);
	BOOST_CHECK_THROW(g.insert(tri_t(v3(0, 0, 0), v3(1, inf, 0), v3(0, 0, 1)), 0), std::invalid_argument);
	BOOST_CHECK_THROW(g.insert(tri_t(v3(1e30f, 0, 0), v3(1, 0, 0), v3(0, 0, 1)), 0), std::out_of_range);
	BOOST_TEST(g.size() == 0u);         // nothing half-inserted

	g.insert(tri_t(v3(0, 0, 0), v3(1, 0, 0), v3(0, 0, 1)), 0);
	std::vector<index_type> got;
	BOOST_CHECK_THROW(g.query_indices(box_t(v3(nan, 0, 0), v3(1, 1, 1)), got), std::invalid_argument);
	BOOST_CHECK_THROW((void)g.trace(ray_type(v3(nan, 0, 0), v3(1, 0, 0)), 0, 1), std::invalid_argument);
	BOOST_CHECK_THROW((void)g.trace(ray_type(v3(0, 0, 0), v3(1, nan, 0)), 0, 1), std::invalid_argument);
	BOOST_CHECK_THROW((void)g.trace(ray_type(v3(0, 0, 0), v3(1, 0, 0)), nan, 1), std::invalid_argument);
	BOOST_CHECK_THROW((void)g.trace(ray_type(v3(0, 0, 0), v3(1, 0, 0)), 1, 0), std::invalid_argument);
	BOOST_CHECK_THROW(g[1], std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
