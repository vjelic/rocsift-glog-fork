# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved
if( NOT DEFINED BUILD_SHARED_LIBS )
  set( BUILD_SHARED_LIBS ON )
  message("BUILD_SHARED_LIBS IS ON")
endif()
set( BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS}
  CACHE BOOL "Build shared library (.so) or not." )

if( NOT ${BUILD_SHARED_LIBS} )
  message("BUILD_SHARED_LIBS IS OFF")
endif()

message("SIFT_TARGET = ${SIFT_TARGET}")


add_library(${SIFT_TARGET} "")
add_library(${SIFT_TARGET}::${SIFT_TARGET} ALIAS ${SIFT_TARGET})
set_target_properties(${SIFT_TARGET} PROPERTIES
  VERSION ${${SIFT_TARGET}_VERSION}
  SOVERSION ${${SIFT_TARGET}_VERSION_MAJOR})

# enforce uniform output file naming
set_property(TARGET ${SIFT_TARGET} PROPERTY
  OUTPUT_NAME ${SIFT_NAME})

target_sources(${SIFT_TARGET} PRIVATE
                        sift/capi.cc
                        kfd/kfdcapi.cc
                        kfd/kfdtop.cc
                        kfd/kfddebugfs.cc
                        kfd/kfdnode.cc
                        sift/engine.cc
                        sift/device.cc
                        sift/partition.cc
                        sift/amdgpu_debugfs.cc
                        sift/debugfs_mmio_accessor.cc
                        sift/debugfs_smn_accessor.cc
                        sift/debugfs_vram_accessor.cc
                        sift/debugfs_iomem_accessor.cc
                        sift/sram_accessor.cc
                        sift/fdowner.cc
                        sift/ip_discovery.cc
                        xlator/xlator.c
                        pm4/pm4.cc
                        pm4/rls_parser.cc
                        drm/drm.cc
)

target_link_options(${SIFT_TARGET}
  PRIVATE
  -static-libgcc
  -static-libstdc++)

target_link_libraries(${SIFT_TARGET} PRIVATE stdc++fs)

target_include_directories(${SIFT_TARGET}
  PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/third_party/spdlog/include>
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

target_compile_options(${SIFT_TARGET} PRIVATE
  -Wall
  -Wfatal-errors
  -march=x86-64
  -fvisibility=hidden
  $<$<CONFIG:PROFILE>:-coverage>
  $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:PROFILE>>:-O0>
  $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:ASAN>,$<CONFIG:EXTERNAL>>:-O3>
  $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:ASAN>,$<CONFIG:PROFILE>>:-ggdb>
  $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:ASAN>,$<CONFIG:PROFILE>>:-gdwarf-4>
  $<$<CONFIG:ASAN>:-fsanitize=address>
)

target_link_options(${SIFT_TARGET} PRIVATE
  $<$<CONFIG:ASAN>:-fsanitize=address>
  $<$<CONFIG:PROFILE>:--coverage>
  $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:EXTERNAL>>:-s>
)

set_target_properties(${SIFT_TARGET} PROPERTIES
  POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
)
