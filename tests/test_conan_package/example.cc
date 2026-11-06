// SPDX-License-Identifier: MIT
// Copyright (c) 2004-2026 Manvel Avetisian

#include <cstdio>
#include <optional>
#include <rove/collision.h>
#include <rove/quaternion.h>
#include <rove/vec.h>

int main()
{
	// a sphere moving into another: the step's first contact
	rove::sphere<3> const a(rove::vec<3>(0, 0, 0), 1), b(rove::vec<3>(3, 0, 0), 1);
	std::optional<rove::contact_info<3>> const contact = rove::first_contact(a, b, rove::vec<3>(-2, 0, 0));
	rove::quaternion<float> const turn = rove::rotation_between(rove::vec<3>(1, 0, 0), rove::vec<3>(0, 0, 1));
	rove::vec<3> const turned = turn * rove::vec<3>(1, 0, 0);
	std::printf("contact: %s, turned: (%.1f, %.1f, %.1f)\n", contact ? "yes" : "no", double(turned.x), double(turned.y), double(turned.z));
	return contact && turned.z > 0.99f ? 0 : 1;
}
