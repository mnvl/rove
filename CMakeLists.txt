cmake_minimum_required(VERSION 3.15)
project(vek VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library sources
set(VEK_SOURCES
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
add_library(vek STATIC ${VEK_SOURCES})
target_include_directories(vek PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Find Boost
find_package(Boost REQUIRED)
target_link_libraries(vek PUBLIC Boost::boost)

# Tests
option(VEK_BUILD_TESTS "Build tests" ON)

if(VEK_BUILD_TESTS)
    find_package(Boost REQUIRED COMPONENTS unit_test_framework serialization)

    set(VEK_TEST_SOURCES
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

    add_executable(vek_tests ${VEK_TEST_SOURCES})
    target_link_libraries(vek_tests PRIVATE vek Boost::unit_test_framework Boost::serialization)

    enable_testing()
    add_test(NAME vek_tests COMMAND vek_tests)
endif()

# Python bindings
option(VEK_BUILD_PYTHON "Build Python bindings" OFF)

if(VEK_BUILD_PYTHON)
    find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

    # Fetch nanobind
    include(FetchContent)
    FetchContent_Declare(
        nanobind
        GIT_REPOSITORY https://github.com/wjakob/nanobind
        GIT_TAG v2.4.0
    )
    FetchContent_MakeAvailable(nanobind)

    nanobind_add_module(pyvek src/python_bindings.cc)
    target_link_libraries(pyvek PRIVATE vek)
    target_include_directories(pyvek PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()
