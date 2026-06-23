set(log "${OUTPUT_DIR}/fake-toolchain.log")
set(output "${OUTPUT_DIR}/fake-toolchain-output.o")
file(REMOVE "${log}" "${output}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${FAKE_TOOLCHAIN}" -target test-triple --version
  RESULT_VARIABLE version_result)
if(NOT version_result EQUAL 0 OR NOT EXISTS "${log}")
  message(FATAL_ERROR "fake tool --version contract failed")
endif()
file(REMOVE "${log}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${FAKE_TOOLCHAIN}" -target test-triple input.s -o "${output}"
  RESULT_VARIABLE run_result)
if(NOT run_result EQUAL 0 OR NOT EXISTS "${output}" OR NOT EXISTS "${log}")
  message(FATAL_ERROR "fake tool invocation did not log and create -o output")
endif()
file(READ "${log}" contents)
get_filename_component(fake_name "${FAKE_TOOLCHAIN}" NAME)
set(expected "${fake_name}\t-target\ttest-triple\tinput.s\t-o\t${output}\n")
if(NOT contents STREQUAL expected)
  message(FATAL_ERROR "unexpected fake tool log: ${contents}")
endif()

set(witness "${OUTPUT_DIR}/fake-toolchain-output-witness")
file(REMOVE "${output}" "${witness}")
file(WRITE "${output}" "placeholder\n")
execute_process(COMMAND "${CMAKE_COMMAND}" -E create_hardlink
                "${output}" "${witness}" RESULT_VARIABLE hardlink_result)
if(NOT hardlink_result EQUAL 0)
  message(FATAL_ERROR "could not create inode-replacement witness")
endif()
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${FAKE_TOOLCHAIN}" -c input.s -o "${output}"
  RESULT_VARIABLE replacement_result)
file(READ "${output}" replacement_contents)
file(READ "${witness}" witness_contents)
if(NOT replacement_result EQUAL 0 OR
   NOT replacement_contents STREQUAL "fake object\n" OR
   NOT witness_contents STREQUAL "placeholder\n")
  message(FATAL_ERROR "fake tool did not replace the output inode")
endif()
file(REMOVE "${witness}")

file(REMOVE "${output}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_OBJECT_EXIT=17"
          "${FAKE_TOOLCHAIN}" -c input.s -o "${output}"
  RESULT_VARIABLE object_result)
if(NOT object_result EQUAL 17 OR EXISTS "${output}")
  message(FATAL_ERROR "fake object failure control did not return 17")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_LINK_EXIT=19"
          "${FAKE_TOOLCHAIN}" input.o -o "${output}"
  RESULT_VARIABLE link_result)
if(NOT link_result EQUAL 19 OR EXISTS "${output}")
  message(FATAL_ERROR "fake link failure control did not return 19")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_SKIP_OUTPUT=1"
          "${FAKE_TOOLCHAIN}" -c input.s -o "${output}"
  RESULT_VARIABLE skip_result)
if(NOT skip_result EQUAL 0 OR EXISTS "${output}")
  message(FATAL_ERROR "fake skip-output control created an output")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CENTRAL_FAKE_TOOL_PROBE_EXIT=23"
          "${FAKE_TOOLCHAIN}" --version
  RESULT_VARIABLE probe_result)
if(NOT probe_result EQUAL 23)
  message(FATAL_ERROR "fake probe failure control did not return 23")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${FAKE_TOOLCHAIN}" -c input.s
  RESULT_VARIABLE missing_output_result)
if(missing_output_result EQUAL 0)
  message(FATAL_ERROR "fake normal invocation without -o unexpectedly passed")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_SKIP_OUTPUT=1"
          "${FAKE_TOOLCHAIN}" -c input.s
  RESULT_VARIABLE skip_missing_output_result)
if(skip_missing_output_result EQUAL 0)
  message(FATAL_ERROR "skip-output implicitly allowed a missing -o")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_ALLOW_MISSING_OUTPUT=1"
          "${FAKE_TOOLCHAIN}" -c input.s
  RESULT_VARIABLE allowed_missing_output_result)
if(NOT allowed_missing_output_result EQUAL 0)
  message(FATAL_ERROR "fake missing-output override was not honored")
endif()
