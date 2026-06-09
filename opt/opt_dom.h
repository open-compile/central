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
