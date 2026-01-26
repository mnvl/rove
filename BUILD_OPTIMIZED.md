# Building Optimized Versions of rove

This guide explains how to build optimized versions of the rove library for maximum performance.

## Quick Start - Release Build

The simplest way to build an optimized version:

```bash
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release -DROVE_BUILD_PYTHON=ON ..
make -j$(nproc)
```

## CMake Build Types

CMake provides several build types with different optimization levels:

### 1. Release (Recommended for Production)

Maximum optimization, no debug symbols:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

- **Optimization**: `-O3` (maximum speed optimization)
- **Debug info**: None
- **Use case**: Production deployments, benchmarks
- **Performance**: Fastest execution

### 2. RelWithDebInfo (Recommended for Profiling)

Optimized but with debug symbols:

```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

- **Optimization**: `-O2` (good optimization)
- **Debug info**: Full debug symbols (`-g`)
- **Use case**: Profiling, performance debugging
- **Performance**: Nearly as fast as Release, easier to debug

### 3. MinSizeRel (For Size-Constrained Environments)

Optimize for binary size:

```bash
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
```

- **Optimization**: `-Os` (optimize for size)
- **Debug info**: None
- **Use case**: Embedded systems, minimal installations
- **Performance**: Good, but smaller binary size

### 4. Debug (Default, Development Only)

No optimization, full debugging:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

- **Optimization**: `-O0` (none) or `-Og` (minimal)
- **Debug info**: Full debug symbols
- **Use case**: Development, debugging
- **Performance**: Slowest, but easiest to debug

## Advanced Optimization Flags

### Maximum Performance Build

For absolute maximum performance on modern CPUs:

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto" \
      -DROVE_BUILD_PYTHON=ON \
      ..
make -j$(nproc)
```

Flags explained:
- `-O3`: Highest optimization level
- `-march=native`: Use all CPU features available on build machine
- `-mtune=native`: Tune for the specific CPU architecture
- `-flto`: Link-time optimization (whole-program optimization)

**Warning**: `-march=native` makes the binary non-portable. It will only run on CPUs with the same or better feature set.

### Portable Optimized Build

Optimized but portable to older CPUs:

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -flto" \
      -DROVE_BUILD_PYTHON=ON \
      ..
make -j$(nproc)
```

### Aggressive Optimization (Use with Caution)

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto -ffast-math -funroll-loops" \
      -DROVE_BUILD_PYTHON=ON \
      ..
make -j$(nproc)
```

Additional flags:
- `-ffast-math`: Relaxed IEEE floating-point compliance (faster but less precise)
- `-funroll-loops`: Unroll loops for speed (may increase binary size)

**Warning**: `-ffast-math` can change numerical results. Don't use for scientific computing that requires exact IEEE compliance.

## Compiler-Specific Optimizations

### GCC/G++

```bash
# GCC with profile-guided optimization
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -flto" \
      ..
make -j$(nproc)
```

### Clang

```bash
# Clang with thin-LTO
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -flto=thin" \
      ..
make -j$(nproc)
```

## Python Bindings Optimization

For optimized Python bindings:

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DROVE_BUILD_PYTHON=ON \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -flto" \
      ..
make -j$(nproc)
```

The Python bindings will automatically use the optimization level of the C++ library.

## Verification

Check that your build is optimized:

```bash
# Check compiler flags used
make VERBOSE=1

# Check if LTO is enabled (Linux)
nm pyrove*.so | grep -i "lto"

# Run benchmark to verify performance
cd ..
./run_benchmark.sh
```

## Recommended Build Configurations

### For Development

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DROVE_BUILD_PYTHON=ON -DROVE_BUILD_TESTS=ON ..
make -j$(nproc)
```

### For Benchmarking

```bash
mkdir build-bench
cd build-bench
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -flto" \
      -DROVE_BUILD_PYTHON=ON \
      -DROVE_BUILD_TESTS=OFF \
      ..
make -j$(nproc)
cd ..
./run_benchmark.sh
```

### For Distribution

```bash
mkdir build-dist
cd build-dist
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -flto" \
      -DROVE_BUILD_PYTHON=ON \
      -DROVE_BUILD_TESTS=OFF \
      ..
make -j$(nproc)
```

## Profile-Guided Optimization (PGO)

For maximum performance with PGO:

### Step 1: Build with instrumentation

```bash
mkdir build-pgo
cd build-pgo
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -fprofile-generate" \
      -DROVE_BUILD_PYTHON=ON \
      ..
make -j$(nproc)
```

### Step 2: Run representative workload

```bash
# Run tests to collect profile data
./rove_tests

# Run Python benchmarks
cd ..
./run_benchmark.sh
cd build-pgo
```

### Step 3: Rebuild with profile data

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -fprofile-use" \
      -DROVE_BUILD_PYTHON=ON \
      ..
make -j$(nproc)
```

## Build Script Examples

### build_release.sh

```bash
#!/bin/bash
# Build optimized release version

BUILD_DIR="build-release"
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -flto" \
      -DROVE_BUILD_PYTHON=ON \
      -DROVE_BUILD_TESTS=OFF \
      ..

make -j$(nproc)
echo "Optimized build complete in $BUILD_DIR/"
```

### build_portable.sh

```bash
#!/bin/bash
# Build portable optimized version

BUILD_DIR="build-portable"
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -flto" \
      -DROVE_BUILD_PYTHON=ON \
      -DROVE_BUILD_TESTS=OFF \
      ..

make -j$(nproc)
echo "Portable optimized build complete in $BUILD_DIR/"
```

## Performance Tips

1. **Always use Release builds for benchmarks** - Debug builds are 5-10x slower
2. **Use `-march=native` for local use** - Can give 10-30% speedup
3. **Enable LTO** - Usually provides 5-15% improvement
4. **Parallel builds** - Use `-j$(nproc)` to speed up compilation
5. **Strip symbols from final binaries** - Use `strip` command to reduce size
6. **Run benchmarks** - Verify your optimizations actually help:
   ```bash
   ./run_benchmark.sh > results-before.txt
   # rebuild with different flags
   ./run_benchmark.sh > results-after.txt
   diff results-before.txt results-after.txt
   ```

## Expected Performance Gains

Typical speedup from Debug to optimized builds:

| Build Type | Relative Speed | Binary Size |
|------------|----------------|-------------|
| Debug      | 1.0x (baseline)| Large       |
| Release -O2| 4-6x faster    | Medium      |
| Release -O3| 5-8x faster    | Medium      |
| -O3 -march=native | 6-10x faster | Medium |
| -O3 -march=native -flto | 7-12x faster | Small-Medium |
| PGO        | 8-15x faster   | Small-Medium |

## Troubleshooting

### Build fails with `-march=native`

Your CPU might not support some optimizations. Try:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3" ..
```

### LTO fails or takes too long

Disable LTO:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..
```

### Benchmark shows no improvement

1. Verify you're using the optimized build
2. Check compiler flags with `make VERBOSE=1`
3. Ensure you're not accidentally using Debug build
4. Profile to find actual bottlenecks

## See Also

- [CMake Build Types Documentation](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)
- [GCC Optimization Options](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)
- [Clang Optimization Options](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-O)
