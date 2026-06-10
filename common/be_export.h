//
// ============================================================================
// CLAUDE-MARKER  STATUS: 修改 (增 include guard + options.h include)
// ============================================================================
// 修改内容:
//   - 加 include guard (_OCC_BE_EXPORT_H_) — 之前没有
//   - 显式 #include "options.h" — 让 cg/opt 代码用 COMPILER_CONFIG 时不必间接
// ============================================================================

#ifndef _OCC_BE_EXPORT_H_
#define _OCC_BE_EXPORT_H_

#include "options.h"

#define BE_MAIN_NAME bemain
#define BE_EXTERNAL_MAIN_NAME Opt_run_optimizer

INT32 BE_MAIN_NAME(INT32 argc, char **argv);
INT32 BE_EXTERNAL_MAIN_NAME(COMPILER_CONFIG &conf);
INT32 CG_full_process(COMPILER_CONFIG &conf);

#endif