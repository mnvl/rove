#!/bin/bash
# Build optimized release version of rove
set -e

BUILD_DIR="build-release"
echo "Building optimized release version..."
echo "Build directory: $BUILD_DIR"

# Detect number of CPU cores
if [[ "$OSTYPE" == "darwin"* ]]; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=$(nproc)
fi

echo "Using $NPROC parallel jobs"

# Create fresh build directory
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with maximum optimization
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto" \
      -DROVE_BUILD_PYTHON=ON \
      -DROVE_BUILD_TESTS=OFF \
      ..

echo "Building..."
make -j"$NPROC"

echo ""
echo "================================"
echo "Optimized build complete!"
echo "================================"
echo "Build directory: $BUILD_DIR"
echo "Python module: $BUILD_DIR/pyrove*.so"
echo ""
echo "To use the optimized build:"
echo "  cd $BUILD_DIR"
echo "  python3 -c 'import sys; sys.path.insert(0, \".\"); import pyrove; print(pyrove.__name__)'"
echo ""
echo "To run benchmarks:"
echo "  ./run_benchmark.sh"
