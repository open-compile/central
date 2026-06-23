if(NOT DEFINED COMPILER OR NOT DEFINED SOURCE OR NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "COMPILER, SOURCE, and OUTPUT_DIR are required")
endif()

execute_process(
  COMMAND "${COMPILER}" -S -CG:lra=0 --target=native --arch=native
          "${SOURCE}" -o "${OUTPUT_DIR}/target-native.s"
  RESULT_VARIABLE native_result ERROR_VARIABLE native_error)
if(NOT native_result EQUAL 0)
  message(FATAL_ERROR "matching native target and arch failed: ${native_error}")
endif()

execute_process(
  COMMAND "${COMPILER}" -S -CG:lra=0 "${SOURCE}" -o "${OUTPUT_DIR}/target-default.s"
  RESULT_VARIABLE default_result ERROR_VARIABLE default_error)
if(NOT default_result EQUAL 0)
  message(FATAL_ERROR "default target failed: ${default_error}")
endif()

execute_process(
  COMMAND "${COMPILER}" -S -CG:lra=0 --arch=arm32 "${SOURCE}" -o "${OUTPUT_DIR}/target-arm32.s"
  RESULT_VARIABLE alias_result ERROR_VARIABLE alias_error)
if(NOT alias_result EQUAL 0)
  message(FATAL_ERROR "arm32 alias failed: ${alias_error}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E compare_files
          "${OUTPUT_DIR}/target-default.s" "${OUTPUT_DIR}/target-arm32.s"
  RESULT_VARIABLE compare_result)
if(NOT compare_result EQUAL 0)
  message(FATAL_ERROR "default and arm32 assembly differ")
endif()

execute_process(
  COMMAND "${COMPILER}" -S --target=mips "${SOURCE}" -o "${OUTPUT_DIR}/target-mips.s"
  RESULT_VARIABLE unknown_result ERROR_VARIABLE unknown_error)
if(NOT unknown_result EQUAL 3 OR NOT unknown_error MATCHES "unknown target 'mips'")
  message(FATAL_ERROR "unknown target diagnostic mismatch: ${unknown_result}: ${unknown_error}")
endif()

execute_process(
  COMMAND "${COMPILER}" -S --target=x64-linux --arch=armv7 "${SOURCE}" -o "${OUTPUT_DIR}/target-conflict.s"
  RESULT_VARIABLE conflict_result ERROR_VARIABLE conflict_error)
if(NOT conflict_result EQUAL 3 OR NOT conflict_error MATCHES "conflicting target options")
  message(FATAL_ERROR "conflicting target diagnostic mismatch: ${conflict_result}: ${conflict_error}")
endif()

execute_process(
  COMMAND "${COMPILER}" -c --target=x64-linux "${SOURCE}" -o "${OUTPUT_DIR}/target-x64.o"
  RESULT_VARIABLE object_result ERROR_VARIABLE object_error)
if(NOT object_result EQUAL 3 OR NOT object_error MATCHES "integrated -c is not supported")
  message(FATAL_ERROR "non-ARM -c diagnostic mismatch: ${object_result}: ${object_error}")
endif()
