cmake_minimum_required(VERSION 3.11)
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
target_link_libraries(${DRIVER_TARGET_NAME} PRIVATE common fe opt cg cgnova)

INSTALL(TARGETS ${DRIVER_TARGET_NAME}
  RUNTIME DESTINATION bin
  RENAME compiler
)

# ---------------------------------------------------------------------------
# irprint -- standalone tool: read a .irb binary IR file and Print it
# ---------------------------------------------------------------------------
set(IRPRINT_TARGET_NAME irprint)
add_executable(${IRPRINT_TARGET_NAME} irprint_main.cxx)
target_include_directories(${IRPRINT_TARGET_NAME} PRIVATE
        ${OCC_MAIN_DIR}/common
        ${OCC_MAIN_DIR}/lib/subprocess
        ${OCC_MAIN_DIR}/lib)
# 仅链接 common (含 ir_io / ir_serial / FILE_MANAGER::Print)
target_link_libraries(${IRPRINT_TARGET_NAME} PRIVATE common)

INSTALL(TARGETS ${IRPRINT_TARGET_NAME}
  RUNTIME DESTINATION bin
  RENAME irprint
)