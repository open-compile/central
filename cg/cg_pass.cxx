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

using std::vector;
using std::unordered_set;
using std::unordered_map;


BOOL CG_BUILD_PASS::Run(PU_INFO *pu){
  
  // Do data layout & calculate frame (activation) size
  Current_cg_mon()->Builder().Data_layout(&pu->Scope());
  ST_IDX func_sym = pu->Proc_sym();
  AssertThat(Cgir()->Layouts().find(func_sym) != Cgir()->Layouts().end(), ("Layout should have this function now. 0x%08x", func_sym));
  Cgir()->Layout()->Calculate_stack_frame_size(); // Calc frame size

  // Build CG CFG and CGIR
  Current_cg_mon()->Builder().Handle_func_body(Cgir()->Current_tree()->Get_root(), 0);

  // Build auxiliary info
  Build_pred_succ(pu);
  Build_def_use(pu);

  // build def use
  return TRUE;
}


void CG_BUILD_PASS::Build_pred_succ(PU_INFO *pu) {
  // LRA Algorithm
  CGIR   *cgir      = Cgir();
  CG_CFG *cfg       = cgir->Cfg();
  UINT32  bb_cnt    = cgir->Cfg()->Size();
  UINT32  cur_bb    = 0; // currently visited bb;
  for (cur_bb = bb_cnt - 1; cur_bb >= 0; cur_bb--) {
    // def of bb
    CGBB *cgbb         = cfg->BB(cur_bb);
    CFG_BB_EDGES &succ = cfg->Edges(cur_bb);
    
  }
}

void CG_BUILD_PASS::Build_def_use(PU_INFO *pu) {
  // Def use calculation
  CGIR   *cgir      = Cgir();
  CG_CFG *cfg       = cgir->Cfg();
  UINT32  bb_cnt    = cgir->Cfg()->Size();
  UINT32  cur_bb    = 0; // currently visited bb;
  for (cur_bb = bb_cnt - 1; cur_bb >= 0; cur_bb--) {
    // def of bb
    CGBB *cgbb         = cfg->BB(cur_bb);
    CFG_BB_EDGES &succ = cfg->Edges(cur_bb);
    
  }
  for (cur_bb = bb_cnt - 1; cur_bb >= 0; cur_bb--) {
      unordered_set<int> old_in = live_ins[cur_bb]; // copy
      unordered_set<int> old_out = live_outs[cur_bb]; // copy
      // def of bb
      CGBB *cgbb = cfg->BB(cur_bb);
      Is_Trace(TR_LRA(), (TFile, "[LRA] Visiting BB : %d \n", cur_bb));
      UINT32 stmt_id = 0;
      for (auto stmt_it = cgbb->First_stmt(); stmt_it != cgbb->Last_stmt(); stmt_it++, stmt_id++) {
        CGOP *cgop = (*stmt_it);
        Is_Trace(TR_LRA(),
                (TFile, "LRA: Processing op = %s\n",
                  Get_cg_opc_info(cgop->getOpcode())->ins_token));
        // TODO(step1: count-uses): 这里是 step 1 — 扫描所有 CGOP, 统计每个 TN
        //   的 use/def 数, 并把它塞进 _tn_freq_map / _tn_live_range.
        //   真正的 RA 流水线要把这一步的结果:
        //     - "每个 TN 被 use 几次" → 用来算 spill cost (cost = uses / degree)
        //     - "每个 TN 在哪些 BB 里被 use" → 用来构造 IFG
        //   见 cg.spec.md §4.4
        if (Get_cg_opc_info(cgop->getOpcode())->n_res >= 1) {
          i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 0);
        }
        if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 1) {
          i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 1);
        }
        if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 2) {
          i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 2);
        }
      }
      // use of bb
    }
}

BOOL CG_LIVE_RANGE_PASS::Run(PU_INFO *pu) {
  // 原逻辑：Cgmon()->Lra().Analyze_live_range(pu_info)
  Current_cg_mon()->Lra().Analyze_live_range(pu);
  return TRUE;
}

void CG_LIVE_RANGE_PASS::Dump(FILE *file) {
  Current_cg_mon()->Lra().Print(file);
}

BOOL CG_REG_ALLOC_PASS::Run(PU_INFO *pu){
  // 原逻辑：Cgmon()->Reg_alloc().Register_allocate(pu_info)
  Current_cg_mon()->Reg_alloc().Register_allocate(pu);
  return TRUE;
}

void CG_REG_ALLOC_PASS::Dump(FILE *file) {
  Current_cg_mon()->Reg_alloc().Print_freq_map(file);
  Current_cg_mon()->Reg_alloc().Print_live_range(file);
}

BOOL CG_FRAME_LAYOUT_PASS::Run(PU_INFO *pu) {
  // 原逻辑
  Cgir()->Layout()->Calculate_stack_frame_size();
  Cgir()->Recalibrate_offset(pu);
  return TRUE;
}

void CG_FRAME_LAYOUT_PASS::Dump(FILE *file) {
  Cgir()->Layout()->Print(file);
}

BOOL CG_EMITTER_PASS::Run(PU_INFO *pu){
  FILE *out = Outfile_safe();
  ST_IDX func_sym = pu->Proc_sym();
  SCOPE *scope    = &pu->Scope();
  UINT32 scope_sym_cnt = File()->Tables()->Sym()->Length(scope);
  Is_Trace(Tracing(COMPONENT_CG_EMIT, TRACE_INFO),
           (out, "# Function has %d non-trivial symbols\n", scope_sym_cnt - 1));
  for (UINT32 i = 1; i < scope_sym_cnt; i++) {
    ST_IDX one = (i << 8) | LOCAL_SYMTAB;
    fprintf(out, "# Id: 0x%08x, Symbol : %s, Type: %d, Ofst: %d\n", one,
            ST_name(one), ST_ty(one), Cgir()->Layout()->Get_sym_sp_ofst(one));
    // Find INITO matching this.
    INITO_IDX inito_idx = ST_st(one)->getInitoIdx();
    // Generate initv
    INITO *inito = INITO_inito(inito_idx);
    for (UINT32 j = 0; j < inito->Size(); j++) {
      if (inito->Value(j)->kind == INITVKIND_VAL) {
        fprintf(out, ".word %lld\n", inito->Value(j)->Val());
      } else if (inito->Value(j)->kind == INITVKIND_PAD) {
        fprintf(out, ".zero %lld\n", inito->Value(j)->Val());
      }
    }
  }

  // Dump the instructions
  const char *func_name = ST_name(func_sym);
  fprintf(out, ".global %s\n", func_name);
  fprintf(out, "%s: \n", func_name);
  // Letting Cgir to point to current function.
  Current_cg_mon()->Emitter().Emit_tree(func_sym, out, File());
  // TODO: Use CGIR's emission instead.
  return TRUE;
}


