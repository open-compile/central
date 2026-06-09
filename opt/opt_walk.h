#ifndef OCC_OPT_WALK_H
#define OCC_OPT_WALK_H

#include "opt_basic.h"
#include "opt_coderep.h"
#include <vector>
#include <stack>

// ====== CFG 遍历 ======

// 计算 RPO（逆后序）：先 DFS 出 post-order，再反转
template <typename CFG_TYPE>
static void Compute_rpo(CFG_TYPE *cfg, std::vector<SSABB *> &rpo) {
  if (cfg->Size() == 0) return;
  std::vector<bool> visited(cfg->Size(), false);
  std::vector<SSABB *> post;
  std::stack<std::pair<SSABB *, UINT32>> stk;
  SSABB *root = (SSABB *)cfg->Node(0);
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
      SSABB *succ = (SSABB *)(*it);
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
}

template <typename CFG_TYPE, typename Visitor>
void Walk_cfg_rpo(CFG_TYPE *cfg, Visitor &vis) {
  std::vector<SSABB *> rpo;
  Compute_rpo(cfg, rpo);
  for (auto *bb : rpo) vis.Visit_bb(bb);
}

// 简单 RPO 顺序迭代器
template <typename Visitor>
void Walk_cfg_rpo(SSA_CFG *cfg, Visitor &vis) {
  std::vector<SSABB *> rpo;
  Compute_rpo(cfg, rpo);
  for (auto *bb : rpo) vis.Visit_bb(bb);
}

// 支配树 DFS 遍历
template <typename Visitor>
void Walk_dom_tree(SSABB *root, Visitor &vis) {
  if (!root) return;
  vis.Visit_bb(root);
  for (auto it = root->Dom_begin(); it != root->Dom_end(); ++it) {
    Walk_dom_tree(*it, vis);
  }
}


// ====== CODEREP 表达式树遍历 ======

// 后序遍历
template <typename Visitor>
void Walk_cr_postorder(CODEREP *cr, Visitor &vis) {
  if (!cr) return;
  if (cr->Is_op()) {
    for (UINT8 i = 0; i < cr->Num_kids(); ++i) {
      Walk_cr_postorder(cr->Opnd(i), vis);
    }
  }
  vis.Visit_cr(cr);
}

// 前序遍历
template <typename Visitor>
void Walk_cr_preorder(CODEREP *cr, Visitor &vis) {
  if (!cr) return;
  vis.Visit_cr(cr);
  if (cr->Is_op()) {
    for (UINT8 i = 0; i < cr->Num_kids(); ++i) {
      Walk_cr_preorder(cr->Opnd(i), vis);
    }
  }
}

#endif //OCC_OPT_WALK_H
