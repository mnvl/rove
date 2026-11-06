// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include "python_bindings.h"
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/vector.h>
#include <memory>
#include <cstring>

#include <rove/a_star.h>

namespace nb = nanobind;
using namespace nb::literals;

struct grid_scorer {
	std::shared_ptr<float[]> data;
	int width, height;

	rove::scalar operator()(rove::vec<2,int> const &p) const {
		if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) return -1;
		return data[p.y * width + p.x];
	}
};

using grid_a_star = rove::a_star<grid_scorer>;

void bind_a_star(nb::module_ &m) {
	nb::class_<grid_a_star>(m, "a_star")
		.def("__init__", [](grid_a_star *self, int start_x, int start_y, int goal_x, int goal_y,
						 nb::ndarray<float, nb::ndim<2>, nb::c_contig, nb::device::cpu> weights) {
			int h = int(weights.shape(0));
			int w = int(weights.shape(1));

			std::shared_ptr<float[]> data(new float[w * h]);
			std::memcpy(data.get(), (float const *)weights.data(), w * h * sizeof(float));

			grid_scorer scorer{data, w, h};
			new (self) grid_a_star(
				rove::vec<2,int>(start_x, start_y),
				rove::vec<2,int>(goal_x, goal_y),
				scorer
			);
		},
		nb::arg("start_x"), nb::arg("start_y"),
		nb::arg("goal_x"), nb::arg("goal_y"),
		nb::arg("weights"))

		.def("calculate_path", &grid_a_star::calculate_path)
		.def("calculate_cost_map", &grid_a_star::calculate_cost_map)
		.def("have_failed", &grid_a_star::have_failed)

		.def("build_path", [](grid_a_star const &self) {
			auto path = self.build_path();
			nb::list result;
			for (auto const &p : path)
				result.append(nb::make_tuple(p.x, p.y));
			return result;
		})

		.def("build_cost_map", [](grid_a_star const &self) {
			int w = self.scorer().width;
			int h = self.scorer().height;
			auto cost_map = self.build_cost_map(w, h);

			float *out = new float[w * h];
			std::memcpy(out, cost_map.data(), w * h * sizeof(float));

			size_t shape[2] = {size_t(h), size_t(w)};
			nb::capsule deleter(out, [](void *p) noexcept { delete[] static_cast<float*>(p); });
			return nb::ndarray<nb::numpy, float, nb::ndim<2>>(out, 2, shape, deleter);
		});
}
