// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file bind_text.h
 * @brief What a Python binding's __repr__ writes with: numbers in the fewest digits that read back as the same number
 */

#pragma once

#include <charconv>
#include <cstddef>
#include <string>
#include <type_traits>

#include <rove/vec.h>

namespace bind_text
{

class repr_stream {
public:
	repr_stream &operator<<(char const *text) { out_ += text; return *this; }
	repr_stream &operator<<(std::string const &text) { out_ += text; return *this; }

	template<class T> requires std::is_arithmetic_v<T>
	repr_stream &operator<<(T value)
	{
		char buffer[64];
		auto const end = std::to_chars(buffer, buffer + sizeof buffer, value).ptr;
		out_.append(buffer, end);
		return *this;
	}

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
	std::string out_;
};

}
