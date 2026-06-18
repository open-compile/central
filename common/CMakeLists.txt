cmake_minimum_required(VERSION 3.11)
project(LibCommon)
add_compile_options(-DDebug_Mode=1 -std=c++11)
include_directories(${OCC_MAIN_DIR}/lib/subprocess ${OCC_MAIN_DIR}/lib)
add_library(common STATIC basic.cxx options.cxx file_util.cxx symtab.cxx tree.cxx symtab_access.cxx arithm_util.cxx
        cfg_common.cxx ir_io.cxx ir_serial.cxx timing.cxx)

add_compile_options(-DDebug_Mode=1 -std=c++11)
add_executable(symtab_test symtab_test.cxx)
target_link_libraries(symtab_test PUBLIC common)
