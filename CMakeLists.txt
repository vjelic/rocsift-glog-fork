# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved


cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set( SIFT_NAME "rocsift" )
set( SIFT_TARGET "${SIFT_NAME}" )
set( SIFT_LIBRARY "lib${SIFT_NAME}" )

project(
  ${SIFT_NAME} 
  VERSION 14.0.0
  DESCRIPTION "C99 ROCm™ Debug Library"
  LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()


## C++ language configuration boilerplate
if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND
    NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Install dev hooks
include(cmake/commit-hooks.cmake)

# Let ROCSIFT_SHARED_LIBS override BUILD_SHARED_LIBS
if(DEFINED ROCSIFT_SHARED_LIBS)
  set(BUILD_SHARED_LIBS "${ROCSIFT_SHARED_LIBS}")
endif()

# Add local repository for FindXXX.cmake modules.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/" ${CMAKE_MODULE_PATH})

set(LOGURU_STACKTRACES 1)
add_subdirectory(src)

include(CTest)
add_subdirectory(tests)

#if building directly build rocsift-tools and add install rules
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CLI11_PRECOMPILED ON)
  add_subdirectory(external/third_party/CLI11)
  add_subdirectory(cmd)
  add_subdirectory(packaging)
endif()

