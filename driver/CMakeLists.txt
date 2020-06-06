cmake_minimum_required(VERSION 2.5)
project(Driver)

add_compile_options(-DDebug_Mode=1 -std=c++11)
add_executable(driver main.cxx)
target_include_directories(driver PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib/subprocess ${OCC_MAIN_DIR}/lib)
target_link_libraries(driver PRIVATE common fe opt)