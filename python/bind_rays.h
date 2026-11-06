// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_rays.h
 * @brief What the batched ray bindings share: rows of (N, 3) arrays in, a unit ray of each, numpy arrays out
 */

#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>

#include <rove/ray.h>
#include <rove/vec.h>

namespace bind_rays
{

namespace nb = nanobind;

/// Rows of three T in C order, taken as they are: a batch converts nothing (a float64 array for float32 rays is a TypeError,
/// not a silent rounding; bind them with .noconvert())
template<class T>
using rows3 = nb::ndarray<T const, nb::shape<-1, 3>, nb::c_contig, nb::device::cpu>;

/// The numpy array that owns `data`
template<class V>
nb::ndarray<nb::numpy, V> owned(std::unique_ptr<V[]> data, std::initializer_list<size_t> shape)
{
	V *const raw = data.release();
	nb::capsule deleter(raw, [](void *p) noexcept { delete[] static_cast<V *>(p); });
	return nb::ndarray<nb::numpy, V>(raw, shape.size(), shape.begin(), deleter);
}

/**
 * @brief Calls trace(i, r) with the unit ray r of each row, origins[i] towards directions[i], in C++ alone (other Python threads
 *        run meanwhile; none may change the arrays or what is traced)
 *
 * std::invalid_argument for origins and directions of different lengths, a max_distance negative or NaN, and a row that is no
 * ray (NaN, infinite, a direction of no length), which it names.  `lock`, where given, is held for reading over the rays.
 */
template<class T, class F>
void each_ray(char const *what, rows3<T> const &origins, rows3<T> const &directions, T max_distance, F &&trace,
	std::shared_mutex *lock = nullptr)
{
	if (origins.shape(0) != directions.shape(0))
		throw std::invalid_argument(std::string(what) + ": origins and directions must have the same number of rows (" +
			std::to_string(origins.shape(0)) + " and " + std::to_string(directions.shape(0)) + ")");
	if (!(max_distance >= 0))
		throw std::invalid_argument(std::string(what) + ": a max_distance of " + std::to_string(max_distance));
	size_t const n = origins.shape(0);
	T const *const o = origins.data(), *const d = directions.data();
	nb::gil_scoped_release release;
	// (taken without the GIL: a thread that holds the lock may be waiting for nothing else but its turn to finish)
	std::shared_lock<std::shared_mutex> held;
	if (lock) held = std::shared_lock<std::shared_mutex>(*lock);
	for (size_t i = 0; i < n; ++i) {
		rove::ray<3, T> r;
		try {
			r = rove::ray<3, T>::unit(rove::vec<3, T>(o[3 * i], o[3 * i + 1], o[3 * i + 2]),
				rove::vec<3, T>(d[3 * i], d[3 * i + 1], d[3 * i + 2]));
		} catch (std::invalid_argument const &e) {
			throw std::invalid_argument(std::string(what) + ": ray " + std::to_string(i) + ": " + e.what());
		}
		trace(i, r);
	}
}

}
