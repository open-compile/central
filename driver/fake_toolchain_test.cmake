set(log "${OUTPUT_DIR}/fake-toolchain.log")
set(output "${OUTPUT_DIR}/fake-toolchain-output.o")
file(REMOVE "${log}" "${output}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${FAKE_TOOLCHAIN}" -target test-triple --version
  RESULT_VARIABLE version_result)
if(NOT version_result EQUAL 0 OR EXISTS "${log}")
  message(FATAL_ERROR "fake tool --version contract failed")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${FAKE_TOOLCHAIN}" -target test-triple input.s -o "${output}"
  RESULT_VARIABLE run_result)
if(NOT run_result EQUAL 0 OR NOT EXISTS "${output}" OR NOT EXISTS "${log}")
  message(FATAL_ERROR "fake tool invocation did not log and create -o output")
endif()
file(READ "${log}" contents)
if(NOT contents MATCHES "fake_toolchain.*-target.*test-triple.*input.s.*-o.*fake-toolchain-output.o")
  message(FATAL_ERROR "unexpected fake tool log: ${contents}")
endif()
