#include "opt_dom.h"
#include <stack>
#include <algorithm>

// CFG_BASE 内部用 CFG_BB_BASE<NODE_TYPE> 作为 BB_TYPE
// 而 SSABB 是派生类，所以下行 cast 是合法的（构造时即用派生类）
static inline SSABB *AS(void *p) {
  return static_cast<SSABB *>(p);
}

void DOM_BUILDER::Build(SSA_CFG *cfg) {
  if (cfg->Size() == 0) return;
  std::vector<SSABB *> rpo;
  Compute_rpo(cfg, rpo);
  Compute_idom(cfg, rpo);
  Compute_dom_list(cfg);
  Compute_df(cfg);
}

void DOM_BUILDER::Compute_rpo(SSA_CFG *cfg, std::vector<SSABB *> &rpo) {
  std::vector<bool> visited(cfg->Size(), false);
  std::vector<SSABB *> post;
  std::stack<std::pair<SSABB *, UINT32>> stk;
  SSABB *root = AS(cfg->Node(0));
  stk.push({root, 0});
  visited[root->Get_id()] = true;
  while (!stk.empty()) {
    auto &top = stk.top();
    SSABB *bb = top.first;
    UINT32 &idx = top.second;
    bool pushed_child = false;
    UINT32 skip = idx;
    idx = 0;
    auto it = bb->Succ_begin();
    for (UINT32 s = 0; s < skip && it != bb->Succ_end(); ++s) ++it;
    while (it != bb->Succ_end()) {
      SSABB *succ = AS(*it);
      ++it;
      ++top.second;
      if (!visited[succ->Get_id()]) {
        visited[succ->Get_id()] = true;
        stk.push({succ, 0});
        pushed_child = true;
        break;
      }
    }
    if (!pushed_child) {
      post.push_back(bb);
      stk.pop();
    }
  }
  for (auto it = post.rbegin(); it != post.rend(); ++it) {
    rpo.push_back(*it);
  }
  // 给 RPO 编号
  _rpo_num.clear();
  for (UINT32 i = 0; i < rpo.size(); ++i) {
    _rpo_num[rpo[i]->Get_id()] = i + 1;  // 从 1 开始，方便判定 undef（0）
  }
}

void DOM_BUILDER::Compute_idom(SSA_CFG *cfg, std::vector<SSABB *> &rpo) {
  if (rpo.empty()) return;
  SSABB *root = rpo[0];
  root->Set_idom(root);

  BOOL changed = TRUE;
  while (changed) {
    changed = FALSE;
    for (size_t i = 1; i < rpo.size(); ++i) {
      SSABB *b = rpo[i];
      // 取第一个已处理过的前驱作为 new_idom 的初值
      SSABB *new_idom = nullptr;
      for (auto it = b->Pred_begin(); it != b->Pred_end(); ++it) {
        SSABB *p = AS(*it);
        if (_rpo_num[p->Get_id()] != 0) {
          new_idom = p;
          break;
        }
      }
      if (!new_idom) continue;
      // 对所有已处理前驱取交集
      for (auto it = b->Pred_begin(); it != b->Pred_end(); ++it) {
        SSABB *p = AS(*it);
        if (_rpo_num[p->Get_id()] != 0) {
          new_idom = Intersect(p, new_idom);
        }
      }
      if (b->Get_idom() != new_idom) {
        b->Set_idom(new_idom);
        changed = TRUE;
      }
    }
  }
}

SSABB *DOM_BUILDER::Intersect(SSABB *b1, SSABB *b2) {
  while (b1 != b2) {
    while (_rpo_num[b1->Get_id()] > _rpo_num[b2->Get_id()]) {
      b1 = AS(b1->Get_idom());
      if (!b1) return nullptr;
    }
    while (_rpo_num[b2->Get_id()] > _rpo_num[b1->Get_id()]) {
      b2 = AS(b2->Get_idom());
      if (!b2) return nullptr;
    }
  }
  return b1;
}

void DOM_BUILDER::Compute_dom_list(SSA_CFG *cfg) {
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *b = AS(cfg->Node(i));
    SSABB *idom = AS(b->Get_idom());
    if (idom && idom != b) {
      idom->add_to_dom_list(b, TRUE);
    }
  }
}

void DOM_BUILDER::Compute_df(SSA_CFG *cfg) {
  // Cytron et al. 算法
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *b = AS(cfg->Node(i));
    if (b->Get_preds_count() < 2) continue;
    // 对每个前驱 p
    for (auto pit = b->Pred_begin(); pit != b->Pred_end(); ++pit) {
      SSABB *p = AS(*pit);
      SSABB *runner = p;
      while (runner != b->Get_idom()) {
        runner->add_to_df_list(b, TRUE);
        SSABB *r_idom = AS(runner->Get_idom());
        if (!r_idom || r_idom == runner) break;
        runner = r_idom;
      }
    }
  }
}
