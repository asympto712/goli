#! /bin/bash

set -e

BUILD_DIR="build/linux"
CMAKE_GENERATOR="Unix Makefiles"

export CC=gcc
export CXX=g++

echo "=== RUNNING CMAKE ==="
cmake -G "${CMAKE_GENERATOR}" \
  -S . \
  -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${1:-debug}" \
  # --debug-find \

echo "=== BUILDING ==="
cmake --build "${BUILD_DIR}" -j$(nproc)

echo "=== BUILD FINISHED ==="
echo "build into ${BUILD_DIR}"


