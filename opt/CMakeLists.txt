cmake_minimum_required(VERSION 2.10)
project(fronend)

add_library(opt main.cxx)
target_compile_options(opt PRIVATE -g)
target_include_directories(opt PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib)