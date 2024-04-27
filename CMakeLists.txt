cmake_minimum_required(VERSION 3.3)
project(OCC)

message(STATUS "=================================")
message(STATUS "   Start to build OCC Compiler   ")
message(STATUS "=================================")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

set(OCC_MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(GLOBAL_OCC_LIB
  lib
  common
  fe
  opt
  cg
  cgnova
)

set(GLOBAL_OCC_TARG
  driver
)



foreach(ONE_TARG IN LISTS GLOBAL_OCC_LIB)
  add_subdirectory(${ONE_TARG})
endforeach()

foreach(ONE_TARG IN LISTS GLOBAL_OCC_TARG)
  add_subdirectory(${ONE_TARG})
endforeach()

message(STATUS "CMake install destination: " ${CMAKE_INSTALL_PREFIX} ".")