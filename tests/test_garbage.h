// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#pragma once

#include <cstring>
#include <new>

// A T default-constructed in memory that held something else first, so a member the constructor leaves alone shows the
// garbage (0xa5 bytes: a float of about -2.9e-16, never zero) instead of the zero a fresh stack page happens to hold.
template<class T>
T constructed_over_garbage()
{
	alignas(T) unsigned char storage[sizeof(T)];
	std::memset(storage, 0xa5, sizeof storage);
	T *const made = new (storage) T;
	T const copy = *made;
	made->~T();
	return copy;
}
