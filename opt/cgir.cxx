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
  Handle_Entry(tree->Get_root(), 0);
}

TN*
Handle_LDID(IR_ITER ldid, TN *result) {
  AssertThat(false, ("HandleLDID not impl."));
  return nullptr;
}

void
CGIR::Handle_STID(IR_ITER stmt, CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_STID\n"));
  AssertThat(OPCODE_operator(tree->Node(stmt)->Opcode()) == OPR_STID, ("Not a stid to be passed to Handle_stid"));
  AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in STID, 1 expected, got %d", tree->Number_of_children(stmt)));

  MTYPE_ID stid_type = OPCODE_desc(tree->Get_node(stmt)->Opcode());
  CG_OPRAND res      = 0;
  OPCODE opcode      = tree->Get_node(stmt)->Opcode();

  if (false /* PREG */) {
    TN *tn_res = PREG_to_ST_TN(tree->Get_node(stmt)->Get_symbol_idx(), tree->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
    // TODO: Conversion may still be needed here.
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = Expand_Expr (tree->Get_operand(stmt, 0), stmt, cur_bb, NULL);
    AssertThat(tn_res != NULL, ("Expand of expr should not return null."));
    res = TN_tn_idx(tn_res);
    Exp_Store (OPCODE_desc(opcode),
      tn_res,
      tree->Get_node(stmt)->Get_symbol_idx(),
      tree->Get_node(stmt)->Get_load_offset(),
      cur_bb,
      variant);
    // Add a map
    AssertThat(res < 4096 && res > 0, ("Result tn should be less than 4096 and greater than zero, but it is : %u", res));
  }
  return;
}

void CGIR::Handle_Entry(IR_ITER entry, CFG_BB_IDX cur_bb) {
  // Create a block
  cur_bb = Cfg()->Add_bb();
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_Entry\n"));
  AssertThat(OPC_FUNC_ENTRY == tree->Get_node(entry)->Opcode(),
             ("Incorrect root entry"));

  // Adding entry BB.
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_ENTRY); //Assuming there is only one BB.
  cur_bb = Cfg()->Add_bb(cur_bb);

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
        Handle_STID(stmt, cur_bb);
        break;
      }
      default: {
        AssertThat(false, ("Opcode: %s should not be in the body", OPCODE_name(tree->Get_node(stmt)->Opcode())));
      }
    }
  }
  // Adding function epilog (exit BB)
  cur_bb = Cfg()->Add_bb(cur_bb);
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_EXIT);
  Cfg()->BB(cur_bb)->Add_stmt(
    new CGOP(CGOPC_BX, cur_bb,
             0,
             TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_ra, REGISTER_ra, 4)),
             0, 0));
}

TN *CGIR::PREG_to_TN(TY_IDX preg_ty, PREG_NUM preg_num) {
  AssertThat(false, ("PREG to TN is not implemented"));
  return nullptr;
}

TN *CGIR::PREG_to_ST_TN(ST_IDX sym_idx, PREG_NUM preg_num) {
  AssertThat(false, ("PREG_to_ST_TN is not implemented"));
  return nullptr;
}


void CGIR::Set_current_cgir(CG_CFG *cgir, ST_IDX sym) {
  _current = cgir;
  _current_sym = sym;
  tree = PU_INFO_pu_info(File()->Tables()->Get_pu_info_by_st_idx(sym))->entry;
}

/**
 * Expanding a expression to CGOP sequence.
 * @param entry
 * @param parent
 * @param cur_bb
 * @param result
 * @return
 */
TN *
CGIR::Expand_Expr(IR_ITER entry, IR_ITER parent, CFG_BB_IDX cur_bb, TN *result) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_expr\n"));
  if (result == NULL) {
    result = TN_tn(Gen_TN(MTYPE_I4));
  }
  // TODO: LDID
  switch (OPCODE_operator(tree->Node(entry)->Opcode())) {
    case OPR_LDID: {
      return TN_tn(Gen_TN(MTYPE_I4));
    }
    case OPR_CONST: {
      UINT64 val = tree->Node(entry)->Get_const_val();
      AssertThat(val < (1llu << 32u), ("val should be with in range"));
      Cfg()->BB(cur_bb)->Add_stmt(
          new CGOP(CGOPC_MOV, cur_bb, TN_tn_idx(result),
                 TN_tn_idx(Gen_Literal_TN((val) & 0xFFFF, 2)), 0, 0));
      if (((val >> 16) & 0xFFFF) != 0) {
        Cfg()->BB(cur_bb)->Add_stmt(
          new CGOP(CGOPC_MOVT, cur_bb, TN_tn_idx(result),
                   TN_tn_idx(Gen_Literal_TN((val >> 16) & 0xFFFF, 2)), 0, 0));
      }
      return result;
    }
    default:
      AssertThat(false, ("Operator not implemented : %s", OPCODE_name(tree->Node(entry)->Opcode())));
      return nullptr;
  }
}

/**
 * Find the symbol represented by the TN.
 * @param sym
 * @return
 */
TN_IDX CGIR::Get_TN_from_symbol(ST_IDX sym) {
  TY_IDX ty_idx = ST_ty(sym);
  AssertThat(MTYPE_to_ty(MTYPE_I4) == ty_idx,
             ("Should be a I4 type, rather, it's %d ", ty_idx));
  return Gen_TN(MTYPE_I4);
}

/**
 * Find the TN mapped by the ir, in the map(cached)
 * @param node
 * @param cur_bb
 * @return
 */
TN_IDX CGIR::Get_TN_by_ir_node(IR_ITER node, CFG_BB_IDX cur_bb) {
  // TODO: This is only for testing. change this later.
  return Gen_TN(MTYPE_I4);
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


//void
//CGIR::Exp_Load (
//  TYPE_ID rtype,
//  TYPE_ID desc,
//  TN *tgt_tn,
//  ST *sym,
//  INT64 ofst,
//  OPS *ops,
//  VARIANT variant)
//{
//  OPCODE opcode = OPCODE_make_op (OPR_LDID, rtype, desc);
//  Exp_Ldst (opcode, tgt_tn, sym, ofst, FALSE, FALSE, TRUE, ops, variant);
//  if (TN_register_class(tgt_tn) == ISA_REGISTER_CLASS_mmx)
//    Build_OP(TOP_emms, ops); // bug 11800
//}

OPCODE OPCODE_make_op(OPERATOR opr, MTYPE_ID res, MTYPE_ID desc) {
  return (OPCODE) (opr + RTYPE(res) + DESC(desc));
}

void
CGIR::Exp_Store (
  MTYPE_ID mtype,
  TN *src_tn,
  ST_IDX sym,
  INT64 ofst_val,
  CFG_BB_IDX bb_idx,
  VARIANT variant)
{
  TN *src  = src_tn;
  TN *base = nullptr;
  UINT64 offset_from_base = ofst_val;
  if (ST_sclass(sym) != SYMC_AUTO) {
    // Create a LDR first
    base = TN_tn(Gen_TN(MTYPE_I4));
    LABEL_IDX lbl = Get_addr_label(sym);
    Cfg()->BB(bb_idx)->Add_stmt(
      new CGOP(CGOPC_LDRLBL, bb_idx, TN_tn_idx(base), TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0));
  } else {
    base = Build_Dedicated_TN(REGISTER_CLASS_sp,
                              REGISTER_sp,
                              MTYPE_size(MTYPE_I4));
  }
  TN *ofst = Gen_Literal_TN(ofst_val, 4);
  CGOPC top = CGOPC_STR;
  AssertThat(TN_is_constant(ofst), ("Expand_Store: Illegal offset TN"));
  if (!TN_has_value(ofst) || TN_value(ofst) < (1 << 16)) {
    Cfg()->BB(bb_idx)->Add_stmt(
      new CGOP(top, bb_idx, TN_tn_idx(src), TN_tn_idx(base),
               TN_tn_idx(ofst), 0));
  } else {
    AssertThat(false, ("Not implmented Exp_store case"));
  }
}

void
CGIR::Exp_Ldst (
  OPCODE opcode,
  TN *tn,
  ST_IDX sym,
  INT64 ofst,
  BOOL indirect_call,
  BOOL is_store,
  BOOL is_load,
  CFG_BB_IDX bb_idx,
  VARIANT variant)
{

}

void CGIR::Exp_op(OPCODE opcode, TN *result, TN *op1, TN *op2, TN *op3,
                  VARIANT variant, CGOP *ops) {
  // ...
  AssertThat(false, ("Exp_op not impl."));
}

VARIANT CGIR::Memop_Variant(IR_ITER iterator) {
  return V_BR_NONE;
}

void CGIR::Emit_label(PU_INFO *func, FILE *out, LABEL_IDX label_idx) {
  fprintf(out, "%s:\n", LABEL_name(label_idx));
}

void CGIR::Emit_operand(CGOP *oper, CGOPR_KIND kind, UINT32 ch_id, FILE* out) {
  AssertThat(ch_id < 4, ("operand count must be less than 4."));
  CG_OPRAND cgoper = oper->getResOpnd()[ch_id];
  AssertThat(cgoper != 0,
             ("Should not be empty, cgopc = %s", Get_cg_opc_info(
               oper->getOpcode())->getName()));
  if (CGOPR_R == kind) {
    TN *tn = TN_tn(cgoper.tn);
    if (TN_is_symbol(tn)) {
      ST_IDX sym = TN_var(tn);
      AssertThat(ST_sclass(sym) == SYMC_FILE_STATIC,
                 ("this should only be used for static/global var."));
      // Generate labels for current func.
      fprintf(out, "%s ", LABEL_name(Get_addr_label(sym)));
    } else if (TN_is_label(tn)) {
      LABEL_IDX lbl = TN_label(tn);
      fprintf(out, "%s ", LABEL_name(lbl));
    } else if (TN_is_constant(tn)) {
      fprintf(out, "#%lld ", TN_value(tn));
    } else if (TN_is_label(tn)) {
      fprintf(out, ".%s ", LABEL_name(TN_label(tn)));
    } else if (TN_is_dedicated(tn)) {
      UINT32 reg_id = TN_register(tn);
      if (reg_id == REGISTER_ra) {
        fprintf(out, "lr ");
      } else if (reg_id == REGISTER_sp) {
        fprintf(out, "sp");
      } else if (reg_id == REGISTER_fp) {
        fprintf(out, "fp");
      } else {
        AssertThat(false, ("not implemented"));
      }
    } else {
      UINT32 reg_id = TN_register(tn);
      fprintf(out, "r%d ", reg_id);
    }
  } else if (CGOPR_IMM == kind) {
    UINT32 val = TN_value(TN_tn(cgoper.tn));
    fprintf(out, "#%d ", val);
  } else {
    AssertThat(false, ("not implemented."));
  }
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

template<typename NODE_TYPE>
CFG_BB_IDX CFG_BASE<NODE_TYPE>::Add_bb(INT pred) {
  CFG_BB_IDX new_bb = Add_bb();
  this->BB(pred)->Add_succ(BB(new_bb));
  this->BB(new_bb)->Add_pred(BB(pred));
  return new_bb;
}

void CGOP::Print(FILE *file) {
  fprintf(file, "[CGOP] opc = %s(%d), index:%d, res/opnd: [%u] [%u] [%u] [%u] \n",
          ISA_OPCODE_name(getOpcode()), getOpcode(), getIndexInBb(),
          (UINT32) res_opnd[0], (UINT32) res_opnd[1],
          (UINT32) res_opnd[2], (UINT32) res_opnd[3]);
}

CGOPC_INFO CGOPC_INFO_LIST[] = {
#define CGOPDEF(enum_name, nres, nopr, opr1, opr2, opr3, ins_name, kind)   \
  { #enum_name, enum_name, nres, nopr, opr1, opr2, opr3, ins_name, kind },
#include "cg_opc.h"
#undef CGOPDEF
};

CGOPC_INFO *CGIR::Get_cg_opc_info(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return &(CGOPC_INFO_LIST[i]);
    }
  }
  AssertThat(false, ("Cannot find opcode in table = %d", cgopc));
  return &(CGOPC_INFO_LIST[0]);
}

LABEL_IDX CGIR::Get_addr_label(ST_IDX sym) {
  char *targ = (char*) malloc(sizeof(".taddr_") + strlen(ST_name(sym)) + 2);
  sprintf(targ, ".taddr_%s", ST_name(sym));
  for (UINT32 i       = 1; i < File()->Tables()->Label()->Length(File()->Scopes()->Current()); i++) {
    LABEL_IDX lbl = (LABEL_IDX) (i << 8) + LOCAL_SYMTAB;
    if (LABEL_label(lbl)->Get_temp_sym() == sym) {
      free(targ);
      return lbl;
    }
  }
  STR_IDX     name    = File()->Save_string(targ);
  LABEL_IDX   lbl_idx = File()->Create_label(name, LABEL_ADDR_PASSED &
                                             LABEL_ADDR_SAVED, LKIND_ASSIGNED);
  LABEL_label(lbl_idx)->Set_temp_sym(sym);
  free(targ);
  return lbl_idx;
}

BOOL CGIR::CGOPC_is_ldst(CGOPC cgopc) {
  return (Get_cg_opc_info(cgopc)->opk == CGOPK_LDST);
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
