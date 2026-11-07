# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

# rove's vcpkg port, as an overlay port: vcpkg install rove --overlay-ports=<rove>/ports.
# ROVE_SOURCE_DIR (passed through with VCPKG_KEEP_ENV_VARS=ROVE_SOURCE_DIR) builds a checkout instead of the release, which is
# how CI tests the port before a version is tagged.
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

if(DEFINED ENV{ROVE_SOURCE_DIR})
    set(SOURCE_PATH "$ENV{ROVE_SOURCE_DIR}")
else()
    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO mnvl/rove
        REF "v${VERSION}"
        # the SHA512 of the release's archive: vcpkg prints it when this is wrong
        SHA512 91e5f93c61b469cff6e8c8bd2118cbbb6d0c72b0ebadc41fbb4482fc9287e9453663c04d5cac858b2411562673622e034ef5f62454cabceca4f195f198b1c4b0
        HEAD_REF main
    )
endif()

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS FEATURES dfloat ROVE_DFLOAT)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DROVE_TESTS=OFF
        -DROVE_PYTHON=OFF
        -DROVE_INSTALL=ON
        -DROVE_LTO=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/rove)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.md")
