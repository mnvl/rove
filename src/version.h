// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/// @file version.h
/// @brief rove's version, at compile time: what project(rove VERSION ...) says (a test holds the two together)

#pragma once

#define ROVE_VERSION_MAJOR 3
#define ROVE_VERSION_MINOR 0
#define ROVE_VERSION_PATCH 1
/// e.g. 30000 for 3.0.0, to compare with #if
#define ROVE_VERSION (ROVE_VERSION_MAJOR * 10000 + ROVE_VERSION_MINOR * 100 + ROVE_VERSION_PATCH)
