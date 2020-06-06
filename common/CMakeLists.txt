cmake_minimum_required(VERSION 2.5)
project(LibCommon)
add_compile_options(-DDebug_Mode=1 -std=c++11)
add_library(common STATIC basic.cxx options.cxx file_util.cxx)