//
// Created by xc5 on 2020/7/13.
// This is the IR definition for CG to use, i.e. CG-IR
//
#include "cgir.h"

INLINE BOOL TR_LRA() {
  return Tracing(COMPONENT_CG_LRA, TRACE_DATA);
}

void CGIR::CG_Expand(SCOPE *scope) {
  // Do data layout
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::CG_Expand\n"));
  ST_IDX func_sym = scope->getSt();
  File()->Scopes()->Goto_function(func_sym);
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
  Get_function(func_sym);
  Data_layout(scope);
  AssertThat(layout.find(func_sym) != layout.end(), ("Layout should have this function now. 0x%08x", func_sym));
  Layout()->Calculate_stack_frame_size(); // Calc frame size
  // Do IR to CGIR conversion
  this->IR_to_CGIR(scope->getSt());
}

void CGIR::Data_layout(SCOPE *scope) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "Perform data layout in CGIR::Data_layout\n"));
  ST_IDX func_sym = scope->getSt();
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
  DATA_LAYOUT *onelayout = new DATA_LAYOUT();
  layout.insert(std::make_pair(func_sym, onelayout));
  AssertThat(layout.find(func_sym) != layout.end(), ("Layout should have this function now. 0x%08x", func_sym));
  layout[func_sym]->Initialize_frame(scope, func_sym);
  Set_current_layout(onelayout);
}

void CGIR::IR_to_CGIR(ST_IDX sym) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "Perform IR to CGIR conversion in CGIR::IR_to_CGIR\n"));
  CG_CFG    *function_cgir = Get_function(sym);
  Set_current_cgir(function_cgir, sym);
  Handle_Entry(tree->Get_root(), 0);
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
    Exp_LDST(opcode,
             OPCODE_desc(opcode),
             tn_res,
             nullptr,
             tree->Get_node(stmt)->Get_symbol_idx(),
             tree->Get_node(stmt)->Get_load_offset(),
             cur_bb,
             variant);
  }
  return;
}


TN *
CGIR::Handle_LDID(IR_ITER stmt, CFG_BB_IDX cur_bb, TN *target_res) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_LDID\n"));
  AssertThat(OPCODE_operator(tree->Node(stmt)->Opcode()) == OPR_LDID,
             ("Not a ldid to be passed to Handle_ldid"));
  AssertThat(tree->Number_of_children(stmt) == 0,
             ("Incorrect number of kid in LDID, 0 expected, got %d", tree->Number_of_children(
               stmt)));

  CG_OPRAND res       = 0;
  OPCODE    opcode    = tree->Get_node(stmt)->Opcode();
  if (target_res != nullptr) {
    res = TN_tn_idx(target_res);
  } else {
    res = Gen_TN(MTYPE_I4);
  }
  if (false /* PREG */) {
    TN *tn_res = PREG_to_ST_TN(tree->Get_node(stmt)->Get_symbol_idx(),
                               tree->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
    // TODO: Conversion may still be needed here.
    return tn_res;
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = TN_tn(res);
    Exp_LDST(opcode,
             OPCODE_desc(opcode),
             tn_res,
             nullptr,
             tree->Get_node(stmt)->Get_symbol_idx(),
             tree->Get_node(stmt)->Get_load_offset(),
             cur_bb,
             variant);
    return tn_res;
  }
}


TN *
CGIR::Handle_ILOAD(IR_ITER stmt, CFG_BB_IDX cur_bb, TN *target_res) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_ILOAD\n"));
  AssertThat(OPCODE_operator(tree->Node(stmt)->Opcode()) == OPR_ILOAD,
             ("Not a ILOAD to be passed to Handle_ILOAD"));
  AssertThat(tree->Number_of_children(stmt) == 1,
             ("Incorrect number of kid in ILOAD, 1 expected, got %d", tree->Number_of_children(
               stmt)));

  TN *base_tn = Expand_Expr (tree->Get_operand(stmt, 0), stmt, cur_bb, NULL);
  AssertThat(base_tn != nullptr, ("base tn should not be null in ILOAD."));

  CG_OPRAND res       = 0;
  OPCODE    opcode    = tree->Get_node(stmt)->Opcode();
  if (target_res != nullptr) {
    res = TN_tn_idx(target_res);
  } else {
    res = Gen_TN(MTYPE_I4);
  }
  if (false /* PREG */) {
    TN *tn_res = PREG_to_ST_TN(tree->Get_node(stmt)->Get_symbol_idx(),
                               tree->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
    // TODO: Conversion may still be needed here.
    return tn_res;
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = TN_tn(res);
    Exp_LDST(opcode,
             OPCODE_desc(opcode),
             tn_res,
             base_tn,
             tree->Get_node(stmt)->Get_symbol_idx(),
             tree->Get_node(stmt)->Get_load_offset(),
             cur_bb,
             variant);
    return tn_res;
  }
}


TN *
CGIR::Handle_ISTORE(IR_ITER stmt, CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_ISTORE\n"))
  AssertThat(OPCODE_operator(tree->Node(stmt)->Opcode()) == OPR_ISTORE,
             ("Not a ILOAD to be passed to Handle_ISTORE"));
  AssertThat(tree->Number_of_children(stmt) == 2,
             ("Incorrect number of kid in ILOAD, 1 expected, got %d", tree->Number_of_children(
               stmt)));

  TN *base_tn = Expand_Expr (tree->Get_operand(stmt, 1), stmt, cur_bb, NULL);
  AssertThat(base_tn != nullptr, ("base tn should not be null in ISTORE."));

  OPCODE    opcode    = tree->Get_node(stmt)->Opcode();
  CG_OPRAND res = 0;
  if (false /* PREG */) {
    TN *tn_res = PREG_to_ST_TN(tree->Get_node(stmt)->Get_symbol_idx(),
                               tree->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
    // TODO: Conversion may still be needed here.
    return tn_res;
  } else {
    VARIANT variant = Memop_Variant(stmt);
    TN *tn_res = Expand_Expr (tree->Get_operand(stmt, 0), stmt, cur_bb, NULL);
    AssertThat(tn_res != NULL, ("Expand of expr should not return null."));
    res = TN_tn_idx(tn_res);
    Exp_LDST(opcode,
             OPCODE_desc(opcode),
             tn_res,
             base_tn,
             tree->Get_node(stmt)->Get_symbol_idx(),
             tree->Get_node(stmt)->Get_load_offset(),
             cur_bb,
             variant);
    return tn_res;
  }
}


TN *
CGIR::Handle_LDA(IR_ITER expr, CFG_BB_IDX cur_bb, TN *target_res) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_LDA\n"));
  AssertThat(OPCODE_operator(tree->Node(expr)->Opcode()) == OPR_LDA,
             ("Not a LDA to be passed to Handle_LDA"));
  AssertThat(tree->Number_of_children(expr) == 0,
             ("Incorrect number of kid in LDA, 0 expected, got %d", tree->Number_of_children(
               expr)));
  AssertThat(tree->Node(expr)->Get_symbol_idx() != 0,
             ("There should be a valid symbol bound to it."));
  ST_IDX sym = tree->Node(expr)->Get_symbol_idx();
  if (ST_sclass(sym) == SYMC_FILE_STATIC) {
    // LOCAL VAR.
    // SP + OFST
    LABEL_IDX lbl = Get_addr_label(sym);
    Cfg()->BB(cur_bb)->Add_stmt(
      new CGOP(CGOPC_LDRLBL, cur_bb, TN_tn_idx(target_res), TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0));
  } else if (ST_sclass(sym) == SYMC_AUTO ||
    ST_sclass(sym) == SYMC_FORMAL) {
    TN *sp_tn = Build_Dedicated_TN(REGISTER_CLASS_sp,
                              REGISTER_sp,
                              MTYPE_size(MTYPE_I4));
    INT64 offset_from_base = Layout()->Get_sym_sp_ofst(sym);
    Cfg()->BB(cur_bb)->Add_stmt(
      new CGOP(CGOPC_ADD, cur_bb,
               TN_tn_idx(target_res),
               TN_tn_idx(sp_tn),
               TN_tn_idx(Gen_Literal_TN(offset_from_base, 4)), 0));
  }
  return target_res;
}

void
CGIR::Handle_ret_val(IR_ITER stmt, CFG_BB_IDX cur_bb) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "CGIR::Handle_ret_val\n"));
  AssertThat(OPCODE_operator(tree->Node(stmt)->Opcode()) == OPR_RETURN_VAL,
             ("Not correct opr to be passed to Handle_ret_val"));
  AssertThat(tree->Number_of_children(stmt) == 1,
             ("Incorrect number of kid in %s, 1 expected, got %d",
               OPCODE_name(tree->Node(stmt)->Opcode()), tree->Number_of_children(
               stmt)));

  MTYPE_ID  ldid_type = OPCODE_desc(tree->Get_node(stmt)->Opcode());
  CG_OPRAND res       = 0;
  OPCODE    opcode    = tree->Get_node(stmt)->Opcode();

  if (false /* PREG */) {
    TN *tn_res = PREG_to_ST_TN(tree->Get_node(stmt)->Get_symbol_idx(),
                               tree->Get_node(stmt)->Get_preg_num());
    res = TN_tn_idx(tn_res);
  } else {
    TN      *tn_res   = Expand_Expr (tree->Get_operand(stmt, 0), stmt, cur_bb, NULL);
    TN      *func_val = Build_Dedicated_TN(REGISTER_CLASS_v0, REGISTER_v0, 4);
    CGOP    *cgop     = new CGOP(CGOPC_ADD, cur_bb,
                                 TN_tn_idx(func_val),
                                 TN_tn_idx(tn_res),
                                 TN_tn_idx(Gen_Literal_TN(0, 4)),
                                 0);
    Cfg()->BB(cur_bb)->Add_stmt(cgop);
  }
  Handle_ret(cur_bb);
}

void CGIR::Handle_ret(CFG_BB_IDX cur_bb) {
  LABEL_IDX lbl   = File()->Get_func_exit_label();
  CGOP      *cgop = new CGOP(CGOPC_B, cur_bb,
                               0, TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0);
  Cfg()->BB(cur_bb)->Add_stmt(cgop);
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
  UINT32 cur_bb_stmt_processed = 0;
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
      case OPR_ISTORE: {
        Handle_ISTORE(stmt, cur_bb);
        break;
      }
      case OPR_RETURN: {
        CFG_BB_IDX next_bb = Cfg()->Add_bb();
        Handle_ret(cur_bb);
        cur_bb = next_bb;
        cur_bb_stmt_processed = 0;
        break;
      }
      case OPR_RETURN_VAL: {
        CFG_BB_IDX next_bb = Cfg()->Add_bb();
        Handle_ret_val(stmt, cur_bb);
        cur_bb = next_bb;
        cur_bb_stmt_processed = 0;
        break;
      }
      case OPR_FALSEBR:
      case OPR_TRUEBR:
      case OPR_GOTO: {
        CFG_BB_IDX next_bb = 0;
        next_bb = Handle_goto(stmt, cur_bb);
        Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
                 (TFile, "Finishing a BB = %d, starting next bb = %d\n",
                  cur_bb, next_bb));
        cur_bb = next_bb;
        cur_bb_stmt_processed = 0;
        break;
      }
      case OPR_LABEL: {
        // Add a label to cur_bb or next_bb;
        LABEL_IDX lbl = tree->Node(stmt)->Get_label_num();
        AssertThat(lbl != 0, ("Label idx cannot be zero"));
        if (cur_bb_stmt_processed == 0) {
          // First stmt, ok
          // Add a bb-label
          Cfg()->BB(cur_bb)->Set_label_id(lbl);
        } else {
          // Create new bb including this as a start
          CFG_BB_IDX next_bb = Cfg()->Add_bb(cur_bb); // fall-thru
          Cfg()->BB(next_bb)->Set_label_id(lbl);
          cur_bb = next_bb;
          cur_bb_stmt_processed = 0;
        }
        break;
      }
      case OPR_CALL: {
        // Create new bb including this as a start
        CFG_BB_IDX next_bb = Cfg()->Add_bb(cur_bb); // fall-thru
        Handle_call(stmt, cur_bb, next_bb);
        cur_bb = next_bb;
        cur_bb_stmt_processed = 0;
        break;
      }
      default: {
        AssertThat(false, ("Opcode: %s should not be in the body", OPCODE_name(tree->Get_node(stmt)->Opcode())));
      }
    }
    cur_bb_stmt_processed ++;
  }
  // Adding function epilog (exit BB)
  cur_bb = Cfg()->Add_bb(cur_bb);
  Cfg()->BB(cur_bb)->Set_flag(BB_FLAG_EXIT);
  Cfg()->BB(cur_bb)->Set_label_id(File()->Get_func_exit_label());
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
  switch (OPCODE_operator(tree->Node(entry)->Opcode())) {
    case OPR_LDID: {
      return Handle_LDID(entry, cur_bb, result);
    }
    case OPR_LDA: {
      return Handle_LDA(entry, cur_bb, result);
    }
    case OPR_ILOAD: {
      return Handle_ILOAD(entry, cur_bb, result);
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
    case OPR_BIOR:
    case OPR_LAND:
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MPY:
    case OPR_DIV: {
      CGOP *exp_res = nullptr;
      TN *rh1_res = TN_tn(Gen_TN(MTYPE_I4));
      Expand_Expr(tree->Get_operand(entry, 0), entry, cur_bb, rh1_res);
      // TODO: If rh2 is a constant, maybe we could do a BIN_OP r1, r2, #const kind of transform.
      TN *rh2_res = TN_tn(Gen_TN(MTYPE_I4));
      Expand_Expr(tree->Get_operand(entry, 1), entry, cur_bb, rh2_res);
      if (result == NULL) {
        result = TN_tn(Gen_TN(MTYPE_I4)); // rh1_res; // A trick to reduce # of register
      }
      Exp_op2(tree->Node(entry)->Opcode(), cur_bb, result, rh1_res, rh2_res, &exp_res);
      return result;
    }
    case OPR_LNOT: {
      CGOP *exp_res = nullptr;
      TN *rh1_res = TN_tn(Gen_TN(MTYPE_I4));
      Expand_Expr(tree->Get_operand(entry, 0), entry, cur_bb, rh1_res);
      if (result == NULL) {
        result = TN_tn(Gen_TN(MTYPE_I4)); // rh1_res; // A trick to reduce # of register
      }
      Exp_op1(tree->Node(entry)->Opcode(), cur_bb, result, rh1_res, &exp_res);
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
           (TFile, "CGIR::Local_register_allocate \n"));
  CGIR *cgir = this;
  // Count registers needed.
  UINT32 i32_register_needed = 0;
  UINT32 bb_cnt = cgir->Cfg()->Size();
  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cfg()->BB(i);
    // Tracings
    if (TR_LRA()) {
      fprintf(TFile, " --------- Processing BB : %d ---------  \n", i);
      _tn_freq_map.clear();
    }
    // If there is a label to it, emit the label
    for (auto stmt_it = cgbb->First_stmt(); stmt_it != cgbb->Last_stmt(); stmt_it++) {
      CGOP *cgop = (*stmt_it);
      Is_Trace(TR_LRA(),
               (TFile, "LRA: Processing op = %s\n",
                Get_cg_opc_info(cgop->getOpcode())->ins_token));
      if (Get_cg_opc_info(cgop->getOpcode())->n_res >= 1) {
        i32_register_needed += Count_needed_register(cgop, CGOPR_R, i, 0);
      }
      if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 1) {
        i32_register_needed += Count_needed_register(cgop, CGOPR_R, i, 1);
      }
      if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 2) {
        i32_register_needed += Count_needed_register(cgop, CGOPR_R, i, 2);
      }
    }

    /*** LRA ***/

    Is_Trace(TR_LRA(),
             (TFile, "Found %lu registers to allocate for \n", _tn_freq_map.size()));
    // This is actually global register allocation.
    REGISTER_SET used = 0;
    REGISTER_SET_EmptyP(used);
    UINT32 used_cnt = 1; // Use r0 for return value
    for (auto tn_freq : _tn_freq_map) {
      TN_IDX tid = tn_freq.first;
      TN *tn = TN_tn(tid);
      // Allocate one-by-one
      Is_Trace(TR_LRA(), (TFile, "LRA: Assigning reg %d to TN : %d\n", used_cnt, tid));
      Set_TN_register(tn, used_cnt);
      Set_TN_is_preallocated(tn);
      Set_TN_register_class(tn, ISA_REGISTER_CLASS_integer);
      if (used_cnt < 7) {
        used_cnt ++;
      } else {
        // We could put the spill on r8.
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
        Layout()->Allocate_object(sym); // re-allocate this.
        Set_TN_flags(tn, TN_SPILL);
        Set_TN_spill(tn, sym);
        Set_TN_register_class(tn, ISA_REGISTER_CLASS_integer);
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

  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cfg()->BB(i);
    auto work_list = cgbb->Get_work_list();
    for (auto work_item : work_list) {
      if (work_item.getPutBefore()) {
        cgbb->Move_stmt_to_before(work_item.getTarget(), work_item.getFrom());
      } else {
        cgbb->Move_stmt_to_after(work_item.getTarget(), work_item.getFrom());
      }
    }
  }
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
void
CGIR::Exp_LDST (
  OPCODE opc,
  MTYPE_ID mtype,
  TN *src_res_tn,
  TN *base, // only for ILOAD/ISTORE
  ST_IDX sym,
  INT64 ofst_val,
  CFG_BB_IDX bb_idx,
  VARIANT variant)
{
  TN *src_res  = src_res_tn;
  INT64 offset_from_base = ofst_val;
  AssertThat(ofst_val == 0, ("There should be no existing ofst, yet = %d", ofst_val));
  if (base != nullptr) {
    // ISTORE or ILOAD case, where base and offset are known.
    // nothing to do.
  } else if (ST_symclass(sym) == SYM_CLASS_PREG) {
    base = Gen_Register_TN(REGISTER_CLASS_sp, MTYPE_size(MTYPE_I4));
    PREG_IDX pgid = ST_st(sym)->offset;
    Set_TN_is_preallocated(base);
    Set_TN_register(base, PREG_preg(pgid)->desire_reg_num);
  } else if (ST_sclass(sym) != SYMC_AUTO) {
    // Create a LDR first
    base = TN_tn(Gen_TN(MTYPE_I4));
    LABEL_IDX lbl = Get_addr_label(sym);
    Cfg()->BB(bb_idx)->Add_stmt(
      new CGOP(CGOPC_LDRLBL, bb_idx, TN_tn_idx(base), TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0));
  } else {
    base = Build_Dedicated_TN(REGISTER_CLASS_sp,
                              REGISTER_sp,
                              MTYPE_size(MTYPE_I4));
    offset_from_base = Layout()->Get_sym_sp_ofst(sym);
  }
  AssertThat(base != nullptr, ("Base cannot be null here."));
  TN *ofst = Gen_Literal_TN(offset_from_base, 4);
  CGOPC top = CGOPC_STR;
  if (OPCODE_operator(opc) == OPR_STID) {
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
  if (!TN_has_value(ofst) || TN_value(ofst) < (1 << 16)) {
    Cfg()->BB(bb_idx)->Add_stmt(
      new CGOP(top, bb_idx, TN_tn_idx(src_res), TN_tn_idx(base),
               TN_tn_idx(ofst), 0));
  } else {
    AssertThat(false, ("Offset too large, need other ways to do this. "
                       "\nNot implmented Exp_LDST situation"));
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

void CGIR::Exp_op(OPCODE opcode, CFG_BB_IDX cur_bb,
                  TN *result, TN *op1, TN *op2, TN *op3,
                  VARIANT variant, CGOP **ops) {
  AssertThat(ops != NULL, ("Nowhere to put results"));
  CGOPC cgop = CGOPC_NOP;
  switch (OPCODE_operator(opcode)) {
    case OPR_ADD: { cgop = CGOPC_ADD; break; }
    case OPR_MPY: { cgop = CGOPC_MUL; break; }
    case OPR_DIV: { cgop = CGOPC_ADD; break; }
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
    CGOP *stmt_ins = new CGOP(cgop, cur_bb, TN_tn_idx(result), TN_tn_idx(op1),
                              TN_tn_idx(op2), 0);
    Cfg()->BB(cur_bb)->Add_stmt(stmt_ins);
  } else {
    CGOP *stmt_ins = new CGOP(cgop, cur_bb, TN_tn_idx(result), TN_tn_idx(op1), 0, 0);
  }
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
      } else if (reg_id == REGISTER_v0) {
        fprintf(out, "r0");
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
CGOP *CFG_BB_BASE<NODE_TYPE>::Last_real_stmt() {
  AssertThat(_stmts.size() > 0, ("No last exist"));
  return _stmts.back();
}

template<typename NODE_TYPE>
UINT32 CFG_BB_BASE<NODE_TYPE>::Get_stmt_count() {
  return _stmts.size();
}

template<typename NODE_TYPE>
void CFG_BB_BASE<NODE_TYPE>::Move_stmt_to_after(NODE_TYPE *position, NODE_TYPE *from) {
  BOOL flag = false;
  for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
    if (*it == position) {
      it++;
      AssertThat(it != _stmts.end(), ("This cannot be, otherwise we are moving [position] after [position]"));
      _stmts.insert(it, from);
      flag = true;
      break;
    }
  }
  AssertThat(flag, ("Cannot find the target stmt %p in stmts.", position));
  flag = false;
  for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
    if (*it == from) {
      if (!flag) {
        flag = true; // skip the first one.
        continue;
      }
      _stmts.erase(it);
      flag = true;
      break;
    }
  }
  AssertThat(flag, ("Cannot find the old value to delete."));
}

template<typename NODE_TYPE>
void CFG_BB_BASE<NODE_TYPE>::Move_stmt_to_before(NODE_TYPE *position, NODE_TYPE *from) {
  BOOL flag = false;
  for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
    if (*it == position) {
      _stmts.insert(it, from);
      flag = true;
      break;
    }
  }
  AssertThat(flag, ("Cannot find the target stmt %p in stmts.", position));
  flag = false;
  for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
    if (*it == from) {
      if (!flag) {
        flag = true; // skip the first one.
        continue;
      }
      _stmts.erase(it);
      flag = true;
      break;
    }
  }
  AssertThat(flag, ("Cannot find the old value to delete."));
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
#define CGOPDEF(enum_name, nres, nopr, is_w, opr1, opr2, opr3, ins_name, kind)   \
  { #enum_name, enum_name, nres, nopr, is_w, opr1, opr2, opr3, ins_name, kind },
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
                                             LABEL_ADDR_SAVED, LKIND_RELOC);
  LABEL_label(lbl_idx)->Set_temp_sym(sym);
  free(targ);
  return lbl_idx;
}

BOOL CGIR::CGOPC_is_ldst(CGOPC cgopc) {
  return (Get_cg_opc_info(cgopc)->opk == CGOPK_LDST);
}

UINT32 CGIR::Count_needed_register(CGOP *oper, CGOPR_KIND kind, UINT32 cur_bb, UINT8 opr_pos) {
  if (kind != CGOPR_R) {
    Is_Trace(TR_LRA(), (TFile, "Found kind != r, no need to allocate \n"));
    return 0;
  }
  CG_OPRAND cgoper = oper->getResOpnd()[opr_pos];
  AssertThat(cgoper.tn != 0, ("TN does not exist"));
  TN *tn = TN_tn(cgoper.tn);
  if (TN_is_symbol(tn) || TN_is_label(tn) ||
      TN_is_constant(tn)  || TN_is_dedicated(tn)) {
    Is_Trace(TR_LRA(), (TFile, "Found tn %d no need to allocate \n", cgoper.tn));
    return 0;
  } else {
    // There is a need for R-A.
    Is_Trace(TR_LRA(), (TFile, "Found tn %d to allocate \n", cgoper.tn));
    AssertThat(TN_register(tn) == 0, ("Should not be allocated already."));
    TN_IDX tid = TN_tn_idx(tn);
    UINT32 old_freq = 0;
    if (_tn_freq_map.find(tid) != _tn_freq_map.end()) {
      old_freq = _tn_freq_map[tid];
    } else {
      _tn_freq_map.insert(std::make_pair(tid, 0));
      _tn_live_range.insert(std::make_pair(tid, vector<UINT32>()));
    }
    _tn_freq_map[tid] = old_freq + 1;
    _tn_live_range[tid].push_back(cur_bb);
    return 1;
  }
  return 0;
}

void CGIR::Process_spill_op(CGOP *oper, CGOPR_KIND kind, UINT32 cur_bb,
                            UINT32 opnd, BOOL is_write) {
  // Process a possible write that may create spilling.
  AssertThat(oper->getResOpnd()[opnd].tn != 0, ("incorrect tn found"));
  TN_IDX tid = oper->getResOpnd()[opnd].tn;
  TN *tn = TN_tn(tid);
  if (TN_flags(tn) & TN_SPILL) {
    // There is a spill.
    TN *spill_tn = Gen_Register_TN(ISA_REGISTER_CLASS_integer, REG_SIZE_I);
    if (is_write) {
      Is_Trace(TR_LRA(), (TFile, "Create store temp tn %d to r%d\n", TN_tn_idx(spill_tn), REGISTER_spill));
      // haven't allocated
      Is_Trace(TR_LRA(), (TFile, "Spill tn %d to r%d\n", TN_tn_idx(tn), REGISTER_spill));
      Set_TN_register(tn,       REGISTER_spill);
      Set_TN_register(spill_tn, REGISTER_spill);
      Set_TN_is_preallocated(spill_tn);
      Exp_LDST(OPC_I4STID, MTYPE_I4,
               spill_tn,
               nullptr,
               TN_spill(tn), 0, cur_bb, V_BR_NONE);
      oper->setResOpnd(opnd, TN_tn_idx(spill_tn));
      CGOP *rs = *(Cfg()->BB(cur_bb)->Last_stmt() - 1);
      rs->setFlags(CGOPF_SPILL);
      AssertThat(rs->getOpcode() == CGOPC_STR, ("Incorrect generated result"));
      Cfg()->BB(cur_bb)->Get_work_list().push_back(CGTODO_ITEM<CGOP> (oper, rs, false));
    } else {
      UINT8 register_num = (opnd <= 1) ? REGISTER_spill : REGISTER_spill_2;
      Is_Trace(TR_LRA(), (TFile, "Spill tn %d to r%d\n", TN_tn_idx(tn), register_num));
      Is_Trace(TR_LRA(), (TFile, "Create store temp tn %d to r%d\n", TN_tn_idx(spill_tn), register_num));
      Set_TN_register(tn,
                      register_num); // Making sure the two register are the same.
      Set_TN_register(spill_tn,
                      register_num); // Making sure the two register are the same.
      Set_TN_is_preallocated(tn);
      Exp_LDST(OPC_I4LDID, MTYPE_I4,
        spill_tn,
        nullptr,
        TN_spill(tn), 0, cur_bb, V_BR_NONE);
      CGOP *rs = Cfg()->BB(cur_bb)->Last_real_stmt();
      rs->setFlags(CGOPF_SPILL);
      oper->setResOpnd(opnd, TN_tn_idx(spill_tn));
      AssertThat(rs->getOpcode() == CGOPC_LDR, ("Incorrect generated result"));
      Cfg()->BB(cur_bb)->Get_work_list().push_back(CGTODO_ITEM<CGOP> (oper, rs, true));
    }
  }
}

CFG_BB_IDX CGIR::Handle_goto(IR_ITER stmt, CFG_BB_IDX cur_bb) {
  CFG_BB_IDX newbb = Cfg()->Add_bb(cur_bb);
  CGOPC out_code = CGOPC_B;
  switch(OPCODE_operator(tree->Node(stmt)->Opcode())) {
    case OPR_GOTO: {
      out_code = CGOPC_B;
      break;
    }
    case OPR_FALSEBR:
    case OPR_TRUEBR: {
      IR_ITER cond = tree->Get_operand(stmt, 0);
      out_code = Get_branch_cond(cond, OPR_TRUEBR == OPCODE_operator(tree->Node(stmt)->Opcode()));
      // Create cmp / tst instructions.
      TN *rh1_res = TN_tn(Gen_TN(MTYPE_I4));
      TN *rh2_res = nullptr;
      Expand_Expr(tree->Get_operand(cond, 0), stmt, cur_bb, rh1_res);
      if (tree->Node(tree->Get_operand(cond, 1))->Opcode() == OPC_I4CONST &&
          tree->Node(tree->Get_operand(cond, 1))->Get_const_val() < 255) {
        rh2_res = Gen_Literal_TN(tree->Node(tree->Get_operand(cond, 1))->Get_const_val(), REG_SIZE_I);
      } else {
        rh2_res = TN_tn(Gen_TN(MTYPE_I4));
        Expand_Expr(tree->Get_operand(cond, 1), stmt, cur_bb, rh2_res);
      }
      CGOP *cmpins = new CGOP(CGOPC_CMP, cur_bb, 0, TN_tn_idx(rh1_res), TN_tn_idx(rh2_res), 0);
      Cfg()->BB(cur_bb)->Add_stmt(cmpins);
      break;
    }
    default: {
      AssertThat(false, ("Cannot convert %s", OPCODE_name(tree->Node(stmt)->Opcode())));
    }
  }
  LABEL_IDX lbl = tree->Node(stmt)->Get_label_num();
  AssertThat(lbl != 0, ("Invalid label num = %d", lbl));
  TN *label_tn = Gen_Label_TN(lbl, 0);
  CGOP *jmp = new CGOP(out_code, cur_bb, 0, TN_tn_idx(label_tn), 0, 0);
  Cfg()->BB(cur_bb)->Add_stmt(jmp);
  return newbb;
}

void CGIR::Handle_call(IR_ITER stmt, CFG_BB_IDX cur_bb, CFG_BB_IDX next_bb) {
  ST_IDX func_sym = tree->Node(stmt)->Get_symbol_idx();
  LABEL_IDX lbl_idx = File()->Create_label(ST_st(func_sym)->getNameIdx(), 0, LKIND_DEFAULT);
  TN *callee_name_tn = Gen_Label_TN(lbl_idx, 0);
  CGOP *jmp = new CGOP(CGOPC_BL, cur_bb, 0, TN_tn_idx(callee_name_tn), 0, 0);
  Cfg()->BB(cur_bb)->Add_stmt(jmp);
}

CGOPC CGIR::Get_branch_cond(IR_ITER cond, BOOL is_true_br) {
  OPERATOR org = OPCODE_operator(tree->Node(cond)->Opcode());
  if (!is_true_br) {
    switch (org) {
      case OPR_LT:
        org = OPR_GE;
        break;
      case OPR_LE:
        org = OPR_GT;
        break;
      case OPR_GE:
        org = OPR_LT;
        break;
      case OPR_GT:
        org = OPR_LE;
        break;
      case OPR_EQ:
        org = OPR_NE;
        break;
      case OPR_NE:
        org = OPR_EQ;
        break;
      default:
        AssertThat(false, ("unknown reverse condition met: %s", OPCODE_name(tree->Node(cond)->Opcode())));
    }
  }
  CGOPC end = CGOPC_B;
  switch (org) {
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
      AssertThat(false, ("unknown condition met: %s", OPCODE_name(tree->Node(cond)->Opcode())));
  }
  return end;
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
