//
// ============================================================================
// CLAUDE-MARKER  STATUS: 修改 (扩展 OPT_KIND + 新增 OPT_CONFIG)
// ============================================================================
// 修改内容:
//   - 扩展 OPT_KIND 加 SSA_BUILD / SSA_DESTRUCT / CPROP / DCE 等
//   - 新增 enum SSA_DUMP_LEVEL (NONE / CFG / DOM / PHI / RENAME / FULL)
//   - 新增 struct OPT_PARAM (cprop_max_iter / dce_aggressive 等)
//   - 新增 struct OPT_CONFIG (run_ssa_phase / dump_after_* / dump_fp)
//   - COMPILER_CONFIG 增 opt_cfg 字段
// ============================================================================

#ifndef _OCC_OPTIONS_H_
#define _OCC_OPTIONS_H_

#include "host.h"
#include <string>
#include <vector>
#include <map>
#include <utility>
#include "args.h" // MIT License

using STRVEC = std::vector<std::string>;
using std::string;

// 各类优化 pass 的开关
enum OPT_KIND {
  // 基础
  OPT_KIND_ARITH       = 1,    // 算术优化
  OPT_KIND_LDID_CONST  = 2,    // 常量 LDID
  // SSA
  OPT_KIND_SSA_BUILD   = 10,   // 构建 SSA
  OPT_KIND_SSA_DESTRUCT= 11,   // 析构 SSA
  // SSA-based passes
  OPT_KIND_CPROP       = 20,   // 常量传播 + 折叠
  OPT_KIND_DCE         = 21,   // 死代码消除
  OPT_KIND_COPYPROP    = 22,   // copy 传播（占位）
  OPT_KIND_GVN         = 23,   // 全局值编号（占位）
  OPT_KIND_INLINE      = 24,   // 函数内联（占位）
};

// 调试 dump 级别
enum SSA_DUMP_LEVEL {
  SSA_DUMP_NONE     = 0,   // 不打印
  SSA_DUMP_CFG      = 1,   // CFG/BB 拓扑
  SSA_DUMP_DOM      = 2,   // + 支配树 / DF
  SSA_DUMP_PHI      = 3,   // + phi 节点
  SSA_DUMP_RENAME   = 4,   // + rename 后 CODEREP def/use
  SSA_DUMP_FULL     = 5,   // 全部内容
};

// 优化 pass 行为参数（每个 pass 通用）
struct OPT_PARAM {
  // 常量传播
  INT32  cprop_max_iter    = 4;    // 不动点迭代上限
  BOOL   cprop_only_const  = TRUE;// 仅做常数折叠，不做 copy prop
  // DCE
  BOOL   dce_aggressive    = FALSE;// 激进 DCE：删任何无 side-effect 的语句
  // SSA 构建
  BOOL   ssa_emit_chi_mu   = FALSE;// 是否生成 CHI/MU 节点（暂未实现）
  // 全局
  INT32  verbose_level     = 0;    // 0=静默，1=概要，2=详细
};

// 优化器配置（嵌入 COMPILER_CONFIG）
struct OPT_CONFIG {
  // 阶段级开关
  BOOL  run_ssa_phase      = TRUE;  // 是否跑 SSA 整个阶段
  BOOL  run_destruct_ssa   = TRUE;  // 是否在 OPT_LOW 之前析构 SSA

  // 单 pass 开关
  BOOL  enable_cprop       = TRUE;
  BOOL  enable_dce         = TRUE;

  // 调试 dump：每个阶段独立控制
  SSA_DUMP_LEVEL dump_after_cfg_build  = SSA_DUMP_NONE;
  SSA_DUMP_LEVEL dump_after_dom        = SSA_DUMP_NONE;
  SSA_DUMP_LEVEL dump_after_phi        = SSA_DUMP_NONE;
  SSA_DUMP_LEVEL dump_after_rename     = SSA_DUMP_NONE;
  SSA_DUMP_LEVEL dump_after_cprop      = SSA_DUMP_NONE;
  SSA_DUMP_LEVEL dump_after_dce        = SSA_DUMP_NONE;
  SSA_DUMP_LEVEL dump_after_destruct   = SSA_DUMP_NONE;

  // 输出目标：缺省 stderr
  FILE *dump_fp            = nullptr;  // nullptr 表示 TFile (stderr)

  // 通用参数
  OPT_PARAM param;
};

struct CG_CONFIG {
  BOOL enable_lra      = TRUE;
  BOOL enable_regalloc = TRUE;
  BOOL enable_sched    = FALSE;
  BOOL enable_resched  = FALSE;
  BOOL enable_vdg      = FALSE;
  BOOL dump_cfg        = FALSE;
  BOOL dump_cfg_graph  = FALSE;
  BOOL dump_tn         = FALSE;
};

class COMPILER_CONFIG {
public:
  INT32  opt_level   = 2;
  BOOL   assembly    = FALSE;
  BOOL   object_gen  = FALSE;
  BOOL   fe_only     = FALSE;
  BOOL   run_prep    = FALSE;
  BOOL   opt_bin     = TRUE;
  BOOL   timing      = FALSE;
  STRVEC files;
  STRVEC assemble_files;
  STRVEC object_files;
  string output_file;

  // 优化器配置（新增）
  OPT_CONFIG opt_cfg;
  CG_CONFIG  cg_cfg;
  std::map<string, INT32> opt_options;
  std::map<string, INT32> cg_options;
  std::map<string, INT32> phase_options;

  // 二进制 IR dump / load (Step 10)
  // 每项 (stage, path); stage ∈ {"fe", "opt-high", "opt-mid",
  //   "opt-after-ssa", "opt-low", "opt-vlow", "opt-cgir", "pre-cg"}
  std::vector<std::pair<std::string,std::string>> dump_ir_stages;
  std::string load_ir_path;
  BOOL        dump_textual_after_dump = FALSE;

  BOOL Opt_enabled(OPT_KIND k) {
    if (k == OPT_KIND::OPT_KIND_ARITH) {
      return false;
    }
    return true;
  }
};

// Parsing arguments
int Parse_args(int argc, char **argv, char **envp, COMPILER_CONFIG &conf);

#endif
