project(jsoncppamal)
cmake_minimum_required(VERSION 2.9)
add_library(jsoncppamal jsoncpp.cpp)
target_compile_options(jsoncppamal PRIVATE -std=c++14)
