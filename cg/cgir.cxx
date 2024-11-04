//
// Created by xc5 on 2020/7/13.
// This is the IR definition for CG to use, i.e. CG-IR
//
#include "cgir.h"
#include <algorithm>

INLINE BOOL TR_LRA() {
  return Tracing(COMPONENT_CG_LRA, TRACE_DATA);
}

void CG_COMPOSITE::CG_convert_function(SCOPE *scope) {
  // Do data layout
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::CG_Expand\n"));
  ST_IDX func_sym = scope->getSt();
  File()->Scopes()->Goto_function(func_sym);
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
  Cgir()->Get_function(func_sym);

  // Do data layout & calculate frame (activation) size
  Cgir_builder().Data_layout(scope);
  AssertThat(Cgir()->Layouts().find(func_sym) != Cgir()->Layouts().end(), ("Layout should have this function now. 0x%08x", func_sym));
  Cgir()->Layout()->Calculate_stack_frame_size(); // Calc frame size

  // Do IR to CGIR conversion
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "Perform IR to CGIR conversion in CGIR::IR_to_CGIR\n"));
  CG_CFG    *function_cgir = Cgir()->Get_function(func_sym);
  Cgir()->Set_current_cgir(function_cgir, func_sym);
  Cgir_builder().Handle_func_body(Cgir()->Current_tree()->Get_root(), 0);
}

void CGIR_BUILDER::Data_layout(SCOPE *scope) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "Perform data layout in CGIR::Data_layout\n"));
  ST_IDX func_sym = scope->getSt();
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
  DATA_LAYOUT *onelayout = new DATA_LAYOUT();
  Cgir()->Layouts().insert(std::make_pair(func_sym, onelayout));
  AssertThat(Cgir()->Layouts().find(func_sym) != Cgir()->Layouts().end(), ("Layout should have this function now. 0x%08x", func_sym));
  Cgir()->Layouts()[func_sym]->Initialize_frame(scope, func_sym);
  Cgir()->Set_current_layout(onelayout);
}

void CGIR_BUILDER::Handle_stid(IR_ITER stmt, CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_STID\n"));
  AssertThat(OPCODE_operator(Tree()->Node(stmt)->Opcode()) == OPR_STID, ("Not a stid to be passed to Handle_stid"));
  AssertThat(Tree()->Number_of_children(stmt) == 1, ("Incorrect number of kid in STID, 1 expected, got %d", Tree()->Number_of_children(stmt)));

  MTYPE_ID stid_type = OPCODE_desc(Tree()->Get_node(stmt)->Opcode());
  CG_OPRAND res      = 0;
  OPCODE opcode      = Tree()->Get_node(stmt)->Opcode();

  ST_IDX sym = Tree()->Node(stmt)->Get_symbol_idx();
  INT64 ofst = Tree()->Node(stmt)->Get_load_offset();
  if (ST_symclass(sym) == SYM_CLASS_PREG) {
    TN *tn_res = Cgir()->PREG_to_TN(ST_ty(sym), ofst);
    res = TN_tn_idx(tn_res);
    Expand_expr (Tree()->Get_operand(stmt, 0), stmt, cur_bb, tn_res);
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = Expand_expr (Tree()->Get_operand(stmt, 0), stmt, cur_bb, NULL);
    AssertThat(tn_res != NULL, ("Expand of expr should not return null."));
    res = TN_tn_idx(tn_res);
    Exp_load_store(opcode,
             OPCODE_desc(opcode),
             tn_res,
             nullptr,
             Tree()->Get_node(stmt)->Get_symbol_idx(),
             Tree()->Get_node(stmt)->Get_load_offset(),
             stmt,
             cur_bb,
             variant);
  }
  return;
}


TN *
CGIR_BUILDER::Handle_ldid(IR_ITER stmt, CFG_BB_IDX cur_bb, TN *target_res) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_LDID\n"));
  AssertThat(OPCODE_operator(Tree()->Node(stmt)->Opcode()) == OPR_LDID,
             ("Not a ldid to be passed to Handle_ldid"));
  AssertThat(Tree()->Number_of_children(stmt) == 0,
             ("Incorrect number of kid in LDID, 0 expected, got %d", Tree()->Number_of_children(
               stmt)));

  CG_OPRAND res       = 0;
  OPCODE    opcode    = Tree()->Get_node(stmt)->Opcode();
  if (target_res != nullptr) {
    res = TN_tn_idx(target_res);
  } else {
    res = Gen_TN(MTYPE_I4);
  }
  ST_IDX sym = Tree()->Node(stmt)->Get_symbol_idx();
  INT64 ofst = Tree()->Node(stmt)->Get_load_offset();
  if (ST_symclass(sym) == SYM_CLASS_PREG) {
    TN *current = Cgir()->PREG_to_TN(ST_ty(sym),
                            ofst);
    res = TN_tn_idx(current);
    if (target_res != nullptr) {
      // Transfer needed
      TN_IDX final = TN_tn_idx(target_res);
      CGOP *cgop = new CGOP(CGOPC_MOV, *stmt, cur_bb, final, res, 0, 0);
      Cfg()->BB(cur_bb)->Add_stmt(cgop);
    } else {
      target_res = current;
    }
    return target_res;
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = TN_tn(res);
    Exp_load_store(opcode,
             OPCODE_desc(opcode),
             tn_res,
             nullptr,
             sym,
             ofst,
             stmt,
             cur_bb,
             variant);
    return tn_res;
  }
}


TN *
CGIR_BUILDER::Handle_iload(IR_ITER stmt, CFG_BB_IDX cur_bb, TN *target_res) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_ILOAD\n"));
  AssertThat(OPCODE_operator(Tree()->Node(stmt)->Opcode()) == OPR_ILOAD,
             ("Not a ILOAD to be passed to Handle_ILOAD"));
  AssertThat(Tree()->Number_of_children(stmt) == 1,
             ("Incorrect number of kid in ILOAD, 1 expected, got %d", Tree()->Number_of_children(
               stmt)));

  TN *base_tn = Expand_expr (Tree()->Get_operand(stmt, 0), stmt, cur_bb, NULL);
  AssertThat(base_tn != nullptr, ("base tn should not be null in ILOAD."));

  CG_OPRAND res       = 0;
  OPCODE    opcode    = Tree()->Get_node(stmt)->Opcode();
  if (target_res != nullptr) {
    res = TN_tn_idx(target_res);
  } else {
    res = Gen_TN(MTYPE_I4);
  }
  if (false /* PREG */) {
    TN *tn_res = Cgir()->PREG_to_ST_TN(Tree()->Get_node(stmt)->Get_symbol_idx(),
                               Tree()->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
    // TODO: Conversion may still be needed here.
    return tn_res;
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = TN_tn(res);
    Exp_load_store(opcode,
             OPCODE_desc(opcode),
             tn_res,
             base_tn,
             Tree()->Get_node(stmt)->Get_symbol_idx(),
             Tree()->Get_node(stmt)->Get_load_offset(),
             stmt,
             cur_bb,
             variant);
    return tn_res;
  }
}


TN *
CGIR_BUILDER::Handle_istore(IR_ITER stmt, CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_ISTORE\n"))
  AssertThat(OPCODE_operator(Tree()->Node(stmt)->Opcode()) == OPR_ISTORE,
             ("Not a ILOAD to be passed to Handle_ISTORE"));
  AssertThat(Tree()->Number_of_children(stmt) == 2,
             ("Incorrect number of kid in ILOAD, 1 expected, got %d", Tree()->Number_of_children(
               stmt)));

  TN *base_tn = Expand_expr (Tree()->Get_operand(stmt, 1), stmt, cur_bb, NULL);
  AssertThat(base_tn != nullptr, ("base tn should not be null in ISTORE."));

  OPCODE    opcode    = Tree()->Get_node(stmt)->Opcode();
  CG_OPRAND res = 0;
  if (false /* PREG */) {
    TN *tn_res = Cgir()->PREG_to_ST_TN(Tree()->Get_node(stmt)->Get_symbol_idx(),
                               Tree()->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
    // TODO: Conversion may still be needed here.
    return tn_res;
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = Expand_expr (Tree()->Get_operand(stmt, 0), stmt, cur_bb, NULL);
    AssertThat(tn_res != NULL, ("Expand of expr should not return null."));
    res = TN_tn_idx(tn_res);
    Exp_load_store(opcode,
             OPCODE_desc(opcode),
             tn_res,
             base_tn,
             0,
             0,
             stmt,
             cur_bb,
             variant);
    return tn_res;
  }
}


/**
 * Convert LDA from IR to CGIR.
 * @param expr IR TREE to convert.
 * @param cur_bb current BB to save the result.
 * @param target_res the target TN to store the result of LDA.
 * @return
 */
TN *
CGIR_BUILDER::Handle_lda(IR_ITER expr, CFG_BB_IDX cur_bb, TN *target_res) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_LDA\n"));
  AssertThat(OPCODE_operator(Tree()->Node(expr)->Opcode()) == OPR_LDA,
             ("Not a LDA to be passed to Handle_LDA"));
  AssertThat(Tree()->Number_of_children(expr) == 0,
             ("Incorrect number of kid in LDA, 0 expected, got %d", Tree()->Number_of_children(
               expr)));
  AssertThat(Tree()->Node(expr)->Get_symbol_idx() != 0,
             ("There should be a valid symbol bound to it."));
  ST_IDX sym = Tree()->Node(expr)->Get_symbol_idx();
  if (target_res == nullptr) {
    target_res = TN_tn(Gen_TN(MTYPE_I4));
  }
  AssertThat(target_res != NULL, ("Expand of expr should not return null."));
  if (ST_sclass(sym) == SYMC_FILE_STATIC) {
    // LOCAL VAR.
    // SP + OFST
    LABEL_IDX lbl = Get_addr_label(sym);
    Cfg()->BB(cur_bb)->Add_stmt(
      new CGOP(CGOPC_LDRLBL, *expr, cur_bb, TN_tn_idx(target_res), TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0));
  } else if (ST_sclass(sym) == SYMC_AUTO ||
    ST_sclass(sym) == SYMC_FORMAL) {
    TN *sp_tn = Build_Dedicated_TN(REGISTER_CLASS_sp,
                              REGISTER_sp,
                              MTYPE_size(MTYPE_I4));
    INT64 offset_from_base = Cgir()->Layout()->Get_sym_sp_ofst(sym);
    Cfg()->BB(cur_bb)->Add_stmt(
      new CGOP(CGOPC_ADD, *expr, cur_bb,
               TN_tn_idx(target_res),
               TN_tn_idx(sp_tn),
               TN_tn_idx(Gen_Literal_TN(offset_from_base, 4)), 0));
  }
  return target_res;
}

void
CGIR_BUILDER::Handle_ret_val(IR_ITER stmt, CFG_BB_IDX cur_bb, CG_CONV_INFO &builder) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_ret_val\n"));
  AssertThat(OPCODE_operator(Tree()->Node(stmt)->Opcode()) == OPR_RETURN_VAL,
             ("Not correct opr to be passed to Handle_ret_val"));
  AssertThat(Tree()->Number_of_children(stmt) == 1,
             ("Incorrect number of kid in %s, 1 expected, got %d",
               OPCODE_name(Tree()->Node(stmt)->Opcode()), Tree()->Number_of_children(
               stmt)));

  MTYPE_ID  ldid_type = OPCODE_desc(Tree()->Get_node(stmt)->Opcode());
  CG_OPRAND res       = 0;
  OPCODE    opcode    = Tree()->Get_node(stmt)->Opcode();

  if (false /* PREG */) {
    TN *tn_res = Cgir()->PREG_to_ST_TN(Tree()->Get_node(stmt)->Get_symbol_idx(),
                                       Tree()->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
  } else {
    TN      *func_val = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
    Set_TN_register(func_val, 0);
    Set_TN_is_preallocated(func_val);
    TN      *tn_res   = Expand_expr (Tree()->Get_operand(stmt, 0), stmt, cur_bb, func_val);
  }
  Handle_ret(stmt, cur_bb, builder);
}

void CGIR_BUILDER::Handle_ret(IR_ITER stmt, CFG_BB_IDX cur_bb, CG_CONV_INFO &builder) {
  LABEL_IDX lbl   = File()->Get_func_exit_label();
  CGOP      *cgop = new CGOP(CGOPC_B, *stmt, cur_bb,
                               0, TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0);
  Cfg()->BB(cur_bb)->Add_stmt(cgop);
}

/**
 * Convert a function's body to CGIR
 * @param entry function body
 * @param cur_bb current BB.
 */
void CGIR_BUILDER::Handle_func_body(IR_ITER entry, CFG_BB_IDX cur_bb) {
  // There could be global stuff here.
  CG_CONV_INFO builder;
  builder.Init(Cfg());

  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_Entry\n"));
  AssertThat(OPC_FUNC_ENTRY == Tree()->Get_node(entry)->Opcode(),
             ("Incorrect root entry"));

  // Initiate the prolog of this function.
  cur_bb = Add_prolog(cur_bb);

  Add_store_formals(entry, cur_bb);

  // Do nothing
  IR_ITER root = Tree()->Get_root();
  AssertThat(root != nullptr, ("root should not be empty"));
  AssertThat(Tree()->Number_of_children(root) == 2, ("there should be exactly 2 nodes in the func_entry"));
  IR_ITER body = Tree()->Get_operand(root, TREE_SEQ_BODY);
  if (Tree()->Number_of_children(body) <= 0) {
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_OPTIONS),
             (TFile, "There is no statement in the body, skip conversion to CGIR\n"));
    return;
  }

  // stmt having already proccessed in BB
  UINT32 cur_bb_stmt_processed = 0;

  //FIXME: There should not be so much BB, There's simply too much BB here.
  // Verifying each statement
  for (UINT32 stmt_idx = 0; stmt_idx < Tree()->Number_of_children(body); stmt_idx++) {
    IR_ITER stmt = Tree()->Get_operand(body, stmt_idx);
    AssertThat(*stmt != 0, ("Incorrect child, node 0 should not be a statement, 0 is only allowed in root position"));
    switch (OPCODE_operator(Tree()->Get_node(stmt)->Opcode())) {
      // What kind of opcode is allowed here.
      case OPR_STID: {
        Handle_stid(stmt, cur_bb);
        cur_bb_stmt_processed ++;
        break;
      }
      case OPR_ISTORE: {
        Handle_istore(stmt, cur_bb);
        cur_bb_stmt_processed ++;
        break;
      }
      case OPR_RETURN: {
        Handle_ret(stmt, cur_bb, builder);
        cur_bb_stmt_processed ++;
        break;
      }
      case OPR_RETURN_VAL: {
        Handle_ret_val(stmt, cur_bb, builder);
        cur_bb_stmt_processed ++;
        break;
      }
      case OPR_FALSEBR:
      case OPR_TRUEBR:
      case OPR_GOTO: {
        // Create a new BB in jumps
        CFG_BB_IDX next_bb = 0;
        if(OPCODE_operator(Tree()->Get_node(stmt)->Opcode()) == OPR_GOTO) {
          next_bb = Cfg()->Add_bb();
        } else {
          next_bb = Cfg()->Add_bb(cur_bb);
        }
        Handle_goto(stmt, cur_bb, next_bb, builder);
        Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
                 (TFile, "After [goto], end of BB = %d, start next BB = %d\n",
                  cur_bb, next_bb));
        cur_bb = next_bb;
        cur_bb_stmt_processed = 0;
        break;
      }
      case OPR_LABEL: {
        // Add a label to cur_bb or next_bb;
        LABEL_IDX lbl = Tree()->Node(stmt)->Get_label_num();
        Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
                 (TFile, "Found label(%d) in BB(%d) .. ", lbl, cur_bb));
        AssertThat(lbl != 0, ("Label idx cannot be zero"));
        if (cur_bb_stmt_processed == 0) {
          // First stmt, ok
          // Add a bb-label
          // There should be no BB label id set already.
          AssertThat(Cfg()->BB(cur_bb)->Get_label_id() == 0,
            ("When processing the first stmt, there should be no label in the BB(%d), instead label = %d",
              cur_bb, Cfg()->BB(cur_bb)->Get_label_id()));
          Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
                 (TFile, "set lbl(%d) for BB(%d)\n", lbl, cur_bb));
          Cfg()->BB(cur_bb)->Set_label_id(lbl);
          builder.Bind_label_to_bb(lbl, cur_bb);
          cur_bb_stmt_processed ++;
        } else {
          // Ending the current-BB, and creating a new-BB
          // Create new bb including this as a start
          CFG_BB_IDX next_bb = Cfg()->Add_bb(cur_bb); // fall-thru
          Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
                 (TFile, "\nBefore [label], end of BB = %d, start next BB(%d), label for new BB = %d\n",
                  cur_bb, next_bb, lbl));
          AssertThat(Cfg()->BB(next_bb)->Get_label_id() == 0,
            ("Before setting up the new bb, bb should contain no label, yet given %d",
              next_bb, Cfg()->BB(next_bb)->Get_label_id()));
          Cfg()->BB(next_bb)->Set_label_id(lbl);
          builder.Bind_label_to_bb(lbl, next_bb);
          cur_bb = next_bb;
          cur_bb_stmt_processed = 1;
        }
        break;
      }
      case OPR_CALL: {
        // Create new bb including this as a start
        CFG_BB_IDX next_bb = Cfg()->Add_bb(cur_bb); // fall-thru
        Handle_call(stmt, cur_bb, next_bb);
        cur_bb_stmt_processed ++;
        break;
      }
      default: {
        AssertThat(false, ("Opcode: %s should not be in the body", OPCODE_name(Tree()->Get_node(stmt)->Opcode())));
      }
    }
  }
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA), (TFile, "Ending function body.\n"));
  Add_epilog(cur_bb);

  // Fixup the predecessor / successor info for the BB CFG.
  builder.Fixup_pred_succ();
  builder.Unbind_all_labels();
}

/**
 *  A unique function to add prolog and epilog to the CGIR;
 *  @param cur_bb the current BB.
 */
CFG_BB_IDX CGIR_BUILDER::Add_prolog(CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA), (TFile, "Adding function prologue BB.\n"));
  // Create a block
  cur_bb = Cfg()->Add_bb();
  // Adding entry BB.
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_ENTRY); //Assuming there is only one BB.
  cur_bb = Cfg()->Add_bb(cur_bb);
  return cur_bb;
}

/**
 *  A unique function to add prolog and epilog to the CGIR;
 *  @param cur_bb the current BB.
 */
CFG_BB_IDX CGIR_BUILDER::Add_epilog(CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA), (TFile, "Adding function epilog BB.\n"));
  // Adding function epilog (exit BB)
  cur_bb = Cfg()->Add_bb(cur_bb);
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_EXIT);
  Cfg()->BB(cur_bb)->Set_label_id(File()->Get_func_exit_label());
  Cfg()->BB(cur_bb)->Add_stmt(
    new CGOP(CGOPC_BX, 0, cur_bb,
             0,
             TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_ra, REGISTER_ra, 4)),
             0, 0));
  return cur_bb;
}

TN *CGIR::PREG_to_TN(TY_IDX preg_ty, PREG_NUM preg_num) {
  AssertThat(preg_num != 0, ("Preg number should not be zero."));
  if (_preg_to_tn.find(preg_num) != _preg_to_tn.end()) {
    return TN_tn(_preg_to_tn[preg_num]);
  }
  PREG *preg = PREG_preg(preg_num);
  TN_IDX base_idx = 0;
  if (preg->getDesireRegNum() == 1) {
    // This is return val.
    TN *base = Gen_Register_TN(ISA_REGISTER_CLASS_integer, MTYPE_size(MTYPE_I4));
    Set_TN_is_preallocated(base);
    Set_TN_register(base, 0);
    base_idx = TN_tn_idx(base);
  } else {
    base_idx = Gen_TN(MTYPE_I4);
    Set_TN_is_gra_cannot_split(TN_tn(base_idx));
  }
  _preg_to_tn.insert(std::make_pair(preg_num, base_idx));
  return TN_tn(base_idx);
}

TN *CGIR::PREG_to_ST_TN(ST_IDX sym_idx, PREG_NUM preg_num) {
  AssertThat(false, ("PREG_to_ST_TN is not implemented"));
  return nullptr;
}


/**
 * Setting up CGIR preparation,
 * @param cgir CGIR module
 * @param sym Current function's PU symbol
 */
void CGIR::Set_current_cgir(CG_CFG *cgir, ST_IDX sym) {
  _current = cgir;
  _current_sym = sym;
  _preg_to_tn.clear();
  _current_tree = PU_INFO_pu_info(File()->Tables()->Get_pu_info_by_st_idx(sym))->entry;
  Set_current_layout(Layouts()[sym]);
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
CGIR_BUILDER::Expand_expr(IR_ITER entry, IR_ITER parent, CFG_BB_IDX cur_bb, TN *result) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_Expr\n"));
  switch (OPCODE_operator(Tree()->Node(entry)->Opcode())) {
    case OPR_LDID: {
      return Handle_ldid(entry, cur_bb, result);
    }
    case OPR_LDA: {
      return Handle_lda(entry, cur_bb, result);
    }
    case OPR_ILOAD: {
      return Handle_iload(entry, cur_bb, result);
    }
    case OPR_CONST: {
      INT64 val = Tree()->Node(entry)->Get_const_val();
      AssertThat(val < (1ll << 32l), ("val should be with in range"));
      if (result == NULL) {
        result = TN_tn(Gen_TN(MTYPE_I4)); // rh1_res; // A trick to reduce # of register
      }
      Cfg()->BB(cur_bb)->Add_stmt(
          new CGOP(CGOPC_MOV, *entry, cur_bb, TN_tn_idx(result),
                 TN_tn_idx(Gen_Literal_TN((val) & 0xFFFF, 2)), 0, 0));
      if (((val >> 16) & 0xFFFF) != 0) {
        Cfg()->BB(cur_bb)->Add_stmt(
          new CGOP(CGOPC_MOVT, *entry, cur_bb, TN_tn_idx(result),
                   TN_tn_idx(Gen_Literal_TN((val >> 16) & 0xFFFF, 2)), 0, 0));
      }
      return result;
    }
    case OPR_MOD:
    case OPR_BIOR:
    case OPR_LAND:
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MPY:
    case OPR_DIV: {
      CGOP *exp_res = nullptr;
      TN *rh1_res = nullptr;
      rh1_res = Expand_expr(Tree()->Get_operand(entry, 0), entry, cur_bb, rh1_res);
      // TODO: If rh2 is a constant, maybe we could do a BIN_OP r1, r2, #const kind of transform.
      TN *rh2_res = nullptr;
      rh2_res = Expand_expr(Tree()->Get_operand(entry, 1), entry, cur_bb, rh2_res);
      AssertThat(rh1_res != nullptr && rh2_res != nullptr, ("Results cannot be null"));
      if (result == NULL) {
        result = TN_tn(Gen_TN(MTYPE_I4)); // rh1_res; // A trick to reduce # of register
      }
      Exp_op2(*entry, Tree()->Node(entry)->Opcode(), cur_bb, result, rh1_res, rh2_res, &exp_res);
      return result;
    }
    case OPR_LNOT: {
      CGOP *exp_res = nullptr;
      TN *rh1_res = nullptr;
      rh1_res = Expand_expr(Tree()->Get_operand(entry, 0), entry, cur_bb, rh1_res);
      AssertThat(rh1_res != nullptr, ("Result cannot be null"));
      if (result == NULL) {
        result = TN_tn(Gen_TN(MTYPE_I4)); // rh1_res; // A trick to reduce # of register
      }
      Exp_op1(*entry, Tree()->Node(entry)->Opcode(), cur_bb, result, rh1_res, &exp_res);
      return result;
    }
    default:
      AssertThat(false, ("Operator not implemented : %s", OPCODE_name(Tree()->Node(entry)->Opcode())));
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

/**
 * Analyze function-level live range for CGIR;
 * @param info current function
 */
void CG_LIVE_RANGE::Analyze_live_range(PU_INFO *info) {

}

/**
 * Global register allocation
 * @param info Current function
 */
void CG_REG_ALLOC::Register_allocate(PU_INFO *info) {
  Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_INVOCATION),
           (TFile, "CGIR::Local_register_allocate \n"));
  CGIR *cgir = this->Cgir();
  // Count registers needed.
  UINT32 i32_register_needed = 0;
  UINT32 bb_cnt = cgir->Cfg()->Size();
  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cfg()->BB(i);
    // Tracings
    if (TR_LRA()) {
      fprintf(TFile, " --------- Processing BB : %d ---------  \n", i);
    }
    _tn_freq_map.clear();
    // If there is a label to it, emit the label
    UINT32 stmt_id = 0;
    for (auto stmt_it = cgbb->First_stmt(); stmt_it != cgbb->Last_stmt(); stmt_it++, stmt_id++) {
      CGOP *cgop = (*stmt_it);
      Is_Trace(TR_LRA(),
               (TFile, "LRA: Processing op = %s\n",
                Get_cg_opc_info(cgop->getOpcode())->ins_token));
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

    /*** LRA ***/
    Is_Trace(TR_LRA(),
             (TFile, "Found %lu registers to allocate for \n", _tn_freq_map.size()));

    // Build interference graph.
    // ...

    // This is actually global register allocation.
    REGISTER_SET used = 0;
    REGISTER_SET_EmptyP(used);
    UINT32 next_register = 1; // Use r0 for return value
    for (auto tn_freq : _tn_freq_map) {
      TN_IDX tid = tn_freq.first;
      TN *tn = TN_tn(tid);
      // Allocate one-by-one
      vector<UINT32> &ded = Cfg()->BB(i)->Get_dedicate_regs();
      if (TN_is_gra_cannot_split(tn)){
        // PREG, must spill here.
        Spill_tn(tid, tn);
      } else {
        if (next_register >= 7) {
          /* Spill all now. */
          Spill_tn(tid, tn);
        } else {
          Is_Trace(TR_LRA(), (TFile, "LRA: Assigning reg %d to TN : %d\n", next_register, tid));
          Cfg()->BB(i)->Get_dedicate_regs().push_back(next_register);
          Set_TN_is_preallocated(tn);
          Set_TN_register_class(tn, ISA_REGISTER_CLASS_integer);
          Set_TN_register(tn, next_register);
          while (std::find(ded.begin(), ded.end(), next_register) != ded.end() &&
                 next_register < 8) {
            next_register++;
          }
        }
      }
    }
  }
  // Re-add spilling etc.,
  // Allocate all registers, mark spilling info.
  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cfg()->BB(i);
    // Tracings
    if (TR_LRA()) {
      fprintf(TFile, " --------- Post-LRA-Spill BB : %d ---------  \n", i);
    }
    // If there is a label to it, emit the label
    UINT32 stmt_cnt = cgbb->Get_stmt_count();
    UINT32 stmt_id = 0;
    for (auto stmt_it = cgbb->First_stmt(); stmt_id  < stmt_cnt; stmt_id++) {
      CGOP *cgop = (*(cgbb->First_stmt() + stmt_id));
      Is_Trace(TR_LRA(),
               (TFile, "Processing : %s\n", Get_cg_opc_info(cgop->getOpcode())->ins_token));
      if (cgop->getFlags() & CGOPF_SPILL) {
        continue;
      }
      if (Get_cg_opc_info(cgop->getOpcode())->getNRes() >= 1) {
        Process_spill_op(cgop, CGOPR_R, i, 0, Get_cg_opc_info(cgop->getOpcode())->isWriteToRd());
      }
      if (Get_cg_opc_info(cgop->getOpcode())->getNOprs() >= 1) {
        Process_spill_op(cgop, CGOPR_R, i, 1, false);
      }
      if (Get_cg_opc_info(cgop->getOpcode())->getNOprs() >= 2) {
        Process_spill_op(cgop, CGOPR_R, i, 2, false);
      }
    }
  }

  // for (UINT32 i = 0; i < bb_cnt; i++) {
  //   CGBB *cgbb = Cfg()->BB(i);
  //   auto work_list = cgbb->Get_work_list();
  //   for (auto work_item : work_list) {
  //     if (work_item.getPutBefore()) {
  //       cgbb->Move_stmt_to_before(work_item.getTarget(), work_item.getFrom());
  //     } else {
  //       cgbb->Move_stmt_to_after(work_item.getTarget(), work_item.getFrom());
  //     }
  //   }
  // }
}

void CG_REG_ALLOC::Spill_tn(TN_IDX tid, TN *tn) {// We could put the spill on r8.
// Now we have at least the R9 to process
  Is_Trace(TR_LRA(),
           (TFile, "Need to spill the TN %d \n", tid));
  // Make TN = ... to Var(.spill) = ...
  Set_TN_register(tn, 0); // not allocating right now, wait for second pass.
  Set_TN_is_preallocated(tn);
  // Allocate spill space.
// Create var, create object space.
  ST_IDX sym = 0; //File()->Find_symbol_by_name(".spill");
  char   name_str[100];
  sprintf(name_str, ".spill_%d", tid);
  STR_IDX name = File()->Save_string(name_str);
  sym = File()->Create_var(name, MTYPE_to_ty(MTYPE_I4), LOCAL_SYMTAB,
                           SYMC_AUTO, SYME_INTERNAL, SYM_CLASS_VAR);
  Cgir()->Layout()->Allocate_object(sym); // re-allocate this.
  Set_TN_flags(tn, TN_SPILL);
  Set_TN_spill(tn, sym);
  Set_TN_register_class(tn, ISA_REGISTER_CLASS_integer);
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
  fprintf(file, "%sPrinting the CGIR's function with sym : name = %s, sym = 0x%08x\n%s",
          DBAR, ST_name(sym), sym, DBAR);
  Get_function(sym)->Print(file);

  if (Tracing(COMPONENT_CG_CONV, TRACE_DEBUG)) {
    fprintf(file, "%sPrinting the TN info\n%s", DBAR, DBAR);
    // Printing TNs
    for (INT32 i = TN_tab_size() - 1; i >= 0; i--) {
      TN *tn_obj = TN_tn(i);
      fprintf(file, "[TN %-8d, 0x%04x]  ", i, i);
      tn_obj->Print(file);
    }
  } else if (Tracing(COMPONENT_CG_CONV, TRACE_DATA)) {
    fprintf(file, "%sPrinting the TN info\n%s", DBAR, DBAR);
    // Printing TNs
    for (INT32 i = TN_tab_size() - 1; i >= _func_tn_begin; i--) {
      TN *tn_obj = TN_tn(i);
      fprintf(file, "[TN %-8d, 0x%04x]  ", i, i);
      tn_obj->Print(file);
    }
  }
}


OPCODE OPCODE_make_op(OPERATOR opr, MTYPE_ID res, MTYPE_ID desc) {
  return (OPCODE) (opr + RTYPE(res) + DESC(desc));
}

/**
 * Generating the LOAD/STORE instructions for LDID/STID/ILOAD/ISTORE...
 * @param opc
 * @param mtype
 * @param src_res_tn
 * @param base_tn
 * @param sym
 * @param ofst_val
 * @param bb_idx
 * @param variant
 */
void CGIR_BUILDER::Exp_load_store (
  OPCODE opc,
  MTYPE_ID mtype,
  TN *src_res_tn,
  TN *base, // only for ILOAD/ISTORE
  ST_IDX sym,
  INT64 ofst_val,
  IR_ITER node,
  CFG_BB_IDX bb_idx,
  VARIANT variant)
{
  UINT32 node_id = 0;
  if (node != nullptr) {
    node_id = *node;
  }
  TN *src_res  = src_res_tn;
  BOOL need_recalibrate = false;
  INT64 offset_from_base = ofst_val;
  CGOPC top = CGOPC_STR;
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
           (TFile, "Exp_LDST sym = %u, ofst = %lld\n", sym, ofst_val));
  if (base != nullptr) {
    // ISTORE or ILOAD case, where base and offset are known.
    // nothing to do.
  } else if (sym != 0 && ST_symclass(sym) == SYM_CLASS_PREG) {
    // Either return-val or just a intermediate number.
    PREG_IDX pgid = ofst_val;
    AssertThat(pgid != 0, ("Preg number should not be zero."));
    PREG *preg = PREG_preg(pgid);
    if (preg->getDesireRegNum() == 1) {
      // This is return val.
      base = Gen_Register_TN(ISA_REGISTER_CLASS_integer, MTYPE_size(MTYPE_I4));
      Set_TN_is_preallocated(base);
      Set_TN_register(base, 0);
      top = CGOPC_ADD;
    } else {
      // Just another TN.
      base = Cgir()->PREG_to_TN(ST_ty(sym), ofst_val);
      top = CGOPC_ADD;
    }
    if (opc == OPC_I4STID) {
      // Reverse the base and res_src.
      TN *mid = base;
      base = src_res;
      src_res = mid;
    }

    offset_from_base = 0;
  } else if (sym != 0 && ST_sclass(sym) != SYMC_AUTO && ST_sclass(sym) != SYMC_FORMAL) {
    // Create a LDR first
    base = TN_tn(Gen_TN(MTYPE_I4));
    LABEL_IDX lbl = Get_addr_label(sym);
    CGOP      *node = new CGOP(CGOPC_LDRLBL, node_id, bb_idx, TN_tn_idx(base),
                               TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0);
    Cfg()->BB(bb_idx)->Add_stmt(node);
    _spill_related.push_back(node);
  } else {
    base = Build_Dedicated_TN(REGISTER_CLASS_sp,
                              REGISTER_sp,
                              MTYPE_size(MTYPE_I4));
    offset_from_base = Cgir()->Layout()->Get_sym_sp_ofst(sym);
    need_recalibrate = true;
  }
  AssertThat(base != nullptr, ("Base cannot be null here."));
  TN *ofst = Gen_Literal_TN(offset_from_base, 4);
  if (need_recalibrate) {
    Cfg()->Get_recalibrate_map().insert(std::make_pair(TN_tn_idx(ofst), sym));
  }
  if (top == CGOPC_ADD) {
    // do nothing, continue on.
  } else if (OPCODE_operator(opc) == OPR_STID) {
    top = CGOPC_STR;
  } else if (OPCODE_operator(opc) == OPR_LDID) {
    top = CGOPC_LDR;
  } else if (OPCODE_operator(opc) == OPR_ISTORE) {
    top = CGOPC_STR;
  } else if (OPCODE_operator(opc) == OPR_ILOAD) {
    top = CGOPC_LDR;
  } else {
    AssertThat(false, ("not impl ldst opcode = %s.", OPCODE_name(opc)));
  }
  AssertThat(TN_is_constant(ofst), ("Exp_LDST: Illegal offset TN"));
  if (!TN_has_value(ofst) || TN_value(ofst) < (1 << 8)) {
    AssertThat(src_res != nullptr && base != nullptr && ofst != nullptr, ("No operands should be null."));
    Cfg()->BB(bb_idx)->Add_stmt(
      new CGOP(top, node_id, bb_idx, TN_tn_idx(src_res), TN_tn_idx(base),
               TN_tn_idx(ofst), 0));
  } else {
    // Somehow calculate a middle number,
    TN *middle_base = TN_tn(Gen_TN(MTYPE_I4));
    CGOP *mov_1     = new CGOP(CGOPC_MOV, node_id, bb_idx,
                               TN_tn_idx(middle_base),
                               TN_tn_idx(
                                 Gen_Literal_TN((offset_from_base) & 0xFFFF,
                                                2)), 0, 0);
    Cfg()->BB(bb_idx)->Add_stmt(mov_1);
    _spill_related.push_back(mov_1);
    if (((offset_from_base >> 16) & 0xFFFF) != 0) {
      CGOP *mov2 = new CGOP(CGOPC_MOVT, node_id, bb_idx, TN_tn_idx(middle_base),
                            TN_tn_idx(
                              Gen_Literal_TN((offset_from_base >> 16) & 0xFFFF,
                                             2)), 0, 0);
      Cfg()->BB(bb_idx)->Add_stmt(mov2);
      _spill_related.push_back(mov2);
    }
    CGOP *add_op = new CGOP(CGOPC_ADD, node_id, bb_idx, TN_tn_idx(middle_base),
             TN_tn_idx(base), TN_tn_idx(middle_base), 0);
    _spill_related.push_back(add_op);
    Cfg()->BB(bb_idx)->Add_stmt(add_op);
    TN_value(ofst) = 0;
    Cfg()->Get_exceed_map().insert(std::make_pair(TN_tn_idx(ofst), offset_from_base));
    CGOP *real_op = new CGOP(top, node_id, bb_idx, TN_tn_idx(src_res),
                             TN_tn_idx(middle_base),
                             TN_tn_idx(ofst), 0);
    Cfg()->BB(bb_idx)->Add_stmt(real_op);
    _spill_related.push_back(real_op);
  }
}

void CGIR_BUILDER::Exp_op(UINT32 expr_id,
                  OPCODE opcode, CFG_BB_IDX cur_bb,
                  TN *result, TN *op1, TN *op2, TN *op3,
                  VARIANT variant, CGOP **ops) {
  AssertThat(ops != NULL, ("Nowhere to put results"));
  CGOPC cgop = CGOPC_NOP;
  switch (OPCODE_operator(opcode)) {
    case OPR_ADD: { cgop = CGOPC_ADD; break; }
    case OPR_MPY: { cgop = CGOPC_MUL; break; }
    case OPR_DIV: { cgop = CGOPC_DIV; break; }
    case OPR_SUB: { cgop = CGOPC_SUBS; break; }
    case OPR_LAND: { cgop = CGOPC_AND; break; }
    case OPR_BIOR: { cgop = CGOPC_ORR; break; }
    case OPR_LNOT: { cgop = CGOPC_MVN; break; }
    default: {
      AssertThat(false, ("Exp_op some opcode = %s not impl.", OPCODE_name(opcode)));
    }
  }
  AssertThat(result != NULL, ("Result should not be null."));
  AssertThat(op1 != NULL, ("OP1 should not be null."));
  if (op2 != nullptr) {
    AssertThat(op2 != NULL, ("OP2 should not be null."));
    CGOP *stmt_ins = new CGOP(cgop, expr_id, cur_bb, TN_tn_idx(result), TN_tn_idx(op1),
                              TN_tn_idx(op2), 0);
    Cfg()->BB(cur_bb)->Add_stmt(stmt_ins);
  } else {
    CGOP *stmt_ins = new CGOP(cgop, expr_id, cur_bb, TN_tn_idx(result), TN_tn_idx(op1), 0, 0);
    Cfg()->BB(cur_bb)->Add_stmt(stmt_ins);
  }
}

VARIANT CGIR_BUILDER::Memop_Variant(IR_ITER iterator) {
  return V_BR_NONE;
}

void CG_EMITTER::Emit_label(PU_INFO *func, FILE *out, LABEL_IDX label_idx) {
  fprintf(out, "%s:\n", LABEL_name(label_idx));
}

void CG_EMITTER::Emit_operand(CGOP *oper, CGOPR_KIND kind, UINT32 ch_id, FILE* out) {
  AssertThat(ch_id < 4, ("operand count must be less than 4."));
  CG_OPRAND cgoper = oper->getResOpnd()[ch_id];
  AssertThat(cgoper != 0,
             ("Should not be empty, cgopc = %s", Get_cg_opc_info(
               oper->getOpcode())->getName()));
  if (CGOPR_R == kind) {
    TN *tn = TN_tn(cgoper);
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
      if (TN_register_class(tn) == REGISTER_CLASS_ra &&
          reg_id == REGISTER_ra) {
        fprintf(out, "lr ");
      } else if (TN_register_class(tn) == REGISTER_CLASS_sp &&
                 reg_id == REGISTER_sp) {
        fprintf(out, "sp");
      } else if (TN_register_class(tn) == REGISTER_CLASS_fp &&
                 reg_id == REGISTER_fp) {
        fprintf(out, "fp");
      } else if (TN_register_class(tn) == REGISTER_CLASS_v0 &&
                 reg_id == REGISTER_v0) {
        fprintf(out, "r0");
      } else {
        AssertThat(false, ("not implemented"));
      }
    } else {
      UINT32 reg_id = TN_register(tn);
      fprintf(out, "r%d ", reg_id);
    }
  } else if (CGOPR_IMM == kind) {
    UINT32 val = TN_value(TN_tn(cgoper));
    fprintf(out, "#%d ", val);
  } else {
    AssertThat(false, ("not implemented."));
  }
}

void CGOP::Print(FILE *file) {
  fprintf(file, "[CGOP] node = %d, opc = %s(%d), index:%d, res/opnd: [%u] [%u] [%u] [%u] \n",
          _tree_node_id, ISA_OPCODE_name(getOpcode()), getOpcode(), getIndexInBb(),
          (UINT32) res_opnd[0], (UINT32) res_opnd[1],
          (UINT32) res_opnd[2], (UINT32) res_opnd[3]);
}

CGOPC_INFO CGOPC_INFO_LIST[] = {
#define CGOPDEF(enum_name, nres, nopr, is_w, opr1, opr2, opr3, ins_name, kind)   \
  { #enum_name, enum_name, nres, nopr, is_w, opr1, opr2, opr3, ins_name, kind },
#include "cg_opc.h"
#undef CGOPDEF
};

CGOPC_INFO *Get_cg_opc_info(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return &(CGOPC_INFO_LIST[i]);
    }
  }
  AssertThat(false, ("Cannot find opcode in table = %d", cgopc));
  return &(CGOPC_INFO_LIST[0]);
}

LABEL_IDX Get_addr_label(ST_IDX sym) {
  char *targ = (char*) malloc(sizeof(".taddr_") + strlen(ST_name(sym)) + 100);
  sprintf(targ, ".taddr_%d_%s", File()->Scopes()->Current()->getSt(), ST_name(sym));
  for (UINT32 i       = 1; i < File()->Tables()->Label()->Length(File()->Scopes()->Current()); i++) {
    LABEL_IDX lbl = (LABEL_IDX) (i << 8) + LOCAL_SYMTAB;
    if (LABEL_label(lbl)->Get_temp_sym() == sym) {
      free(targ);
      return lbl;
    }
  }
  STR_IDX     name    = File()->Save_string(targ);
  LABEL_IDX   lbl_idx = File()->Create_label(name, LABEL_ADDR_PASSED &
                                             LABEL_ADDR_SAVED, LKIND_RELOC);
  LABEL_label(lbl_idx)->Set_temp_sym(sym);
  free(targ);
  return lbl_idx;
}

BOOL CGOPC_is_ldst(CGOPC cgopc) {
  return (Get_cg_opc_info(cgopc)->opk == CGOPK_LDST);
}

UINT32 CG_REG_ALLOC::Count_needed_register(CGOP *oper, UINT32 cgop_id,
                                   CGOPR_KIND kind, UINT32 cur_bb, UINT8 opr_pos) {
  if (kind != CGOPR_R) {
    Is_Trace(TR_LRA(), (TFile, "Found kind != r, no need to allocate \n"));
    return 0;
  }
  CG_OPRAND cgoper;
  cgoper = oper->getResOpnd()[opr_pos];
  AssertThat(cgoper != 0, ("TN does not exist on CGOP_id %d, cur_bb = %d, opr_pos = %d",
             cgop_id, cur_bb, opr_pos));
  AssertThat(cgoper < Cgir()->Get_tn_table().size(),
             ("TN %d exceed the table length %d, on CGOP_id %d, cur_bb = %d, opr_pos = %d",
              cgoper, Cgir()->Get_tn_table().size(), cgop_id, cur_bb, opr_pos));
  TN *tn = TN_tn(cgoper);
  if (TN_is_symbol(tn) || TN_is_label(tn) || TN_is_preallocated(tn) ||
      TN_is_constant(tn)  || TN_is_dedicated(tn)) {
    if (TN_is_dedicated(tn) || TN_is_preallocated(tn)) {
      UINT32 reg_id = 1024;
      if (TN_register_class(tn) == REGISTER_CLASS_v0 &&
          TN_register(tn) == REGISTER_v0) {
        reg_id = 0;
      } else if (TN_is_preallocated(tn) && TN_register(tn) >= 0 && TN_register(tn) < 1024) {
        reg_id = TN_register(tn);
      }
      if (reg_id != 1024) {
        vector<UINT32> &ded = Cfg()->BB(cur_bb)->Get_dedicate_regs();
        if (std::find(ded.begin(), ded.end(), reg_id) == ded.end()) {
          ded.push_back(reg_id);
        }
      }
    }
    Is_Trace(TR_LRA(), (TFile, "Found tn %d no need to allocate \n", cgoper));
    return 0;
  } else {
    // There is a need for R-A.
    Is_Trace(TR_LRA(), (TFile, "Found tn %d to allocate \n", cgoper));
    AssertThat(TN_register(tn) == 0,
               ("TN: %d, Should not be allocated already. %d",
                cgoper, TN_register(tn)));
    TN_IDX tid = TN_tn_idx(tn);
    UINT32 old_freq = 0;
    if (_tn_freq_map.find(tid) != _tn_freq_map.end()) {
      old_freq = _tn_freq_map[tid];
    } else {
      _tn_freq_map.insert(std::make_pair(tid, 0));
      _tn_live_range.insert(std::make_pair(tid, vector<UINT64>()));
    }
    _tn_freq_map[tid] = old_freq + 1;
    UINT64 bb_stmt = (((UINT64) cur_bb) << 32l) & cgop_id;
    _tn_live_range[tid].push_back(bb_stmt);
    return 1;
  }
  return 0;
}

void CG_REG_ALLOC::Process_spill_op(CGOP *oper, CGOPR_KIND kind, UINT32 cur_bb,
                            UINT32 opnd, BOOL is_write) {
  Builder()->Enable_spill_recording();
  // Process a possible write that may create spilling.
  AssertThat(oper->getResOpnd()[opnd] != 0, ("incorrect tn found"));
  TN_IDX tid = oper->getResOpnd()[opnd];
  TN *tn = TN_tn(tid);
  if (TN_flags(tn) & TN_SPILL) {
    // There is a spill.
    TN *spill_tn = Gen_Register_TN(ISA_REGISTER_CLASS_integer, REG_SIZE_I);
    Set_TN_is_preallocated(spill_tn);
    Set_TN_flags(spill_tn, TN_SPILL_MEDIUM);
    UINT32 reg_num_to_use = opnd == 0 ? 8 : ((opnd == 1) ?  10 : 7);
    if (is_write) { // this is dependent on the oper currently we're visiting.
      Is_Trace(TR_LRA(), (TFile, "Create store temp tn %d to r%d\n", TN_tn_idx(spill_tn), REGISTER_spill));
      // haven't allocated
      Is_Trace(TR_LRA(), (TFile, "Spill tn %d to r%d\n", TN_tn_idx(tn), REGISTER_spill));
      Set_TN_register(spill_tn, reg_num_to_use);
      Builder()->Exp_load_store(OPC_I4STID, MTYPE_I4,
               spill_tn,
               nullptr,
               TN_spill(tn), 0,
               nullptr,
               cur_bb, V_BR_NONE);
      oper->setResOpnd(opnd, TN_tn_idx(spill_tn));
      vector <CGOP*> &sp_related = Builder()->Get_spill_related();
      for (INT32 i = sp_related.size() - 1; i >= 0; i--) {
        CGOP *rs = sp_related[i];
        if (rs->getOpcode() == CGOPC_ADD) {
          // Probably a too long stack, in this sense, we need to allocate middle base.
          TN_IDX middle_base = rs->getResOpnd()[0];
          TN *middle = TN_tn(middle_base);
          AssertThat(!TN_is_preallocated(middle) && !TN_is_dedicated(middle),
                     ("Tn must be clean. %d", middle_base));
          Set_TN_is_preallocated(middle);
          Set_TN_register(middle, 10);
        }
        rs->setFlags(CGOPF_SPILL);
        Cfg()->BB(cur_bb)->Get_work_list().push_back(
          CG_REVISIT_ITEM<CGOP>(oper, rs, false));
      }
    } else {
      Is_Trace(TR_LRA(), (TFile, "Spill tn %d to r%d\n", TN_tn_idx(tn), reg_num_to_use));
      Is_Trace(TR_LRA(), (TFile, "Create store temp tn %d to r%d\n", TN_tn_idx(spill_tn), reg_num_to_use));
      Set_TN_register(spill_tn, reg_num_to_use); // Making sure the two register are the same.
      Builder()->Exp_load_store(OPC_I4LDID, MTYPE_I4,
        spill_tn,
        nullptr,
        TN_spill(tn), 0, 0, cur_bb, V_BR_NONE);
      oper->setResOpnd(opnd, TN_tn_idx(spill_tn));
      vector<CGOP*> &sp_related = Builder()->Get_spill_related();
      for (UINT32 i = 0; i < sp_related.size(); i++) {
        CGOP *rs = sp_related[i];
        if (rs->getOpcode() == CGOPC_ADD) {
          // Probably a too long stack, in this sense, we need to allocate middle base.
          TN_IDX middle_base = rs->getResOpnd()[0];
          TN *middle = TN_tn(middle_base);
          AssertThat(!TN_is_preallocated(middle) && !TN_is_dedicated(middle),
                     ("Tn must be clean. %d", middle_base));
          Set_TN_is_preallocated(middle);
          Set_TN_register(middle, 0);
        }
        rs->setFlags(CGOPF_SPILL);
        Cfg()->BB(cur_bb)->Get_work_list().push_back(CG_REVISIT_ITEM<CGOP> (oper, rs, true));
      }
    }
  }
  Builder()->Disable_spill_recording();
}

/**
 * Handle goto, falsebr, branch ....
 * @param stmt STMT to process
 * @param cur_bb
 */
void CGIR_BUILDER::Handle_goto(IR_ITER stmt, CFG_BB_IDX cur_bb, CFG_BB_IDX next_bb, CG_CONV_INFO &conv_info) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR_BUILDER::Handle_Goto\n"));
  CGOPC out_code = CGOPC_B;
  switch(OPCODE_operator(Tree()->Node(stmt)->Opcode())) {
    case OPR_GOTO: {
      // Add to todo list.
      out_code = CGOPC_B;
      break;
    }
    case OPR_FALSEBR:
    case OPR_TRUEBR: {
      IR_ITER cond = Tree()->Get_operand(stmt, 0);
      out_code = Get_branch_cond(cond, OPR_TRUEBR == OPCODE_operator(Tree()->Node(stmt)->Opcode()));
      // Create cmp / tst instructions.
      TN *rh1_res = TN_tn(Gen_TN(MTYPE_I4));
      TN *rh2_res = nullptr;
      Expand_expr(Tree()->Get_operand(cond, 0), stmt, cur_bb, rh1_res);
      if (Tree()->Node(Tree()->Get_operand(cond, 1))->Opcode() == OPC_I4CONST &&
          Tree()->Node(Tree()->Get_operand(cond, 1))->Get_const_val() < 255) {
        rh2_res = Gen_Literal_TN(Tree()->Node(Tree()->Get_operand(cond, 1))->Get_const_val(), REG_SIZE_I);
      } else {
        rh2_res = TN_tn(Gen_TN(MTYPE_I4));
        Expand_expr(Tree()->Get_operand(cond, 1), stmt, cur_bb, rh2_res);
      }
      CGOP *cmpins = new CGOP(CGOPC_CMP, cur_bb, 0, TN_tn_idx(rh1_res), TN_tn_idx(rh2_res), 0);
      Cfg()->BB(cur_bb)->Add_stmt(cmpins);
      break;
    }
    default: {
      AssertThat(false, ("Cannot convert %s", OPCODE_name(Tree()->Node(stmt)->Opcode())));
    }
  }
  LABEL_IDX lbl = Tree()->Node(stmt)->Get_label_num();
  AssertThat(lbl != 0, ("Invalid label num = %d", lbl));
  TN *label_tn = Gen_Label_TN(lbl, 0);
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO),
    (TFile, "Create label tn for goto target : TN(%d), label-id = %d\n", TN_tn_idx(label_tn), lbl));
  CGOP *jmp = new CGOP(out_code, cur_bb, 0, TN_tn_idx(label_tn), 0, 0);
  Cfg()->BB(cur_bb)->Add_stmt(jmp);

  // Adding this jump info to correctly mark up BB's relations graph.
  conv_info.Mark_goto_in_bb(cur_bb, lbl);
}

void CGIR_BUILDER::Handle_call(IR_ITER stmt, CFG_BB_IDX cur_bb, CFG_BB_IDX next_bb) {
  ST_IDX func_sym      = Tree()->Node(stmt)->Get_symbol_idx();
  LABEL_IDX lbl_idx    = File()->Create_label(ST_st(func_sym)->getNameIdx(), 0,
                                              LKIND_DEFAULT);
  TN *callee_name_tn   = Gen_Label_TN(lbl_idx, 0);
  UINT32 call_args        = Tree()->Number_of_children(stmt);
  TY_IDX callee_proto     = ST_ty(func_sym);
  UINT32 callargs_size    = 0; // including the first four param.
  UINT32 abi_callarg_size = 0; // not including the first four param.
  UINT32 arg_id           = 0;
  const UINT32 CALL_PUSH_SIZE = 12;
  for (TYLIST_IDX ty_list = TY_tylist_id(callee_proto) + 1; ;
       arg_id ++,
       ty_list++) {
    if (TYLIST_tylist(ty_list)->Ty_idx() == 0) {
      // end met.
      break;
    }
    TY_IDX ty_idx = TYLIST_tylist(ty_list)->Ty_idx();
    if (TY_kind(ty_idx) == KIND_VOID) {
      // do nothing.
      call_args = 0;
      arg_id = 0;
      break;
    } else if (TY_kind(ty_idx) == KIND_ARRAY) {
      callargs_size += 4;
      if (arg_id >= 4) abi_callarg_size += 4;
    } else if (TY_kind(ty_idx) == KIND_SCALAR) {
      callargs_size += TY_size(ty_idx);
      if (arg_id >= 4) abi_callarg_size += TY_size(ty_idx);
    } else {
      AssertThat(false,
                 ("Unexpected type kind : %d, in type: %d",
                   TY_kind(ty_idx), ty_idx));
    }
  }

  AssertThat(arg_id == call_args,
             ("Insufficient arguments %d needed , %d present",
               arg_id, call_args));

  // Function call-site
  // Make sure that all values store to formal.
  // Currently use the same bb for storing formals on mem/reg
  UINT32 to_mem_bb = cur_bb;
  UINT32 to_reg_bb = cur_bb;

  // Create a new BB for storing to memory.
  for (UINT32 i = 0; i < call_args; i++) {
    IR_ITER expr = Tree()->Get_operand(stmt, i);
    TN *result = TN_tn(Gen_TN(MTYPE_I4));
    Expand_expr(expr, stmt, to_mem_bb, result);
    INT32 sp_ofst = -callargs_size - CALL_PUSH_SIZE + (i * 4);
    // SP related store
    Exp_load_store(OPC_I4STID, MTYPE_I4,
             result,
             Build_Dedicated_TN(REGISTER_CLASS_sp, REGISTER_sp, 4), 0,
             sp_ofst,
             expr,
             to_mem_bb, V_BR_NONE);
  }

  // Create a new BB for loading to r0 to r3.
  for (UINT32 i = 0; i < 4 && i < call_args; i++) {
    IR_ITER expr = Tree()->Get_operand(stmt, i);
    Cfg()->BB(to_reg_bb)->Dedicate_reg(i);
    INT32 sp_ofst = -callargs_size - CALL_PUSH_SIZE + (i * 4);
    // SP related store
    TN *dedic = TN_tn(Gen_TN(MTYPE_I4));
    Set_TN_is_preallocated(dedic);
    Set_TN_register(dedic, i);
    Exp_load_store(OPC_I4LDID, MTYPE_I4,
             dedic,
             Build_Dedicated_TN(REGISTER_CLASS_sp, REGISTER_sp, 4), 0, sp_ofst,
             expr,
             to_reg_bb, V_BR_NONE);
  }

  if (call_args > 0) {
    // Adjust SP, sub the arguments.
    CGOP *push_sp   = new CGOP(CGOPC_PUSHR, cur_bb, 0, 0, 0, 0);
    Cfg()->BB(to_reg_bb)->Add_stmt(push_sp);
    CGOP *adjust_sp = new CGOP(CGOPC_SUBS, cur_bb,
                               TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                            REGISTER_sp, 4)),
                               TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                            REGISTER_sp, 4)),
                               TN_tn_idx(Gen_Literal_TN(abi_callarg_size, 4)),
                               0);
    Cfg()->BB(to_reg_bb)->Add_stmt(adjust_sp);
  }

  // Call
  CGOP *jmp = new CGOP(CGOPC_BL, cur_bb, 0, TN_tn_idx(callee_name_tn), 0, 0);
  Cfg()->BB(to_reg_bb)->Add_stmt(jmp);

  // After calling, handling results.
  // Re-adjust sp
  CFG_BB_IDX after_call_bb = cur_bb;
  if (call_args > 0) {
    CGOP *readjust_sp = new CGOP(CGOPC_ADD, cur_bb,
                                 TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                              REGISTER_sp, 4)),
                                 TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                              REGISTER_sp, 4)),
                                 TN_tn_idx(Gen_Literal_TN(abi_callarg_size, 4)),
                                 0);
    Cfg()->BB(after_call_bb)->Add_stmt(readjust_sp);
    CGOP *pop_sp   = new CGOP(CGOPC_POPR, cur_bb, 0, 0, 0, 0);
    Cfg()->BB(after_call_bb)->Add_stmt(pop_sp);
  }
}

CGOPC CGIR_BUILDER::Get_branch_cond(IR_ITER cond, BOOL is_true_br) {
  OPERATOR curopr = OPCODE_operator(Tree()->Node(cond)->Opcode());
  Is_Trace(Tracing(COMPONENT_CG, TRACE_DEBUG), (TFile,
    "CGIR::Get_branch_cond, operator = OPR(%d) %s, is_true_br = %d\n", curopr, OPERATOR_name(curopr), is_true_br));
  if (!is_true_br) {
    switch (curopr) {
      case OPR_LT:
        curopr = OPR_GE;
        break;
      case OPR_LE:
        curopr = OPR_GT;
        break;
      case OPR_GE:
        curopr = OPR_LT;
        break;
      case OPR_GT:
        curopr = OPR_LE;
        break;
      case OPR_EQ:
        curopr = OPR_NE;
        break;
      case OPR_NE:
        curopr = OPR_EQ;
        break;
      default:
        AssertThat(false, ("unknown reverse condition met: %s", OPCODE_name(Tree()->Node(cond)->Opcode())));
    }
  }
  // Not true branch, aka false branch
  CGOPC end = CGOPC_B;
  switch (curopr) {
    case OPR_LT:
      end = CGOPC_BLT;
      break;
    case OPR_LE:
      end = CGOPC_BLE;
      break;
    case OPR_GE:
      end = CGOPC_BGE;
      break;
    case OPR_GT:
      end = CGOPC_BGT;
      break;
    case OPR_EQ:
      end = CGOPC_BEQ;
      break;
    case OPR_NE:
      end = CGOPC_BNE;
      break;
    default:
      AssertThat(false, ("unknown condition met: %s", OPCODE_name(Tree()->Node(cond)->Opcode())));
  }
  return end;
}

UINT32 CGIR::TN_tab_size() {
  return _global_tn_vec.size();
}

void CGIR_BUILDER::Add_store_formals(IR_ITER entry, CFG_BB_IDX bb) {
  ST_IDX cur_func = File()->Scopes()->Current()->st_idx;
  TY_IDX ty = ST_ty(cur_func);
  vector<ST_IDX> &sym_on_reg = Cgir()->Layout()->Get_sym_on_formal_reg();
  AssertThat(sym_on_reg.size() <= 4,
             ("There is at most 4 var on formal reg. yet = %d", sym_on_reg.size()));
  for(UINT32 i = 0; i < sym_on_reg.size(); i++) {
    ST_IDX sym   = sym_on_reg[i];
    TN *from_reg = Gen_Register_TN(ISA_REGISTER_CLASS_integer, MTYPE_size(MTYPE_I4));
    Set_TN_is_preallocated(from_reg);
    Set_TN_register(from_reg, Cgir()->Layout()->Get_sym_reg_num(sym));
    Exp_load_store(OPC_I4STID, MTYPE_I4, from_reg, nullptr, sym, 0,
             entry,
             bb, V_BR_NONE);
  }
}

void CGIR::Recalibrate_offset(PU_INFO *pInfo) {
  for (auto item : Cfg()->Get_recalibrate_map()) {
    TN *ofst_tn = TN_tn(item.first);
    INT64 new_value = Layout()->Get_sym_sp_ofst(item.second);
    if (Cfg()->Get_exceed_map().find(item.first) !=
        Cfg()->Get_exceed_map().end()) {
      // Oversided TN, use offset instead.
      INT64 orig_val = Cfg()->Get_exceed_map()[item.first];
      Set_TN_value(ofst_tn, new_value - orig_val);
    } else {
      AssertThat(TN_is_constant(ofst_tn), ("Not a constant TN to calibrate"));
      Set_TN_value(ofst_tn, new_value);
    }
  }
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
class CFG_BASE<CGOP, CG_CFG_BB_BASE<CGOP>>;     // CG_CFG
