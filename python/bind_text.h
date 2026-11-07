// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_text.h
 * @brief What a Python binding's __repr__ writes with: numbers in the fewest digits that read back as the same number (with the GIL held)
 */

#pragma once

#include <nanobind/nanobind.h>

#include <cstddef>
#include <new>
#include <string>
#include <type_traits>

#include <rove/vec.h>

namespace bind_text
{

class repr_stream {
public:
	repr_stream &operator<<(char const *text) { out_ += text; return *this; }
	repr_stream &operator<<(std::string const &text) { out_ += text; return *this; }

	repr_stream &operator<<(double value) { return text(number(value, 0, 'r')); }

	repr_stream &operator<<(float value)
	{
		// the fewest digits that read back as this float: 0.1, where the double it widens to would say 0.10000000149011612
		for (int digits = 1; digits < 9; ++digits) {
			std::string const candidate = number(value, digits, 'g');
			if (float(PyOS_string_to_double(candidate.c_str(), nullptr, nullptr)) == value) return text(candidate);
		}
		return text(number(value, 9, 'g'));
	}

	template<class T> requires std::is_integral_v<T>
	repr_stream &operator<<(T value) { return text(std::to_string(value)); }

	template<int N, class T, class S>
	repr_stream &operator<<(rove::vec<N, T, S> const &v)
	{
		*this << "(";
		for (int n = 0; n < N; ++n) {
			if (n) *this << ", ";
			*this << v.i[n];
		}
		return *this << ")";
	}

	std::string const &str() const { return out_; }

private:
	/// By Python's own formatting, which is the same on every platform and in every locale (std::to_chars of a float is macOS 13.3 on,
	/// and wheels go back further).  It needs the GIL.
	static std::string number(double value, int digits, char format)
	{
		char *const made = PyOS_double_to_string(value, format, digits, 0, nullptr);
		if (!made) throw std::bad_alloc();
		std::string result(made);
		PyMem_Free(made);
		return result;
	}

	repr_stream &text(std::string const &part)
	{
		out_ += part;
		return *this;
	}

	std::string out_;
};

}
