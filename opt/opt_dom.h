//
// ============================================================================
// CLAUDE-MARKER  STATUS: REWRITE-REFERENCE (算法会重写)
// ============================================================================
// 文件作用: DOM_BUILDER  — 支配树 (Cooper) + 支配边界 (Cytron)
// 关键函数:
//   - Build                  — 主入口
//   - Compute_rpo            — 迭代 DFS 求 RPO
//   - Compute_idom           — Cooper 算法
//   - Intersect              — idom 求交
//   - Compute_dom_list       — 父-子链表
//   - Compute_df             — Cytron et al. 算法
// 已知问题 (重写时建议):
//   1. Compute_idom 中使用 b->get_idom(true) 而非 Get_idom() 因为后者有断言
//   2. 多 BB 函数上 RPO 只含 BB[0] (rpo.size()=1), 迭代从未执行 — 待 fix
//   3. _rpo_num 是 unordered_map, 可换成 vector 提速
//   4. Cooper 是 O(N^2); 大函数可换 Lengauer-Tarjan (O((N+E) α))
// ============================================================================

#ifndef OCC_OPT_DOM_H
#define OCC_OPT_DOM_H

#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_walk.h"
#include <vector>
#include <unordered_map>

class DOM_BUILDER {
public:
  // 计算 CFG 的 idom / dom_list / df_list
  void Build(SSA_CFG *cfg);
  // 单独计算 RPO（供其它 pass 复用）
  void Compute_rpo(SSA_CFG *cfg, std::vector<SSABB *> &rpo);

private:
  void Compute_idom(SSA_CFG *cfg, std::vector<SSABB *> &rpo);
  void Compute_dom_list(SSA_CFG *cfg);
  void Compute_df(SSA_CFG *cfg);
  SSABB *Intersect(SSABB *b1, SSABB *b2);

  std::unordered_map<UINT32, UINT32> _rpo_num;  // bb_id -> RPO 顺序编号
};

#endif //OCC_OPT_DOM_H
