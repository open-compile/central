//
// ============================================================================
// CLAUDE-MARKER  STATUS: REWRITE-REFERENCE (算法会重写)
// ============================================================================
// 文件作用: DCE  — Mark-Sweep 死代码消除
// 关键函数:
//   - Run        — 主入口
//   - Mark_root  — 标 side-effect 根 (call / io / store / asm)
//   - Mark_cr    — 递归 mark CODEREP 的 def stmt
//   - Mark_stmt  — 通过 _stmt_work 工作队列 mark
//   - Mark_phi   — mark phi + 其 opnd 来源 stmt
//   - Sweep      — 删除未 mark 的 stmt
// 重写提示: 真正的 SSA-aware DCE 应该走:
//   1. 控制流 DCE — 先删 unreachable BB
//   2. 标 side-effect 根 (call/io/asm/store/return) 作为 live
//   3. 反向 liveness: 从 live stmt 的 use 反向 mark def stmt
//   4. 同时处理 phi (phi 自身可能死, 但 opnd 必须 live)
//   5. aggressive DCE 还可删完全死循环 (while(0) ... )
// ============================================================================

#ifndef OCC_OPT_DCE_H
#define OCC_OPT_DCE_H

#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_stab.h"
#include "opt_walk.h"
#include <queue>

class DCE {
public:
  void Run(SSA_CFG *cfg, OPT_STAB *stab);
private:
  std::queue<STMTREP *>  _stmt_work;
  std::queue<PHI_NODE *> _phi_work;

  void Mark_root(SSA_CFG *cfg);
  void Mark_cr(CODEREP *cr);
  void Mark_stmt(STMTREP *s);
  void Mark_phi(PHI_NODE *phi);
  void Sweep(SSA_CFG *cfg);
};

#endif //OCC_OPT_DCE_H
