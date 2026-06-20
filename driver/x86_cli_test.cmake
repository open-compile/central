if(NOT DEFINED COMPILER OR NOT DEFINED SIMPLE_SOURCE OR
   NOT DEFINED ARGS_SOURCE OR NOT DEFINED GLOBALS_SOURCE OR
   NOT DEFINED DIV_SOURCE OR NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "x86 CLI test inputs are required")
endif()

find_program(CLANG_EXECUTABLE clang REQUIRED)

foreach(target IN ITEMS i386-linux-gnu x86_64-linux-gnu x86_64-apple-darwin)
  if(target STREQUAL "x86_64-apple-darwin")
    set(clang_target x86_64-apple-macos)
  else()
    set(clang_target ${target})
  endif()
  foreach(case_name IN ITEMS simple args globals div)
    if(case_name STREQUAL "simple")
      set(source "${SIMPLE_SOURCE}")
    elseif(case_name STREQUAL "args")
      set(source "${ARGS_SOURCE}")
    elseif(case_name STREQUAL "globals")
      set(source "${GLOBALS_SOURCE}")
    else()
      set(source "${DIV_SOURCE}")
    endif()
    set(assembly "${OUTPUT_DIR}/${target}-${case_name}.s")
    set(object "${OUTPUT_DIR}/${target}-${case_name}.o")
    execute_process(
      COMMAND "${COMPILER}" -S -CG:lra=0 --target=${target}
              "${source}" -o "${assembly}"
      RESULT_VARIABLE compiler_result ERROR_VARIABLE compiler_error)
    if(NOT compiler_result EQUAL 0)
      message(FATAL_ERROR "${target}/${case_name} compiler failure: ${compiler_error}")
    endif()
    execute_process(
      COMMAND "${CLANG_EXECUTABLE}" -target ${clang_target} -c
              "${assembly}" -o "${object}"
      RESULT_VARIABLE assembler_result ERROR_VARIABLE assembler_error)
    if(NOT assembler_result EQUAL 0)
      message(FATAL_ERROR "${target}/${case_name} assembler failure: ${assembler_error}")
    endif()
  endforeach()
endforeach()
