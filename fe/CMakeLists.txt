cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0003 NEW)
project(fronend)


set(FE_OBJS
        fe
        fe_test)

add_library(fe fe_main.cxx grammar.cpp token.cpp)
add_executable(fe_test fe_main.cxx grammar.cpp token.cpp fe_standalone.cxx)

foreach(ONE_TARG IN LISTS FE_OBJS)
    target_compile_options(${ONE_TARG} PUBLIC -g)
    target_include_directories(${ONE_TARG} PRIVATE
            ${OCC_MAIN_DIR}/lib/jsoncpp/include
            ${OCC_MAIN_DIR}/common
            ${OCC_MAIN_DIR}/lib)
    target_link_libraries(${ONE_TARG} PRIVATE -L/usr/local/lib jsoncpp)
endforeach()

target_link_libraries(fe_test PUBLIC common)
