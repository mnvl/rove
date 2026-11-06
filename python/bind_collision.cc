// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_collision.cc
 * @brief Python bindings for the collision tests (collision.h)
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>

#include <rove/collision.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace
{

/// None when they do not touch, else a dict of the contact_info's fields.
template<int N, class T>
nb::object first_contact_py(rove::capsule<N, T> const &from, rove::capsule<N, T> const &to, rove::sphere<N, T> const &ball,
	rove::vec<N, T> const &move, T tolerance)
{
	auto const hit = rove::first_contact(from, to, ball, move, tolerance);
	if (!hit) return nb::none();
	nb::dict d;
	d["time"] = hit->time;
	d["position"] = hit->position;
	d["normal"] = hit->normal;
	d["penetrated"] = hit->penetrated;
	d["penetration_depth"] = hit->penetration_depth;
	return d;
}

char const *const FIRST_CONTACT_DOC =
	"When a sphere and a capsule, both moving linearly through a step of unit time, first touch: the sphere's centre "
	"moves by `move`, the capsule's ends (and radius) go from `start` to `end`.  None if they do not touch within the "
	"step, else a dict with time (in [0, 1]), position (on the capsule's surface), normal (from the capsule's axis to "
	"the sphere's centre), penetrated and penetration_depth (if they overlap at the start).  The touch is reported "
	"when the gap is at most `tolerance`, never after the surfaces meet.";

}

void bind_collision_functions(nb::module_ &m) {
	m.def("first_contact", &first_contact_py<3, float>, "start"_a, "end"_a, "ball"_a, "move"_a, "tolerance"_a, FIRST_CONTACT_DOC);
	m.def("first_contact", &first_contact_py<3, double>, "start"_a, "end"_a, "ball"_a, "move"_a, "tolerance"_a, FIRST_CONTACT_DOC);
	m.def("first_contact", &first_contact_py<2, float>, "start"_a, "end"_a, "ball"_a, "move"_a, "tolerance"_a, FIRST_CONTACT_DOC);
	m.def("first_contact", &first_contact_py<2, double>, "start"_a, "end"_a, "ball"_a, "move"_a, "tolerance"_a, FIRST_CONTACT_DOC);
}
