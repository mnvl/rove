// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include "scalar.h"

namespace rove
{

template<class T = scalar>
class interval
{
public:

	T low, high;

	/// [0, 0] (it was left uninitialised)
	interval():
		low(0),
		high(0)
	{
	}

	interval(T l, T h):
		low(l),
		high(h)
	{
	}

	T length() const { return high - low; }
};

}
