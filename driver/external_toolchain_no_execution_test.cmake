set(fake_bin "${OUTPUT_DIR}/fake-toolchain-bin")
set(log "${OUTPUT_DIR}/fake-toolchain-no-execution.log")
file(REMOVE_RECURSE "${fake_bin}")
file(MAKE_DIRECTORY "${fake_bin}")
file(REMOVE "${log}")

foreach(alias IN ITEMS clang gcc arm-linux-gnueabihf-gcc aarch64-linux-gnu-gcc)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E create_symlink "${FAKE_TOOLCHAIN}"
            "${fake_bin}/${alias}"
    RESULT_VARIABLE alias_result)
  if(NOT alias_result EQUAL 0)
    message(FATAL_ERROR "could not create fake tool alias ${alias}")
  endif()
endforeach()

set(clang_output "${OUTPUT_DIR}/fake-path-clang.s")
set(gcc_output "${OUTPUT_DIR}/fake-path-cross-gcc.s")
file(REMOVE "${clang_output}" "${gcc_output}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${COMPILER}" -S -CG:lra=0 -clang "${SOURCE}" -o "${clang_output}"
  RESULT_VARIABLE clang_result
  OUTPUT_VARIABLE clang_stdout
  ERROR_VARIABLE clang_stderr)
if(NOT clang_result EQUAL 0 OR NOT EXISTS "${clang_output}")
  message(FATAL_ERROR
    "forced Clang -S failed (${clang_result}): ${clang_stdout}${clang_stderr}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${COMPILER}" -S -CG:lra=0 -gcc --target=armv7-linux-gnueabihf
          "${SOURCE}" -o "${gcc_output}"
  RESULT_VARIABLE gcc_result
  OUTPUT_VARIABLE gcc_stdout
  ERROR_VARIABLE gcc_stderr)
if(NOT gcc_result EQUAL 0 OR NOT EXISTS "${gcc_output}")
  message(FATAL_ERROR
    "forced cross-GCC -S failed (${gcc_result}): ${gcc_stdout}${gcc_stderr}")
endif()

if(EXISTS "${log}")
  file(READ "${log}" contents)
  message(FATAL_ERROR "-S unexpectedly executed a fake tool: ${contents}")
endif()
