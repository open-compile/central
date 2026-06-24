// ============================================================================
// CLAUDE-MARKER  STATUS: KEEP (Print 实现)
// 配套 opt_print.h; 实现 SSABB / SSA_CFG / DOM / DF 的打印
// ============================================================================
#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_stab.h"
#include "opt_walk.h"
#include "opt_ssa.h"
#include "opt_dom.h"
#include "opt_print.h"
#include "opt_dbg.h"

#include <cstdio>
#include <vector>

// ===== SSABB Print =====

template <typename NODE_TYPE>
void SSA_CFG_BB_BASE<NODE_TYPE>::Print(FILE *f) const {
  if (!f) f = stderr;
  UINT32 id = this->Get_id();
  fprintf(f, "BB[%u] flags=0x%x preds=", id, this->Get_flags());
  for (auto it = this->Pred_begin(); it != this->Pred_end(); ++it) {
    fprintf(f, "%u ", (*it)->Get_id());
  }
  fprintf(f, "succs=");
  for (auto it = this->Succ_begin(); it != this->Succ_end(); ++it) {
    fprintf(f, "%u ", (*it)->Get_id());
  }
  fprintf(f, "\n");
  for (auto *phi : _phi_list) phi->Print(f);
  for (auto *s : _stmtreps) s->Print(f);
}

template <typename NODE_TYPE>
void SSA_CFG_BB_BASE<NODE_TYPE>::Print_pretty(FILE *f) const {
  if (!f) f = stderr;
  UINT32 id = this->Get_id();
  fprintf(f, "BB[%u] flags=0x%x\n", id, this->Get_flags());
  fprintf(f, "  preds:");
  for (auto it = this->Pred_begin(); it != this->Pred_end(); ++it) {
    fprintf(f, " %u", (*it)->Get_id());
  }
  fprintf(f, "\n  succs:");
  for (auto it = this->Succ_begin(); it != this->Succ_end(); ++it) {
    fprintf(f, " %u", (*it)->Get_id());
  }
  fprintf(f, "\n");
  if (this->get_idom(true)) {
    fprintf(f, "  idom=%u\n", this->get_idom(true)->Get_id());
  }
  for (auto *phi : _phi_list) phi->Print(f);
  for (auto *s : _stmtreps) s->Print_pretty(f);
}

template <typename NODE_TYPE>
void SSA_CFG_BB_BASE<NODE_TYPE>::Print_dom(FILE *f) const {
  if (!f) f = stderr;
  fprintf(f, "BB[%u] idom=%u dom_kids=",
          this->Get_id(),
          this->get_idom(true) ? this->get_idom(true)->Get_id() : 0);
  for (auto it = this->Dom_begin(); it != this->Dom_end(); ++it) {
    fprintf(f, " %u", (*it)->Get_id());
  }
  fprintf(f, "\n");
}

template <typename NODE_TYPE>
void SSA_CFG_BB_BASE<NODE_TYPE>::Print_df(FILE *f) const {
  if (!f) f = stderr;
  fprintf(f, "BB[%u] DF=[", this->Get_id());
  for (auto it = this->Df_begin(); it != this->Df_end(); ++it) {
    fprintf(f, " %u", (*it)->Get_id());
  }
  fprintf(f, " ]\n");
}

// 显式实例化 SSABB 用到的 NODE_TYPE=SSANODE
template class SSA_CFG_BB_BASE<SSANODE>;

// ===== SSA_CFG Print =====

static void print_ssa_cfg(SSA_CFG *cfg, FILE *f, BOOL pretty) {
  if (!f) f = stderr;
  fprintf(f, "%s SSA_CFG: %u BBs %s\n", SSA_DBAR, cfg->Size(), SSA_DBAR);
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = (SSABB *)cfg->Node(i);
    if (pretty) bb->Print_pretty(f);
    else        bb->Print(f);
  }
  fprintf(f, "%s end SSA_CFG %s\n", SSA_DBAR, SSA_DBAR);
}

void Print_ssa_cfg(SSA_CFG *cfg, FILE *f, BOOL pretty) {
  print_ssa_cfg(cfg, f, pretty);
}

// ===== DOM Print =====

void Print_dom_tree(SSA_CFG *cfg, FILE *f) {
  if (!f) f = stderr;
  if (cfg->Size() == 0) return;
  fprintf(f, "%s Dominator Tree %s\n", SSA_DBAR, SSA_DBAR);
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = (SSABB *)cfg->Node(i);
    bb->Print_dom(f);
  }
  fprintf(f, "%s end %s\n", SSA_DBAR, SSA_DBAR);
}

void Print_df(SSA_CFG *cfg, FILE *f) {
  if (!f) f = stderr;
  if (cfg->Size() == 0) return;
  fprintf(f, "%s Dominance Frontier %s\n", SSA_DBAR, SSA_DBAR);
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = (SSABB *)cfg->Node(i);
    bb->Print_df(f);
  }
  fprintf(f, "%s end %s\n", SSA_DBAR, SSA_DBAR);
}
