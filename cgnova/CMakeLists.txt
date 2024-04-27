cmake_minimum_required(VERSION 3.2)
project(OCC_CODEGEN_NOVA)

add_library(cgnova cg_nova.cxx)
target_compile_options(cgnova PRIVATE -g)
target_include_directories(cgnova PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/opt ${OCC_MAIN_DIR}/cg)