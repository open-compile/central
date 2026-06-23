if(NOT DEFINED COMPILER OR NOT DEFINED FAKE_TOOLCHAIN OR
   NOT DEFINED SIMPLE_SOURCE OR
   NOT DEFINED ARGS_SOURCE OR NOT DEFINED GLOBALS_SOURCE OR
   NOT DEFINED DIV_SOURCE OR NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "x86 CLI test inputs are required")
endif()

find_program(CLANG_EXECUTABLE clang)

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
    if(CLANG_EXECUTABLE)
      execute_process(
        COMMAND "${CLANG_EXECUTABLE}" -target ${clang_target} -c
                "${assembly}" -o "${object}"
        RESULT_VARIABLE assembler_result ERROR_VARIABLE assembler_error)
      if(NOT assembler_result EQUAL 0)
        message(FATAL_ERROR "${target}/${case_name} assembler failure: ${assembler_error}")
      endif()
    endif()
  endforeach()
endforeach()

if(DEFINED FAKE_TOOLCHAIN)
  set(fake_bin "${OUTPUT_DIR}/x86-cli-fake-bin")
  set(fake_log "${OUTPUT_DIR}/x86-cli-fake.log")
  file(REMOVE_RECURSE "${fake_bin}")
  file(REMOVE "${fake_log}")
  file(MAKE_DIRECTORY "${fake_bin}")
  foreach(alias IN ITEMS clang gcc)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink
                    "${FAKE_TOOLCHAIN}" "${fake_bin}/${alias}")
  endforeach()
  foreach(target IN ITEMS i386-linux-gnu x86_64-linux-gnu x86_64-apple-darwin)
    set(object "${OUTPUT_DIR}/${target}-integrated.o")
    if(target STREQUAL "x86_64-apple-darwin")
      set(family -clang)
    else()
      set(family -gcc)
    endif()
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E env "PATH=${fake_bin}:$ENV{PATH}"
              "CENTRAL_FAKE_TOOL_LOG=${fake_log}"
              "${COMPILER}" -c -CG:lra=0 --target=${target} ${family}
              "${SIMPLE_SOURCE}" -o "${object}"
      RESULT_VARIABLE object_result ERROR_VARIABLE object_error)
    if(NOT object_result EQUAL 0 OR NOT EXISTS "${object}")
      message(FATAL_ERROR "${target} -c failed: ${object_result}: ${object_error}")
    endif()
  endforeach()
endif()
