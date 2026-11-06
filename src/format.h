// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

/**
 * @file format.h
 * @brief std::format for rove's types: std::format("{:.3f}", v)
 *
 * A vector or a quaternion is its components separated by spaces, as operator<< writes a vector; an AABB is "lo x hi", as
 * operator<< writes it; a matrix is its rows, as written on paper (m[0][r] m[1][r] ...), separated by "; ".  The format spec is the
 * scalar's, and applies to each component: "{:8.3f}" pads every one.  Not included by the other headers, so that only who wants
 * it pays for <format>.
 */

#pragma once

#include <cstddef>
#include <format>
#include "aabb.h"
#include "matrix.h"
#include "quaternion.h"
#include "vec.h"
#ifdef ROVE_DFLOAT
#include "dfloat.h"
#endif

namespace rove::detail
{

template<class CharT, class Out>
Out put_separator(Out out, char const *separator)
{
	for (; *separator; ++separator)
		*out++ = CharT(*separator);
	return out;
}

/// std::formatter<T>'s spec, parsed once and applied to each component
template<class T, class CharT>
struct components_formatter: std::formatter<T, CharT> {
	template<class Context>
	auto format_components(T const *components, std::size_t count, Context &ctx) const
	{
		auto out = ctx.out();
		for (std::size_t n = 0; n < count; ++n) {
			if (n) out = put_separator<CharT>(out, " ");
			ctx.advance_to(out);
			out = std::formatter<T, CharT>::format(components[n], ctx);
		}
		return out;
	}
};

}

#ifdef ROVE_DFLOAT
/// As the float it holds
template<class CharT>
struct std::formatter<rove::dfloat, CharT>: std::formatter<float, CharT> {
	template<class Context>
	auto format(rove::dfloat d, Context &ctx) const
	{
		return std::formatter<float, CharT>::format(d.value(), ctx);
	}
};
#endif

/// Every storage: positions, colours, the ground plane, any N
template<int N, class T, class S, class CharT>
struct std::formatter<rove::vec<N, T, S>, CharT>: rove::detail::components_formatter<T, CharT> {
	template<class Context>
	auto format(rove::vec<N, T, S> const &v, Context &ctx) const
	{
		return this->format_components(v.i, N, ctx);
	}
};

/// x y z w
template<class T, class CharT>
struct std::formatter<rove::quaternion<T>, CharT>: rove::detail::components_formatter<T, CharT> {
	template<class Context>
	auto format(rove::quaternion<T> const &q, Context &ctx) const
	{
		T const components[] = {q.x, q.y, q.z, q.w};
		return this->format_components(components, 4, ctx);
	}
};

/// Row by row: m[0][0] m[1][0] m[2][0]; m[0][1] ...
template<int N, class T, class CharT>
struct std::formatter<rove::matrix<N, N, T>, CharT>: rove::detail::components_formatter<T, CharT> {
	template<class Context>
	auto format(rove::matrix<N, N, T> const &m, Context &ctx) const
	{
		auto out = ctx.out();
		for (int r = 0; r < N; ++r) {
			T row[N];
			for (int c = 0; c < N; ++c)
				row[c] = m.ij[c][r];
			if (r) out = rove::detail::put_separator<CharT>(out, "; ");
			ctx.advance_to(out);
			out = this->format_components(row, N, ctx);
		}
		return out;
	}
};

/// lo x hi
template<int N, class T, class CharT>
struct std::formatter<rove::aabb<N, T>, CharT>: rove::detail::components_formatter<T, CharT> {
	template<class Context>
	auto format(rove::aabb<N, T> const &box, Context &ctx) const
	{
		auto out = this->format_components(box.lo.i, N, ctx);
		out = rove::detail::put_separator<CharT>(out, " x ");
		ctx.advance_to(out);
		return this->format_components(box.hi.i, N, ctx);
	}
};
