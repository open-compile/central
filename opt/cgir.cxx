//
// Created by xc5 on 2020/7/13.
// This is the IR definition for CG to use, i.e. CG-IR
//
#include "cgir.h"

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
  Set_current_cgir(function_cgir, sym);
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
  AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in STID, 1 expected, got %d", tree->Number_of_children(stmt)));
  MTYPE_ID stid_type = OPCODE_desc(tree->Get_node(stmt)->Opcode());
  CG_OPRAND res = Get_TN_from_symbol(tree->Get_node(stmt)->Get_symbol_idx());
  IR_ITER expr_val = tree->Get_operand(stmt, 0);
  Handle_Expr(tree, expr_val, cur_bb);
  CG_OPRAND expr = Get_TN_by_ir_node(tree, expr_val, cur_bb);
  CGOP *cgop = new CGOP(CGOPC_STR, cur_bb, res, expr, 0, 0); // Definition a CGOP
  Cfg()->BB(cur_bb)->Add_stmt(cgop);
}

void CGIR::Handle_Entry(TREE *tree, IR_ITER entry, CFG_BB_IDX cur_bb) {
  // Create a block
  cur_bb = Cfg()->Add_bb();
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_Entry\n"));
  AssertThat(OPC_FUNC_ENTRY == tree->Get_node(entry)->Opcode(),
             ("Incorrect root entry"));
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_ENTRY); //Assuming there is only one BB.
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
  // Verifying each statement
  for (UINT32 stmt_idx = 0; stmt_idx < tree->Number_of_children(body); stmt_idx++) {
    IR_ITER stmt = tree->Get_operand(body, stmt_idx);
    AssertThat(*stmt != 0, ("Incorrect child, node 0 should not be a statement, 0 is only allowed in root position"));
    switch (OPCODE_operator(tree->Get_node(stmt)->Opcode())) {
      // What kind of opcode is allowed here.
      case OPR_STID: {
        Handle_STID(tree, stmt, cur_bb);
        break;
      }
      default: {
        AssertThat(false, ("Opcode: %s should not be in the body", tree->Get_node(stmt)->OPCODE_name(tree->Get_node(stmt)->Opcode())));
      }
    }
  }
}

TN_IDX CGIR::Gen_TN(MTYPE_ID preg_ty) {
  return 1;
}

TN_IDX CGIR::PREG_To_TN(TY_IDX preg_ty, PREG_NUM preg_num) {
  AssertThat(false, ("PREG to TN is not implemented"));
  return 1;
}

void CGIR::Set_current_cgir(CG_CFG *cgir, ST_IDX sym) {
  _current = cgir;
  _current_sym = sym;
}

void CGIR::Handle_Expr(TREE *tree, IR_ITER entry, CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_expr\n"));
}

TN_IDX CGIR::Get_TN_from_symbol(ST_IDX sym) {
  return 2;
}

TN_IDX CGIR::Get_TN_by_ir_node(TREE *tree, IR_ITER node, CFG_BB_IDX cur_bb) {
  return 3;
}

void CGIR::Local_register_allocate(PU_INFO *info) {
  Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_INVOCATION),
           (TFile, "CGIR::Local_register_allocate\n"));
}

void CGIR::Print(FILE *file) {
  // Print by functions.
  fprintf(file, "%sPrinting the complete CGIR with %lu functions \n%s",
          DBAR, this->trees.size(), DBAR);
  for (auto iter : this->trees) {
    AssertThat(ST_st(iter.first) != nullptr, ("invalid ST_IDX in the cgir map"));
    fprintf(file, "%sPrinting the CGIR's function : name = %s, sym = 0x%08x\n%s",
            DBAR, ST_name(iter.first), iter.first, DBAR);
    iter.second->Print(file);
  }
}

void CGIR::Print(ST_IDX sym, FILE *file) {
  // Print a functions detail.
  AssertThat(ST_st(sym) != nullptr, ("invalid function ST_IDX to print in CGIR"));
  fprintf(file, "%sPrinting the CGIR's function : name = %s, sym = 0x%08x\n%s",
          DBAR, ST_name(sym), sym, DBAR);
  Get_function(sym)->Print(file);
}

template<typename NODE_TYPE>
void CFG_BB_BASE<NODE_TYPE>::Print(FILE * file) {
  fprintf(file, "===== Printing CFG_BB_BASE id = %d =======\n", _id);
  fprintf(file, "===== with %lu statements inside =======\n", _stmts.size());
  for (UINT32 i = 0; i < _stmts.size(); i++) {
    // STMTs
    _stmts[i]->Print(file);
  }
}


template<typename NODE_TYPE>
void CFG_BASE<NODE_TYPE>::Print(FILE * file) {
  // Print ...
  fprintf(file, "%sPrinting CFG_BASE of size : %d\n%s",
          DBAR, this->Size(), DBAR);
  for (UINT32 i = 0; i < this->Size(); i++) {
    this->Node(i)->Print(file);
  }
}

void CGOP::Print(FILE *file) {
  fprintf(file, "[CGOP] opc = %s(%d), index:%d, operands: [%u] [%u] [%u] [%u] \n",
          ISA_OPCODE_name(getOpcode()), getOpcode(), getIndexInBb(),
          (UINT32) res_opnd[0], (UINT32) res_opnd[1],
          (UINT32) res_opnd[2], (UINT32) res_opnd[3]);

}

CGOPC_INFO CGOPC_INFO_LIST[] = {
  // Opcode     n_res,   n_operands
  // Memory,
  { "CGOPC_MOV",    CGOPC_MOV   , 1,     1 },
  { "CGOPC_STR",    CGOPC_STR   , 1,     2 },
  { "CGOPC_LDR",    CGOPC_LDR   , 1,     2 },
  // Control
  { "CGOPC_LEAVE",  CGOPC_LEAVE , 1,     0 },
  { "CGOPC_CALL",   CGOPC_CALL  , 1,     1 },
  { "CGOPC_BR",     CGOPC_BR    , 1,     1 },
  { "CGOPC_B",      CGOPC_B     , 1,     1 }, // branch as well
  { "CGOPC_BEQ",    CGOPC_BEQ   , 1,     1 },
  { "CGOPC_BNE",    CGOPC_BNE   , 1,     1 },
  { "CGOPC_BGE",    CGOPC_BGE   , 1,     1 },
  { "CGOPC_BLT",    CGOPC_BLT   , 1,     1 },
  { "CGOPC_BGT",    CGOPC_BGT   , 1,     1 },
  { "CGOPC_BLE",    CGOPC_BLE   , 1,     1 },
  // Arithmetic ... TODO: to be addeed
  { "CGOPC_ADD",    CGOPC_ADD   , 1,     2 },
  { "CGOPC_MUL",    CGOPC_MUL   , 1,     2 },
  { "CGOPC_SUBS",   CGOPC_SUBS  , 1,     2 },
};

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

const char *ISA_OPCODE_name(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return CGOPC_INFO_LIST[i].name;
    }
  }
  AssertThat(false, ("CGOPC not found in table = %d", cgopc));
  return nullptr;
}

template
class CFG_BB_BASE<CGOP>;  // CGBB

template
class CFG_BASE<CGOP>;     // CG_CFG