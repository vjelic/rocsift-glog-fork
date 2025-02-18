function(add_rocsift_tool NAME)
  add_executable(${NAME} ${NAME}.cc)
  target_link_libraries(${NAME} rocsift::rocsift CLI11::CLI11)
  target_compile_options(${NAME} PRIVATE
                         -Wall
                         -Wfatal-errors
                         -march=x86-64
                         $<$<CONFIG:PROFILE>:-coverage>
                         $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:PROFILE>>:-O0>
                         $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:ASAN>,$<CONFIG:EXTERNAL>>:-O3>
                         $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:ASAN>,$<CONFIG:PROFILE>>:-ggdb>
                         $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:ASAN>,$<CONFIG:PROFILE>>:-gdwarf-4>
                         $<$<CONFIG:ASAN>:-fsanitize=address>
  )
  set_property(TARGET ${NAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH true)
  install(
      TARGETS ${NAME}
      RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/rocsift-tools COMPONENT ${SIFT_NAME}_tools
    )
endfunction()


add_rocsift_tool(dumprls)
add_rocsift_tool(pskfd)
add_rocsift_tool(va2pa)
add_rocsift_tool(read_reg32)
add_rocsift_tool(write_reg32)
add_rocsift_tool(list_partitions)
add_rocsift_tool(cpc_scratch)
add_rocsift_tool(read_mem)
add_rocsift_tool(status)

add_executable(rocsift-tools rocsift_tools.cc)
target_link_libraries(rocsift-tools CLI11::CLI11 stdc++fs)
target_compile_options(rocsift-tools PRIVATE
                       -Wall
                       -Wfatal-errors
                       -march=x86-64
                       $<$<CONFIG:PROFILE>:-coverage>
                       $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:PROFILE>>:-O0>
                       $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:ASAN>,$<CONFIG:EXTERNAL>>:-O3>
                       $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:ASAN>,$<CONFIG:PROFILE>>:-ggdb>
                       $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:ASAN>,$<CONFIG:PROFILE>>:-gdwarf-4>
                       $<$<CONFIG:ASAN>:-fsanitize=address>
)
set_property(TARGET rocsift-tools PROPERTY INSTALL_RPATH_USE_LINK_PATH true)
install(
    TARGETS rocsift-tools
    RUNTIME COMPONENT ${SIFT_NAME}_tools
  )
