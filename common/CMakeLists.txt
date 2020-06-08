cmake_minimum_required(VERSION 2.5)
project(LibCommon)
add_compile_options(-DDebug_Mode=1 -std=c++11)
include_directories(${OCC_MAIN_DIR}/lib/subprocess ${OCC_MAIN_DIR}/lib)
add_library(common STATIC basic.cxx options.cxx file_util.cxx symtab.cxx tree.cxx)

add_executable(symtab_test symtab_test.cxx)
target_link_libraries(symtab_test PUBLIC common)