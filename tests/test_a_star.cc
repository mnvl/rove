// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <chrono>
#include <map>
#include <memory>
#include <random>
#include <rove/a_star.h>

BOOST_AUTO_TEST_SUITE(a_star)

struct simple_scorer_with_wall {
	rove::scalar operator()(rove::vec<2,int> const &p) const {
		if (p.x == 0 && (p.y == 0 || p.y == -1)) return -1.0f;
		return 1.0f;
	}
};

BOOST_AUTO_TEST_CASE(simple_path_1)
{
	using point = rove::vec<2,int>;

	rove::a_star<simple_scorer_with_wall> a_star(point(-1, 0), point(1, 0), simple_scorer_with_wall());
	a_star.calculate_path();
	BOOST_REQUIRE(!a_star.have_failed());

	std::vector<point> path = a_star.build_path();
	BOOST_REQUIRE(path.front() == point(-1, 0));
	BOOST_REQUIRE(path.back() == point(1, 0));
	BOOST_REQUIRE(path.size() >= 3);
}

BOOST_AUTO_TEST_CASE(simple_path_2)
{
	using point = rove::vec<2,int>;

	rove::a_star<simple_scorer_with_wall> a_star(point(-2, 0), point(1, 0), simple_scorer_with_wall());
	a_star.calculate_path();
	BOOST_REQUIRE(!a_star.have_failed());

	std::vector<point> path = a_star.build_path();
	BOOST_REQUIRE(path.front() == point(-2, 0));
	BOOST_REQUIRE(path.back() == point(1, 0));
	BOOST_REQUIRE(path.size() >= 4);
}

BOOST_AUTO_TEST_CASE(simple_path_3)
{
	using point = rove::vec<2,int>;

	rove::a_star<simple_scorer_with_wall> a_star(point(-1, 0), point(2, 0), simple_scorer_with_wall());
	a_star.calculate_path();
	BOOST_REQUIRE(!a_star.have_failed());

	std::vector<point> path = a_star.build_path();
	BOOST_REQUIRE(path.front() == point(-1, 0));
	BOOST_REQUIRE(path.back() == point(2, 0));
	BOOST_REQUIRE(path.size() >= 4);
}

struct uniform_scorer {
	rove::scalar operator()(rove::vec<2,int> const &) const {
		return 1.0f;
	}
};

BOOST_AUTO_TEST_CASE(simple_path_without_obstacles)
{
	using point = rove::vec<2,int>;

	rove::a_star<uniform_scorer> a_star(point(0, 0), point(10, 0), uniform_scorer());
	a_star.calculate_path();
	BOOST_REQUIRE(!a_star.have_failed());

	std::vector<point> path = a_star.build_path();
	BOOST_REQUIRE(path.front() == point(0, 0));
	BOOST_REQUIRE(path.back() == point(10, 0));
}

struct wall_scorer {
	rove::scalar operator()(rove::vec<2,int> const &p) const {
		if (p.x == 5 && (p.y > -5 && p.y < 3)) return -1.0f;
		return 1.0f;
	}
};

BOOST_AUTO_TEST_CASE(simple_path_4)
{
	using point = rove::vec<2,int>;

	rove::a_star<wall_scorer> a_star(point(0, 0), point(10, 0), wall_scorer());
	a_star.calculate_path();
	BOOST_REQUIRE(!a_star.have_failed());

	std::vector<point> path = a_star.build_path();
	BOOST_REQUIRE(path.front() == point(0, 0));
	BOOST_REQUIRE(path.back() == point(10, 0));
	BOOST_REQUIRE(path.size() >= 11);
}

struct blocked_scorer {
	rove::scalar operator()(rove::vec<2,int> const &p) const {
		if (p.x == 0 && p.y == 0) return 1.0f;
		return -1.0f;
	}
};

BOOST_AUTO_TEST_CASE(unreachable_goal)
{
	using point = rove::vec<2,int>;

	rove::a_star<blocked_scorer> a_star(point(0, 0), point(5, 5), blocked_scorer());
	a_star.calculate_path();
	BOOST_REQUIRE(a_star.have_failed());

	std::vector<point> path = a_star.build_path();
	BOOST_REQUIRE(path.empty());
}

struct cost_map_scorer {
	rove::scalar operator()(rove::vec<2,int> const &p) const {
		if (p.x < 0 || p.x >= 10 || p.y < 0 || p.y >= 10) return -1.0f;
		return 1.0f;
	}
};

BOOST_AUTO_TEST_CASE(build_cost_map_test)
{
	using point = rove::vec<2,int>;

	rove::a_star<cost_map_scorer> a_star(point(0, 0), point(5, 0), cost_map_scorer());
	a_star.calculate_path();
	BOOST_REQUIRE(!a_star.have_failed());

	std::vector<rove::scalar> cost_map = a_star.build_cost_map(10, 10);
	BOOST_REQUIRE(cost_map.size() == 100);
	BOOST_REQUIRE(cost_map[0] == 0.0f); // start has cost 0
	BOOST_REQUIRE(cost_map[5] > 0.0f); // goal has positive cost
}


// ---- the contract of the search itself: cheapest paths, no corner cutting, honest costs ----

namespace
{

using pt = rove::vec<2,int>;

struct random_grid
{
	int w, h;
	std::vector<rove::scalar> cost;      // < 0 blocked
	rove::scalar operator()(pt const &p) const {
		if (p.x < 0 || p.x >= w || p.y < 0 || p.y >= h) return -1;
		return cost[size_t(p.y) * size_t(w) + size_t(p.x)];
	}
};

random_grid make_grid(int w, int h, unsigned seed, int wall_percent, int max_cost)
{
	std::mt19937 rng(seed);
	random_grid g{w, h, std::vector<rove::scalar>(size_t(w) * size_t(h))};
	for (auto &c : g.cost)
		c = int(rng() % 100) < wall_percent ? -1.0f : rove::scalar(1 + int(rng() % unsigned(max_cost)));
	return g;
}

/// What a path costs by the rules: entering a cell costs its score, times sqrt(2) on a diagonal step.
double path_cost(random_grid const &g, std::vector<pt> const &path)
{
	double total = 0;
	for (size_t i = 1; i < path.size(); ++i)
	{
		int const dx = path[i].x - path[i - 1].x, dy = path[i].y - path[i - 1].y;
		BOOST_REQUIRE(std::abs(dx) <= 1 && std::abs(dy) <= 1 && (dx || dy));
		BOOST_REQUIRE(g(path[i]) >= 0);
		if (dx && dy)      // no corner cut: both cells the step passes between are open
		{
			BOOST_REQUIRE(g(pt(path[i - 1].x + dx, path[i - 1].y)) >= 0);
			BOOST_REQUIRE(g(pt(path[i - 1].x, path[i - 1].y + dy)) >= 0);
		}
		total += double(g(path[i])) * (dx && dy ? std::sqrt(2.0) : 1.0);
	}
	return total;
}

}

BOOST_AUTO_TEST_CASE(the_path_is_a_cheapest_one_on_random_grids_with_walls_and_varied_costs)
{
	int found = 0, none = 0;
	for (unsigned seed = 1; seed <= 60; ++seed)
	{
		random_grid g = make_grid(24, 24, seed, 28, 4);
		g.cost[0] = 1;
		g.cost[size_t(23) * 24 + 23] = 1;
		rove::a_star<random_grid> reference(pt(0, 0), pt(23, 23), g);
		reference.calculate_cost_map();      // the exact distances: Dijkstra
		std::vector<rove::scalar> const dist = reference.build_cost_map(24, 24);
		rove::a_star<random_grid> finder(pt(0, 0), pt(23, 23), g);
		finder.calculate_path();
		BOOST_REQUIRE(finder.have_failed() == (dist[size_t(23) * 24 + 23] < 0));       // reachable exactly when the flood fill reaches it
		if (finder.have_failed())
		{
			++none;
			BOOST_REQUIRE(finder.build_path().empty());
			continue;
		}
		++found;
		std::vector<pt> const path = finder.build_path();
		BOOST_REQUIRE(path.front() == pt(0, 0) && path.back() == pt(23, 23));
		BOOST_TEST(path_cost(g, path) == double(dist[size_t(23) * 24 + 23]), boost::test_tools::tolerance(2e-4));      // within the 0.01 % of its tie break
	}
	BOOST_TEST(found > 20);        // the test is not vacuous: most grids have a way, some do not
	BOOST_TEST(none > 0);
}

BOOST_AUTO_TEST_CASE(a_diagonal_costs_sqrt_2_and_a_corner_is_not_cut)
{
	rove::a_star<uniform_scorer> open(pt(0, 0), pt(5, 5), uniform_scorer());
	open.calculate_path();
	std::vector<pt> const p = open.build_path();
	BOOST_REQUIRE(p.size() == 6u);         // five diagonal steps
	std::vector<rove::scalar> const map = open.build_cost_map(6, 6);
	BOOST_TEST(map[5 * 6 + 5] == 5 * std::sqrt(2.0f), boost::test_tools::tolerance(1e-5f));
	// two walls meeting at (0,0)'s corner: the diagonal step between them is shut, so no path goes (0,0) -> (1,1) directly
	struct corner { rove::scalar operator()(pt const &q) const { return (q == pt(1, 0) || q == pt(0, 1) || std::abs(q.x) > 3 || std::abs(q.y) > 3) ? -1.0f : 1.0f; } };
	rove::a_star<corner> finder(pt(0, 0), pt(1, 1), corner());
	finder.calculate_path();
	BOOST_REQUIRE(!finder.have_failed());
	std::vector<pt> const q = finder.build_path();
	BOOST_TEST(q.size() > 2u);
}

BOOST_AUTO_TEST_CASE(the_goal_is_judged_when_taken_not_when_first_seen)
{
	// A cheap-looking diagonal into an expensive goal cell versus a longer way into it through cheap cells
	struct trap {
		rove::scalar operator()(pt const &q) const {
			if (q.x < 0 || q.x > 6 || q.y < 0 || q.y > 6) return -1;
			if (q == pt(6, 6)) return 1;
			if (q.x == 5 && q.y == 5) return 50;      // the diagonal approach passes this
			return 1;
		}
	};
	rove::a_star<trap> finder(pt(0, 0), pt(6, 6), trap());
	finder.calculate_path();
	BOOST_REQUIRE(!finder.have_failed());
	for (pt const &q : finder.build_path())
		BOOST_TEST(!(q == pt(5, 5)));      // it goes round the 50
}

BOOST_AUTO_TEST_CASE(start_equal_to_goal_and_blocked_ends_and_a_bad_min_cost)
{
	rove::a_star<uniform_scorer> same(pt(3, 3), pt(3, 3), uniform_scorer());
	same.calculate_path();
	BOOST_REQUIRE(!same.have_failed());
	BOOST_TEST(same.build_path().size() == 1u);
	rove::a_star<blocked_scorer> blocked_goal(pt(0, 0), pt(1, 0), blocked_scorer());
	blocked_goal.calculate_path();
	BOOST_TEST(blocked_goal.have_failed());
	struct nothing { rove::scalar operator()(pt const &) const { return -1.0f; } };
	rove::a_star<nothing> blocked_start(pt(0, 0), pt(1, 0), nothing());
	blocked_start.calculate_path();
	BOOST_TEST(blocked_start.have_failed());
	BOOST_TEST(blocked_start.build_path().empty());
	BOOST_TEST(blocked_start.build_cost_map(4, 4)[0] == -1.0f);
	BOOST_CHECK_THROW(rove::a_star<uniform_scorer>(pt(0, 0), pt(1, 1), uniform_scorer(), 0.0f), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(a_cheaper_than_one_cell_is_fine_with_the_min_cost_given)
{
	struct cheap { rove::scalar operator()(pt const &q) const { return (q.x < 0 || q.y < 0 || q.x > 30 || q.y > 30) ? -1.0f : (q.y == 10 ? 0.1f : 1.0f); } };
	rove::a_star<cheap> exact(pt(0, 0), pt(30, 0), cheap(), 0.1f);
	exact.calculate_path();
	rove::a_star<cheap> flood(pt(0, 0), pt(30, 0), cheap());
	flood.calculate_cost_map();
	BOOST_TEST(exact.build_cost_map(31, 31)[30] == flood.build_cost_map(31, 31)[30], boost::test_tools::tolerance(2e-4f));
}

BOOST_AUTO_TEST_CASE(the_scorer_is_asked_once_a_cell_and_a_search_can_be_run_twice)
{
	struct counting {
		std::shared_ptr<std::map<std::pair<int,int>, int>> asked = std::make_shared<std::map<std::pair<int,int>, int>>();
		rove::scalar operator()(pt const &q) const { ++(*asked)[{q.x, q.y}]; return (q.x < 0 || q.y < 0 || q.x > 40 || q.y > 40) ? -1.0f : 1.0f; }
	};
	counting c;
	rove::a_star<counting> finder(pt(0, 0), pt(40, 40), c);
	finder.calculate_path();
	for (auto const &[cell, n] : *c.asked)
		BOOST_REQUIRE_MESSAGE(n == 1, "cell " << cell.first << "," << cell.second << " scored " << n << " times");
	std::vector<pt> const first = finder.build_path();
	finder.calculate_path();
	BOOST_TEST((finder.build_path() == first));
}

BOOST_AUTO_TEST_CASE(a_goal_straight_ahead_is_not_found_by_flooding_the_grid)
{
	struct open_field { rove::scalar operator()(pt const &q) const { return (std::abs(q.x) > 2000 || std::abs(q.y) > 2000) ? -1.0f : 1.0f; } };
	rove::a_star<open_field> finder(pt(0, 0), pt(400, 300), open_field());
	finder.calculate_path();
	BOOST_REQUIRE(!finder.have_failed());
	BOOST_TEST(finder.expanded() < 5000u);          // the heuristic steers it: a flood would take 640000 cells
	BOOST_TEST(finder.build_path().size() == 401u);
}

BOOST_AUTO_TEST_CASE(a_big_search_is_fast_enough_to_run_once_a_second_with_room_to_spare)
{
	random_grid g = make_grid(800, 800, 5, 0, 1);
	for (int i = 100; i < 700; ++i)      // a long wall with a gap at the far end
		g.cost[size_t(400) * 800 + size_t(i)] = -1;
	auto const t0 = std::chrono::steady_clock::now();
	rove::a_star<random_grid> finder(pt(10, 10), pt(790, 790), g);
	finder.calculate_path();
	double const ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
	BOOST_REQUIRE(!finder.have_failed());
	BOOST_TEST_MESSAGE("800 x 800 grid, path of " << finder.build_path().size() << " cells, " << finder.expanded() << " expanded: " << ms << " ms");
	BOOST_TEST(ms < 200.0);
}

BOOST_AUTO_TEST_CASE(a_search_confined_to_a_box_finds_what_the_unconfined_one_does_and_never_leaves_it)
{
	for (unsigned seed = 1; seed <= 40; ++seed)
	{
		random_grid g = make_grid(24, 24, seed, 28, 4);
		g.cost[0] = 1;
		g.cost[size_t(23) * 24 + 23] = 1;
		rove::a_star<random_grid> free_search(pt(0, 0), pt(23, 23), g), boxed(pt(0, 0), pt(23, 23), g);
		boxed.restrict_to(pt(0, 0), pt(24, 24));
		free_search.calculate_path();
		boxed.calculate_path();
		BOOST_REQUIRE(boxed.have_failed() == free_search.have_failed());
		BOOST_REQUIRE((boxed.build_path() == free_search.build_path()));      // the same search, only quicker to look cells up
		rove::a_star<random_grid> f1(pt(0, 0), pt(23, 23), g), f2(pt(0, 0), pt(23, 23), g);
		f2.restrict_to(pt(0, 0), pt(24, 24));
		f1.calculate_cost_map();
		f2.calculate_cost_map();
		BOOST_REQUIRE((f1.build_cost_map(24, 24) == f2.build_cost_map(24, 24)));
	}
	// the box bounds an open field the scorer does not
	rove::a_star<uniform_scorer> field(pt(0, 0), pt(100, 100), uniform_scorer());
	field.restrict_to(pt(0, 0), pt(50, 50));
	field.calculate_path();
	BOOST_TEST(field.have_failed());          // the goal is outside
	rove::a_star<uniform_scorer> around(pt(10, 10), pt(10, 11), uniform_scorer());
	around.restrict_to(pt(10, 10), pt(11, 12));   // a box of two cells
	around.calculate_path();
	BOOST_TEST(around.build_path().size() == 2u);
	rove::a_star<uniform_scorer> outside_start(pt(-5, 0), pt(3, 3), uniform_scorer());
	outside_start.restrict_to(pt(0, 0), pt(10, 10));
	outside_start.calculate_path();
	BOOST_TEST(outside_start.have_failed());
	BOOST_CHECK_THROW(around.restrict_to(pt(3, 3), pt(3, 9)), std::invalid_argument);
}

// build_cost_map() of a confined search is its box, row by row from the box's
// corner: what an unconfined flood over a grid walled off outside the box
// holds in the same cells.
BOOST_AUTO_TEST_CASE(the_cost_map_of_a_confined_search_is_its_box)
{
	pt const lo(5, 3), hi(21, 19);
	int const w = hi.x - lo.x, h = hi.y - lo.y;
	for (unsigned seed = 1; seed <= 20; ++seed)
	{
		random_grid g = make_grid(24, 24, seed, 20, 4);
		g.cost[size_t(10) * 24 + 10] = 1;
		random_grid walled = g;
		for (int y = 0; y < 24; ++y)
			for (int x = 0; x < 24; ++x)
				if (x < lo.x || x >= hi.x || y < lo.y || y >= hi.y)
					walled.cost[size_t(y) * 24 + size_t(x)] = -1;
		rove::a_star<random_grid> boxed(pt(10, 10), pt(0, 0), g), reference(pt(10, 10), pt(0, 0), walled);
		boxed.restrict_to(lo, hi);
		boxed.calculate_cost_map();
		reference.calculate_cost_map();
		std::vector<rove::scalar> const got = boxed.build_cost_map(), all = reference.build_cost_map(24, 24);
		BOOST_REQUIRE_EQUAL(got.size(), size_t(w) * size_t(h));
		int reached = 0;
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				rove::scalar const expected = all[size_t(y + lo.y) * 24 + size_t(x + lo.x)];
				BOOST_TEST_CONTEXT("seed " << seed << " cell " << x + lo.x << ", " << y + lo.y)
					BOOST_TEST(got[size_t(y) * size_t(w) + size_t(x)] == expected);
				reached += expected >= 0;
			}
		BOOST_TEST(reached > 50);
	}
	// an unconfined search has no box to give
	rove::a_star<random_grid> free_search(pt(0, 0), pt(3, 3), make_grid(8, 8, 1, 0, 1));
	free_search.calculate_cost_map();
	BOOST_CHECK_THROW(free_search.build_cost_map(), std::logic_error);
}

BOOST_AUTO_TEST_SUITE_END()
