cmake_minimum_required(VERSION 3.2)
project(OCC_OPTIMIZER)

add_library(opt opt_main.cxx opt_ssa.cxx)
target_compile_options(opt PRIVATE -g)
target_include_directories(opt PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/cg)