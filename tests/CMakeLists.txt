# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved


enable_testing()
function(add_rocsift_test TEST_NAME)
  add_executable(${TEST_NAME} ${TEST_NAME}.cc)
  target_link_libraries(${TEST_NAME} PRIVATE rocsift)
  target_include_directories(${TEST_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/third_party>
  )
  add_test(${TEST_NAME} ${TEST_NAME})
endfunction()

function(add_rocisft_test_internal TEST_NAME)
  add_rocsift_test(${TEST_NAME})
  target_include_directories(${TEST_NAME}
  PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  )
endfunction()


add_rocsift_test(enumerate)
