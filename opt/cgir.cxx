//
// Created by xc5 on 2020/7/13.
// This is the IR definition for CG to use, i.e. CG-IR
//
#include "cgir.h"

template
class CFG_BB_BASE<CGOP>;  // CGBB

template
class CFG_BASE<CGOP>;     // CG_CFG

void CGIR::CG_Init(SCOPE *scope) {
  // Do data layout
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::CG_Init\n"));
  ST_IDX func_sym = scope->getSt();
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
  Get_function(func_sym);
  Data_layout(scope);
  AssertThat(layout.find(func_sym) != layout.end(), ("Layout should have this function now. 0x%08x", func_sym));
  // Do IR to CGIR conversion
  this->IR_to_CGIR(scope->getSt());
}

void CGIR::Data_layout(SCOPE *scope) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "Perform data layout in CGIR::Data_layout\n"));
  ST_IDX func_sym = scope->getSt();
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
  layout.insert(std::make_pair(func_sym, new DATA_LAYOUT()));
  AssertThat(layout.find(func_sym) != layout.end(), ("Layout should have this function now. 0x%08x", func_sym));
  layout[func_sym]->Initialize_frame();
  layout[func_sym]->Allocate_file_statics();
}

void CGIR::IR_to_CGIR(ST_IDX sym) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "Perform IR to CGIR conversion in CGIR::IR_to_CGIR\n"));
  File()->Scopes()->Goto_function(sym);
  CG_CFG    *function_cgir = Get_function(sym);
  Set_current_cgir(function_cgir);
  PU_INFO_IDX   info = File()->Tables()->Get_pu_info_by_st_idx(sym);
  PU_INFO      *pu_info = PU_INFO_pu_info(info);
  TREE         *tree = pu_info->entry;
  Handle_Entry(tree, tree->Get_root(), 0);
}

void
CGIR::Handle_STID(TREE *tree, IR_ITER stmt, CFG_BB_IDX cur_bb) {
  TN_IDX result = 0;
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_STID\n"));
  AssertThat(OPCODE_operator(tree->Node(stmt)->Opcode()) == OPR_STID, ("Not a stid to be passed to Handle_stid"));
  if (tree->Node(stmt)) {
    Cfg()->BB(cur_bb);
  }
}

void CGIR::Handle_Entry(TREE *tree, IR_ITER entry, CFG_BB_IDX cur_bb) {
  // Create a block
  cur_bb = Cfg()->Add_bb();
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_Entry\n"));
  AssertThat(OPC_FUNC_ENTRY == tree->Get_node(entry)->Opcode(),
             ("Incorrect root entry"));
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_ENTRY);
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_EXIT);
  // Do nothing
  IR_ITER root = tree->Get_root();
  AssertThat(root != nullptr, ("root should not be empty"));
  AssertThat(tree->Number_of_children(root) == 2, ("there should be exactly 2 nodes in the func_entry"));
  IR_ITER body = tree->Get_operand(root, TREE_SEQ_BODY);
  if (tree->Number_of_children(body) <= 0) {
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_OPTIONS),
             (TFile, "There is no statement in the body, skip conversion to CGIR\n"));
    return;
  }
}

TN_IDX CGIR::PREG_To_TN(TY_IDX preg_ty, PREG_NUM preg_num) {
  AssertThat(false, ("PREG to TN is not implemented"));
  return 1;
}

void CGIR::Set_current_cgir(CG_CFG *cgir) {
  _current = cgir;
}


UINT8 ISA_OPCODE_results(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return CGOPC_INFO_LIST[i].n_res;
    }
  }
  AssertThat(false, ("CGOPC not found in table = %d", cgopc));
  return 0;
}

UINT8 ISA_OPCODE_operands(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return CGOPC_INFO_LIST[i].n_oprs;
    }
  }
  AssertThat(false, ("CGOPC not found in table = %d", cgopc));
  return 0;
}