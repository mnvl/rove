#!/bin/bash
# Build debug version for development
set -e

BUILD_DIR="build-debug"
echo "Building debug version..."
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

# Configure for debugging
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DROVE_BUILD_PYTHON=ON \
      -DROVE_BUILD_TESTS=ON \
      ..

echo "Building..."
make -j"$NPROC"

echo ""
echo "================================"
echo "Debug build complete!"
echo "================================"
echo "Build directory: $BUILD_DIR"
echo "Python module: $BUILD_DIR/pyrove*.so"
echo "Tests: $BUILD_DIR/rove_tests"
echo ""
echo "To run tests:"
echo "  cd $BUILD_DIR && ctest"
echo ""
echo "To debug:"
echo "  cd $BUILD_DIR && lldb ./rove_tests"
echo "  or: gdb ./rove_tests"
