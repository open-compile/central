#include <stdio.h>
#include <iostream>
#include "basic.h"
#include "options.h"
#include "fe_export.h"
#include "be_export.h"
#include "main.h"
#include "file_util.h"
#include "timing.h"
#include "target.h"
#include <set>
#include <vector>
#include <string>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include "ir.h"
#include "ir_io.h"

#ifdef SUBPROCESS_ENABLED
#include "subprocess.h" // Unlicense
#endif

static std::string Lower_string(std::string s) {
  for (size_t i = 0; i < s.size(); ++i) {
    s[i] = static_cast<char>(tolower(static_cast<unsigned char>(s[i])));
  }
  return s;
}

static std::string Trim_string(const std::string &s) {
  size_t start = 0;
  while (start < s.size() &&
         isspace(static_cast<unsigned char>(s[start]))) {
    ++start;
  }

  size_t end = s.size();
  while (end > start &&
         isspace(static_cast<unsigned char>(s[end - 1]))) {
    --end;
  }
  return s.substr(start, end - start);
}

static std::string Normalize_option_name(std::string s) {
  s = Lower_string(Trim_string(s));
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '-') s[i] = '_';
  }
  return s;
}

static BOOL Parse_on_off_value(const std::string &value, INT32 *out) {
  std::string v = Lower_string(value);
  if (v == "1" || v == "on" || v == "true" || v == "yes") {
    *out = TRUE;
    return TRUE;
  }
  if (v == "0" || v == "off" || v == "false" || v == "no") {
    *out = FALSE;
    return TRUE;
  }
  return FALSE;
}

static BOOL Split_prefixed_option(const std::string &arg,
                                  const std::string &prefix,
                                  std::string *key,
                                  std::string *value) {
  if (arg.find(prefix) != 0) return FALSE;
  std::string body = arg.substr(prefix.size());
  size_t eq = body.find('=');
  if (eq == std::string::npos) {
    *key = Lower_string(body);
    *value = "1";
  } else {
    *key = Lower_string(body.substr(0, eq));
    *value = body.substr(eq + 1);
  }
  return !key->empty();
}

static BOOL Parse_trace_numeric_value(const std::string &text,
                                      TRACE_KIND *out) {
  std::string v = Trim_string(text);
  if (v.empty()) return FALSE;

  errno = 0;
  char *end = nullptr;
  long parsed = strtol(v.c_str(), &end, 0);
  if (errno != 0 || end == v.c_str() || *end != '\0') return FALSE;

  *out = static_cast<TRACE_KIND>(parsed);
  return TRUE;
}

static BOOL Parse_trace_level_token(const std::string &token,
                                    TRACE_KIND *out) {
  std::string t = Normalize_option_name(token);
  if (t.empty()) return FALSE;

  if (t == "all" || t == "verbose") {
    *out = TRACE_OPT_VERBOSE;
  } else if (t == "default") {
    *out = TRACE_OPT_DEFAULT;
  } else if (t == "nolineno" || t == "no_lineno" || t == "no_line_no") {
    *out = TRACE_OPT_NOLINENO;
  } else if (t == "debug") {
    *out = TRACE_DEBUG;
  } else if (t == "data") {
    *out = TRACE_DATA;
  } else if (t == "info") {
    *out = TRACE_INFO;
  } else if (t == "performance" || t == "perf") {
    *out = TRACE_PERFORMANCE;
  } else if (t == "invocation" || t == "invoke") {
    *out = TRACE_INVOCATION;
  } else if (t == "options" || t == "option") {
    *out = TRACE_OPTIONS;
  } else if (t == "emit_core") {
    *out = TRACE_EMIT_CORE;
  } else if (t == "emit_basic") {
    *out = TRACE_EMIT_BASIC;
  } else if (t == "warn" || t == "warning") {
    *out = TRACE_WARN;
  } else if (t == "error") {
    *out = TRACE_ERROR;
  } else if (t == "fatal") {
    *out = TRACE_FATAL;
  } else if (t == "custom1") {
    *out = TRACE_CUSTOM1;
  } else if (t == "custom2") {
    *out = TRACE_CUSTOM2;
  } else {
    return Parse_trace_numeric_value(t, out);
  }
  return TRUE;
}

static BOOL Parse_trace_level_value(const std::string &value_text,
                                    TRACE_KIND *out,
                                    std::string *err) {
  TRACE_KIND numeric = TRACE_ERROR;
  if (Parse_trace_numeric_value(value_text, &numeric)) {
    *out = numeric;
    return TRUE;
  }

  TRACE_KIND result = static_cast<TRACE_KIND>(0);
  size_t start = 0;
  while (start <= value_text.size()) {
    size_t bar = value_text.find('|', start);
    std::string token = value_text.substr(
      start, bar == std::string::npos ? std::string::npos : bar - start);
    TRACE_KIND token_value = TRACE_ERROR;
    if (!Parse_trace_level_token(token, &token_value)) {
      *err = "Invalid TRACE level '" + Trim_string(token) +
             "'; use a number, ALL, or names like DEBUG|DATA|OPTIONS";
      return FALSE;
    }
    result = static_cast<TRACE_KIND>(
      static_cast<INT32>(result) | static_cast<INT32>(token_value));
    if (bar == std::string::npos) break;
    start = bar + 1;
  }

  *out = result;
  return TRUE;
}

static BOOL Apply_trace_option(const std::string &key_text,
                               TRACE_KIND value,
                               std::string *err) {
  std::string key = Normalize_option_name(key_text);
  if (key == "all" || key == "global") {
    Set_tracing_option(value);
  } else if (key == "driver") {
    Set_mod_tracing_option(COMPONENT_DRIVER, value);
  } else if (key == "symtab") {
    Set_mod_tracing_option(COMPONENT_SYMTAB, value);
  } else if (key == "prep" || key == "preprocess") {
    Set_mod_tracing_option(COMPONENT_PREP, value);
  } else if (key == "fe" || key == "front_end") {
    Set_mod_tracing_option(COMPONENT_FE, value);
  } else if (key == "be" || key == "back_end") {
    Set_mod_tracing_option(COMPONENT_BE, value);
    Set_mod_tracing_option(COMPONENT_VHO, value);
    Set_mod_tracing_option(COMPONENT_LNO, value);
    Set_mod_tracing_option(COMPONENT_GOPT, value);
    Set_mod_tracing_option(COMPONENT_IPA, value);
  } else if (key == "vho") {
    Set_mod_tracing_option(COMPONENT_VHO, value);
  } else if (key == "lno") {
    Set_mod_tracing_option(COMPONENT_LNO, value);
  } else if (key == "gopt") {
    Set_mod_tracing_option(COMPONENT_GOPT, value);
  } else if (key == "ipa") {
    Set_mod_tracing_option(COMPONENT_IPA, value);
  } else if (key == "ssa") {
    Set_mod_tracing_option(COMPONENT_SSA, value);
  } else if (key == "ssa_conv") {
    Set_mod_tracing_option(COMPONENT_SSA_CONV, value);
  } else if (key == "cg" || key == "code_gen") {
    Set_mod_tracing_option(COMPONENT_CG, value);
    Set_mod_tracing_option(COMPONENT_CG_IR_IN, value);
    Set_mod_tracing_option(COMPONENT_CG_CONV, value);
    Set_mod_tracing_option(COMPONENT_CG_LRA, value);
    Set_mod_tracing_option(COMPONENT_CG_REGALLOC, value);
    Set_mod_tracing_option(COMPONENT_CG_LAYOUT, value);
    Set_mod_tracing_option(COMPONENT_CG_EMIT, value);
  } else if (key == "cg_ir_in" || key == "ir_in") {
    Set_mod_tracing_option(COMPONENT_CG_IR_IN, value);
  } else if (key == "cg_conv" || key == "conv") {
    Set_mod_tracing_option(COMPONENT_CG_CONV, value);
  } else if (key == "lra" || key == "cg_lra") {
    Set_mod_tracing_option(COMPONENT_CG_LRA, value);
  } else if (key == "regalloc" || key == "cg_regalloc" || key == "gra") {
    Set_mod_tracing_option(COMPONENT_CG_REGALLOC, value);
  } else if (key == "layout" || key == "cg_layout") {
    Set_mod_tracing_option(COMPONENT_CG_LAYOUT, value);
  } else if (key == "emit" || key == "cg_emit") {
    Set_mod_tracing_option(COMPONENT_CG_EMIT, value);
  } else if (key == "asm") {
    Set_mod_tracing_option(COMPONENT_ASM, value);
  } else if (key == "ld" || key == "linker") {
    Set_mod_tracing_option(COMPONENT_LD, value);
  } else if (key == "cross" || key == "cross_phase" || key == "xphase") {
    Set_mod_tracing_option(COMPONENT_CROSS_PHASE, value);
  } else {
    *err = "Unknown TRACE component '" + key_text +
           "'; examples: cg, cg_conv, lra, regalloc, fe, be, cross";
    return FALSE;
  }
  return TRUE;
}

static void Apply_opt_option(COMPILER_CONFIG &conf,
                             const std::string &key,
                             INT32 value) {
  conf.opt_options[key] = value;
  if (key == "ssa") {
    conf.opt_cfg.run_ssa_phase = value;
    if (!value) {
      conf.opt_cfg.run_destruct_ssa = FALSE;
      conf.opt_cfg.enable_cprop = FALSE;
      conf.opt_cfg.enable_dce = FALSE;
    }
  } else if (key == "dce") {
    conf.opt_cfg.enable_dce = value;
  } else if (key == "cprop") {
    conf.opt_cfg.enable_cprop = value;
  } else if (key == "ssa-destruct" || key == "destruct-ssa") {
    conf.opt_cfg.run_destruct_ssa = value;
  }
}

static void Apply_cg_option(COMPILER_CONFIG &conf,
                            const std::string &key,
                            INT32 value) {
  conf.cg_options[key] = value;
  if (key == "lra") {
    conf.cg_cfg.enable_lra = value;
  } else if (key == "regalloc") {
    conf.cg_cfg.enable_regalloc = value;
  } else if (key == "sched") {
    conf.cg_cfg.enable_sched = value;
  } else if (key == "resched") {
    conf.cg_cfg.enable_resched = value;
  } else if (key == "vdg") {
    conf.cg_cfg.enable_vdg = value;
  } else if (key == "cfg") {
    conf.cg_cfg.dump_cfg = value;
  } else if (key == "cfggraph" || key == "cfg-graph") {
    conf.cg_cfg.dump_cfg_graph = value;
  } else if (key == "tn" || key == "tns") {
    conf.cg_cfg.dump_tn = value;
  } else if (key == "dump") {
    conf.cg_cfg.dump_cfg = value;
    conf.cg_cfg.dump_cfg_graph = value;
    conf.cg_cfg.dump_tn = value;
  }
}

static void Apply_phase_option(COMPILER_CONFIG &conf,
                               const std::string &key,
                               INT32 value) {
  conf.phase_options[key] = value;
  if (key == "ssa") {
    Apply_opt_option(conf, "ssa", value);
  }
}

static BOOL Parse_grouped_driver_option(const std::string &arg,
                                        COMPILER_CONFIG &conf,
                                        std::string *err) {
  std::string key;
  std::string value_text;
  std::string opt_arg = arg;
  if (opt_arg.find("--") == 0) {
    opt_arg = "-" + opt_arg.substr(2);
  }
  if (opt_arg == "-timing") {
    conf.timing = TRUE;
    return TRUE;
  }
  enum { GROUP_NONE, GROUP_OPT, GROUP_CG, GROUP_PHASE, GROUP_SSA, GROUP_TRACE } group = GROUP_NONE;
  if (Split_prefixed_option(opt_arg, "-OPT:", &key, &value_text)) {
    group = GROUP_OPT;
  } else if (Split_prefixed_option(opt_arg, "-CG:", &key, &value_text)) {
    group = GROUP_CG;
  } else if (Split_prefixed_option(opt_arg, "-PHASE:", &key, &value_text)) {
    group = GROUP_PHASE;
  } else if (Split_prefixed_option(opt_arg, "-TRACE:", &key, &value_text) ||
             Split_prefixed_option(opt_arg, "-trace:", &key, &value_text)) {
    group = GROUP_TRACE;
  } else if (opt_arg.find("-ssa=") == 0) {
    key = "ssa";
    value_text = opt_arg.substr(strlen("-ssa="));
    group = GROUP_SSA;
  } else {
    return FALSE;
  }

  if (group == GROUP_TRACE) {
    TRACE_KIND trace_value = TRACE_ERROR;
    if (!Parse_trace_level_value(value_text, &trace_value, err)) {
      *err += " in option '" + arg + "'";
      return TRUE;
    }
    Apply_trace_option(key, trace_value, err);
    return TRUE;
  }

  INT32 value = 0;
  if (!Parse_on_off_value(value_text, &value)) {
    *err = "Invalid value '" + value_text + "' in option '" + arg +
           "'; use 0/1/on/off/true/false";
    return TRUE;
  }

  switch (group) {
    case GROUP_OPT: Apply_opt_option(conf, key, value); break;
    case GROUP_CG: Apply_cg_option(conf, key, value); break;
    case GROUP_PHASE:
    case GROUP_SSA: Apply_phase_option(conf, key, value); break;
    default: break;
  }
  return TRUE;
}

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
    "-----------\n"
    "Trace options:\n"
    "  -TRACE:<component>=<level>\n"
    "  -timing or --timing: print wall/user/sys time and RSS per stage\n"
    "\n"
    "Supported level forms include decimal/hex numbers, ALL, VERBOSE, DEBUG,\n"
    "DATA, INFO, PERFORMANCE, INVOCATION, OPTIONS, EMIT_CORE, EMIT_BASIC,\n"
    "WARN, ERROR, FATAL, CUSTOM1, CUSTOM2, and | combinations.\n"
    "\n"
    "Supported component names include cg, cg_conv, lra, regalloc, layout,\n"
    "emit, fe, be, ssa, driver, symtab, asm, ld, and cross.\n"
    "\n"
    "Shell note: quote the pipe form, otherwise the shell treats | as a\n"
    "pipeline.\n"
    "\n"
    "Examples:\n"
    "  compiler -TRACE:cg=1 input.c\n"
    "  compiler -TRACE:cg_conv=1 input.c\n"
    "  compiler -TRACE:lra=ALL input.c\n"
    "  compiler -TRACE:lra=DEBUG input.c\n"
    "  compiler '-TRACE:lra=DEBUG|DATA|OPTIONS' input.c\n"
    "  compiler -TRACE:lra=0xFFFF input.c\n"
    "  compiler -TRACE:cross=DATA input.c\n"
    "  compiler -TRACE:cross=EMIT_CORE input.c\n"
    "\n"
    "Target options:\n"
    "  --target=<arch> or --march=<arch>\n"
    "  Supported targets: armv8-a32/aarch32, armv8-a64/armv9-a64/aarch64,\n"
    "  x86-64.\n"
    "  armv8-a32 is the currently implemented code generation target;\n"
    "  armv8/armv9-a64 and x86-64 are accepted as explicit target selections and\n"
    "  fail before emission until their builders/emitters are implemented.\n"
    "\n"
    "All Rights Reserved to the Compiler Group in Shenzhen Univ.\n"
    "Contact lu.gt@163.com for details.\n");
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
  args::Flag timing(debug_group, "timing",
                    "Trace timing and memory usage for compiler stages",
                    {"timing"});
  args::Flag front_end_only(debug_group, "feonly",
                            "Run up to front-end, skip opt and further stages",
                            {"feonly"});
  args::Flag disable_ssa(debug_group, "disable_ssa",
                         "Disable the SSA optimization pipeline",
                         {"no-ssa", "disable-ssa"});
  args::Flag disable_cprop(debug_group, "disable_cprop",
                           "Disable SSA constant propagation",
                           {"no-cprop", "disable-cprop"});
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
                                          "Specify target architecture: armv8-a32/aarch32, armv8-a64/armv9-a64/aarch64, x86-64",
                                          {"march", "arch"});
  args::ValueFlag<std::string> target_name(opt_group, "target",
                                          "Specify canonical target triple",
                                          {"target"});
  args::ValueFlagList<std::string> include_list(file_group, "includeDir",
                                          "Specify include directories that preceed normal include dir",
                                          {'I', "include"});
  args::ValueFlagList<std::string> include_sys_list(file_group, "includeSysDir",
                                          "Specify include directories that preceed system include dir",
                                          {'i', "include-sys"});
  // ---- 二进制 IR Dump / Load (Step 10) ----
  args::ValueFlagList<std::string> dump_ir_after(file_group, "dumpIrAfter",
      "--dump-ir-after=<stage>=<path>; repeatable. Stages: fe,opt-high,"
      "opt-mid,opt-after-ssa,opt-low,opt-vlow,opt-cgir,pre-cg",
      {"dump-ir-after"});
  args::ValueFlag<std::string> load_ir(file_group, "loadIr",
      "--load-ir=<path>; load binary IR and skip front-end",
      {"load-ir"});
  args::Flag dump_textual(debug_group, "dumpTextual",
      "Also call FILE_MANAGER::Print() to <path>.txt after each binary dump",
      {"dump-textual"});
  args::PositionalList<std::string> files(parser, "files", "The file of input");
  args::CompletionFlag completion(parser, {"complete"});

  // Init tracing options first.
  Init_trace_opts();
  std::vector<std::string> filtered_args;
  filtered_args.reserve(argc);
  filtered_args.push_back(argv[0]);
  for (INT32 i = 1; i < argc; ++i) {
    std::string err;
    if (Parse_grouped_driver_option(argv[i], conf, &err)) {
      if (!err.empty()) {
        std::cerr << err << std::endl;
        exit(EXIT_OPTION_ERR);
      }
      continue;
    }
    filtered_args.push_back(argv[i]);
  }
  std::vector<char *> filtered_argv;
  filtered_argv.reserve(filtered_args.size());
  for (UINT32 i = 0; i < filtered_args.size(); ++i) {
    filtered_argv.push_back(const_cast<char *>(filtered_args[i].c_str()));
  }
  // Parse the cmd line args.
  try {
    parser.ParseCLI(static_cast<int>(filtered_argv.size()), filtered_argv.data());
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

  std::string target_error;
  std::string requested_target;
  if (target_name) {
    requested_target = target_name.Get();
  }
  if (architecture_name) {
    if (requested_target.empty()) {
      requested_target = architecture_name.Get();
    } else {
      const TARGET_INFO *canonical_target = nullptr;
      const TARGET_INFO *architecture_target = nullptr;
      if (!Resolve_target(requested_target, &canonical_target, &target_error) ||
          !Resolve_target(architecture_name.Get(), &architecture_target,
                          &target_error)) {
        std::cerr << target_error << std::endl;
        exit(EXIT_OPTION_ERR);
      }
      if (canonical_target->triple != architecture_target->triple) {
        std::cerr << "conflicting target options: --target="
                  << requested_target << " and --arch="
                  << architecture_name.Get() << std::endl;
        exit(EXIT_OPTION_ERR);
      }
    }
  }
  // Width-based target selection (-m 32/-m 64) used only when no explicit target was given
  if (requested_target.empty() && architecture_width) {
    INT32 width = architecture_width.Get();
    if (width == 32) {
      requested_target = "armv7-linux-gnueabihf";
    } else if (width == 64) {
      requested_target = "aarch64-linux-gnu";
    } else {
      std::cerr << "Unsupported architecture width '" << width
                << "'; use 32 or 64" << std::endl;
      exit(EXIT_OPTION_ERR);
    }
  }
  if (!Configure_target(requested_target, &conf, &target_error)) {
    std::cerr << target_error << std::endl;
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

  if (conf.object_gen && !conf.target.integrated_object_supported) {
    std::cerr << "integrated -c is not supported for target "
              << conf.target.triple << "; emit assembly with -S and run: "
              << conf.target.external_assembler_hint << std::endl;
    exit(EXIT_OPTION_ERR);
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
    Set_mod_tracing_option(COMPONENT_CG_REGALLOC, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_CG_EMIT, static_cast<TRACE_KIND>(res));
  }

  if (graloglevel) {
    INT32 res = graloglevel.Get();
    Set_mod_tracing_option(COMPONENT_CG_LRA, static_cast<TRACE_KIND>(res));
    Set_mod_tracing_option(COMPONENT_CG_REGALLOC, static_cast<TRACE_KIND>(res));
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

  if (disable_ssa) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Disable SSA optimization pipeline\n"));
    conf.opt_cfg.run_ssa_phase = FALSE;
    conf.opt_cfg.run_destruct_ssa = FALSE;
    conf.opt_cfg.enable_cprop = FALSE;
    conf.opt_cfg.enable_dce = FALSE;
    conf.opt_options["ssa"] = FALSE;
    conf.opt_options["cprop"] = FALSE;
    conf.opt_options["dce"] = FALSE;
  }

  if (disable_cprop) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Disable SSA constant propagation\n"));
    conf.opt_cfg.enable_cprop = FALSE;
    conf.opt_options["cprop"] = FALSE;
  }

  if (real_preprocess) {
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Run real preprocessing \n"));
    conf.run_prep = TRUE;
  }
  if (timing) {
    conf.timing = TRUE;
  }
  Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS),
           (TFile, "Target architecture: %s (triple: %s)\n",
            Target_arch_name(conf.target_arch), conf.target.triple.c_str()));

  // ---- 二进制 IR dump / load 选项解析 (Step 10) ----
  for (auto const &kv : args::get(dump_ir_after)) {
    auto eq = kv.find('=');
    AssertThat(eq != std::string::npos,
               ("--dump-ir-after expects <stage>=<path>, got '%s'", kv.c_str()));
    conf.dump_ir_stages.emplace_back(kv.substr(0, eq), kv.substr(eq + 1));
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS),
             (TFile, "dump-ir-after: stage=%s path=%s\n",
              conf.dump_ir_stages.back().first.c_str(),
              conf.dump_ir_stages.back().second.c_str()));
  }
  if (load_ir) {
    conf.load_ir_path = load_ir.Get();
    Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS),
             (TFile, "load-ir from: %s\n", conf.load_ir_path.c_str()));
  }
  if (dump_textual) {
    conf.dump_textual_after_dump = TRUE;
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

static void Trace_cross_phase_ir(const char *stage, FILE_MANAGER *file) {
  if (Tracing(COMPONENT_CROSS_PHASE, TRACE_DATA) ||
      Tracing(COMPONENT_CROSS_PHASE, TRACE_EMIT_CORE)) {
    fprintf(TFile, "\n========== CROSS-PHASE IR DUMP: %s ==========\n", stage);
    file->Print(TFile);
    fprintf(TFile, "========== END CROSS-PHASE IR DUMP: %s ==========\n\n", stage);
  } else if (Tracing(COMPONENT_CROSS_PHASE, TRACE_EMIT_BASIC)) {
    fprintf(TFile, "\n========== CROSS-PHASE FUNCTION DUMP: %s ==========\n", stage);
    file->Tables()->Print_functions(TFile);
    fprintf(TFile, "========== END CROSS-PHASE FUNCTION DUMP: %s ==========\n\n", stage);
  }
}

static const char *Component_timing_name(COMPONENTS_WHOLE component) {
  switch (component) {
    case COMPONENT_FE: return "fe";
    case COMPONENT_BE: return "be";
    case COMPONENT_CG: return "cg";
    case COMPONENT_ASM: return "asm";
    default: return "component";
  }
}

/**
 * Start to run a component
 * @param component
 * @param config
 */
INT32 Run_component(COMPONENTS_WHOLE component, COMPILER_CONFIG &config) {
  Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_INVOCATION), (TFile, "Before running component [%d]\n", component));
  AssertThat(config.files.size() > 0, ("Failed to find enough files to process"));
  TIMING_SNAPSHOT timing_start;
  if (config.timing) {
    timing_start = Timing_snapshot();
  }
  const char **operands = new const char *[10];
  for (INT32 i = 0; i < 10; i ++) {
    operands[i] = NULL;
  }
  switch (component) {
    case COMPONENT_FE: {
      Compilation_Phase = COMP_PHASE_IR_GEN;
      if (!config.load_ir_path.empty()) {
        Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS),
                 (TFile, "Skipping front-end, load IR from %s\n",
                  config.load_ir_path.c_str()));
        Load_ir_file(File(), config.load_ir_path.c_str());
      } else {
        for (INT32 file_id = 0; file_id < config.files.size(); file_id++) {
          operands[0] = "fe";
          operands[1] = config.files[file_id].c_str();
          Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_OPTIONS), (TFile, "Invoking component [%s %s]\n", operands[0], operands[1]));
          femain(config, *File(), config.files[file_id].c_str());
        }
      }
      Maybe_dump_ir("fe", config, File());
      Trace_cross_phase_ir("after-fe-before-opt", File());
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
      Maybe_dump_ir("pre-cg", config, File());
      Trace_cross_phase_ir("after-opt-before-cg", File());
      CG_full_process(config);
      break;
    }
    case COMPONENT_ASM: {
      // "[A.s] -o [A.o]"
      Compilation_Phase = COMP_PHASE_ASM;
#ifdef SUBPROCESS_ENABLED
      for (INT32 file_id = 0; file_id < config.files.size(); file_id++) {
        operands[0] = config.target.assembler.c_str();
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
  if (config.timing) {
    Timing_trace_stage(TFile, Component_timing_name(component), timing_start,
                       Timing_snapshot());
  }
  Is_Trace(Tracing(COMPONENT_DRIVER, TRACE_INVOCATION), (TFile, "After running component [%d]\n", component));
  return 0;
}
