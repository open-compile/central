#include "basic.h"
#include "host.h"
#include "options.h"
#include "be_export.h"
#include "file_util.h"
#include <fstream>
#include "symtab.h"
#include "opt_main.h"

INT32 BE_MAIN_NAME(INT32 argc, char **argv) {
  AssertThat(argc > 1, ("not enough arguments"));
  return 0;
}

INT32 BE_EXTERNAL_MAIN_NAME(COMPILER_CONFIG &conf) {
  AssertThat(conf.opt_level >= 0, ("not enough compile level"));
  Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
           (TFile, "Writing assembly to %s\n", conf.output_file.c_str()));
  AssertThat(conf.output_file.size() > 0, ("Incorrect output file name"));
  if (File_exists(conf.output_file)) {
    // delete the file if it exists
    Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
             (TFile, "Removing old output file under %s\n", conf.output_file.c_str()));
    if (remove(conf.output_file.c_str()) != 0) {
      Comp_Failure("Cannot delete file : %s", conf.output_file.c_str());
    }
  }
  // Run emitting of assembly code.
  FILE *output_assembly_file = fopen(conf.output_file.c_str(), "w+");
  if (!output_assembly_file) {
    Comp_Failure("Cannot open output file to write = %s",
                 conf.output_file.c_str());
  }
  Emit_section_data(output_assembly_file, File());
  if (fclose(output_assembly_file) != 0) {
    Comp_Failure("Cannot close output file to write = %s",
                 conf.output_file.c_str());
  }
  return 0;
}

INT32 Emit_section_data(FILE *out, FILE_MANAGER *manager) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (TFile, "%sEmitting section: data\n%s", DBAR, DBAR));
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Debugging info enabled, writing file-level data section\n"));
  fprintf(out, ".section data\n\n");
  for (UINT32 it = 1; it < manager->Tables()->Sym()->Length(); it++) {
    ST_IDX new_idx = (it << 8) + 1;
    if (ST_st(new_idx) != NULL && ST_st(new_idx)->sym_class == SYM_CLASS_VAR) {
      Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# # Variable ST_IDX = %d, name = %s \n",  new_idx, ST_name(new_idx)));
      fprintf(out, "%s: \n", ST_name(new_idx));
      for (INT32 curs = 0; curs < TY_size(ST_ty(new_idx)); curs ++) {
        fprintf(out, ".byte 0x0\n");
      }
    }
  }
  return 0;
}

INT32 Emit_function() {
  return 0;
}