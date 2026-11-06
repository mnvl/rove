// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <tuple>
#include <type_traits>
#include <rove/segment.h>

namespace
{

using scalars = std::tuple<float, double>;

static_assert(std::is_same_v<rove::segment<3, double>, rove::segment<3, double>>);
static_assert(std::is_same_v<rove::segment<2>, rove::segment<2, rove::scalar>>);

template<class T> auto tight() { return boost::test_tools::tolerance(T(1e-5)); }

}

BOOST_AUTO_TEST_SUITE(test_line)

BOOST_AUTO_TEST_CASE_TEMPLATE(degenerate_segment_measures_to_its_point, T, scalars)
{
	rove::vec<3, T> const a(3, 7, -2);
	rove::segment<3, T> const s(a, a);
	rove::vec<3, T> const p(10, 10, 10);

	BOOST_TEST(std::isfinite(s.distance_sq(p)));
	BOOST_TEST(s.distance(p) == (p - a).length(), tight<T>());
	BOOST_TEST(s.distance(a) == T(0));
	BOOST_TEST(s.parameter(p) == T(0));
	BOOST_TEST((s.closest_point(p) == a));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(point_beyond_A_measures_to_A, T, scalars)
{
	rove::vec<3, T> const a(2, 0, 0), b(8, 0, 0);
	rove::segment<3, T> const s(a, b);
	rove::vec<3, T> const p(-5, 3, 0);

	BOOST_TEST(s.parameter(p) == T(0));
	BOOST_TEST((s.closest_point(p) == a));
	BOOST_TEST(s.distance(p) == std::sqrt(T(58)), tight<T>());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(point_beyond_B_measures_to_B, T, scalars)
{
	rove::vec<3, T> const a(2, 0, 0), b(8, 0, 0);
	rove::segment<3, T> const s(a, b);
	rove::vec<3, T> const p(12, 0, 4);

	BOOST_TEST(s.parameter(p) == T(1));
	BOOST_TEST((s.closest_point(p) == b));
	BOOST_TEST(s.distance(p) == std::sqrt(T(32)), tight<T>());
}

// Coordinates with no exact binary form: the endpoints must still come back
// exactly, not as A + (B - A) with its rounding.
BOOST_AUTO_TEST_CASE_TEMPLATE(endpoints_are_on_the_segment_exactly, T, scalars)
{
	rove::vec<3, T> const a(T(0.1), T(0.7), T(-3.3)), b(T(5.9), T(-2.2), T(1.3));
	rove::segment<3, T> const s(a, b);

	BOOST_TEST(s.parameter(a) == T(0));
	BOOST_TEST(s.parameter(b) == T(1));
	BOOST_TEST((s.point(0) == a));
	BOOST_TEST((s.point(1) == b));
	BOOST_TEST((s.closest_point(a) == a));
	BOOST_TEST((s.closest_point(b) == b));
	BOOST_TEST(s.distance(a) == T(0));
	BOOST_TEST(s.distance(b) == T(0));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(perpendicular_offset_is_the_distance, T, scalars)
{
	rove::segment<3, T> const s(rove::vec<3, T>(0, 0, 0), rove::vec<3, T>(10, 0, 0));
	rove::vec<3, T> const p(4, 3, 0);

	BOOST_TEST(s.parameter(p) == T(0.4), tight<T>());
	BOOST_TEST(s.distance(p) == T(3), tight<T>());
	BOOST_TEST(s.distance_sq(p) == T(9), tight<T>());
	BOOST_TEST((s.closest_point(p) - rove::vec<3, T>(4, 0, 0)).length() < T(1e-5));
}

// A skew segment in 3D: p is built as A + d t0 + n h with n perpendicular to
// d, so the answer is known without repeating the projection.
BOOST_AUTO_TEST_CASE_TEMPLATE(skew_segment_recovers_parameter_and_offset, T, scalars)
{
	rove::vec<3, T> const a(1, -2, 3), d(4, 1, -2);
	rove::vec<3, T> n = cross(d, rove::vec<3, T>(0, 1, 0));
	n.normalize();
	rove::segment<3, T> const s(a, a + d);

	for (T const t0 : {T(0.05), T(0.3), T(0.5), T(0.95)})
		for (T const h : {T(0), T(0.25), T(7)})
		{
			rove::vec<3, T> const p = a + d * t0 + n * h;
			BOOST_TEST(s.parameter(p) == t0, boost::test_tools::tolerance(T(1e-4)));
			BOOST_TEST(s.distance(p) == h, boost::test_tools::tolerance(T(1e-4)));
		}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(parameter_is_monotonic_and_clamped, T, scalars)
{
	rove::vec<2, T> const a(-3, 1), b(5, 7);
	rove::segment<2, T> const s(a, b);
	rove::vec<2, T> const side(T(-0.6), T(0.8)); // perpendicular to b - a

	T previous = -1;
	for (int k = -10; k <= 30; ++k)
	{
		T const t0 = T(k) / 20;
		T const t = s.parameter(a + (b - a) * t0 + side * T(2));
		BOOST_TEST(t >= previous);
		BOOST_TEST(t >= T(0));
		BOOST_TEST(t <= T(1));
		if (k >= 0 && k <= 20)
			BOOST_TEST(t == t0, boost::test_tools::tolerance(T(1e-5)));
		previous = t;
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(reversed_segment_mirrors_the_parameter, T, scalars)
{
	rove::vec<3, T> const a(1, 2, 3), b(-4, 0, 5), p(0, 5, 1);
	rove::segment<3, T> const ab(a, b), ba(b, a);

	BOOST_TEST(ba.parameter(p) == T(1) - ab.parameter(p), tight<T>());
	BOOST_TEST(ba.distance(p) == ab.distance(p), tight<T>());
	BOOST_TEST(ab.distance(p) * ab.distance(p) == ab.distance_sq(p), tight<T>());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(point_interpolates_between_the_ends, T, scalars)
{
	rove::segment<2, T> const s(rove::vec<2, T>(2, -4), rove::vec<2, T>(6, 8));

	BOOST_TEST((s.point(T(0.5)) == rove::vec<2, T>(4, 2)));
	BOOST_TEST((s.point(T(0.25)) == rove::vec<2, T>(3, -1)));
}

// A NaN in the point is a bug upstream; the answer must say so, not clamp it
// into a plausible parameter.
BOOST_AUTO_TEST_CASE_TEMPLATE(nan_point_gives_nan, T, scalars)
{
	rove::segment<3, T> const s(rove::vec<3, T>(0, 0, 0), rove::vec<3, T>(1, 0, 0));
	rove::vec<3, T> const p(std::numeric_limits<T>::quiet_NaN(), 0, 0);

	BOOST_TEST(std::isnan(s.parameter(p)));
	BOOST_TEST(std::isnan(s.distance(p)));
}

// closest_points between two segments: the pair of points, one on each, that
// are nearest.  Checked against known answers and against a dense sampling of
// both parameters, which the answer can never beat and must come within the
// sampling step of.

namespace
{

template<class T>
bool on_segment(rove::segment<3, T> const &s, rove::vec<3, T> const &p, T tolerance)
{
	return s.distance(p) <= tolerance;
}

template<class T>
T sampled_min_sq(rove::segment<3, T> const &a, rove::segment<3, T> const &b, int n)
{
	T best = std::numeric_limits<T>::max();
	for (int i = 0; i <= n; ++i)
		for (int j = 0; j <= n; ++j)
			best = std::min(best, (a.point(T(i) / n) - b.point(T(j) / n)).length_sq());
	return best;
}

}

BOOST_AUTO_TEST_CASE_TEMPLATE(crossing_skew_segments_meet_at_the_common_perpendicular, T, scalars)
{
	rove::segment<3, T> const a(rove::vec<3, T>(-1, 0, 0), rove::vec<3, T>(1, 0, 0));
	rove::segment<3, T> const b(rove::vec<3, T>(0, -1, 2), rove::vec<3, T>(0, 1, 2));
	rove::vec<3, T> ca, cb;

	BOOST_TEST(a.closest_points(b, ca, cb) == T(4), tight<T>());
	BOOST_TEST((ca - rove::vec<3, T>(0, 0, 0)).length() < T(1e-5));
	BOOST_TEST((cb - rove::vec<3, T>(0, 0, 2)).length() < T(1e-5));
}

// The infinite lines meet at a point beyond both ends: the answer is the
// nearest ends, not the lines' common perpendicular.
BOOST_AUTO_TEST_CASE_TEMPLATE(lines_meeting_beyond_the_ends_measure_end_to_end, T, scalars)
{
	rove::segment<3, T> const a(rove::vec<3, T>(2, 0, 0), rove::vec<3, T>(5, 0, 0));
	rove::segment<3, T> const b(rove::vec<3, T>(0, 3, 0), rove::vec<3, T>(0, 7, 0));
	rove::vec<3, T> ca, cb;

	BOOST_TEST(a.closest_points(b, ca, cb) == T(13), tight<T>());
	BOOST_TEST((ca == a.A));
	BOOST_TEST((cb == b.A));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(parallel_overlapping_segments_measure_their_offset, T, scalars)
{
	rove::segment<3, T> const a(rove::vec<3, T>(0, 0, 0), rove::vec<3, T>(4, 0, 0));
	rove::segment<3, T> const b(rove::vec<3, T>(1, 3, 0), rove::vec<3, T>(6, 3, 0));
	rove::vec<3, T> ca, cb;

	T const d = a.closest_points(b, ca, cb);
	BOOST_TEST(d == T(9), tight<T>());
	BOOST_TEST((ca - cb).length_sq() == d, tight<T>());
	BOOST_TEST(on_segment(a, ca, T(1e-5)));
	BOOST_TEST(on_segment(b, cb, T(1e-5)));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(parallel_disjoint_segments_measure_gap_to_gap, T, scalars)
{
	rove::segment<3, T> const a(rove::vec<3, T>(0, 0, 0), rove::vec<3, T>(4, 0, 0));
	rove::segment<3, T> const b(rove::vec<3, T>(7, 4, 0), rove::vec<3, T>(9, 4, 0));
	rove::vec<3, T> ca, cb;

	BOOST_TEST(a.closest_points(b, ca, cb) == T(25), tight<T>());
	BOOST_TEST((ca == a.B));
	BOOST_TEST((cb == b.A));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(a_point_segment_measures_as_a_point, T, scalars)
{
	rove::segment<3, T> const s(rove::vec<3, T>(0, 0, 0), rove::vec<3, T>(10, 0, 0));
	rove::vec<3, T> const p(4, 3, 1);
	rove::segment<3, T> const point(p, p);
	rove::vec<3, T> c1, c2;

	BOOST_TEST(s.closest_points(point, c1, c2) == s.distance_sq(p), tight<T>());
	BOOST_TEST((c2 == p));
	BOOST_TEST(point.closest_points(s, c1, c2) == s.distance_sq(p), tight<T>());
	BOOST_TEST((c1 == p));

	rove::vec<3, T> const q(-1, 2, 2);
	BOOST_TEST(point.closest_points(rove::segment<3, T>(q, q), c1, c2) == (p - q).length_sq(), tight<T>());
	BOOST_TEST((c1 == p));
	BOOST_TEST((c2 == q));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(closest_points_match_a_dense_sampling, T, scalars)
{
	std::mt19937 rng(20261001);
	std::uniform_real_distribution<T> u(-5, 5);
	auto v = [&] { return rove::vec<3, T>{u(rng), u(rng), u(rng)}; };
	int const n = 300;
	for (int k = 0; k < 200; ++k)
	{
		rove::segment<3, T> a(v(), v()), b(v(), v());
		if (k % 4 == 1)
			b = rove::segment<3, T>(b.A, b.A + a.direction() * T(0.7));        // parallel
		if (k % 4 == 2)
			b = rove::segment<3, T>(a.point(T(0.3)) + v() * T(0.01), b.B);       // nearly touching
		rove::vec<3, T> ca, cb;
		T const d = a.closest_points(b, ca, cb);
		T const sampled = sampled_min_sq(a, b, n);
		// a sample is within half a step of the true pair on each segment
		T const slack = std::sqrt(sampled) * (a.direction().length() + b.direction().length()) / n
			+ (a.direction().length_sq() + b.direction().length_sq()) / (n * n);
		BOOST_TEST_CONTEXT("pair " << k)
		{
			BOOST_TEST(d <= sampled + T(1e-4));
			BOOST_TEST(d >= sampled - slack - T(1e-4));
			BOOST_TEST((ca - cb).length_sq() == d, boost::test_tools::tolerance(T(1e-3)));
			BOOST_TEST(on_segment(a, ca, T(1e-3)));
			BOOST_TEST(on_segment(b, cb, T(1e-3)));
			rove::vec<3, T> cb2, ca2;
			BOOST_TEST(b.closest_points(a, cb2, ca2) == d, boost::test_tools::tolerance(T(1e-3)));
		}
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(closest_points_work_in_two_dimensions, T, scalars)
{
	rove::segment<2, T> const a(rove::vec<2, T>(0, 0), rove::vec<2, T>(4, 4));
	rove::segment<2, T> const b(rove::vec<2, T>(0, 4), rove::vec<2, T>(4, 0));
	rove::vec<2, T> ca, cb;

	BOOST_TEST(a.closest_points(b, ca, cb) < T(1e-6));
	BOOST_TEST((ca - rove::vec<2, T>(2, 2)).length() < T(1e-5));
}

BOOST_AUTO_TEST_CASE(a_segment_contains_its_ends)
{
	rove::segment<3> const s(rove::vec<3>(1, 2, 3), rove::vec<3>(4, 6, 3));
	BOOST_TEST(s.contains(s.A));
	BOOST_TEST(s.contains(s.B));
	BOOST_TEST(s.contains(s.point(0.5f)));
	BOOST_TEST(!s.contains(s.point(1.5f)));
	BOOST_TEST(!s.contains(s.point(-0.5f)));
	BOOST_TEST(!s.contains(s.A + rove::vec<3>(0, 0, 1)));
	// a point segment contains its point
	rove::segment<3> const point(rove::vec<3>(1, 2, 3), rove::vec<3>(1, 2, 3));
	BOOST_TEST(point.contains(rove::vec<3>(1, 2, 3)));
	BOOST_TEST(!point.contains(rove::vec<3>(1, 2, 4)));
}

BOOST_AUTO_TEST_SUITE_END()
