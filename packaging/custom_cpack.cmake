include("build-shared/CPackConfig.cmake")

# Files shared between both builds will overwrite in the following build
# Build shared last to prefer shared versions of all files over static
set(CPACK_INSTALL_CMAKE_PROJECTS
    build-static rocsift ALL /
    build-shared rocsift ALL /
)
