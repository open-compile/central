// ============================================================================
// CLAUDE-MARKER  STATUS: REWRITE-REFERENCE (算法会重写)
// 配套 opt_dce.h
// ============================================================================
#include "opt_dce.h"
#include <algorithm>

void DCE::Run(SSA_CFG *cfg, OPT_STAB *stab) {
  if (cfg->Size() == 0) return;
  Mark_root(cfg);

  while (!_stmt_work.empty() || !_phi_work.empty()) {
    if (!_phi_work.empty()) {
      PHI_NODE *p = _phi_work.front();
      _phi_work.pop();
      Mark_phi(p);
    }
    if (!_stmt_work.empty()) {
      STMTREP *s = _stmt_work.front();
      _stmt_work.pop();
      Mark_stmt(s);
    }
  }
  Sweep(cfg);
}

void DCE::Mark_root(SSA_CFG *cfg) {
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = cfg->Node(i);
    for (auto *s : bb->_stmtreps) {
      if (s->Has_side_effect()) {
        s->_live = TRUE;
        _stmt_work.push(s);
      }
    }
    // phi 默认不死：仅在 phi 的 result 被使用时才被标记
  }
}

void DCE::Mark_cr(CODEREP *cr) {
  if (!cr) return;
  // 直接用类内成员 + DCE 实例
  DCE *self = this;
  struct V {
    DCE *self;
    void Visit_cr(CODEREP *c) {
      if (!c->Is_var()) return;
      if (c->Version() == 0) return;  // 未版本化（如地址），跳过
      // 找到 def_stmt
      STMTREP *def_s = c->Def_stmt();
      PHI_NODE *def_p = c->Def_phi();
      if (def_s && !def_s->_live) {
        def_s->_live = TRUE;
        self->_stmt_work.push(def_s);
      } else if (def_p && !def_p->_live) {
        def_p->_live = TRUE;
        self->_phi_work.push(def_p);
      }
    }
  } vis{self};
  Walk_cr_postorder(cr, vis);
}

void DCE::Mark_stmt(STMTREP *s) {
  if (s->_rhs) Mark_cr(s->_rhs);
  if (s->_cond) Mark_cr(s->_cond);
  if (s->_lhs && s->_lhs->Is_ivar()) Mark_cr(s->_lhs->_ivar._base);
  // lhs 是定义：不需要追溯
}

void DCE::Mark_phi(PHI_NODE *phi) {
  // phi 的所有操作数都是对此变量某个版本的 use
  // 用 OPT_STAB 找每个 opnd_ver 的 def
  for (UINT32 i = 0; i < phi->_opnd_vers.size(); ++i) {
    UINT32 v = phi->_opnd_vers[i];
    if (v == 0) continue;
    // 我们需要 stab 来查 def
    // 简单实现：使用 CODEREP opnd_crs，若没有就直接置 0
    // 在 SSA_COMPOSITE::SSA_Rename 阶段已通过 _opnd_crs 记录了
    CODEREP *cr = phi->_opnd_crs[i];
    if (cr) Mark_cr(cr);
  }
}

void DCE::Sweep(SSA_CFG *cfg) {
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = cfg->Node(i);
    bb->_stmtreps.erase(
        std::remove_if(bb->_stmtreps.begin(), bb->_stmtreps.end(),
                       [](STMTREP *s) { return !s->_live; }),
        bb->_stmtreps.end());
    bb->_phi_list.erase(
        std::remove_if(bb->_phi_list.begin(), bb->_phi_list.end(),
                       [](PHI_NODE *p) { return !p->_live; }),
        bb->_phi_list.end());
  }
}
