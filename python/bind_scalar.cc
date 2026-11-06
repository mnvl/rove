// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_scalar.cc
 * @brief Python bindings for scalar.h: angles, wraps, remaps, and the smoothing steps (also on vectors)
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/pair.h>
#include <utility>

#include <rove/scalar.h>
#include <rove/vec.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace {

// damp, move_towards and critically_damped of vectors of one type; critically_damped returns (value, velocity)
template<class V, class T>
void bind_steps(nb::module_ &m) {
	m.def("damp", [](V const &current, V const &target, T rate, T dt) { return rove::damp(current, target, rate, dt); },
		"current"_a, "target"_a, "rate"_a, "dt"_a);
	m.def("move_towards", [](V const &current, V const &target, T max_delta) {
		return rove::move_towards(current, target, max_delta);
	}, "current"_a, "target"_a, "max_delta"_a);
	m.def("critically_damped", [](V value, V velocity, V const &target, T omega, T dt) {
		rove::critically_damped(value, velocity, target, omega, dt);
		return std::make_pair(value, velocity);
	}, "value"_a, "velocity"_a, "target"_a, "omega"_a, "dt"_a);
}

}

void bind_scalar_functions(nb::module_ &m) {
	m.def("radians", &rove::radians<double>, "degrees"_a, "Degrees to radians: 180 is exactly pi");
	m.def("degrees", &rove::degrees<double>, "radians"_a, "Radians to degrees");
	m.def("fract", &rove::fract<double>, "x"_a, "What floor leaves, in [0, 1): fract(-0.25) is 0.75");
	m.def("lerp", &rove::lerp<double>, "a"_a, "b"_a, "t"_a, "a + (b - a) t, unclamped");
	m.def("inverse_lerp", &rove::inverse_lerp<double>, "a"_a, "b"_a, "x"_a,
		"Where x is from a (0) to b (1), unclamped; ValueError for a == b");
	m.def("remap", &rove::remap<double>, "x"_a, "a0"_a, "a1"_a, "b0"_a, "b1"_a,
		"x moved from [a0, a1] to [b0, b1], unclamped; ValueError for a0 == a1");
	m.def("smoothstep", &rove::smoothstep<double>, "edge0"_a, "edge1"_a, "x"_a,
		"GLSL's smoothstep; ValueError for equal edges");
	m.def("wrap", &rove::wrap<double>, "x"_a, "lo"_a, "hi"_a, "x into [lo, hi), which must hold zero (ValueError otherwise)");
	m.def("wrap_to_pi", &rove::wrap_to_pi<double>, "a"_a, "An angle into (-pi, pi]");
	m.def("wrap_to_two_pi", &rove::wrap_to_two_pi<double>, "a"_a, "An angle into [0, 2 pi)");
	m.def("delta_angle", &rove::delta_angle<double>, "from_"_a, "to"_a,
		"The turn (-pi, pi] from one angle to another, the short way round");

	// the steps: numbers first (Python's floats are doubles), then vectors of each type
	m.def("damp", &rove::damp<double, double>, "current"_a, "target"_a, "rate"_a, "dt"_a,
		"current moved towards target by the share 1 - exp(-rate dt): however dt is cut up, the same place; never past the target. "
		"ValueError for a negative rate or dt. For numbers and vectors.");
	m.def("move_towards", &rove::move_towards<double>, "current"_a, "target"_a, "max_delta"_a,
		"current moved towards target by max_delta at most, landing on it exactly; ValueError for a negative max_delta. "
		"For numbers and vectors.");
	m.def("critically_damped", [](double value, double velocity, double target, double omega, double dt) {
		rove::critically_damped(value, velocity, target, omega, dt);
		return std::make_pair(value, velocity);
	}, "value"_a, "velocity"_a, "target"_a, "omega"_a, "dt"_a,
		"One step of dt of a critically damped spring, its exact solution: (value, velocity) after it. "
		"ValueError for a negative omega or dt. For numbers and vectors.");
	bind_steps<rove::vec<2, float>, float>(m);
	bind_steps<rove::vec<3, float>, float>(m);
	bind_steps<rove::vec<2, double>, double>(m);
	bind_steps<rove::vec<3, double>, double>(m);
}
