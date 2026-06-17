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

BOOL TR_BUILD() {
  return Tracing(COMPONENT_CG_CONV, TRACE_OPTIONS);
}

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
  TN_FREQ_MAP &tn_freq_map    = cgir->TN_freq_map();
  tn_freq_map.clear();
  for (cur_bb = bb_cnt - 1; cur_bb >= 0; cur_bb--) {
    // def of bb
    CGBB *cgbb         = cfg->BB(cur_bb);
    CFG_BB_EDGES &succ = cfg->Edges(cur_bb);
    Is_Trace(TR_BUILD(), (TFile, "[Def-use] visiting BB : %d \n", cur_bb));

    // Setup stmt_def_use;
    cgbb->Setup_stmt_def_use();

    UINT32 stmt_id = 0;
    for (auto stmt_it = cgbb->Begin_stmt(); 
         stmt_it != cgbb->End_stmt(); stmt_it++, stmt_id++) {
      CGOP *cgop = (*stmt_it);
      Is_Trace(TR_BUILD(),
              (TFile, "Def-use builder: %s\n",
                Get_cg_opc_info(cgop->getOpcode())->ins_token));
      CGOPC_INFO *opc_info = Get_cg_opc_info(cgop->getOpcode());
      UINT32 opr_count = opc_info->getNOprs();
      UINT32 res_count = opc_info->getNRes();

      AssertThat(opr_count  <= 3, ("operand count must be less or eq than 3."));
      AssertThat(res_count  <= 1, ("operand count must be 0 or 1."));

      if (res_count == 1) {
        TN_IDX cgoper = cgop->getResOpnd()[0]; // CG_OPRAND to TN_IDX conversion
        AssertThat(cgoper != 0,
                  ("Adding def in cgbb, oper should not be empty, cgopc(stmt) = %s", (cgop->Print(TFile), 
                   Get_cg_opc_info(cgop->getOpcode())->getName())));

        tn_freq_map[cgoper]++;
        // def TN
        cgbb->Defs().emplace(cgoper);
        cgbb->Stmt_defs(stmt_id).emplace(cgoper);
      }

      for (UINT32 i = 0; i < opr_count; i++) {
        TN_IDX cgoper = cgop->getResOpnd()[res_count + i];
        AssertThat(cgoper != 0,
                  ("Adding use in cgbb, oper should not be empty, cgopc(stmt) = %s", (cgop->Print(TFile), 
                   Get_cg_opc_info(cgop->getOpcode())->getName())));

        tn_freq_map[cgoper]++;
        cgbb->Uses().emplace(cgoper);
        cgbb->Stmt_uses(stmt_id).emplace(cgoper);
      }
    }
  }
}
