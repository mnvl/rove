#!/bin/bash
# Compare performance between debug and optimized builds

set -e

echo "================================"
echo "Build Comparison Script"
echo "================================"
echo ""

# Check if builds exist
if [ ! -f "build/pyrove.cpython-314-darwin.so" ] && [ ! -f "build/pyrove*.so" ]; then
    echo "Debug build not found. Building..."
    ./build_debug.sh
fi

if [ ! -f "build-release/pyrove.cpython-314-darwin.so" ] && [ ! -f "build-release/pyrove*.so" ]; then
    echo "Release build not found. Building..."
    ./build_release.sh
fi

echo ""
echo "Build Information:"
echo "================================"

# Find Python interpreter
if [ -f ".venv/bin/python3.14" ]; then
    PYTHON=".venv/bin/python3.14"
elif [ -f ".venv/bin/python3" ]; then
    PYTHON=".venv/bin/python3"
else
    PYTHON="python3"
fi

echo "Python: $PYTHON ($($PYTHON --version))"
echo ""

# Compare file sizes
echo "Binary Sizes:"
echo "--------------------------------"
if [ -f build/pyrove*.so ]; then
    DEBUG_SIZE=$(ls -lh build/pyrove*.so | awk '{print $5}')
    echo "Debug build:   $DEBUG_SIZE"
fi
if [ -f build-release/pyrove*.so ]; then
    RELEASE_SIZE=$(ls -lh build-release/pyrove*.so | awk '{print $5}')
    echo "Release build: $RELEASE_SIZE"
fi
echo ""

# Run quick performance test
echo "Quick Performance Test:"
echo "--------------------------------"

# Create minimal test script
cat > /tmp/rove_perf_test.py << 'EOF'
import sys
import os
import time

# Add current directory to path
sys.path.insert(0, os.getcwd())

try:
    import pyrove
except ImportError as e:
    print(f"Failed to import pyrove: {e}")
    sys.exit(1)

# Quick vector operations benchmark
iterations = 1000000

start = time.perf_counter()
for i in range(iterations):
    v1 = pyrove.vec3(1.0, 2.0, 3.0)
    v2 = pyrove.vec3(4.0, 5.0, 6.0)
    v3 = v1 + v2
    length = v3.length()
end = time.perf_counter()

time_per_op = (end - start) / iterations * 1e6
print(f"Time per operation: {time_per_op:.2f} µs")
print(f"Total time: {end - start:.3f} seconds")
EOF

for BUILD_DIR in build-debug build build-release; do
    if [ -d "$BUILD_DIR" ] && [ -f "$BUILD_DIR"/pyrove*.so ]; then
        BUILD_NAME=$(echo "$BUILD_DIR" | sed 's/build-//' | sed 's/build/debug/')
        echo "$BUILD_NAME build:"
        (cd "$BUILD_DIR" && $PYTHON /tmp/rove_perf_test.py 2>&1) || echo "  (skipped)"
        echo ""
    fi
done

echo ""
echo "================================"
echo "For detailed benchmarks, run:"
echo "  ./run_benchmark.sh"
echo "================================"

rm -f /tmp/rove_perf_test.py
