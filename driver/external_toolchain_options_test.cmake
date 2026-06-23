function(run_success name)
  execute_process(
    COMMAND "${COMPILER}" ${ARGN}
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${name} failed (${result}): ${stdout}${stderr}")
  endif()
endfunction()

function(run_option_error name expected)
  execute_process(
    COMMAND "${COMPILER}" ${ARGN}
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr)
  if(NOT result EQUAL 3 OR NOT "${stdout}${stderr}" MATCHES "${expected}")
    message(FATAL_ERROR
      "${name} did not produce expected option error (${result}): ${stdout}${stderr}")
  endif()
endfunction()

set(preprocess_output "${OUTPUT_DIR}/preprocess-only.out")
file(REMOVE "${preprocess_output}" "${OUTPUT_DIR}/expr.s"
            "${OUTPUT_DIR}/expr.o" "${OUTPUT_DIR}/a.out")
file(GLOB old_preprocess_intermediates "${OUTPUT_DIR}/.expr.central-*")
file(REMOVE ${old_preprocess_intermediates})
run_success(preprocess_only
  -E -CG:lra=0 --toolchain=off "${SECOND_SOURCE}" -o "${preprocess_output}")
file(GLOB preprocess_intermediates "${OUTPUT_DIR}/.expr.central-*")
if(EXISTS "${preprocess_output}" OR EXISTS "${OUTPUT_DIR}/expr.s" OR
   EXISTS "${OUTPUT_DIR}/expr.o" OR EXISTS "${OUTPUT_DIR}/a.out" OR
   preprocess_intermediates)
  message(FATAL_ERROR "-E created a compiler output or intermediate")
endif()

set(explicit_output "${OUTPUT_DIR}/external-options-explicit.s")
file(REMOVE "${explicit_output}")
run_success(recognizes_external_options
  -S -CG:lra=0 --toolchain=off -clang -Wa,-g,-Iinc -Ws,--fatal-warnings
  -Wl,-Map,out.map -L/sdk/lib -lfoo "${SOURCE}" helper.o libextra.a
  -o "${explicit_output}")
if(NOT EXISTS "${explicit_output}")
  message(FATAL_ERROR "-S -o did not create ${explicit_output}")
endif()

set(default_output "${OUTPUT_DIR}/simple.s")
file(REMOVE "${default_output}")
run_success(default_assembly_path -S -CG:lra=0 --gcc "${SOURCE}")
if(NOT EXISTS "${default_output}")
  message(FATAL_ERROR "-S default did not create ${default_output}")
endif()

run_option_error(mutually_exclusive_families "mutually exclusive"
  -S -clang -gcc "${SOURCE}")
run_option_error(invalid_toolchain_policy "invalid --toolchain value"
  -S --toolchain=sometimes "${SOURCE}")
run_option_error(malformed_assembler_group "empty pass-through"
  -S -Wa, "${SOURCE}")
run_option_error(malformed_linker_group "empty pass-through"
  -S -Wl,-Map,,out.map "${SOURCE}")
run_option_error(toolchain_off_object "--toolchain=off"
  -c --toolchain=off "${SOURCE}")
run_option_error(toolchain_off_link "--toolchain=off"
  --toolchain=off "${SOURCE}")
run_option_error(multiple_sources "exactly one source"
  -S "${SOURCE}" "${SECOND_SOURCE}")
