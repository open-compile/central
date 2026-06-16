cmake_minimum_required(VERSION 3.11)
project(OCC_CODEGEN)

add_library(cg cg_main.cxx cgir.cxx data_layout.cxx register.cxx tn.cxx targ_abi.cxx util.cxx cg_pass.cxx)
target_compile_options(cg PRIVATE -g)
target_include_directories(cg PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt)