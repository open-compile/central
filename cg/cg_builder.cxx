#include "basic.h"
#include "cfg_common.h"
#include "cg_basic.h"
#include "cgir.h"
#include "cg_pass.h"
#include "stdio.h"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <vector>

/**
 * Building the predecessor list for each CG_CFG_BB
 */
void CGIR_BUILDER::Build_pred_succ() {
  // LRA Algorithm
  CGIR   *cgir      = Cgir();
  CG_CFG *cfg       = cgir->Cfg();
  UINT32  bb_cnt    = cgir->Cfg()->Size();
  INT32   cur_bb    = 0; // currently visited bb;
  CFG_BB_EDGES_STORE preds;
  preds.assign(bb_cnt, {});
  for (cur_bb = bb_cnt - 1; cur_bb >= 0; cur_bb--) {
    // def of bb
    const CGBB             *cgbb         = cfg->BB(cur_bb);
    const CFG_BB_EDGES     &succ         = cfg->Edges(cur_bb);
    CFG_BB_EDGES::iterator  it;
    AssertThat(preds.size() > cur_bb, ("Invalid pred initialization, size = %u, cur_bb = %u",
                 preds.size(), cur_bb));
    for(it = succ.begin(); it != succ.end(); it++){  
      CFG_BB_IDX              succ_idx = *it;
      preds.at(succ_idx).emplace(cur_bb);
    }
  }
  cfg->Set_preds(preds);
}

/**
 * Build def/use information inside the CGBB
 */
void CGIR_BUILDER::Build_def_use() {
  // Def use calculation
  CGIR        *cgir      = Cgir();
  CG_CFG      *cfg       = cgir->Cfg();
  UINT32       bb_cnt    = cgir->Cfg()->Size();
  INT32        cur_bb    = 0; // currently visited bb;
  TN_FREQ_MAP &tn_map    = cgir->TN_freq_map();
  
  tn_map.clear();

  for (cur_bb = bb_cnt - 1; cur_bb >= 0; cur_bb--) {
    // def of bb
    CGBB *cgbb         = cfg->BB(cur_bb);
    CFG_BB_EDGES &succ = cfg->Edges(cur_bb);
    // Is_Trace(TR_LRA(), (TFile, "[LRA] Visiting BB : %d \n", cur_bb));
    // UINT32 stmt_id = 0;
    // for (auto stmt_it = cgbb->First_stmt(); 
    //      stmt_it != cgbb->Last_stmt(); stmt_it++, stmt_id++) {
    //   CGOP *cgop = (*stmt_it);
    //   Is_Trace(TR_LRA(),
    //           (TFile, "Def-use builder: %s\n",
    //             Get_cg_opc_info(cgop->getOpcode())->ins_token));
    //   Cgmon
    //   // TODO(step1: count-uses): 这里是 step 1 — 扫描所有 CGOP, 统计每个 TN
    //   //   的 use/def 数, 并把它塞进 _tn_freq_map / _tn_live_range.
    //   //   真正的 RA 流水线要把这一步的结果:
    //   //     - "每个 TN 被 use 几次" → 用来算 spill cost (cost = uses / degree)
    //   //     - "每个 TN 在哪些 BB 里被 use" → 用来构造 IFG
    //   //   见 cg.spec.md §4.4
    //   if (Get_cg_opc_info(cgop->getOpcode())->n_res >= 1) {
    //     i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 0);
    //   }
    //   if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 1) {
    //     i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 1);
    //   }
    //   if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 2) {
    //     i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 2);
    //   }
    // }
  }
}
