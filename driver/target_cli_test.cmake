if(NOT DEFINED COMPILER OR NOT DEFINED FAKE_TOOLCHAIN OR
   NOT DEFINED SOURCE OR NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "COMPILER, FAKE_TOOLCHAIN, SOURCE, and OUTPUT_DIR are required")
endif()

execute_process(
  COMMAND "${COMPILER}" --help
  RESULT_VARIABLE help_result OUTPUT_VARIABLE help_output
  ERROR_VARIABLE help_error)
if(NOT help_result EQUAL 0)
  message(FATAL_ERROR "compiler --help failed: ${help_error}")
endif()

foreach(spelling IN ITEMS
    "armv7-linux-gnueabihf" "aarch64-linux-gnu" "arm64-apple-darwin"
    "i386-linux-gnu" "x86_64-linux-gnu" "x86_64-apple-darwin"
    "armv7" "arm32" "aarch64-linux" "arm64-linux" "arm64-macos"
    "x86-linux" "x64-linux" "x64-macos" "--target=<triple>"
    "--target=native" "--arch" "--march"
    "Default: armv7-linux-gnueabihf (ARMv7 Linux)."
    "-S" "-c" "default: link" "--toolchain=auto|required|off"
    "-clang/--clang" "-gcc/--gcc" "mutually exclusive"
    "forcing either family disables fallback" "-Wa,<args>"
    "-Ws,<args>" "-Wl,<args>" "--keep")
  string(FIND "${help_output}" "${spelling}" spelling_index)
  if(spelling_index EQUAL -1)
    message(FATAL_ERROR "compiler --help is missing '${spelling}'")
  endif()
endforeach()

foreach(stale IN ITEMS
    "currently implemented code generation target"
    "fail before emission until their builders/emitters are implemented"
    "only for the default ARMv7 toolchain")
  string(FIND "${help_output}" "${stale}" stale_index)
  if(NOT stale_index EQUAL -1)
    message(FATAL_ERROR "compiler --help retains stale text '${stale}'")
  endif()
endforeach()

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

if(DEFINED FAKE_TOOLCHAIN)
  set(fake_bin "${OUTPUT_DIR}/target-cli-fake-bin")
  set(fake_log "${OUTPUT_DIR}/target-cli-fake.log")
  file(REMOVE_RECURSE "${fake_bin}")
  file(REMOVE "${fake_log}")
  file(MAKE_DIRECTORY "${fake_bin}")
  foreach(alias IN ITEMS clang gcc)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink
                    "${FAKE_TOOLCHAIN}" "${fake_bin}/${alias}")
  endforeach()
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "PATH=${fake_bin}:$ENV{PATH}"
            "CENTRAL_FAKE_TOOL_LOG=${fake_log}"
            "${COMPILER}" -c -CG:lra=0 --target=x64-linux -gcc
            "${SOURCE}" -o "${OUTPUT_DIR}/target-x64.o"
    RESULT_VARIABLE object_result ERROR_VARIABLE object_error)
  if(NOT object_result EQUAL 0 OR NOT EXISTS "${OUTPUT_DIR}/target-x64.o")
    message(FATAL_ERROR "non-ARM -c failed: ${object_result}: ${object_error}")
  endif()
endif()
