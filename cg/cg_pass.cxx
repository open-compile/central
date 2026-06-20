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

  // build def use
  return TRUE;
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
  Current_cg_mon()->Backend().Emit_function_header(out, ST_name(func_sym));
  // Letting Cgir to point to current function.
  Current_cg_mon()->Emitter().Emit_tree(func_sym, out, File());
  // TODO: Use CGIR's emission instead.
  return TRUE;
}
