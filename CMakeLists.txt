cmake_minimum_required(VERSION 3.15)
project(rove VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    message(STATUS "Build type not specified, defaulting to Release")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Enable interprocedural optimization (LTO) for Release builds if supported
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
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
    src/a_star.cc
    src/bresenham_supercover.cc
    src/capsule.cc
    src/contact_info.cc
    src/frustum.cc
    src/heightfield.cc
    src/interval.cc
    src/line.cc
    src/matrix.cc
    src/obb.cc
    src/plane.cc
    src/ray.cc
    src/sphere.cc
    src/vec.cc
)

# Create static library
add_library(rove STATIC ${ROVE_SOURCES})
target_include_directories(rove PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Compiler-specific optimizations for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rove PRIVATE
            -Wall -Wextra
            $<$<CONFIG:Release>:-O3>
        )
    elseif(MSVC)
        target_compile_options(rove PRIVATE
            /W4
            $<$<CONFIG:Release>:/O2>
        )
    endif()
endif()

# Find Boost
find_package(Boost REQUIRED)
target_link_libraries(rove PUBLIC Boost::boost)

# Tests
option(ROVE_BUILD_TESTS "Build tests" ON)

if(ROVE_BUILD_TESTS)
    find_package(Boost REQUIRED COMPONENTS unit_test_framework serialization)

    set(ROVE_TEST_SOURCES
        src/test_main.cc
        src/test_aabb.cc
        src/test_a_star.cc
        src/test_bresenham_supercover.cc
        src/test_capsule.cc
        src/test_collision.cc
        src/test_frustum.cc
        src/test_heightfield.cc
        src/test_matrix.cc
        src/test_obb.cc
        src/test_plane.cc
        src/test_quaternion.cc
        src/test_ray.cc
        src/test_sphere.cc
        src/test_triangle.cc
        src/test_vec.cc
    )

    add_executable(rove_tests ${ROVE_TEST_SOURCES})
    target_link_libraries(rove_tests PRIVATE rove Boost::unit_test_framework Boost::serialization)

    enable_testing()
    add_test(NAME rove_tests COMMAND rove_tests)
endif()

# Python bindings
option(ROVE_BUILD_PYTHON "Build Python bindings" OFF)

if(ROVE_BUILD_PYTHON)
    find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

    # Fetch nanobind
    include(FetchContent)
    FetchContent_Declare(
        nanobind
        GIT_REPOSITORY https://github.com/wjakob/nanobind
        GIT_TAG v2.4.0
    )
    FetchContent_MakeAvailable(nanobind)

    # Python binding sources (organized by class)
    set(PYROVE_SOURCES
        src/python_bindings.cc
        src/bind_vec.cc
        src/bind_matrix.cc
        src/bind_quaternion.cc
        src/bind_sphere.cc
        src/bind_aabb.cc
        src/bind_obb.cc
        src/bind_ray.cc
        src/bind_line.cc
        src/bind_plane.cc
        src/bind_triangle.cc
        src/bind_capsule.cc
        src/bind_frustum.cc
    )

    nanobind_add_module(pyrove ${PYROVE_SOURCES})
    target_link_libraries(pyrove PRIVATE rove)
    target_include_directories(pyrove PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()

# Documentation
option(ROVE_BUILD_DOCS "Build documentation" OFF)

if(ROVE_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    find_program(SPHINX_EXECUTABLE sphinx-build REQUIRED)

    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs)
    set(SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs)
    set(SPHINX_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs/_build)

    # Doxygen target - generates XML for Breathe
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating Doxygen XML documentation"
        VERBATIM
    )

    # Sphinx target - generates HTML documentation
    add_custom_target(sphinx
        COMMAND ${SPHINX_EXECUTABLE} -b html ${SPHINX_SOURCE_DIR} ${SPHINX_BUILD_DIR}/html
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating Sphinx HTML documentation"
        VERBATIM
    )
    add_dependencies(sphinx doxygen)

    # Main docs target
    add_custom_target(docs DEPENDS sphinx)
endif()
