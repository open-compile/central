cmake_minimum_required(VERSION 3.2)
project(OCC_CODEGEN)

add_library(cg cg_nova.cxx)
target_compile_options(cg PRIVATE -g)
target_include_directories(cg PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt)