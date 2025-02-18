#!/bin/sh

# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved


SRCDIR="$(realpath "$(dirname "$(readlink -f "$0")")/../")"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"

mkdir -p "$SRCDIR/build"


cmake	\
	-DCMAKE_EXPORT_COMPILE_COMMANDS=1	\
  -DINSTALL_GIT_PRECOMMIT_HOOK=1 \
	-DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"	\
	-B"$SRCDIR/build"		\
	-S"$SRCDIR"
