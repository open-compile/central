//
// ============================================================================
// CLAUDE-MARKER  STATUS: REWRITE-REFERENCE (算法会重写)
// ============================================================================
// 文件作用: CONST_PROP  — 常量折叠 (简化版, 非 lattice propagation)
// 关键函数:
//   - Run         — RPO 顺序的 fixed-point 迭代
//   - Fold        — 递归 fold CODEREP
//   - Fold_rhs    — fold STMTREP 的 rhs
//   - Eval_bin    — 二元运算求值
//   - Eval_un     — 一元运算求值
// 局限性 (重写时建议): 真正的 SSA CPROP 需要:
//   1. 对每个 aux 维护常量 lattice (TOP / CONST(c) / BOTTOM / UNDEFINED)
//   2. 沿 use-def 链做 worklist-based 传播 (Wegman-Zadeck)
//   3. condition folding (TRUEBR/FALSEBR 跳过的死分支要 DCE 配合)
//   4. PHI 节点处理: 所有 opnd 都为同一个常数 c 时, phi result = c
//   5. 范围 propagation: 0 <= x < N 这种区间信息
// ============================================================================

#ifndef OCC_OPT_CPROP_H
#define OCC_OPT_CPROP_H

#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_stab.h"
#include "opt_walk.h"

class CONST_PROP {
public:
  // 在 SSA 上做常量传播 + 折叠
  // 返回是否发生了变化
  BOOL Run(SSA_CFG *cfg, OPT_STAB *stab);

private:
  CODEREP *Fold(CODEREP *cr);
  CODEREP *Fold_rhs(CODEREP *cr);
  INT64    Eval_bin(OPCODE opc, INT64 a, INT64 b);
  INT64    Eval_un(OPCODE opc, INT64 a);
  BOOL changed;
};

#endif //OCC_OPT_CPROP_H
