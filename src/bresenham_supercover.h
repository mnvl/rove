// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <tuple>
#include "vec.h"

namespace rove
{

// based on http://lifc.univ-fcomte.fr/~ededu/projects/bresenham/
template<class T=int>
class bresenham_supercover {
public:
	using int_type = T;
	using vec_type = vec<2, int_type>;

	bresenham_supercover(vec_type const &v1, vec_type const &v2);
	~bresenham_supercover();

	std::tuple<vec_type, bool> get();

private:
	int_type i_;
	int_type x1_, y1_, x2_, y2_;
	int_type xstep_, ystep_;
	int_type error_, errorprev_;
	int_type x_, y_;
	int_type dx_, dy_, ddx_, ddy_;
	vec_type results_[3];
	size_t firstresult_, nresults_;
	bool finished_;

	void next();
};

}

