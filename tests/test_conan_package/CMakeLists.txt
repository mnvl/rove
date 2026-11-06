# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

# A program that uses an installed rove, as a package manager's user writes it (Conan's test_package, vcpkg's, CI's).
cmake_minimum_required(VERSION 3.15)
project(rove_example CXX)

find_package(rove 3 CONFIG REQUIRED)
add_executable(rove_example example.cc)
target_link_libraries(rove_example PRIVATE rove::rove)
target_compile_features(rove_example PRIVATE cxx_std_20)
