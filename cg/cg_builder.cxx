//
// Created by jason.lu on 2026/6/16
// This is the builder for CG CFG from IR, also calculates def/use, pred/succ.,
//

#include "basic.h"
#include "cfg_common.h"
#include "cg_basic.h"
#include "cgir.h"
#include "cg_pass.h"
#include "stdio.h"
#include "tn.h"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <vector>

BOOL TR_BUILD() {
  return Tracing(COMPONENT_CG_CONV, TRACE_OPTIONS);
}

VARIANT CGIR_BUILDER::Memop_Variant(IR_ITER iterator) {
  return V_BR_NONE;
}

void CGIR_BUILDER::Data_layout(SCOPE *scope) {
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INVOCATION),
           (TFile, "Perform data layout in CGIR::Data_layout\n"));
  ST_IDX func_sym = scope->getSt();
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));

  // Get a reference to the map (crucial to avoid copying!)
  auto &layouts_map = Cgir()->Layouts(); 
  
  // Try to find if it already exists
  auto it = layouts_map.find(func_sym);
  DATA_LAYOUT *onelayout = nullptr;

  if (it != layouts_map.end()) {
    // 1. Key exists! Reuse the previously allocated one
    onelayout = it->second;
    
    // Safety check: if the existing pointer is somehow null, fix it
    if (onelayout == nullptr) {
        onelayout = new DATA_LAYOUT(Target().abi);
        it->second = onelayout;
    }
  } else {
    // 2. Key does not exist! Allocate a new one and insert it
    onelayout = new DATA_LAYOUT(Target().abi);
    layouts_map.insert(std::make_pair(func_sym, onelayout));
  }
  
  // 3. Proceed using the correct pointer safely
  onelayout->Frame_final_size();
  onelayout->Initialize_frame(scope, func_sym);
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
  Set_TN_size(target_res, Target().abi.pointer_size);
  AssertThat(target_res != NULL, ("Expand of expr should not return null."));
  if (ST_sclass(sym) == SYMC_FILE_STATIC) {
    // LOCAL VAR.
    // SP + OFST
    LABEL_IDX lbl = Get_addr_label(sym);
    Cfg()->BB(cur_bb)->Add_stmt(
      new CGOP(CGOPC_LADDR, *expr, cur_bb, TN_tn_idx(target_res), TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0));
  } else if (ST_sclass(sym) == SYMC_AUTO ||
    ST_sclass(sym) == SYMC_FORMAL) {
    TN *sp_tn = Build_Dedicated_TN(REGISTER_CLASS_sp,
                              REGISTER_sp,
                              Target().abi.pointer_size);
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
  CG_CONV_INFO convinfo;
  convinfo.Init(Cfg());

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
        Handle_ret(stmt, cur_bb, convinfo);
        cur_bb_stmt_processed ++;
        break;
      }
      case OPR_RETURN_VAL: {
        Handle_ret_val(stmt, cur_bb, convinfo);
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
        Handle_goto(stmt, cur_bb, next_bb, convinfo);
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
          convinfo.Bind_label_to_bb(lbl, cur_bb);
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
          convinfo.Bind_label_to_bb(lbl, next_bb);
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
  convinfo.Fixup_pred_succ();
  convinfo.Unbind_all_labels();

  // Build auxiliary info
  Build_pred_succ();
  Build_def_use();
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
    new CGOP(CGOPC_RET, 0, cur_bb, 0, 0, 0, 0));
  return cur_bb;
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
          new CGOP(CGOPC_LIMM, *entry, cur_bb, TN_tn_idx(result),
                   TN_tn_idx(Gen_Literal_TN(val, 4)), 0, 0));
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
  const UINT32 register_arg_count = Target().abi.register_formal_count;
  const UINT32 pointer_size = Target().abi.pointer_size;
  const UINT32 CALL_PUSH_SIZE =
      Target().abi.kind == TARGET_ABI_KIND::AAPCS32_HARD_FLOAT ? 12 : 0;
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
      callargs_size += pointer_size;
      if (arg_id >= register_arg_count) abi_callarg_size += pointer_size;
    } else if (TY_kind(ty_idx) == KIND_SCALAR) {
      callargs_size += std::max<UINT32>(TY_size(ty_idx), pointer_size);
      if (arg_id >= register_arg_count) {
        abi_callarg_size += std::max<UINT32>(TY_size(ty_idx), pointer_size);
      }
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

  const BOOL armv7_call_sequence =
      Target().abi.kind == TARGET_ABI_KIND::AAPCS32_HARD_FLOAT;
  const UINT32 stack_arg_count =
      call_args > register_arg_count ? call_args - register_arg_count : 0;
  const UINT32 stack_arg_area = stack_arg_count * pointer_size;
  const UINT32 staged_arg_size = armv7_call_sequence || call_args == 0
      ? 0
      : ((call_args * pointer_size + Target().abi.stack_alignment - 1) /
         Target().abi.stack_alignment) * Target().abi.stack_alignment;
  if (staged_arg_size != 0) {
    Cfg()->BB(to_mem_bb)->Add_stmt(new CGOP(
        CGOPC_ADJSP, cur_bb, 0,
        TN_tn_idx(Gen_Literal_TN(-static_cast<INT64>(staged_arg_size),
                                 pointer_size)),
        0, 0));
  }

  auto argument_stack_offset = [&](UINT32 argument_index) -> INT32 {
    if (armv7_call_sequence) {
      return -static_cast<INT32>(callargs_size) - CALL_PUSH_SIZE +
             argument_index * pointer_size;
    }
    if (argument_index < register_arg_count) {
      return stack_arg_area + argument_index * pointer_size;
    }
    return (argument_index - register_arg_count) * pointer_size;
  };

  // Create a new BB for storing to memory.
  for (UINT32 i = 0; i < call_args; i++) {
    IR_ITER expr = Tree()->Get_operand(stmt, i);
    TN *result = TN_tn(Gen_TN(MTYPE_I4));
    Expand_expr(expr, stmt, to_mem_bb, result);
    INT32 sp_ofst = argument_stack_offset(i);
    // SP related store
    Exp_load_store(OPC_I4STID, MTYPE_I4,
             result,
             Build_Dedicated_TN(REGISTER_CLASS_sp, REGISTER_sp, pointer_size), 0,
             sp_ofst,
             expr,
             to_mem_bb, V_BR_NONE);
  }

  // Load register arguments according to the selected ABI.
  for (UINT32 i = 0; i < register_arg_count && i < call_args; i++) {
    IR_ITER expr = Tree()->Get_operand(stmt, i);
    Cfg()->BB(to_reg_bb)->Dedicate_reg(i);
    INT32 sp_ofst = argument_stack_offset(i);
    // SP related store
    TN *dedic = TN_tn(Gen_TN(MTYPE_I4));
    Set_TN_is_preallocated(dedic);
    Set_TN_register(dedic, i);
    Exp_load_store(OPC_I4LDID, MTYPE_I4,
             dedic,
             Build_Dedicated_TN(REGISTER_CLASS_sp, REGISTER_sp, pointer_size), 0, sp_ofst,
             expr,
             to_reg_bb, V_BR_NONE);
  }

  if (call_args > 0 && armv7_call_sequence) {
    // Adjust SP, sub the arguments.
    CGOP *push_sp   = new CGOP(CGOPC_PUSHR, cur_bb, 0, 0, 0, 0);
    Cfg()->BB(to_reg_bb)->Add_stmt(push_sp);
    CGOP *adjust_sp = new CGOP(CGOPC_SUBS, cur_bb,
                               TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                            REGISTER_sp, pointer_size)),
                               TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                            REGISTER_sp, pointer_size)),
                               TN_tn_idx(Gen_Literal_TN(abi_callarg_size, pointer_size)),
                               0);
    Cfg()->BB(to_reg_bb)->Add_stmt(adjust_sp);
  }

  // Call
  CGOP *jmp = new CGOP(CGOPC_BL, cur_bb, 0, TN_tn_idx(callee_name_tn), 0, 0);
  Cfg()->BB(to_reg_bb)->Add_stmt(jmp);

  // After calling, handling results.
  // Re-adjust sp
  CFG_BB_IDX after_call_bb = cur_bb;
  if (call_args > 0 && armv7_call_sequence) {
    CGOP *readjust_sp = new CGOP(CGOPC_ADD, cur_bb,
                                 TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                              REGISTER_sp, pointer_size)),
                                 TN_tn_idx(Build_Dedicated_TN(REGISTER_CLASS_sp,
                                                              REGISTER_sp, pointer_size)),
                                 TN_tn_idx(Gen_Literal_TN(abi_callarg_size, pointer_size)),
                                 0);
    Cfg()->BB(after_call_bb)->Add_stmt(readjust_sp);
    CGOP *pop_sp   = new CGOP(CGOPC_POPR, cur_bb, 0, 0, 0, 0);
    Cfg()->BB(after_call_bb)->Add_stmt(pop_sp);
  } else if (staged_arg_size != 0) {
    Cfg()->BB(after_call_bb)->Add_stmt(new CGOP(
        CGOPC_ADJSP, cur_bb, 0,
        TN_tn_idx(Gen_Literal_TN(staged_arg_size, pointer_size)), 0, 0));
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


void CGIR_BUILDER::Add_store_formals(IR_ITER entry, CFG_BB_IDX bb) {
  ST_IDX cur_func = File()->Scopes()->Current()->st_idx;
  TY_IDX ty = ST_ty(cur_func);
  vector<ST_IDX> &sym_on_reg = Cgir()->Layout()->Get_sym_on_formal_reg();
  AssertThat(sym_on_reg.size() <= Target().abi.register_formal_count,
             ("formal register count %d exceeds target ABI limit %d",
              sym_on_reg.size(), Target().abi.register_formal_count));
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

/**
 * Building the predecessor list for each CG_CFG_BB
 */
void CGIR_BUILDER::Build_pred_succ() {
  // LRA Algorithm
  CGIR   *cgir      = Cgir();
  CG_CFG *cfg       = cgir->Cfg();
  UINT32  bb_cnt    = cgir->Cfg()->Size();
  INT32   cur_bb    = 0; // currently visited bb;
  CFG_BB_EDGES_STORE &all_preds = cfg->Get_preds();
  all_preds.clear();
  all_preds.assign(bb_cnt, {});
  CFG_BB_EDGES_STORE &all_succs = cfg->Internal_edges();
  all_succs.clear();
  all_succs.assign(bb_cnt, {});
  AssertThat(all_succs.size() > cur_bb, ("Invalid succs(edges) initialization, size = %u, cur_bb = %u",
             all_succs.size(), cur_bb));
  AssertThat(all_preds.size() > cur_bb, ("Invalid preds initialization, size = %u, cur_bb = %u",
             all_preds.size(), cur_bb));
    
  for (cur_bb = bb_cnt - 1; cur_bb >= 0; cur_bb--) {
    // def of bb
    const CGBB *cgbb = cfg->BB(cur_bb);
    CFG_BB_EDGES &preds = cfg->Preds(cur_bb);
    CFG_BB_EDGES &succs = cfg->Edges(cur_bb);
    for (auto prd_id = cgbb->Pred_begin(); prd_id != cgbb->Pred_end(); prd_id++) {
      preds.insert((*prd_id)->Get_id());
    }
    for (auto prd_id = cgbb->Succ_begin(); prd_id != cgbb->Succ_end(); prd_id++) {
      succs.insert((*prd_id)->Get_id());
    }
  }
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

      TN_SET defs = cgir->Stmt_defs(cgop);
      TN_SET uses = cgir->Stmt_uses(cgop);

      cgbb->Defs().insert(defs.begin(), defs.end());
      cgbb->Stmt_defs(stmt_id).insert(defs.begin(), defs.end());
      
      cgbb->Uses().insert(uses.begin(), uses.end());
      cgbb->Stmt_uses(stmt_id).insert(uses.begin(), uses.end());

      for (TN_IDX tnidx: defs) {
	tn_freq_map[tnidx]++;
      }
      for (TN_IDX tnidx: uses) {
	tn_freq_map[tnidx]++;
      }
    }
  }
}
