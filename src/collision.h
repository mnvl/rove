// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <cassert>
#include <cmath>
#include <optional>
#include <stdexcept>
#include "scalar.h"
#include "vec.h"
#include "sphere.h"
#include "segment.h"
#include "aabb.h"
#include "obb.h"
#include "capsule.h"
#include "contact_info.h"

namespace rove
{

namespace detail
{

/// Keep the candidate where it comes first, as contact_info::worse_than ranks them: a penetration over a touch, the shallower
/// penetration, the earlier touch.
template<int N, class S>
void keep_first(std::optional<contact_info<N, S>> &best, std::optional<contact_info<N, S>> const &candidate)
{
	if (candidate && (!best || best->worse_than(*candidate)))
		best = candidate;
}

}

/// When a sphere moving by `move` over a step of unit time first touches a sphere standing still.
///
/// Overlapping by more than `skin` at the start is a penetration at time 0, `penetration_depth` how far; overlapping
/// by less, or just touching, is a touch at time 0.  `position` is the moving sphere's centre at a touch; on a penetration it is
/// the deepest point of the moving sphere inside the still one.  `normal` is the unit vector from the still sphere's centre to the
/// moving one.  Empty when they do not touch: `time` is not bounded above,
/// a touch further on than `move` comes back with a time past 1.
template<int N, class S>
[[nodiscard]] std::optional<contact_info<N, S>> first_contact(sphere<N, S> const &still, sphere<N, S> const &ball, vec<N, S> const &move,
	S skin = scalar_traits<S>::contact_skin)
{
	// |x1 - x2| = R1 + R2
	// x2(t) = x2 + v * t
	// |x1 - x2 - v*t| = R1 + R2
	// (x1 - x2 - v*t, x1 - x2 - v*t) = (R1 + R2)^2
	// (v, v) * t^2 - (v, x1 - x2) * 2 * t + (x1 - x2, x1 - x2) - (R1 + R2)^2 = 0
	// A = (v, v)
	// B = -2 * (v, x1 - x2)
	// C = (x1 - x2, x1 - x2) - (R1 + R2)^2

	S const R = still.radius + ball.radius;
	vec<N, S> const delta = still.centre - ball.centre;

	contact_info<N, S> ci{};
	ci.happened = true;

	if (square(R) >= delta.length_sq()) {
		S const depth = R - delta.length();

		if (depth > skin) {
			ci.penetrated = true;
			ci.penetration_depth = depth;
			ci.position = still.centre - rove::normalize_or_zero(delta) * (still.radius - depth);
			ci.normal = rove::normalize_or_zero(-delta);         // zero for spheres of one centre
			ci.time = 0;
			return ci;
		}

		// touching within the skin: the touch is now, whichever way the ball goes
		ci.time = 0;
		ci.position = ball.centre;
		ci.normal = rove::normalize_or_zero(ball.centre - still.centre);
		return ci;
	}

	S const A = move.length_sq();
	if (A < scalar_traits<S>::zero_length_sq) return std::nullopt;     // apart and not moving
	S const B = -2 * dot(move, delta);
	S const C = delta.length_sq() - square(R);

	S const discriminant = square(B) - 4 * A * C;
	if (discriminant < 0) return std::nullopt;

	if (abs(discriminant) < scalar_traits<S>::zero_length_sq) {
		ci.time = -B / (2 * A);

		if (ci.time < 0) return std::nullopt;
	} else {
		using std::sqrt;          // (for a T of one's own, like dfloat, its own sqrt, by argument lookup)
		S const discriminant_sqrt = sqrt(discriminant);
		S const t1 = (-B + discriminant_sqrt) / (2 * A);
		S const t2 = (-B - discriminant_sqrt) / (2 * A);

		if (t1 > 0) {
			if (t2 > 0) ci.time = std::min(t1, t2);
			else ci.time = t1;
		} else {
			if (t2 > 0) ci.time = t2;
			else return std::nullopt;
		}
	}

	ci.position = ball.centre + move * ci.time;
	ci.normal = rove::normalize_or_zero(ci.position - still.centre);
	return ci;
}

/// When a circle moving by `move` first touches a segment standing still: its side or either end.
///
/// Overlapping the segment by `skin` or more at the start is a penetration at time 0.  `position` is the point of the
/// side touched, or the circle's centre where it touches an end; `normal` points from the segment to the circle.
template<class S>
[[nodiscard]] std::optional<contact_info<2, S>> first_contact(segment<2, S> const &l, sphere<2, S> const &ball, vec<2, S> const &move,
	S skin = scalar_traits<S>::contact_skin)
{
	contact_info<2, S> ci{};
	vec<2, S> p1, p2;

	if (ball.query_intersection(l, p1, p2) == 2) {
		ci.position = (p1 + p2) / S(2);
		S const depth = ball.radius - (ball.centre - ci.position).length();

		if (depth >= skin) {
			ci.happened = ci.penetrated = true;
			ci.penetration_depth = depth;
			ci.normal = rove::normalize_or_zero(ball.centre - ci.position);
			ci.time = 0;
			return ci;
		}
	}

	vec<2, S> const l_dir = rove::normalize_or_zero(l.direction());      // zero for a segment that is a point
	vec<2, S> const l_norm(-l_dir.y, l_dir.x);
	S const l_vel = dot(move, l_norm);
	vec<2, S> const l_norm_v = l_norm * sign(l_vel);

	std::optional<contact_info<2, S>> best;

	ci.position = ball.centre + l_norm_v * ball.radius;
	if (ray<2, S>(ci.position, l_norm_v * abs(l_vel)).query_intersection(l.to_ray(), ci.time) && ci.time >= 0) {
		ci.normal = ((dot(l_norm, l_norm_v) > 0) ? -l_norm : l_norm);
		ci.position += move * ci.time;

		S const pos = dot(ci.position - l.A, l_dir);
		if (pos >= 0 && pos * pos <= l.length_sq()) {
			ci.happened = true;
			best = ci;
		}
	}

	detail::keep_first(best, first_contact(sphere<2, S>(l.A, 0), ball, move, skin));
	detail::keep_first(best, first_contact(sphere<2, S>(l.B, 0), ball, move, skin));
	return best;
}

/// When a circle moving by `move` first touches a box standing still: the first of its sides it touches.
///
/// With `not_from_inside`, a circle whose centre starts inside the box penetrates it at time 0, as deep as the side
/// it is furthest past.  Empty when the circle does not move.
template<class S>
[[nodiscard]] std::optional<contact_info<2, S>> first_contact(obb<2, S> const &o, sphere<2, S> const &ball, vec<2, S> const &move,
	bool not_from_inside = true, S skin = scalar_traits<S>::contact_skin)
{
	if (move.length_sq() < scalar_traits<S>::zero_length_sq) return std::nullopt;

	std::optional<contact_info<2, S>> best;

	if (not_from_inside && o.contains(ball.centre)) {
		for (size_t i = 0; i < obb<2, S>::sides_count; i++) {
			vec<2, S> const vertex = o.vertex(i), normal = o.side_normal(i);
			S const depth = -dot(ball.centre - vertex, normal) + ball.radius;

			if (depth > 0) {
				contact_info<2, S> ci{};
				ci.penetrated = ci.happened = true;
				ci.penetration_depth = depth;
				ci.position = ball.centre + normal * depth;
				ci.normal = normal;
				ci.time = 0;
				detail::keep_first(best, std::optional<contact_info<2, S>>(ci));
			}
		}
	}

	for (size_t i = 0; i < obb<2, S>::sides_count; i++)
		detail::keep_first(best, first_contact(o.side(i), ball, move, skin));

	return best;
}

/// first_contact() with the box as an obb
template<class S>
[[nodiscard]] std::optional<contact_info<2, S>> first_contact(aabb<2, S> const &a, sphere<2, S> const &ball, vec<2, S> const &move,
	bool not_from_inside = true, S skin = scalar_traits<S>::contact_skin)
{
	return first_contact(obb<2, S>(a), ball, move, not_from_inside, skin);
}

/// When a sphere and a capsule, both moving through a step of unit time,
/// first touch.  The sphere's centre goes from `ball.centre` to
/// `ball.centre + move`; the capsule's two ends each go from where they are
/// in `from` to where they are in `to`, and its radius from one to the
/// other, all linearly, so a capsule may turn and stretch on its way (a
/// limb swung between two frames of an animation).
///
/// Conservative advancement: the gap between the surfaces changes no faster
/// than the sphere moves plus the faster end of the capsule plus its
/// growth, so a step of gap / that rate can never pass a touch.  The touch
/// is reported at the first time the gap is at most `tolerance`, never
/// after the surfaces meet; the loop stops after at most rate / tolerance
/// steps.
///
/// The result has `time` in [0, 1], `normal` the unit vector from the
/// capsule's axis to the sphere's centre (zero if the centre is on the
/// axis, which only a deep overlap at the start can give), `position` the
/// point of the capsule's surface along it; `penetrated` and
/// `penetration_depth` say how far they overlap if they already do at the
/// start.  Empty when they do not touch within the step.
/// std::invalid_argument for a tolerance that is not positive, a negative
/// radius or anything not finite.
template<int N, class T>
[[nodiscard]] std::optional<contact_info<N, T>> first_contact(capsule<N, T> const &from, capsule<N, T> const &to,
	sphere<N, T> const &ball, vec<N, T> const &move, T tolerance)
{
	using vec_type = vec<N, T>;
	if (!(tolerance > 0))
		throw std::invalid_argument("first_contact: the tolerance must be positive");
	if (!(from.radius >= 0 && to.radius >= 0 && ball.radius >= 0))
		throw std::invalid_argument("first_contact: a radius is negative or not a number");
	for (vec_type const *v : {&from.axe.A, &from.axe.B, &to.axe.A, &to.axe.B, &ball.centre, &move})
		v->check_finite("first_contact: a point or a move");

	T const rate = move.length() + std::max((to.axe.A - from.axe.A).length(), (to.axe.B - from.axe.B).length()) +
		abs(to.radius - from.radius);
	T t = 0;
	while (true)
	{
		segment<N, T> const axis(lerp(from.axe.A, to.axe.A, t), lerp(from.axe.B, to.axe.B, t));
		vec_type const centre = ball.centre + move * t;
		vec_type const nearest = axis.closest_point(centre);
		T const radius = lerp(from.radius, to.radius, t);
		T const apart = (centre - nearest).length();
		T const gap = apart - radius - ball.radius;
		if (gap <= tolerance)
		{
			contact_info<N, T> ci{};
			ci.happened = true;
			ci.time = t;
			ci.penetrated = t == 0 && gap < 0;
			ci.penetration_depth = ci.penetrated ? -gap : T(0);
			ci.normal = apart > 0 ? (centre - nearest) / apart : vec_type();
			ci.position = nearest + ci.normal * radius;
			return ci;
		}
		if (rate == 0)
			return std::nullopt;
		T const next_t = t + gap / rate;
		// The gap left is below what time can tell at this t: nothing nearer can be found, so this is the touch.
		if (!(next_t > t))
		{
			contact_info<N, T> ci{};
			ci.happened = true;
			ci.time = t;
			ci.normal = apart > 0 ? (centre - nearest) / apart : vec_type();
			ci.position = nearest + ci.normal * radius;
			return ci;
		}
		t = next_t;
		if (t > 1)
			return std::nullopt;
	}
}

}

