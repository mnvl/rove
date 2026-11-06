// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_random.cc
 * @brief Python bindings for random.h, hash.h and noise.h: the same numbers as rove's C++ from the same seed
 */

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <cstdint>
#include <numeric>
#include <vector>

#include <string>

#include <nanobind/stl/string.h>

#include <rove/hash.h>
#include <rove/noise.h>
#include <rove/random.h>

namespace nb = nanobind;
using namespace nb::literals;

void bind_random(nb::module_ &m) {
	using rove::pcg32;

	nb::class_<pcg32>(m, "pcg32",
		"O'Neill's PCG32: the same numbers on every machine, and its state is its value (a copy is a rollback). The reals are "
		"doubles, as Python's floats; unit_vector() is a vec3 of floats, as rove's C++ draws one in float.")
		.def(nb::init<std::uint64_t, std::uint64_t>(), "seed"_a, "stream"_a = 0xda3e39cb94b95bdbull,
			"The generator of seed on stream: different streams give unrelated numbers from the same seed")
		.def("next", &pcg32::next, "The next 32 random bits")
		.def("below", &pcg32::below, "n"_a, "A whole number in [0, n), each as likely (no modulo's bias); ValueError for n of 0")
		.def("unit", &pcg32::unit<double>, "A number in [0, 1) of 53 random bits: the next number high, the one after low")
		.def("uniform", &pcg32::uniform<double>, "lo"_a, "hi"_a, "lo + (hi - lo) unit(); ValueError for lo above hi")
		.def("chance", &pcg32::chance<double>, "p"_a, "True with probability p: never for 0, always for 1")
		.def("normal", &pcg32::normal<double>, "mean"_a, "deviation"_a,
			"Normally distributed (Box-Muller); ValueError for a negative deviation")
		.def("unit_vector", &pcg32::unit_vector<float>, "A direction uniformly over the sphere")
		.def("shuffle", [](pcg32 &rng, nb::list items) {
			std::vector<std::size_t> order(nb::len(items));
			std::iota(order.begin(), order.end(), std::size_t(0));
			rng.shuffle(order.begin(), order.end());
			std::vector<nb::object> was;
			for (nb::handle item : items) {
				was.push_back(nb::borrow(item));
			}
			for (std::size_t k = 0; k < order.size(); ++k) {
				items[k] = was[order[k]];
			}
		}, "items"_a, "Puts the list in a random order in place (Fisher-Yates: the same order everywhere)")
		.def("weighted", [](pcg32 &rng, std::vector<double> const &weights) { return rng.weighted(weights.begin(), weights.end()); },
			"weights"_a, "An index into the weights, each as likely as its share; ValueError for a negative weight or a sum of none")
		.def("__getstate__", [](pcg32 const &rng) { return nb::make_tuple(rng.state(), rng.increment()); })
		.def("__setstate__", [](pcg32 &rng, nb::tuple t) {
			check_state(t, 2, "pcg32.__setstate__");
			new (&rng) pcg32(pcg32::from_state(nb::cast<std::uint64_t>(t[0]), nb::cast<std::uint64_t>(t[1])));
		})
		.def("__repr__", [](pcg32 const &rng) {
			return "pcg32(state=" + std::to_string(rng.state()) + ", increment=" + std::to_string(rng.increment()) + ")";
		});

	m.def("splitmix64", &rove::splitmix64, "x"_a, "Vigna's splitmix64 mix of 64 bits");
	m.def("hash_combine", &rove::hash_combine, "seed"_a, "value"_a, "A seed and a value mixed into one hash (order matters)");
	m.def("hash_to_unit", &rove::hash_to_unit<double>, "h"_a, "A hash into [0, 1), from its top 53 bits");

	m.def("value_noise", nb::overload_cast<double, std::uint64_t>(&rove::value_noise<double>), "x"_a, nb::kw_only(), "seed"_a,
		"Value noise in [-1, 1], smooth, the seed's own");
	m.def("value_noise", nb::overload_cast<double, double, std::uint64_t>(&rove::value_noise<double>), "x"_a, "y"_a, nb::kw_only(),
		"seed"_a);
	m.def("value_noise", nb::overload_cast<double, double, double, std::uint64_t>(&rove::value_noise<double>), "x"_a, "y"_a, "z"_a,
		nb::kw_only(), "seed"_a);
	m.def("fbm", &rove::fbm<double>, "x"_a, "y"_a, "octaves"_a, "lacunarity"_a, "gain"_a, "seed"_a,
		"Octaves of 2D value noise, each lacunarity times finer and gain times fainter; ValueError for no octaves.  Other Python threads "
		"run meanwhile.", nb::call_guard<nb::gil_scoped_release>());
}
