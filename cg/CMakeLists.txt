cmake_minimum_required(VERSION 3.11)
project(OCC_CODEGEN)

add_library(cg cg_main.cxx cgir.cxx data_layout.cxx register.cxx tn.cxx targ_abi.cxx target_configure.cxx target_backend.cxx armv7_linux_target.cxx arm64_target.cxx x86_target.cxx util.cxx cg_pass.cxx cg_builder.cxx lra.cxx)
target_compile_options(cg PRIVATE -g)
target_include_directories(cg PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt)

if(BUILD_TESTING)
  add_executable(target_info_test target_info_test.cxx target_configure.cxx)
  target_include_directories(target_info_test PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib)
  target_link_libraries(target_info_test PRIVATE common)
  add_test(NAME target_info_test COMMAND target_info_test)

  # Default 模拟 x86_64 + ILP32 变体。可通过 -DSIMULATE_HOST_AS_PLATFORM=... 覆盖。
  # 格式：<arch>-<os>[+<variant>...]，由 common/target.cxx::Parse_host_desc 解析。
  set(SIMULATE_HOST_AS_PLATFORM "x86_64-linux+ilp32" CACHE STRING
    "Platform name to simulate for native_ilp32_target_test (default x86_64-linux+ilp32)")

  # 输入校验：CMake 列表、shell 引号、反斜杠都可能让 SIMULATE_HOST_AS_PLATFORM
  # 传到编译器时变形，事先拒绝。
  foreach(_bad_char ";" "\"" "\\")
    string(FIND "${SIMULATE_HOST_AS_PLATFORM}" "${_bad_char}" _idx)
    if(NOT _idx EQUAL -1)
      message(FATAL_ERROR
        "SIMULATE_HOST_AS_PLATFORM may not contain '${_bad_char}' "
        "(got: '${SIMULATE_HOST_AS_PLATFORM}')")
    endif()
  endforeach()

  message(STATUS
    "native_ilp32_target_test: simulating host=${SIMULATE_HOST_AS_PLATFORM}")

  add_executable(native_ilp32_target_test native_ilp32_target_test.cxx
    ${OCC_MAIN_DIR}/common/target.cxx)
  target_compile_definitions(native_ilp32_target_test PRIVATE
    SIMULATE_HOST_AS_PLATFORM="${SIMULATE_HOST_AS_PLATFORM}")
  target_include_directories(native_ilp32_target_test PRIVATE
    ${OCC_MAIN_DIR}/common)
  add_test(NAME native_ilp32_target_test COMMAND native_ilp32_target_test)

  # Direct unit test for Parse_host_desc — pure C++, no simulation needed.
  add_executable(host_desc_test
    ${OCC_MAIN_DIR}/common/host_desc_test.cxx
    ${OCC_MAIN_DIR}/common/target.cxx)
  target_include_directories(host_desc_test PRIVATE
    ${OCC_MAIN_DIR}/common)
  add_test(NAME host_desc_test COMMAND host_desc_test)

  add_executable(data_layout_target_test data_layout_target_test.cxx)
  target_include_directories(data_layout_target_test PRIVATE
    ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt)
  target_link_libraries(data_layout_target_test PRIVATE cg common)
  add_test(NAME data_layout_target_test COMMAND data_layout_target_test)

  add_executable(armv7_linux_target_test armv7_linux_target_test.cxx)
  target_include_directories(armv7_linux_target_test PRIVATE
    ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt)
  target_link_libraries(armv7_linux_target_test PRIVATE cg common)
  add_test(NAME armv7_linux_target_test COMMAND armv7_linux_target_test)

  add_executable(arm64_target_test arm64_target_test.cxx)
  target_include_directories(arm64_target_test PRIVATE
    ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt)
  target_link_libraries(arm64_target_test PRIVATE cg common)
  add_test(NAME arm64_target_test COMMAND arm64_target_test)

  add_executable(x86_target_test x86_target_test.cxx)
  target_include_directories(x86_target_test PRIVATE
    ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt)
  target_link_libraries(x86_target_test PRIVATE cg common)
  add_test(NAME x86_target_test COMMAND x86_target_test)
endif()
