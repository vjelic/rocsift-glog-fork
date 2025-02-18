include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED ${SIFT_NAME}_INSTALL_CMAKEDIR)
  set(${SIFT_NAME}_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/rocsift"
    CACHE STRING "Path to rocsift CMake files")
endif()

install(TARGETS ${SIFT_TARGET} EXPORT ${SIFT_NAME}Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          COMPONENT ${SIFT_NAME}_runtime
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT ${SIFT_NAME}_runtime
          NAMELINK_COMPONENT ${SIFT_NAME}_dev
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT ${SIFT_NAME}_dev
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(DIRECTORY "${${SIFT_NAME}_SOURCE_DIR}/include/" #"${${SIFT_NAME}_BINARY_DIR}/include"
  TYPE INCLUDE
  COMPONENT ${SIFT_NAME}_dev)

if (BUILD_SHARED_LIBS)
  set(type shared)
else()
  set(type static)
endif()

install(EXPORT ${SIFT_NAME}Targets
        DESTINATION "${${SIFT_NAME}_INSTALL_CMAKEDIR}"
        NAMESPACE ${SIFT_TARGET}::
        FILE ${SIFT_NAME}-${type}-targets.cmake
        COMPONENT ${SIFT_NAME}_dev)

write_basic_package_version_file(${SIFT_NAME}ConfigVersion.cmake COMPATIBILITY SameMajorVersion)

install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/rocsiftConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${SIFT_NAME}ConfigVersion.cmake"
  DESTINATION "${${SIFT_NAME}_INSTALL_CMAKEDIR}"
  COMPONENT ${SIFT_NAME}_dev)

# install public headers
# install(
#   DIRECTORY ${PROJECT_SOURCE_DIR}/include/rocsift
#   DESTINATION include
#   COMPONENT dev)

include(packaging.cmake)
