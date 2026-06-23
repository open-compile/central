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
