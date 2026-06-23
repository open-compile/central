cmake_minimum_required(VERSION 3.11)
project(Driver)
cmake_policy(SET CMP0003 NEW)

# Set due to the fact that test system said so.
set(DRIVER_TARGET_NAME compiler)

add_compile_options(-DDebug_Mode=1 -std=c++11)
add_executable(${DRIVER_TARGET_NAME} main.cxx toolchain.cxx)
target_include_directories(${DRIVER_TARGET_NAME} PRIVATE
        ${OCC_MAIN_DIR}/common
        ${OCC_MAIN_DIR}/cg
        ${OCC_MAIN_DIR}/lib/subprocess
        ${OCC_MAIN_DIR}/lib)
target_link_libraries(${DRIVER_TARGET_NAME} PRIVATE common fe opt cg cgnova)

INSTALL(TARGETS ${DRIVER_TARGET_NAME}
  RUNTIME DESTINATION bin
  RENAME compiler
)

if(BUILD_TESTING)
  add_executable(toolchain_test toolchain_test.cxx toolchain.cxx)
  target_include_directories(toolchain_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME toolchain_test COMMAND toolchain_test)

  add_test(NAME target_cli_test
    COMMAND ${CMAKE_COMMAND}
      -DCOMPILER=$<TARGET_FILE:${DRIVER_TARGET_NAME}>
      -DSOURCE=${OCC_MAIN_DIR}/testcase/simple/simple.c
      -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/target_cli_test.cmake)
  add_test(NAME arm64_cli_test
    COMMAND ${CMAKE_COMMAND}
      -DCOMPILER=$<TARGET_FILE:${DRIVER_TARGET_NAME}>
      -DSIMPLE_SOURCE=${OCC_MAIN_DIR}/testcase/simple/simple.c
      -DARGS_SOURCE=${OCC_MAIN_DIR}/testcase/cgcase/arm64_args.c
      -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/arm64_cli_test.cmake)
  add_test(NAME x86_cli_test
    COMMAND ${CMAKE_COMMAND}
      -DCOMPILER=$<TARGET_FILE:${DRIVER_TARGET_NAME}>
      -DSIMPLE_SOURCE=${OCC_MAIN_DIR}/testcase/simple/simple.c
      -DARGS_SOURCE=${OCC_MAIN_DIR}/testcase/cgcase/arm64_args.c
      -DGLOBALS_SOURCE=${OCC_MAIN_DIR}/testcase/simple/expr.c
      -DDIV_SOURCE=${OCC_MAIN_DIR}/testcase/simple/mul1.sy
      -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/x86_cli_test.cmake)
endif()

# ---------------------------------------------------------------------------
# irprint -- standalone tool: read a .irb binary IR file and Print it
# ---------------------------------------------------------------------------
set(IRPRINT_TARGET_NAME irprint)
add_executable(${IRPRINT_TARGET_NAME} irprint_main.cxx)
target_include_directories(${IRPRINT_TARGET_NAME} PRIVATE
        ${OCC_MAIN_DIR}/common
        ${OCC_MAIN_DIR}/lib/subprocess
        ${OCC_MAIN_DIR}/lib)
# 仅链接 common (含 ir_io / ir_serial / FILE_MANAGER::Print)
target_link_libraries(${IRPRINT_TARGET_NAME} PRIVATE common)

INSTALL(TARGETS ${IRPRINT_TARGET_NAME}
  RUNTIME DESTINATION bin
  RENAME irprint
)
