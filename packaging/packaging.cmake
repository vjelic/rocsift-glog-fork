# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved


# by default compile both deb and rpm
set(CPACK_GENERATOR "DEB;RPM" CACHE STRING "Package types to build")
set(ENABLE_LDCONFIG ON CACHE BOOL "set library links and caches using ldconfig")

set(CPACK_COMPONENTS_ALL rocsift_tools rocsift_runtime rocsift_dev)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)

set(CPACK_PACKAGE_NAME "rocsift")

set(CPACK_PACKAGE_VENDOR "Advanced Micro Devices, Inc.")
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION_STRING})
set(CPACK_PACKAGE_CONTACT "AMD")
set(CPACK_PACKAGE_ROCSIFT_RUNTIME_DESCRIPTION_SUMMARY "A C99 ROCm™ Debug API")
set(CPACK_PACKAGE_ROCSIFT_DEV_DESCRIPTION_SUMMARY "A C99 ROCm™ Debug API Development Package")
set(CPACK_PACKAGE_ROCSIFT_TOOLS_DESCRIPTION_SUMMARY
  "Toolbox for debugging ROCM supported GPUs")

# Debian package specific variables
if (DEFINED ENV{CPACK_DEBIAN_PACKAGE_RELEASE})
	set(CPACK_DEBIAN_PACKAGE_RELEASE $ENV{CPACK_DEBIAN_PACKAGE_RELEASE})
  # if deb release set only build debian
  set(CPACK_GENERATOR "DEB" CACHE STRING "Package types to build" FORCE)
else()
	set(CPACK_DEBIAN_PACKAGE_RELEASE "local")
endif()
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/ROCm/rocsift")
set(CPACK_DEBIAN_ROCSIFT_RUNTIME_PACKAGE_NAME "rocsift")
set(CPACK_DEBIAN_ROCSIFT_DEV_PACKAGE_NAME "rocsift-dev")
set(CPACK_DEBIAN_ROCSIFT_DEV_PACKAGE_DEPENDS "rocsift")

#DEB settings for rocsift-tools
set(CPACK_DEBIAN_ROCSIFT_TOOLS_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_ROCSIFT_TOOLS_PACKAGE_HOMEPAGE
  "https://github.com/ROCm/rocsift")
set(CPACK_DEBIAN_ROCSIFT_TOOLS_PACKAGE_NAME "rocsift-tools")
if(BUILD_SHARED_LIBS)
  set(CPACK_DEBIAN_ROCSIFT_TOOLS_PACKAGE_DEPENDS "rocsift")
endif()

# # RPM specific variables
if (DEFINED ENV{CPACK_RPM_PACKAGE_RELEASE})
	set(CPACK_RPM_PACKAGE_RELEASE $ENV{CPACK_RPM_PACKAGE_RELEASE})
  # if rpm release set only build rpm
  set(CPACK_GENERATOR "RPM" CACHE STRING "Package types to build" FORCE)
  execute_process(COMMAND rpm --eval %{?dist}
      RESULT_VARIABLE PROC_RESULT
      OUTPUT_VARIABLE EVAL_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (PROC_RESULT EQUAL "0" AND NOT EVAL_RESULT STREQUAL "")
    string(APPEND CPACK_RPM_PACKAGE_RELEASE "%{?dist}")
  endif()
else()
	set(CPACK_RPM_PACKAGE_RELEASE "local")
endif()
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_ROCSIFT_RUNTIME_PACKAGE_NAME "rocsift")
set(CPACK_RPM_ROCSIFT_DEV_PACKAGE_NAME "rocsift-devel")
set(CPACK_RPM_ROCSIFT_DEV_PACKAGE_REQUIRES "rocsift")

#RPM Settings for rocsift-tools
set(CPACK_RPM_ROCSIFT_TOOLS_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_ROCSIFT_TOOLS_PACKAGE_NAME "rocsift-tools")
if(BUILD_SHARED_LIBS)
  set(CPACK_RPM_ROCSIFT_TOOLS_PACKAGE_REQUIRES "rocsift")
endif()

set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/cmake/util/rpm_ldconfig_scriptlet)
set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/cmake/util/rpm_ldconfig_scriptlet)

message("GENERATOR=${CPACK_GENERATOR}")
include(CPack)
