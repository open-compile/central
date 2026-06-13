// =============================================================================
// irprint -- Standalone CLI: load a binary IR (.irb) file and Print it
// =============================================================================
// 用途：读 OCC 二进制 IR 文件，调 FILE_MANAGER::Print 把全部内容打印到 stdout
//       (或 -o 指定的文件)。这是 --dump-textual 的离线版本：你已经有
//       一个 .irb 文件，想看里面长啥样，但又不想再跑一次完整 driver。
//
// Usage:
//   irprint <input.irb>                      # 打印到 stdout
//   irprint <input.irb> -o <out.txt>         # 打印到文件
//   irprint <input.irb> --header-only        # 只打印 file header + section table
//
// 与 driver/compiler 的关系：
//   compiler --dump-ir-after=<stage>=<path> --dump-textual <src.sy>
//     -> 在编译过程中产生 <path> + <path>.txt
//   irprint <path>
//     -> 等价于事后产生 <path>.txt 的内容；不需要源 .sy
// =============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include "basic.h"
#include "symtab.h"
#include "ir_io.h"
#include "args.h"

static void print_header_only(const char *path) {
  IR_READER r(path);
  if (!r.Is_open()) {
    fprintf(stderr, "irprint: cannot open '%s' for read\n", path);
    exit(2);
  }
  r.Read_header();
  printf("=== IRB File: %s ===\n", path);
  printf("file_format_version  = %u\n", (unsigned) r.File_format_version());
  printf("flags                = 0x%04x\n", (unsigned) r.File_flags());
  printf("\nSections:\n");
  printf("  %-20s %-10s %-12s %-12s %-8s\n",
         "kind", "name", "offset", "size", "version");
  static const struct {
    UINT16 kind; const char *name;
  } SK_NAMES[] = {
    {SK_STRTAB,        "STRTAB"},
    {SK_TY_TAB,        "TY_TAB"},
    {SK_TYLIST_TAB,    "TYLIST_TAB"},
    {SK_ARB_TAB,       "ARB_TAB"},
    {SK_ST_GLOBAL,     "ST_GLOBAL"},
    {SK_LABEL_GLOBAL,  "LABEL_GLOBAL"},
    {SK_PREG_GLOBAL,   "PREG_GLOBAL"},
    {SK_INITO_GLOBAL,  "INITO_GLOBAL"},
    {SK_PU_TAB,        "PU_TAB"},
    {SK_PU_INFO_TAB,   "PU_INFO_TAB"},
    {SK_FUNC_TAB,      "FUNC_TAB"},
    {SK_FUNC_BLOB,     "FUNC_BLOB"},
    {SK_TCON_TAB,      "TCON_TAB"},
    {SK_FILE_INFO_TAB, "FILE_INFO_TAB"},
    {SK_SSA_GLOBAL,    "SSA_GLOBAL"},
    {SK_CGIR_GLOBAL,   "CGIR_GLOBAL"},
    {SK_DEBUG_INFO,    "DEBUG_INFO"},
    {0, NULL}
  };
  for (UINT16 i = 0; SK_NAMES[i].name != NULL; i++) {
    UINT16 k = SK_NAMES[i].kind;
    if (!r.Has_section(k)) continue;
    printf("  %-20u %-10s %-12llu %-12llu %-8u\n",
           (unsigned) k, SK_NAMES[i].name,
           (unsigned long long) r.Section_offset(k),
           (unsigned long long) r.Section_size(k),
           (unsigned) r.Section_version(k));
  }
  r.Close();
}

int main(int argc, char **argv) {
  Compilation_Phase = "IR_PRINT";

  args::ArgumentParser parser(
      "irprint -- load a binary OCC IR (.irb) file and print its contents",
      "Sister tool of `compiler`; reads files produced by --dump-ir-after.");
  parser.Prog("irprint");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::ValueFlag<std::string> output(parser, "output",
      "Output file (default: stdout)", {'o', "output"});
  args::Flag header_only(parser, "headerOnly",
      "Print only the file header + section table; do not parse contents",
      {"header-only"});
  args::Flag verbose(parser, "verbose",
      "Verbose tracing while parsing", {'v', "verbose"});
  args::Positional<std::string> input(parser, "input.irb",
      "Path to a binary IR file (produced by `compiler --dump-ir-after=...`)");

  Init_trace_opts();
  try {
    parser.ParseCLI(argc, argv);
  }
  catch (const args::Help &) {
    std::cout << parser;
    return 0;
  }
  catch (const args::ParseError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 2;
  }

  if (!input) {
    std::cerr << "irprint: missing input.irb\n";
    std::cerr << parser;
    return 2;
  }
  std::string path = args::get(input);

  if (header_only) {
    print_header_only(path.c_str());
    return 0;
  }

  if (verbose) {
    Set_tracing_option(TRACE_OPT_VERBOSE);
  }

  // 1) Load — File() 单例首次访问触发 Initialize()，Load_ir_file 内部不再重复
  FILE_MANAGER *fm = File();
  Load_ir_file(fm, path.c_str());

  // 2) Open 输出
  FILE *out = stdout;
  bool own_out = false;
  if (output) {
    out = fopen(args::get(output).c_str(), "w");
    if (out == NULL) {
      fprintf(stderr, "irprint: cannot open '%s' for write\n",
              args::get(output).c_str());
      return 3;
    }
    own_out = true;
  }

  // 3) Print
  fprintf(out, "=== IRB File: %s ===\n\n", path.c_str());
  fm->Print(out);

  if (own_out) fclose(out);
  return 0;
}
