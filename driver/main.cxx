#include <stdio.h>
#include <iostream>
#include "basic.h"
#include "options.h"
#include "fe_export.h"
#include "be_export.h"
#include "main.h"
#include "file_util.h"
#include <set>
#include <vector>
#include <string>
#include <string.h>
#include "ir.h"

#ifdef SUBPROCESS_ENABLED
#include "subprocess.h" // Unlicense
#endif

/**
 * Parsing the user input command line options
 * @param argc
 * @param argv
 * @param envp
 * @param conf
 * @return
 */
int Parse_args(int argc, char **argv, char **envp, COMPILER_CONFIG &conf) {
  args::ArgumentParser parser(
    "OCC, a compiler used for the SYSY language (subset of C) ",
    "-----------\nAll Rights Reserved to the Compiler Group in Shenzhen Univ.\nContact lu.gt@163.com for details.\n");
  parser.Prog("compiler");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Group opt_group(parser, "Optimizations",
                        args::Group::Validators::DontCare);
  args::Group debug_group(parser, "Debugging",
                          args::Group::Validators::DontCare);
  args::Group action_group(parser, "Actions",
                           args::Group::Validators::AtMostOne);
  args::Group file_group(parser, "File Related",
                         args::Group::Validators::DontCare);
  args::Flag assembly(action_group, "assembly", "To generate the assembly code",
                      {'S', "assembly"});
  args::Flag object(action_group, "object", "To generate the object file",
                    {'c', "object"});
  args::Flag preprocess(action_group, "preprocess",
                        "Run preprocessor before parsing",
                        {'E', "preprocessor"});
  args::Flag linked(action_group, "linked", "To generate the executable file",
                    {"link"});                        
  args::Flag verbose(debug_group, "verbose", "With more verbosity",
                     {'v', "verbose"});
  args::Flag minimal(debug_group, "minimal", "With more verbosity",
                     {'q', "quiet"});
  args::Flag keep(debug_group, "keep", "Keeping the intermediate file",
                  {"keep"});
  args::Flag show(debug_group, "show",
                  "Displaying the current step the compiler is in", {"show"});
  args::Flag real_preprocess(debug_group, "real_preprocess",
                            "Run the preprocessor",
                            {"realprep"});
  args::Flag front_end_only(debug_group, "feonly",
                            "Run up to front-end, skip opt and further stages",
                            {"feonly"});
  args::ValueFlag<std::string> output_file(file_group, "output",
                                          "Output file location",
                                          {'o', "output"});
  args::ValueFlag<std::string> language(file_group, "language",
                                          "Specify the source code language",
                                          {'x', "language"});
  args::ValueFlag<int> loglevel(opt_group, "log",
                                          "logging options in or-ed form, within [0, LOG_MAX], LOG_MAX = 0xfffff",
{"log", "loglevel"});
  args::ValueFlag<int> feloglevel(opt_group, "felevel",
                                          "front-end logging options in or-ed form, within [0, LOG_MAX]",
                                          {"felevel", "feloglevel"});
  args::ValueFlag<int> beloglevel(opt_group, "belevel",
                                          "back-end logging options in or-ed form, within [0, LOG_MAX]",
                                          {"belevel", "beloglevel"});
  args::ValueFlag<int> cgloglevel(opt_group, "cglevel",
                                          "code-gen logging options in or-ed form, within [0, LOG_MAX]",
                                          {"cgloglevel", "cglevel"});
  args::ValueFlag<int> graloglevel(opt_group, "gralevel",
                                          "global register allocation (gra) log option, 0->LOG_MAX",
                                          {"graloglevel"});
  args::ValueFlag<int> linkerloglevel(opt_group, "linkerlevel",
                                          "asm+linker logging options in or-ed form, within [0, LOG_MAX]",
                                          {"linkerloglevel", "linkerlevel"});
  args::ValueFlag<int> symtabloglevel(opt_group, "linkerlevel",
                                          "symtab logging options in or-ed form, within [0, LOG_MAX]",
                                          {"symtabloglevel", "symtablevel"});
  args::ValueFlag<int> optimization_level(opt_group, "optlevel",
                                         "Optimisation level, within [1,4]",
                                         {'O'});
  args::ValueFlag<int> architecture_width(opt_group, "width",
                                         "Address bit width, between 8, 16, 32, 64",
                                         {'m', "width"});
  args::ValueFlag<std::string> architecture_name(opt_group, "architecture",
                                          "Specify target architecture",
                                          {"march", "arch"});
  args::ValueFlagList<std::string> include_list(file_group, "includeDir",
                                          "Specify include directories that preceed normal include dir",
                                          {'I', "include"});
  args::ValueFlagList<std::string> include_sys_list(file_group, "includeSysDir",
                                          "Specify include directories that preceed system include dir",
                                          {'i', "include-sys"});
  args::PositionalList<std::string> files(parser, "files", "The file of input");
  args::CompletionFlag completion(parser, {"complete"});

  // Init tracing options first.
  Init_trace_opts();
  // Parse the cmd line args.
  try {
    parser.ParseCLI(argc, argv);
  }
  catch (const args::Completion &e) {
    std::cout << e.what();
    return 0;
  }
  catch (const args::Help &) {
    std::cout << parser;
    exit(0);
  }
  catch (const args::ParseError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    exit(EXIT_OPTION_ERR);
  }

  const std::vector<std::string> file_vec(args::get(files));
  // Prepare files
  if(Tracing(COMPONENT_DRIVER, TRACE_INFO)) {
    std::cout << "Files count: " << file_vec.size() << std::endl;
  }
  for (auto it = file_vec.begin(); it != file_vec.end(); it++) {
    if(Tracing(COMPONENT_DRIVER, TRACE_INFO)) {
      std::cout << "File specified: " << *it << std::endl;
    }
    conf.files.push_back(*it);
  }
  if (file_vec.size() <= 0) {
    Comp_Usual_Error("no input files");
    Comp_Failure("No input file given.");
  }
  if (preprocess) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Enabled Preprocess Mode \n"));
  }
  if (assembly) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Enabled Assembly Mode \n"));
    conf.assembly = TRUE;
  } else if (object) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Enabled Object Generation Mode \n"));
    conf.assembly = TRUE;
    conf.object_gen = TRUE;
  } else if (preprocess) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Only Running Preprocess \n"));
  } else {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "By Default Enabled Assembly Mode \n"));
    conf.assembly = TRUE; // by
  }

  if (verbose) {
    Set_tracing_option(TRACE_OPT_VERBOSE);
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Only Running Preprocess \n"));
  } else if (minimal) {
    Set_tracing_option(TRACE_ERROR);
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Only Running Preprocess \n"));
  }

  // This is overriding the above -v or -quiet
  if (loglevel) {
    INT32 res = loglevel.Get();
    Set_tracing_option(static_cast<TRACE_KIND>(res));
  }

  if (beloglevel) {
    INT32 res = beloglevel.Get();
    Set_mod_tracing_option(COMPONENT_BE, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_VHO, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_LNO, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_GOPT, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_IPA, static_cast<TRACE_KIND>(res));
  }

  if (symtabloglevel) {
    INT32 res = symtabloglevel.Get();
    Set_mod_tracing_option(COMPONENT_SYMTAB, static_cast<TRACE_KIND>(res));
  }

  if (feloglevel) {
    INT32 res = feloglevel.Get();
    Set_mod_tracing_option(COMPONENT_FE, static_cast<TRACE_KIND>(res));
  }

  if (cgloglevel) {
    INT32 res = cgloglevel.Get();
    Set_mod_tracing_option(COMPONENT_CG, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_CG_IR_IN, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_CG_CONV, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_CG_LRA, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_CG_GRA, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(CO_CG_EMIT, static_cast<TRACE_KIND>(res));
  }

  if (graloglevel) {
    INT32 res = graloglevel.Get();
    Set_mod_tracing_option(COMPONENT_CG_LRA, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_CG_GRA, static_cast<TRACE_KIND>(res));
  }

  if (linkerloglevel) {
    INT32 res = linkerloglevel.Get();
    Set_mod_tracing_option(COMPONENT_ASM, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_LD, static_cast<TRACE_KIND>(res));
  }

  if (front_end_only) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Run only up to front-end, skip opt and cg \n"));
    conf.fe_only = TRUE;
  }

  if (real_preprocess) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Run real preprocessing \n"));
    conf.run_prep = TRUE;
  }

  /***
   *  Add intermediate result file names to the list
   **/
  if (conf.object_gen) {
    for (auto it = file_vec.begin(); it != file_vec.end(); it++) {
      std::string assembly_file_name = (*it) + ".s";
      std::string object_file_name = (*it) + ".o";
      std::cout << "Assemble file added to worklist: " << assembly_file_name << std::endl;
      std::cout << "Object file added to worklist: " << object_file_name << std::endl;
      conf.assemble_files.push_back(assembly_file_name);
      conf.object_files.push_back(object_file_name);
    } 
  }

  if (optimization_level) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Optimisation Level: %d\n", optimization_level.Get()));
    conf.opt_level = optimization_level.Get();
  }

  // Check if use has specified an output file name.
  if (output_file) {
    // If so, use the user specified file name.
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Output file : %s\n", output_file.Get().c_str()));
    conf.output_file = output_file.Get();
    AssertThat(strlen(conf.output_file.c_str()) > 0, ("Invalid file name specified"));
  } else {
    // If not, then we'd calculate a default file name for the user.
    // Finding base name of file (without directory names)
    AssertThat(file_vec.size() > 0, ("Failed to find any file"));
    const std::set<char> delims({'\\', '/'});
    std::vector<std::string> result = File_split_path(file_vec[0], delims);
    AssertThat(result.size() > 0, ("Unknwon filename met : %s", file_vec[0].c_str()));

    // Changing extension
    std::string base_name = result[result.size() - 1];
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Input file base name : %s\n", base_name.c_str()));
    File_change_extension(base_name, "s"); // This will change the base_name, by replacing the extension part
    conf.output_file = base_name;
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Default output file : %s\n", conf.output_file.c_str()));
  }

  return 0;
}

/**
 * Whole program entry
 * @param argc
 * @param argv
 * @param envp
 * @return
 */
int main(int argc, char **argv, char **envp) {
  COMPILER_CONFIG *conf = new COMPILER_CONFIG();
  // Parsing the arguments to thhe conf object
  Parse_args(argc, argv, envp, *conf);
  // Construct Workflow List and Execute the Compilation
  File();
  try {
    return Execute(*conf);
  } catch (std::exception err) {
    std::cout << err.what() << std::endl;
    std::cout << "Program returning error" << std::endl;
    return EXIT_UNKNOWN;
  }

}

/**
 * Execute the whole process
 * @param config
 */
INT32 Execute(COMPILER_CONFIG &config) {
  if (config.run_prep) {
    Run_preprocess(config);
    // Run_component(COMPONENT_PREP, config);
  }
  // Run FE
  Run_component(COMPONENT_FE, config);

  if (!config.fe_only) {
    if (config.assembly || config.object_gen) {
      // Run OPT
      Run_component(COMPONENT_BE, config);

      // Run CG
      Run_component(COMPONENT_CG, config);
    }
    if (config.object_gen) {
      // Run ASM
      Run_component(COMPONENT_ASM, config);
    }
  }
  return 0;
}

void Run_preprocess(COMPILER_CONFIG &config) {

}

/**
 * Start to run a component
 * @param component
 * @param config
 */
INT32 Run_component(COMPONENTS_WHOLE component, COMPILER_CONFIG &config) {
  Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_INVOCATION), (TFile, "Before running component [%d]\n", component));
  AssertThat(config.files.size() > 0, ("Failed to find enough files to process"));
  const char **operands = new const char *[10];
  for (INT32 i = 0; i < 10; i ++) {
    operands[i] = NULL;
  }
  switch (component) {
    case COMPONENT_FE: {
      Compilation_Phase = COMP_PHASE_IR_GEN;
      for (INT32 file_id = 0; file_id < config.files.size(); file_id++) {
        operands[0] = "fe";
        operands[1] = config.files[file_id].c_str();
        Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Invoking component [%s %s]\n", operands[0], operands[1]));
        femain(config, *File(), config.files[file_id].c_str());
      }
      break;
    }
    case COMPONENT_BE: {
      Compilation_Phase = COMP_PHASE_OPT_LOWER;
      Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Invoking direct component: back-end\n"));
      BE_EXTERNAL_MAIN_NAME(config);
      break;
    }
    case COMPONENT_CG: {
      Compilation_Phase = COMP_PHASE_CG;
      Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Invoking direct component: code generation\n"));
      CG_full_process(config);
      break;
    }
    case COMPONENT_ASM: {
      // "[A.s] -o [A.o]"
      Compilation_Phase = COMP_PHASE_ASM;
#ifdef SUBPROCESS_ENABLED
      for (INT32 file_id = 0; file_id < config.files.size(); file_id++) {
        operands[0] = "arm-linux-gnueabihf-as";
        operands[1] = (config.files[file_id] + ASSEMBLY_EXT_SUFFIX).c_str();
        operands[2] = "-o";
        operands[3] = (config.files[file_id] + OBJECT_EXT_SUFFIX).c_str();
        operands[4] = NULL;
        struct subprocess_s subprocess;
        INT32 result = subprocess_create(operands, 0, &subprocess);
        if (0 != result) {
          // an error occurred!s
          Comp_Failure("Error occured in invoking assembler, ret = %d\n", result);
        }
        FILE* p_stdout = subprocess_stdout(&subprocess);
        char hello_world[32];
        fgets(hello_world, 32, p_stdout);
        INT32 process_return;
        result = subprocess_join(&subprocess, &process_return);
        if (0 != result) {
          // an error occurred!
          Comp_Failure("Error occured in running assembler, ret = %d\n", result);
        }
      }
#else
      std::cout << "ASM stage Skipped" << std::endl;
#endif
      break;
    }
    default:
      break;
  }
  Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_INVOCATION), (TFile, "After running component [%d]\n", component));
  return 0;
}
