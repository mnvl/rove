// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <rove/scalar.h>

// How close the tests take a computed result to be to the one expected.
// The library's own tolerances are rove::scalar_traits; this is the tests'.
inline constexpr rove::scalar TOLERANCE = 1.0e-6f;
