//
// ============================================================================
// CLAUDE-MARKER  STATUS: KEEP (Print 接口)
// ============================================================================
// 统一的 SSA 调试打印接口; 任意位置调用, 全部无副作用 (只读 FILE* 输出)
//   - Print_ssa_cfg
//   - Print_dom_tree / Print_df
//   - SSADump_all / SSADump_function
// ============================================================================

#ifndef OCC_OPT_PRINT_H
#define OCC_OPT_PRINT_H

#include "opt_basic.h"
#include <cstdio>

// 统一的 SSA 调试打印接口
// 任意位置调用，全部无副作用（只读 FILE* 输出）

// SSABB 的 Print/Print_pretty 已在 SSA_CFG_BB_BASE 中定义

// SSA_CFG 整体打印
void Print_ssa_cfg(SSA_CFG *cfg, FILE *f = stderr, BOOL pretty = FALSE);

// 单独打印支配树 / DF
void Print_dom_tree(SSA_CFG *cfg, FILE *f = stderr);
void Print_df(SSA_CFG *cfg, FILE *f = stderr);

// SSAIR 整体 dump（见 opt_ssa.cxx 实现）
void SSADump_all(FILE *f = stderr);
void SSADump_function(ST_IDX sym, FILE *f = stderr);

#endif //OCC_OPT_PRINT_H
