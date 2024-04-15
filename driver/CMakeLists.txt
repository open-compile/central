cmake_minimum_required(VERSION 3.2)
project(Driver)
cmake_policy(SET CMP0003 NEW)

# Set due to the fact that test system said so.
set(DRIVER_TARGET_NAME compiler)

add_compile_options(-DDebug_Mode=1 -std=c++11)
add_executable(${DRIVER_TARGET_NAME} main.cxx)
target_include_directories(${DRIVER_TARGET_NAME} PRIVATE
        ${OCC_MAIN_DIR}/common
        ${OCC_MAIN_DIR}/lib/subprocess
        ${OCC_MAIN_DIR}/lib)
target_link_libraries(${DRIVER_TARGET_NAME} PRIVATE common fe opt)

INSTALL(TARGETS ${DRIVER_TARGET_NAME}
  RUNTIME DESTINATION bin
  RENAME compiler
)