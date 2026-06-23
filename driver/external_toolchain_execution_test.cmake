set(fake_bin "${OUTPUT_DIR}/execution-fake-bin")
set(log "${OUTPUT_DIR}/external-toolchain-execution.log")
file(REMOVE_RECURSE "${fake_bin}")
file(MAKE_DIRECTORY "${fake_bin}")
foreach(alias clang gcc aarch64-linux-gnu-gcc arm-linux-gnueabihf-gcc)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink
                  "${FAKE_TOOLCHAIN}" "${fake_bin}/${alias}")
endforeach()
file(GLOB stale_hidden "${OUTPUT_DIR}/.simple.central-*")
file(REMOVE ${stale_hidden})

function(run_compiler result_var error_var)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "PATH=${fake_bin}:$ENV{PATH}"
            "CENTRAL_FAKE_TOOL_LOG=${log}"
            ${ARGN}
    RESULT_VARIABLE result ERROR_VARIABLE error)
  set(${result_var} "${result}" PARENT_SCOPE)
  set(${error_var} "${error}" PARENT_SCOPE)
endfunction()

set(object "${OUTPUT_DIR}/execution.o")
file(REMOVE "${log}" "${object}")
run_compiler(result error "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu
             -gcc "${SOURCE}" -o "${object}")
if(NOT result EQUAL 0 OR NOT EXISTS "${object}")
  message(FATAL_ERROR "object pipeline failed (${result}): ${error}")
endif()
file(GLOB hidden "${OUTPUT_DIR}/.simple.central-*")
if(hidden)
  message(FATAL_ERROR "ordinary object success left hidden intermediates: ${hidden}")
endif()
file(READ "${log}" contents)
string(REGEX MATCHALL "[^\n]+" lines "${contents}")
list(LENGTH lines count)
if(NOT count EQUAL 2)
  message(FATAL_ERROR "expected probe + object, got: ${contents}")
endif()

set(program "${OUTPUT_DIR}/execution-program")
file(REMOVE "${log}" "${program}")
run_compiler(result error "${COMPILER}" -CG:lra=0 --target=x86_64-linux-gnu
             -gcc before.a -Wl,--start-group -Wa,-g -Ws,--fatal-warnings
             "${SOURCE}" -Wl,--end-group after.a
             -o "${program}")
if(NOT result EQUAL 0 OR NOT EXISTS "${program}")
  message(FATAL_ERROR "link pipeline failed (${result}): ${error}")
endif()
file(GLOB hidden "${OUTPUT_DIR}/.simple.central-*")
if(hidden)
  message(FATAL_ERROR "ordinary link success left hidden intermediates: ${hidden}")
endif()
file(READ "${log}" contents)
string(REGEX MATCHALL "[^\n]+" lines "${contents}")
list(LENGTH lines count)
if(NOT count EQUAL 3)
  message(FATAL_ERROR "expected probe + object + link, got: ${contents}")
endif()
list(GET lines 1 object_line)
list(GET lines 2 link_line)
if(NOT object_line MATCHES "-Wa,-g" OR
   NOT object_line MATCHES "-Wa,--fatal-warnings" OR
   object_line MATCHES "-Wl," OR link_line MATCHES "-Wa," OR
   NOT link_line MATCHES "before.a.*--start-group.*central-.*[.]o.*--end-group.*after.a")
  message(FATAL_ERROR "phase/order log mismatch: ${contents}")
endif()

file(REMOVE "${log}" "${object}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_PROBE_FAIL_PROGRAM=gcc"
          "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu "${SOURCE}"
          -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(NOT result EQUAL 0 OR NOT EXISTS "${object}")
  message(FATAL_ERROR "auto fallback failed (${result}): ${error}")
endif()
file(READ "${log}" contents)
if(NOT contents MATCHES "gcc.*--version.*clang.*--version.*clang.*-c")
  message(FATAL_ERROR "auto fallback order mismatch: ${contents}")
endif()

file(REMOVE "${log}" "${object}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_PROBE_FAIL_PROGRAM=gcc"
          "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu -gcc "${SOURCE}"
          -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR error MATCHES "clang.*-c")
  message(FATAL_ERROR "forced GCC unexpectedly fell back: ${error}")
endif()

file(REMOVE "${log}" "${object}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_OBJECT_EXIT=17"
          "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu -gcc "${SOURCE}"
          -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(NOT result EQUAL 17 OR
   NOT error MATCHES "target=x86_64-linux-gnu" OR
   NOT error MATCHES "phase=object" OR NOT error MATCHES "argv=\\[" OR
   NOT error MATCHES "status=17")
  message(FATAL_ERROR "object failure contract mismatch (${result}): ${error}")
endif()
file(GLOB retained "${OUTPUT_DIR}/.simple.central-*.s"
                         "${OUTPUT_DIR}/.simple.central-*.o")
if(NOT retained)
  message(FATAL_ERROR "failed object stage did not retain intermediates")
endif()
file(REMOVE ${retained})

file(REMOVE "${log}" "${program}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_LINK_SKIP_OUTPUT=1"
          "${COMPILER}" -CG:lra=0 --target=x86_64-linux-gnu -gcc
          "${SOURCE}" -o "${program}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "phase=link" OR
   NOT error MATCHES "missing output")
  message(FATAL_ERROR "missing link output was not rejected: ${error}")
endif()
file(GLOB retained "${OUTPUT_DIR}/.simple.central-*")
file(REMOVE ${retained})

file(REMOVE "${log}" "${program}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_LINK_EXIT=19"
          "${COMPILER}" -CG:lra=0 --target=x86_64-linux-gnu -gcc "${SOURCE}"
          -o "${program}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(NOT result EQUAL 19 OR NOT error MATCHES "target=x86_64-linux-gnu" OR
   NOT error MATCHES "phase=link" OR NOT error MATCHES "argv=\\[" OR
   NOT error MATCHES "status=19")
  message(FATAL_ERROR "link failure contract mismatch (${result}): ${error}")
endif()
file(GLOB retained "${OUTPUT_DIR}/.simple.central-*")
if(NOT retained)
  message(FATAL_ERROR "failed link stage did not retain intermediates")
endif()
file(REMOVE ${retained})

file(REMOVE "${object}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_OBJECT_SKIP_OUTPUT=1"
          "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu -gcc "${SOURCE}"
          -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "missing output")
  message(FATAL_ERROR "missing object output was not rejected: ${error}")
endif()
file(GLOB retained "${OUTPUT_DIR}/.simple.central-*")
if(NOT retained)
  message(FATAL_ERROR "missing-output failure did not retain intermediates")
endif()
file(REMOVE ${retained})

set(keep_program "${OUTPUT_DIR}/keep-program")
file(REMOVE "${keep_program}" "${OUTPUT_DIR}/simple.s" "${OUTPUT_DIR}/simple.o")
run_compiler(result error "${COMPILER}" --keep -CG:lra=0 --target=x86_64-linux-gnu
             -gcc "${SOURCE}" -o "${keep_program}")
if(NOT result EQUAL 0 OR NOT EXISTS "${keep_program}" OR
   NOT EXISTS "${OUTPUT_DIR}/simple.s" OR NOT EXISTS "${OUTPUT_DIR}/simple.o")
  message(FATAL_ERROR "--keep publication failed (${result}): ${error}")
endif()
file(GLOB hidden "${OUTPUT_DIR}/.simple.central-*")
if(hidden)
  message(FATAL_ERROR "success left hidden intermediates: ${hidden}")
endif()

set(keep_object "${OUTPUT_DIR}/keep-object.o")
file(REMOVE "${keep_object}" "${OUTPUT_DIR}/simple.s" "${OUTPUT_DIR}/simple.o")
run_compiler(result error "${COMPILER}" -c --keep -CG:lra=0
             --target=x86_64-linux-gnu -gcc "${SOURCE}" -o "${keep_object}")
if(NOT result EQUAL 0 OR NOT EXISTS "${keep_object}" OR
   NOT EXISTS "${OUTPUT_DIR}/simple.s" OR EXISTS "${OUTPUT_DIR}/simple.o")
  message(FATAL_ERROR "object --keep publication failed (${result}): ${error}")
endif()
file(GLOB hidden "${OUTPUT_DIR}/.simple.central-*")
if(hidden)
  message(FATAL_ERROR "object success left hidden intermediates: ${hidden}")
endif()

file(REMOVE "${log}" "${object}")
run_compiler(result error "${COMPILER}" -c --show -CG:lra=0
             --target=x86_64-linux-gnu -gcc "${SOURCE}" -o "${object}")
if(NOT result EQUAL 0 OR NOT error MATCHES "phase=object.*argv=")
  message(FATAL_ERROR "--show did not expose selected command: ${error}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "PATH=${fake_bin}:$ENV{PATH}"
          "${COMPILER}" -c -CG:lra=0 --target=arm64-apple-darwin -gcc
          "${SOURCE}" -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "GNU GCC is not supported for Apple target arm64-apple-darwin")
  message(FATAL_ERROR "Apple forced-GCC diagnostic mismatch: ${error}")
endif()

file(REMOVE "${log}" "${object}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_MASQUERADE_PROGRAM=gcc"
          "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu
          "${SOURCE}" -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "family-aware auto fallback failed (${result}): ${error}")
endif()
file(READ "${log}" contents)
if(NOT contents MATCHES "gcc.*--version.*clang.*--version.*clang.*-c")
  message(FATAL_ERROR "masquerading GCC was not rejected: ${contents}")
endif()

file(REMOVE "${log}" "${object}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_MASQUERADE_PROGRAM=gcc"
          "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu -gcc
          "${SOURCE}" -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "family-identity-mismatch" OR
   error MATCHES "clang.*-c")
  message(FATAL_ERROR "forced GCC accepted Clang identity: ${error}")
endif()

file(REMOVE "${log}" "${object}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "CENTRAL_FAKE_TOOL_IDENTITY=gcc"
          "${COMPILER}" -c -CG:lra=0 --target=x86_64-linux-gnu -clang
          "${SOURCE}" -o "${object}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "family-identity-mismatch" OR
   error MATCHES "gcc.*-c")
  message(FATAL_ERROR "forced Clang accepted GNU identity: ${error}")
endif()

file(GLOB stale_hidden "${OUTPUT_DIR}/.simple.central-*")
file(REMOVE ${stale_hidden})
file(REMOVE "${log}")
set(feonly_output "${OUTPUT_DIR}/feonly.o")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "PATH=${fake_bin}:$ENV{PATH}"
          "CENTRAL_FAKE_TOOL_LOG=${log}"
          "${COMPILER}" -c --feonly -CG:lra=0 --target=x86_64-linux-gnu
          "${SOURCE}" -o "${feonly_output}"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(NOT result EQUAL 0 OR EXISTS "${feonly_output}")
  message(FATAL_ERROR "--feonly compatibility failed (${result}): ${error}")
endif()
if(EXISTS "${log}")
  message(FATAL_ERROR "--feonly unexpectedly executed an external tool")
endif()
file(GLOB hidden "${OUTPUT_DIR}/.simple.central-*")
if(hidden)
  message(FATAL_ERROR "--feonly leaked reserved intermediates: ${hidden}")
endif()
