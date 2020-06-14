//
// Created by xc5 on 2020/6/14.
//
#include "basic.h"
#include "options.h"
#include "fe_export.h"

INT32 main(INT32 argc, char **argv, char **envp) {
  COMPILER_CONFIG config;
  // Parse_args(argc, argv, envp, config);
  AssertThat(argc >= 2, ("Usage: fe_test <file>"));
  config.files.push_back(std::string(argv[1]));
  if (config.files.size() > 0) {
    Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO), (TFile, "Processing file : %s", config.files[0].c_str()));
    femain(config, *File(), config.files[0].c_str());
  } else {
    Comp_Failure("No file to process.");
  }
  return 0;
}