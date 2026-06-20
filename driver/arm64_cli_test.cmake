if(NOT DEFINED COMPILER OR NOT DEFINED SIMPLE_SOURCE OR
   NOT DEFINED ARGS_SOURCE OR NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "Arm64 CLI test inputs are required")
endif()

find_program(CLANG_EXECUTABLE clang REQUIRED)

foreach(target IN ITEMS aarch64-linux-gnu arm64-apple-darwin)
  foreach(case_name IN ITEMS simple args)
    if(case_name STREQUAL "simple")
      set(source "${SIMPLE_SOURCE}")
    else()
      set(source "${ARGS_SOURCE}")
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

    if(target STREQUAL "arm64-apple-darwin")
      set(clang_target arm64-apple-macos)
    else()
      set(clang_target aarch64-linux-gnu)
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

execute_process(
  COMMAND "${COMPILER}" -c --target=arm64-apple-darwin
          "${SIMPLE_SOURCE}" -o "${OUTPUT_DIR}/arm64-integrated.o"
  RESULT_VARIABLE object_result ERROR_VARIABLE object_error)
if(NOT object_result EQUAL 3 OR NOT object_error MATCHES "integrated -c is not supported")
  message(FATAL_ERROR "Arm64 integrated -c diagnostic mismatch: ${object_result}: ${object_error}")
endif()
