set(arm64_program "${OUTPUT_DIR}/real-macos-arm64")
file(REMOVE "${arm64_program}")
execute_process(
  COMMAND "${COMPILER}" -CG:lra=0 --target=arm64-apple-darwin
          "${SOURCE}" -o "${arm64_program}"
  RESULT_VARIABLE compile_result ERROR_VARIABLE compile_error)
if(NOT compile_result EQUAL 0 OR NOT EXISTS "${arm64_program}")
  message(FATAL_ERROR "real arm64 link failed (${compile_result}): ${compile_error}")
endif()
execute_process(COMMAND /usr/bin/file "${arm64_program}"
                OUTPUT_VARIABLE file_output RESULT_VARIABLE file_result)
if(NOT file_result EQUAL 0 OR
   NOT file_output MATCHES "Mach-O 64-bit executable arm64")
  message(FATAL_ERROR "real arm64 output is not Mach-O: ${file_output}")
endif()
execute_process(COMMAND "${arm64_program}" RESULT_VARIABLE program_result)
if(NOT program_result EQUAL 3)
  message(FATAL_ERROR "real arm64 program returned ${program_result}, expected 3")
endif()

set(x86_program "${OUTPUT_DIR}/real-macos-x86_64")
file(REMOVE "${x86_program}")
execute_process(
  COMMAND "${COMPILER}" -CG:lra=0 --target=x86_64-apple-darwin
          "${SOURCE}" -o "${x86_program}"
  RESULT_VARIABLE x86_compile_result ERROR_VARIABLE x86_compile_error)
if(x86_compile_result EQUAL 0)
  execute_process(COMMAND /usr/bin/file "${x86_program}"
                  OUTPUT_VARIABLE x86_file_output RESULT_VARIABLE x86_file_result)
  if(NOT x86_file_result EQUAL 0 OR
     NOT x86_file_output MATCHES "Mach-O 64-bit executable x86_64")
    message(FATAL_ERROR "real x86_64 output is not Mach-O: ${x86_file_output}")
  endif()
  execute_process(COMMAND "${x86_program}" RESULT_VARIABLE x86_program_result)
  if(NOT x86_program_result EQUAL 3)
    message(FATAL_ERROR "real x86_64 program returned ${x86_program_result}, expected 3")
  endif()
endif()

file(GLOB hidden "${OUTPUT_DIR}/.add2_main.central-*")
if(hidden)
  message(FATAL_ERROR "real toolchain success left hidden work directories: ${hidden}")
endif()
