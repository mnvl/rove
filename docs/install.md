# Installing rove

rove is a static C++20 library with no dependencies, plus an optional Python module. Pick one of the ways below. Each
gives a CMake project the target `rove::rove`, and code includes rove's headers as `<rove/...>`:

```cpp
#include <rove/vec.h>
#include <rove/collision.h>
```

Requirements for C++: CMake 3.15 or newer and a C++20 compiler (GCC 12, Clang 15, Apple Clang 15, MSVC 19.30 or newer).

## Python: pip

```bash
pip install pyrove                                    # a released version, from PyPI
pip install "git+https://github.com/mnvl/rove@v3.0.0" # any tag or commit; builds from source
pip install ./rove                                    # a local checkout; builds from source
```

PyPI has wheels for Linux, macOS and Windows on CPython 3.9 to 3.14, including the free-threaded 3.14t. A build from
source needs CMake and a C++20 compiler; pip fetches the rest.

```python
import pyrove
v = pyrove.vec3(1, 2, 3)
print(pyrove.dot(v, v))   # 14.0
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

The port is in `ports/rove`. Until rove is in vcpkg's registry, use it as an overlay port. In manifest mode, your
project's `vcpkg.json`:

```json
{
  "dependencies": ["rove"]
}
```

```bash
git clone --branch v3.0.0 https://github.com/mnvl/rove
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
cmake -S rove -B rove/build -DCMAKE_BUILD_TYPE=Release
cmake --build rove/build
cmake --install rove/build --prefix /opt/rove
```

Then `find_package(rove 3 CONFIG REQUIRED)` with `-DCMAKE_PREFIX_PATH=/opt/rove`. The install has the library, the
headers under `include/rove/`, and the CMake package under `lib/cmake/rove/`.

## Versions

rove follows semantic versioning: `find_package(rove 3)` accepts any 3.x. What changed between versions is in
[CHANGELOG.md](../CHANGELOG.md).
