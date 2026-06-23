cmake_minimum_required(VERSION 3.11)
project(Driver)
cmake_policy(SET CMP0003 NEW)

# Set due to the fact that test system said so.
set(DRIVER_TARGET_NAME compiler)

add_compile_options(-DDebug_Mode=1 -std=c++11)
add_executable(${DRIVER_TARGET_NAME} main.cxx driver_options.cxx toolchain.cxx)
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
  add_executable(fake_toolchain fake_toolchain.cxx)
  add_test(NAME fake_toolchain_fixture_test
    COMMAND ${CMAKE_COMMAND}
      -DFAKE_TOOLCHAIN=$<TARGET_FILE:fake_toolchain>
      -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/fake_toolchain_test.cmake)

  add_executable(driver_options_test driver_options_test.cxx driver_options.cxx)
  target_include_directories(driver_options_test PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${OCC_MAIN_DIR}/common
          ${OCC_MAIN_DIR}/lib)
  add_test(NAME driver_options_test COMMAND driver_options_test)

  add_executable(driver_options_release_test
    driver_options_test.cxx driver_options.cxx)
  target_compile_definitions(driver_options_release_test PRIVATE NDEBUG)
  target_include_directories(driver_options_release_test PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${OCC_MAIN_DIR}/common
          ${OCC_MAIN_DIR}/lib)
  add_test(NAME driver_options_release_test COMMAND driver_options_release_test)

  add_test(NAME external_toolchain_options_test
    COMMAND ${CMAKE_COMMAND}
      -DCOMPILER=$<TARGET_FILE:${DRIVER_TARGET_NAME}>
      -DSOURCE=${OCC_MAIN_DIR}/testcase/simple/simple.c
      -DSECOND_SOURCE=${OCC_MAIN_DIR}/testcase/simple/expr.c
      -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/external_toolchain_options_test.cmake)

  add_test(NAME external_toolchain_no_execution_test
    COMMAND ${CMAKE_COMMAND}
      -DCOMPILER=$<TARGET_FILE:${DRIVER_TARGET_NAME}>
      -DFAKE_TOOLCHAIN=$<TARGET_FILE:fake_toolchain>
      -DSOURCE=${OCC_MAIN_DIR}/testcase/simple/simple.c
      -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/external_toolchain_no_execution_test.cmake)

  add_test(NAME external_toolchain_execution_test
    COMMAND ${CMAKE_COMMAND}
      -DCOMPILER=$<TARGET_FILE:${DRIVER_TARGET_NAME}>
      -DFAKE_TOOLCHAIN=$<TARGET_FILE:fake_toolchain>
      -DSOURCE=${OCC_MAIN_DIR}/testcase/simple/simple.c
      -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/external_toolchain_execution_test.cmake)

  if(APPLE)
    add_test(NAME real_macos_toolchain_test
      COMMAND ${CMAKE_COMMAND}
        -DCOMPILER=$<TARGET_FILE:${DRIVER_TARGET_NAME}>
        -DSOURCE=${OCC_MAIN_DIR}/testcase/simple/add2_main.sy
        -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/real_macos_toolchain_test.cmake)
  endif()

  add_executable(toolchain_test toolchain_test.cxx toolchain.cxx
    driver_options.cxx)
  target_include_directories(toolchain_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib)
  target_link_libraries(toolchain_test PRIVATE common)
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
