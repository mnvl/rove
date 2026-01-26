#!/bin/bash
# Build portable optimized version (works on different CPUs)
set -e

BUILD_DIR="build-portable"
echo "Building portable optimized version..."
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

# Configure with portable optimization (no -march=native)
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -flto" \
      -DROVE_BUILD_PYTHON=ON \
      -DROVE_BUILD_TESTS=OFF \
      ..

echo "Building..."
make -j"$NPROC"

echo ""
echo "================================"
echo "Portable optimized build complete!"
echo "================================"
echo "Build directory: $BUILD_DIR"
echo "Python module: $BUILD_DIR/pyrove*.so"
echo ""
echo "This build will work on different CPU architectures."
echo ""
echo "To use the optimized build:"
echo "  cd $BUILD_DIR"
echo "  python3 -c 'import sys; sys.path.insert(0, \".\"); import pyrove; print(pyrove.__name__)'"
