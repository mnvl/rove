#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

# Whether ports/rove/portfile.cmake holds the SHA512 of the archive of the release it names.  CI builds the port from the checkout, so it
# would never notice a wrong hash; a user would.  A version with no tag yet has no archive to compare with.
set -eu
version=$(sed -n 's/^project(rove VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)
in_port=$(sed -n 's/^ *SHA512 \([0-9a-f]*\)$/\1/p' ports/rove/portfile.cmake)
tag="v$version"
if ! git ls-remote --exit-code --tags https://github.com/mnvl/rove "$tag" >/dev/null; then
    echo "$tag is not tagged yet: nothing to compare"
    exit 0
fi
actual=$(curl -sSfL "https://github.com/mnvl/rove/archive/$tag.tar.gz" | sha512sum | cut -d' ' -f1)
if [ "$in_port" != "$actual" ]; then
    echo "ports/rove/portfile.cmake has SHA512 $in_port; the archive of $tag is $actual" >&2
    exit 1
fi
echo "$tag: the port's SHA512 is the archive's"
