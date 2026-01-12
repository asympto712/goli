#! /bin/bash

set -e

BUILD_DIR="build/linux"
CMAKE_GENERATOR="Unix Makefiles"

# set this to the directory that includes "ft2build.h"
# if you don't set any custom option during the installation of Freetype package,
# there should be no need to change
PREFIX_PATH="/usr/local/include"

export CC=gcc
export CXX=g++

echo -e "\n=== RUNNING CMAKE ===\n"
cmake -G "${CMAKE_GENERATOR}" \
  -S . \
  -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${1:-Debug}" \
  -DCMAKE_PREFIX_PATH="${PREFIX_PATH}" \
  --debug-find \

echo -e "\n=== BUILDING ===\n"
cmake --build "${BUILD_DIR}" -j$(nproc)

echo -e "\n=== BUILD FINISHED ===\n"
echo "build into ${BUILD_DIR}"


