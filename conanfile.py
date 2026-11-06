# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""Conan 2 recipe of rove: `conan create .` builds and tests the package from this checkout."""
import os
import re

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load, rmdir


class RoveConan(ConanFile):
    name = "rove"
    description = ("Math and geometry for games, robotics and simulation: vectors, matrices, quaternions, shapes, "
                   "collisions and spatial indices")
    license = "MIT"
    url = "https://github.com/mnvl/rove"
    homepage = "https://github.com/mnvl/rove"
    topics = ("math", "geometry", "3d", "collision", "game-development", "robotics")
    package_type = "static-library"
    settings = "os", "arch", "compiler", "build_type"
    # (no fPIC: CMakeLists.txt makes rove position-independent however it is built)
    options = {"dfloat": [True, False]}
    default_options = {"dfloat": False}
    exports_sources = "CMakeLists.txt", "LICENSE.md", "src/*", "include/*"
    test_package_folder = "tests/test_conan_package"

    def set_version(self):
        text = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
        self.version = re.search(r"project\(rove VERSION ([0-9.]+)", text).group(1)

    def validate(self):
        check_min_cppstd(self, 20)
        if self.options.dfloat and self.settings.compiler == "msvc":
            raise ConanInvalidConfiguration("rove: no flag is known to keep MSVC from fusing multiply-adds, which dfloat needs")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["ROVE_TESTS"] = False
        tc.cache_variables["ROVE_PYTHON"] = False
        tc.cache_variables["ROVE_INSTALL"] = True
        tc.cache_variables["ROVE_LTO"] = False
        tc.cache_variables["ROVE_DFLOAT"] = bool(self.options.dfloat)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE.md", self.source_folder, os.path.join(self.package_folder, "licenses"))
        CMake(self).install()
        # (CMakeDeps writes a package's CMake files from package_info: rove's own would only be in its way)
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "rove")
        self.cpp_info.set_property("cmake_target_name", "rove::rove")
        self.cpp_info.libs = ["rove"]
        if self.options.dfloat:
            self.cpp_info.defines = ["ROVE_DFLOAT"]
            if self.settings.compiler in ("gcc", "clang", "apple-clang"):
                self.cpp_info.cxxflags = ["-ffp-contract=off"]
        if self.settings.os in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs = ["m"]
