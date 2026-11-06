# Installing rove

rove is a static C++20 library with no dependencies, plus an optional Python module. Pick one of the ways below. Each
gives a CMake project the target `rove::rove`, and code includes rove's headers as `<rove/...>`:

```cpp
#include <rove/vec.h>
#include <rove/collision.h>
```

Requirements for C++: CMake 3.15 or newer and a C++20 compiler. CI builds and tests with GCC 13, Clang 18 and the Apple Clang of
macOS 15, and builds with MSVC of Visual Studio 2022; older ones are not tested. `<rove/format.h>` also needs a standard library that
has `<format>` (libstdc++ 13); nothing else includes it.

## Python: pip

```bash
pip install pyrove                                    # a released version, from PyPI
pip install "git+https://github.com/mnvl/rove@v3.0.0" # any tag or commit; builds from source
pip install ./rove                                    # a local checkout; builds from source
```

PyPI has wheels for Linux (x86_64 and aarch64, glibc and musl), macOS on Apple silicon and 64-bit Windows, on CPython 3.9 to 3.14,
including the free-threaded 3.14t. Elsewhere (an Intel Mac, Windows on ARM, 32-bit Linux) pip builds from source, which needs CMake and
a C++20 compiler; pip fetches the rest.

```python
import pyrove
v = pyrove.vec3(1, 2, 3)
print(pyrove.dot(v, v))   # 14.0
```

### An extension of your own that passes rove types

nanobind shares types only between modules with the same ABI tag, which comes from the compiler that built them. An extension compiled here
gets a `TypeError` on every `pyrove.vec3` it takes or returns against a PyPI wheel built by another compiler (or another nanobind). Build
pyrove with the same toolchain as the extension, and it works:

```bash
pip install --no-binary pyrove pyrove
```

## C++: a git submodule

```bash
git submodule add https://github.com/mnvl/rove external/rove
```

```cmake
add_subdirectory(external/rove)
target_link_libraries(my_app PRIVATE rove::rove)
```

Added as a subdirectory, rove builds only the library: no tests, no Python module, no install rules (turn them on with
`ROVE_TESTS`, `ROVE_PYTHON`, `ROVE_INSTALL`). It leaves the build type and link-time optimisation to the parent project.
The directory may have any name.

If you also use [edge](https://github.com/mnvl/edge), check rove and edge out side by side (`external/rove`,
`external/edge`) and add rove first: edge then uses that rove.

## C++: FetchContent

Without a submodule, CMake can fetch rove at configure time:

```cmake
include(FetchContent)
FetchContent_Declare(rove GIT_REPOSITORY https://github.com/mnvl/rove GIT_TAG v3.0.0)
FetchContent_MakeAvailable(rove)
target_link_libraries(my_app PRIVATE rove::rove)
```

## C++: Conan 2

The recipe is `conanfile.py` in this repository. Until rove is in ConanCenter, create the package from a checkout:

```bash
git clone --branch v3.0.0 https://github.com/mnvl/rove
conan create rove -s compiler.cppstd=20
```

`conan create` builds rove and runs `tests/test_conan_package/`, a small program that links it. Then, in your project's
`conanfile.txt`:

```ini
[requires]
rove/3.0.0

[generators]
CMakeDeps
CMakeToolchain

[layout]
cmake_layout
```

```bash
conan install . -s compiler.cppstd=20 --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
```

```cmake
find_package(rove 3 CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE rove::rove)
```

Options: `rove/*:dfloat=True` builds `rove::dfloat`, the deterministic float (it adds `-ffp-contract=off` to rove and to
what links it). `-s compiler.cppstd=20` is needed because rove is C++20 and Conan's default profile is not.

## C++: vcpkg

The port is in `ports/rove`. Until rove is in vcpkg's registry, use it as an overlay port, from `main`: the port names a release and
the SHA512 of its archive, and a tag's own copy cannot hold the hash of the archive made from it. In manifest mode, your project's
`vcpkg.json`:

```json
{
  "dependencies": ["rove"]
}
```

```bash
git clone https://github.com/mnvl/rove
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_OVERLAY_PORTS=$PWD/rove/ports
```

or in classic mode: `vcpkg install rove --overlay-ports=rove/ports`. Then:

```cmake
find_package(rove 3 CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE rove::rove)
```

Feature `dfloat` (`"rove[dfloat]"`) is the Conan option of the same name.

## C++: installed from source

```bash
cmake -S rove -B rove/build -DCMAKE_BUILD_TYPE=Release -DROVE_TESTS=OFF
cmake --build rove/build
cmake --install rove/build --prefix /opt/rove
```

Then `find_package(rove 3 CONFIG REQUIRED)` with `-DCMAKE_PREFIX_PATH=/opt/rove`. The install has the library, the
headers under `include/rove/`, and the CMake package under `lib/cmake/rove/`. rove itself needs nothing else; `ROVE_TESTS=OFF` is what
says so, since its tests need Boost.Test. `<rove/src/x.h>`, the older spelling of `<rove/x.h>`, works from an install, from
FetchContent and from a checkout of any name.

The library is built without link-time optimisation unless `-DROVE_LTO=ON` is given, so any compiler that can read the headers can link it.
With `ROVE_LTO=ON` GCC writes objects that only the same GCC version can link.

## Versions

rove follows semantic versioning: `find_package(rove 3)` accepts any 3.x. What changed between versions is in
[CHANGELOG.md](../CHANGELOG.md).
