# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

cmake_minimum_required(VERSION 3.15)
project(rove VERSION 3.0.0 LANGUAGES CXX)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Built by itself, rove picks its build type; added by a project (add_subdirectory), it leaves that to it.  Link-time optimisation is
# asked for with ROVE_LTO: it is off by default whoever builds rove, because GCC then writes objects of bytecode alone, an archive that
# only that same compiler version can link (and an installed rove is linked by whoever installs it).
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(ROVE_TOP_LEVEL ON)
else()
    set(ROVE_TOP_LEVEL OFF)
endif()

if(ROVE_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    message(STATUS "Build type not specified, defaulting to Release")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(ROVE_LTO "Link-time optimisation of rove in Release and RelWithDebInfo; only that compiler version can then link it" OFF)
if(ROVE_LTO AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        message(STATUS "IPO/LTO enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO/LTO not supported: ${ipo_error}")
    endif()
endif()

# Library sources
set(ROVE_SOURCES
    src/aabb.cc
    src/bresenham_supercover.cc
    src/capsule.cc
    src/frustum.cc
    src/heightfield.cc
    src/intersect.cc
    src/matrix.cc
    src/obb.cc
    src/octree.cc
    src/plane.cc
    src/ray.cc
    src/sphere.cc
    src/triangle.cc
    src/vec.cc
)

# dfloat (src/dfloat.h): a float that gives the same bits on every platform, for lockstep simulations.  Off by default: without it
# rove is built and behaves as before.  On, it builds dfloat and rove's instantiations for it, and adds -ffp-contract=off to rove and
# to whoever links it (a multiply and an add fused into one rounding on one machine and not on another would undo it).
option(ROVE_DFLOAT "Build rove::dfloat, the deterministic float" OFF)
if(ROVE_DFLOAT)
    list(APPEND ROVE_SOURCES src/dfloat.cc)
endif()

# Create static library
add_library(rove STATIC ${ROVE_SOURCES})
# Position-independent, so it links into a shared object (pyrove, pyedge, an Android app's libmain.so) whoever builds it.  It used
# to link there only because rove forced link-time optimisation on itself, whose code is generated at the final link.
set_target_properties(rove PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(ROVE_DFLOAT)
    target_compile_definitions(rove PUBLIC ROVE_DFLOAT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rove PUBLIC -ffp-contract=off)
    else()
        message(FATAL_ERROR "ROVE_DFLOAT: no flag known to keep ${CMAKE_CXX_COMPILER_ID} from fusing multiply-adds")
    endif()
endif()

add_library(rove::rove ALIAS rove)
set_target_properties(rove PROPERTIES VERSION ${PROJECT_VERSION})

# The public headers are include/rove/*.h (one-line shims to src/), included as <rove/vec.h>.  Code written for <rove/src/vec.h> keeps
# compiling, whatever the checkout is called and whether built or installed: rove/src/ here is a directory of forwarders to the headers
# (it used to be rove's parent directory on the path, which put whatever else lives there on every consumer's too).  To who uses rove
# the headers are SYSTEM, so its own warnings flags do not judge them; rove's own sources still see them as its own.
file(GLOB ROVE_PUBLIC_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/rove/*.h)
set(ROVE_FORWARDERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/rove_forwarders)
foreach(header ${ROVE_PUBLIC_HEADERS})
    get_filename_component(header_name ${header} NAME)
    file(CONFIGURE OUTPUT ${ROVE_FORWARDERS_DIR}/rove/src/${header_name}
        CONTENT "#include <rove/${header_name}>\n" @ONLY)
endforeach()
target_include_directories(rove PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(rove SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${ROVE_FORWARDERS_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(rove PUBLIC cxx_std_20)

# Warnings for rove's own sources, in every build type
option(ROVE_WARNINGS "Compile rove with the compiler's warnings on" ${ROVE_TOP_LEVEL})
if(ROVE_WARNINGS)
    if(MSVC)
        # (C4201: a vector's components are named through anonymous structs in a union, on purpose)
        target_compile_options(rove PRIVATE /W4 /wd4201)
    else()
        target_compile_options(rove PRIVATE -Wall -Wextra)
    endif()
endif()

# Tests
option(ROVE_TESTS "Build tests" ${ROVE_TOP_LEVEL})

if(ROVE_TESTS)
    find_package(Boost REQUIRED COMPONENTS unit_test_framework)

    set(ROVE_TEST_SOURCES
        tests/test_main.cc
        tests/test_aabb.cc
        tests/test_a_star.cc
        tests/test_bresenham_supercover.cc
        tests/test_capsule.cc
        tests/test_checks.cc
        tests/test_collision.cc
        tests/test_float_maths.cc
        tests/test_format.cc
        tests/test_frustum.cc
        tests/test_hash_grid.cc
        tests/test_heightfield.cc
        tests/test_line.cc
        tests/test_matrix.cc
        tests/test_obb.cc
        tests/test_octree.cc
        tests/test_plane.cc
        tests/test_quaternion.cc
        tests/test_random.cc
        tests/test_ray.cc
        tests/test_scalar.cc
        tests/test_sphere.cc
        tests/test_traits.cc
        tests/test_transform.cc
        tests/test_triangle.cc
        tests/test_vec.cc
        tests/test_legacy_include.cc
        tests/test_version.cc
        tests/test_windows_macros.cc
    )
    if(ROVE_DFLOAT)
        list(APPEND ROVE_TEST_SOURCES tests/test_dfloat.cc)
    endif()

    add_executable(rove_tests ${ROVE_TEST_SOURCES})
    # (a double rounded back to float in what this instantiates is an error: rove's templates of float compute in float)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(tests/test_float_maths.cc PROPERTIES COMPILE_OPTIONS "-Werror=float-conversion")
    endif()
    target_link_libraries(rove_tests PRIVATE rove Boost::unit_test_framework)
    set_source_files_properties(tests/test_version.cc PROPERTIES COMPILE_DEFINITIONS "ROVE_PROJECT_VERSION=\"${PROJECT_VERSION}\"")

    # The headers, compiled as rove's own and not as a consumer's SYSTEM ones: what a consumer's -Wall -Wextra -Werror would see if it
    # took them in by another path.
    add_library(rove_header_warnings OBJECT tests/test_header_warnings.cc)
    target_include_directories(rove_header_warnings PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(rove_header_warnings PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rove_header_warnings PRIVATE -Wall -Wextra -Werror)
    endif()
    add_dependencies(rove_tests rove_header_warnings)
    # The tests compare rove's results bit for bit with the same expressions written out: a compiler that fuses a multiply and an add
    # in one and not the other (Apple Clang does by default) would make them differ in the last bit.
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rove_tests PRIVATE -ffp-contract=off)
    endif()

    enable_testing()
    add_test(NAME rove_tests COMMAND rove_tests)
endif()

# Python bindings
option(ROVE_PYTHON "Build Python bindings" OFF)

if(ROVE_PYTHON)
    find_package(Python 3.9 COMPONENTS Interpreter Development.Module REQUIRED)

    # nanobind: a wheel build takes the one pip installed for it (pyproject.toml), a CMake build fetches it
    if(SKBUILD)
        execute_process(COMMAND ${Python_EXECUTABLE} -m nanobind --cmake_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)
        find_package(nanobind CONFIG REQUIRED)
    else()
        include(FetchContent)
        FetchContent_Declare(
            nanobind
            GIT_REPOSITORY https://github.com/wjakob/nanobind
            GIT_TAG v2.4.0
        )
        FetchContent_MakeAvailable(nanobind)
    endif()

    # Python binding sources (organized by class)
    set(PYROVE_SOURCES
        python/python_bindings.cc
        python/bind_vec.cc
        python/bind_matrix.cc
        python/bind_quaternion.cc
        python/bind_sphere.cc
        python/bind_aabb.cc
        python/bind_obb.cc
        python/bind_ray.cc
        python/bind_line.cc
        python/bind_plane.cc
        python/bind_triangle.cc
        python/bind_capsule.cc
        python/bind_collision.cc
        python/bind_frustum.cc
        python/bind_octree.cc
        python/bind_a_star.cc
        python/bind_scalar.cc
        python/bind_transform.cc
        python/bind_random.cc
    )

    # FREE_THREADED: declares the module safe without the GIL on a free-threaded
    # interpreter (python3.14t), which would otherwise turn the GIL back on for
    # the whole process when it is imported; no effect on a GIL build.
    nanobind_add_module(pyrove_bind FREE_THREADED ${PYROVE_SOURCES})
    target_link_libraries(pyrove_bind PRIVATE rove)
    target_include_directories(pyrove_bind PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/python)

    # Override nanobind's default -Os with -O3 for maximum performance.
    # nanobind_add_module adds -Os to minimize binary size, but for a math
    # library the binding code benefits from full optimization. The last -O
    # flag wins with GCC/Clang.
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(pyrove_bind PRIVATE -O3)
        endif()
    endif()

    # a wheel (scikit-build-core) puts the module inside the pyrove package
    if(SKBUILD)
        install(TARGETS pyrove_bind LIBRARY DESTINATION pyrove)
    endif()

    # pyrove's own tests (pytest and numpy), against the module just built
    if(ROVE_TESTS)
        add_test(NAME pyrove_py COMMAND ${Python_EXECUTABLE} -m pytest -q -p no:cacheprovider tests
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        set_tests_properties(pyrove_py PROPERTIES ENVIRONMENT
            "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR};ROVE_PYTHON_BINDINGS_DIR=$<TARGET_FILE_DIR:pyrove_bind>;PYTHONDONTWRITEBYTECODE=1")
    endif()
endif()

# Install: the library, the headers as <rove/x.h>, and a CMake package (find_package(rove 3 CONFIG), target rove::rove)
option(ROVE_INSTALL "Generate rove's install rules" ${ROVE_TOP_LEVEL})
if(ROVE_INSTALL AND NOT SKBUILD)
    include(CMakePackageConfigHelpers)
    install(TARGETS rove EXPORT roveTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    # the real headers go where the shims would point: <rove/x.h> is the header itself once installed
    install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rove FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${ROVE_FORWARDERS_DIR}/rove/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rove)
    install(EXPORT roveTargets NAMESPACE rove:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rove)
    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/roveConfigVersion.cmake COMPATIBILITY SameMajorVersion)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/roveConfig.cmake "include(\${CMAKE_CURRENT_LIST_DIR}/roveTargets.cmake)\n")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/roveConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/roveConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rove)
endif()
