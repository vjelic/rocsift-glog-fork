#!/bin/sh

# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved


SRCDIR="$(realpath "$(dirname "$(readlink -f "$0")")/../")"

# mkdir -p "$SRCDIR/build"

cmake	\
    -G Ninja \
	-DCMAKE_BUILD_TYPE=Release	\
    -DBUILD_SHARED_LIBS=ON \
	-B"$SRCDIR/build/build-shared"		\
	-S"$SRCDIR"

cmake	\
    -G Ninja \
	-DCMAKE_BUILD_TYPE=Release	\
    -DBUILD_SHARED_LIBS=OFF \
	-B"$SRCDIR/build/build-static"		\
	-S"$SRCDIR"

cmake --build build/build-shared
cmake --build build/build-static

# Run cpack from inside build to prevent cluttering root project dir
cd build
cpack --config ../packaging/custom_cpack.cmake
