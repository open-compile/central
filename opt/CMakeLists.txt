cmake_minimum_required(VERSION 3.11)
project(OCC_OPTIMIZER)

add_library(opt
  opt_main.cxx
  opt_ssa.cxx
  opt_coderep.cxx
  opt_stmt.cxx
  opt_stab.cxx
  opt_cfg_builder.cxx
  opt_dom.cxx
  opt_dce.cxx
  opt_cprop.cxx
)
target_compile_options(opt PRIVATE -g)
target_include_directories(opt PRIVATE ${OCC_MAIN_DIR}/common ${OCC_MAIN_DIR}/lib ${OCC_MAIN_DIR}/cg)