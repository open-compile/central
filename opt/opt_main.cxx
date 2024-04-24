#include "basic.h"
#include "host.h"
#include "options.h"
#include "be_export.h"
#include "file_util.h"
#include <fstream>
#include <ir.h>
#include "symtab.h"
#include "opt_main.h"
#include <vector>
#include <map>
#include "cgir.h"
#include "cg_main.h"

IR_ITER &Opt_lower_if_stmt(IR_ITER &stmt, const PU_INFO *func, TREE *tree,
                           char *name_buf);

IR_ITER &
Opt_lower_while_do(IR_ITER &stmt, const PU_INFO *func, TREE *tree,
                   char *name_buf);

IR_ITER Opt_lower_array_expr(IR_ITER &expr, TREE *tree);

IR_ITER Opt_lower_mod_op(const IR_ITER &expr, TREE *tree);

IR_ITER Opt_lower_div_op(IR_ITER &expr, TREE *tree);

using std::vector;
using std::map;

INT32 BE_MAIN_NAME(INT32 argc, char **argv) {
  AssertThat(argc > 1, ("not enough arguments"));
  return 0;
}

INT32 BE_EXTERNAL_MAIN_NAME(COMPILER_CONFIG &conf) {
  AssertThat(conf.opt_level >= 0, ("not enough compile level"));
  // Verifying the results from front end.

  Opt_verify(File(), LEVEL_VHIGH, conf);
  // Lowering functions in side the current file

  Opt_lower(File(), LEVEL_HIGH, conf);
  Opt_verify(File(), LEVEL_HIGH, conf);

  // If OPT >= 3, enable IPA, LNO ....

  Opt_lower(File(), LEVEL_HIGH, conf);
  Opt_verify(File(), LEVEL_HIGH, conf);
  // Optimizations on High IR

  Opt_lower(File(), LEVEL_MID, conf);
  Opt_verify(File(), LEVEL_MID, conf);
  // Optimizations on Mid IR, SSA, DCE, CSE ...

  Opt_lower(File(), LEVEL_LOW, conf);
  Opt_verify(File(), LEVEL_LOW, conf);
  // Optimizations done in low IR, not much though

  Opt_lower(File(), LEVEL_VLOW, conf);
  Opt_verify(File(), LEVEL_VLOW, conf);

  // To CGIR
  Opt_lower(File(), LEVEL_CGIR, conf);
  Opt_verify(File(), LEVEL_CGIR, conf);

  if(Tracing(COMPONENT_BE, TRACE_EMIT_CORE)) {
    // Dump the tree again after all optimizations
    Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
             (TFile, "Completed all lowering, dumping the IR again. \n"));
    File()->Print(TFile);
  }
  return 0;
}

void Opt_verify(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &conf) {
  // Check over all IR constructs
  // TODO: Check Symtab
  // Check over all pu_info functions
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    // Iterate over each pu_info (functions), dump each of the function
    PU_INFO *pu_info = file->Tables()->Pu_info()->Get(it);
    if (pu_info->proc_sym != 0) {
      // Valid pu_info.
      Opt_verify_function(pu_info, file, level, conf);
    } else {
      AssertThat(false, ("Incomoplete pu_infoo for PU_INFO_IDX = %u, or %0#x", it, it));
    }
  }
}

void Opt_verify_expr(IR_ITER expr, IR_ITER stmt, PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                     COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  // Verifying each statement
  switch (OPCODE_operator(tree->Get_node(expr)->Opcode())) {
    case OPR_ARRAY: {
      AssertThat(level < LEVEL_MID, ("%s not allowed after MIDDLE level", OPCODE_name(tree->Get_node(expr)->Opcode())));
      break;
    }
    case OPR_RETURN:
    case OPR_GOTO:
    case OPR_LDA:
    case OPR_LABEL:
    case OPR_LDID: {
      AssertThat(tree->Number_of_children(expr) == 0, ("%s should have zero operand", OPCODE_name(tree->Get_node(expr)->Opcode())));
      break;
    }
    case OPR_RETURN_VAL:
    case OPR_FALSEBR:
    case OPR_TRUEBR:
    case OPR_STID:
    case OPR_ILOAD: {
      AssertThat(tree->Number_of_children(expr) == 1, ("%s should have exactly 1 operand", OPCODE_name(tree->Get_node(expr)->Opcode())));
      break;
    }
    case OPR_NE:
    case OPR_EQ:
    case OPR_LT:
    case OPR_GT:
    case OPR_LE:
    case OPR_GE:
    case OPR_ISTORE:
    case OPR_ADD:
    case OPR_MPY:
    case OPR_REM: {
      AssertThat(tree->Number_of_children(expr) == 2, ("BINOP %s not 2 operands", OPCODE_name(tree->Get_node(expr)->Opcode())));
      break;
    }
    case OPR_DIV: {
      AssertThat(tree->Number_of_children(expr) == 2, ("not 2 operands"));
      IR_ITER divisor = tree->Get_operand(expr, 0);
      AssertThat(tree->Node(divisor)->Opcode() != OPC_I4CONST ||
                 tree->Node(divisor)->Get_const_val() != 0, ("cannot divide by zero."));
      break;
    }
    default: {
      // Do nothing.
    }
  }
}

void Opt_verify_block(IR_ITER body, PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                      COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  // Verifying each statement
  for (UINT32 stmt_idx = 0; stmt_idx < tree->Number_of_children(body); stmt_idx++) {
    IR_ITER stmt = tree->Get_operand(body, stmt_idx);
    AssertThat(*stmt != 0, ("Incorrect child, node 0 should not be a statement, 0 is only allowed in root position"));
    switch (OPCODE_operator(tree->Get_node(stmt)->Opcode())) {
      // What kind of opcode is allowed here.
      case OPR_STID: {
        AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in STID, 1 expected, got %d", tree->Number_of_children(stmt)));
        IR_ITER expr_val = tree->Get_operand(stmt, 0);
        // MTYPE_ID stid_type = OPCODE_desc(tree->Get_node(stmt)->Opcode());
        // AssertThat(OPCODE_rtype(tree->Get_node(expr_val)->Opcode()) == stid_type, ("The stid's operand should have same type"));
        Opt_verify_expr(expr_val, stmt, func, file, level, conf);
        break;
      }
      case OPR_ISTORE: {
        AssertThat(tree->Number_of_children(stmt) == 2, ("Incorrect number of kid in ISTORE, 1 expected, got %d", tree->Number_of_children(stmt)));
        IR_ITER expr_val = tree->Get_operand(stmt, 0);
        Opt_verify_expr(expr_val, stmt, func, file, level, conf);
        expr_val = tree->Get_operand(stmt, 1);
        Opt_verify_expr(expr_val, stmt, func, file, level, conf);
        break;
      }
      case OPR_IF: {
        AssertThat(level < LEVEL_MID, ("IF should not be present in level %d", level));
        AssertThat(tree->Number_of_children(stmt) == 3, ("Incorrect number of kid in IF, 3 expected, got %d", tree->Number_of_children(stmt)));

        Opt_verify_expr(tree->Get_operand(stmt, 0), stmt, func, file, level, conf);

        // The then block.
        IR_ITER block_inside = tree->Get_operand(stmt, 1);
        AssertThat(tree->Node(block_inside)->Opcode() == OPC_BLOCK, ("Should be a then block."));
        Opt_verify_block(block_inside, func, file, level, conf);

        // The else block
        block_inside = tree->Get_operand(stmt, 2);
        AssertThat(tree->Node(block_inside)->Opcode() == OPC_BLOCK, ("Should be an else block."));
        Opt_verify_block(block_inside, func, file, level, conf);
        break;
      }
      case OPR_FALSEBR:
      case OPR_TRUEBR: {
        AssertThat(level >= LEVEL_HIGH, ("TRUEBR/FALSEBR should not be present in level %d", level));
        AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in truebr/falsebr, 2 expected, got %d", tree->Number_of_children(stmt)));
        AssertThat(tree->Node(stmt)->Get_label_num() != 0, ("There should be a valid label for truebr/falsebr."));
        Opt_verify_expr(tree->Get_operand(stmt, 0), stmt, func, file, level, conf);
        break;
      }
      case OPR_WHILE_DO: {
        AssertThat(level <= LEVEL_MID, ("WHILE_DO should not be present in level %d", level));
        AssertThat(tree->Number_of_children(stmt) == 2, ("Incorrect number of kid in WHILE_DO, 2 expected, got %d", tree->Number_of_children(stmt)));
        Opt_verify_expr(tree->Get_operand(stmt, 0), stmt, func, file, level, conf);
        // Previous one should be label, next should be label
        IR_ITER block_inside = tree->Get_operand(stmt, 1);
        AssertThat(tree->Node(block_inside)->Opcode() == OPC_BLOCK, ("Should be a block."));
        Opt_verify_block(block_inside, func, file, level, conf);
        break;
      }
      case OPR_GOTO_OUT: {
        AssertThat(tree->Node(stmt)->Get_label_num() == GOTO_OUT_BREAK ||
                   tree->Node(stmt)->Get_label_num() == GOTO_OUT_CONTINUE, ("Not correct label_num"));
        break;
      }
      case OPR_RETURN_VAL: {
        AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in RETURN_VAL, 1 expected, got %d", tree->Number_of_children(stmt)));
        IR_ITER expr_val = tree->Get_operand(stmt, 0);
        Opt_verify_expr(expr_val, stmt, func, file, level, conf);
        MTYPE_ID ret_type = OPCODE_rtype(tree->Get_node(expr_val)->Opcode());
        AssertThat(MTYPE_I4 == ret_type, ("Should return mtype i4"));
        break;
      }
      case OPR_RETURN: {
        AssertThat(tree->Number_of_children(stmt) == 0, ("Incorrect number of kid in RETURN, 0 expected, got %d", tree->Number_of_children(stmt)));
        break;
      }
      case OPR_CALL: {
        AssertThat(tree->Node(stmt)->Get_symbol_idx() != 0, ("Incorrect call stmt target symbol"));
        AssertThat(ST_sclass(tree->Node(stmt)->Get_symbol_idx()) == SYMC_TEXT ||
                   ST_sclass(tree->Node(stmt)->Get_symbol_idx()) == SYMC_EXTERN, ("Either should this be extern or text"));
        // Verify # of arguments matching TY declaration.
        break;
      }
      case OPR_LABEL: {
        AssertThat(tree->Node(stmt)->Get_label_num() != 0, ("Incorrect label idx"));
        break;
      }
      case OPR_GOTO: {
        AssertThat(tree->Node(stmt)->Get_label_num() != 0, ("Incorrect label idx"));
        break;
      }
      default: {
        AssertThat(false, ("Opcode: %s should not be in the body", OPCODE_name(tree->Get_node(stmt)->Opcode())));
      }
    }
  }
}

void Opt_verify_function(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                         COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  AssertThat(tree != nullptr, ("Tree should not be empty"));
  IR_ITER root = tree->Get_root();
  AssertThat(root != nullptr, ("root should not be empty"));
  AssertThat(tree->Number_of_children(root) == 2, ("there should be exactly 2 nodes in the func_entry"));
  IR_ITER body = tree->Get_operand(root, TREE_SEQ_BODY);
  if (tree->Number_of_children(body) <= 0) {
    Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
             (TFile, "There is no statement in the body, skip verification\n"));
    return;
  }
  Opt_verify_block(body, func, file, level, conf);
}

void Opt_lower(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  // Lower IR constructs
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    // Iterate over each pu_info (functions), dump each of the function
    PU_INFO *pu_info = file->Tables()->Pu_info()->Get(it);
    if (pu_info->proc_sym != 0) {
      Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
               (TFile, "Lowering function for pu_info_id = %u\n", it));
      Opr_lower_function(pu_info, file, level, config);
    } else {
      AssertThat(false, ("Incomoplete pu_infoo for PU_INFO_IDX = %u, or %0#x", it, it));
    }
  }
}

// Whether if this node is known to be constant, or LDID of constants
BOOL Opt_is_const(IR_ITER node, TREE *tree, PU_INFO *func, FILE_MANAGER *file,
                  IR_LEVEL level, COMPILER_CONFIG &conf) {
  return (OPCODE_is_const(tree->Get_node(node)->Opcode()));
}

IR_ITER
Opt_prpgate_const_bin_op(IR_ITER expr, TREE *tree, PU_INFO *func, FILE_MANAGER *file,
                 IR_LEVEL level, COMPILER_CONFIG &config) {
  if (!(Opt_is_const(tree->Get_operand(expr, 0), tree, func, file, level, config) &&
        Opt_is_const(tree->Get_operand(expr, 1), tree, func, file, level, config))) {
    // Not constants, skip
    return expr;
  }
  // Constants on both sides.
  AssertThat(tree->Get_node(tree->Get_operand(expr, 0))->Opcode() == OPC_I4CONST, ("Unknown pattern"));
  AssertThat(tree->Get_node(tree->Get_operand(expr, 1))->Opcode() == OPC_I4CONST, ("Unknown pattern"));
  UINT64 lhs = tree->Get_node(tree->Get_operand(expr, 0))->Get_const_val();
  UINT64 rhs = tree->Get_node(tree->Get_operand(expr, 1))->Get_const_val();
  AssertThat(OPCODE_rtype(tree->Get_node(expr)->Opcode()) == MTYPE_I4,
             ("Incorrect type used in op = add, expected i4, given %d", OPCODE_rtype(tree->Get_node(expr)->Opcode())));
  IRNODE_IDX opr_node = tree->Create_node(OPC_I4CONST);
  switch (OPCODE_operator(tree->Get_node(expr)->Opcode())) {
    case OPR_ADD: {
      tree->Get_node(opr_node)->Set_const_val(lhs + rhs);
      break; // not optimizing anything
    }
    case OPR_REM: {
      tree->Get_node(opr_node)->Set_const_val(lhs % rhs);
      break; // not optimizing anything
    }
    case OPR_DIV: {
      AssertThat(rhs != 0, ("Cannot divide by zero."));
      tree->Get_node(opr_node)->Set_const_val(lhs / rhs);
      break; // not optimizing anything
    }
    case OPR_SUB: {
      tree->Get_node(opr_node)->Set_const_val(lhs - rhs);
      break; // not optimizing anything
    }
    case OPR_MPY: {
      tree->Get_node(opr_node)->Set_const_val(lhs * rhs);
      break; // not optimizing anything
    }
    case OPR_MOD: {
      tree->Get_node(opr_node)->Set_const_val(lhs % rhs);
      break; // not optimizing anything
    }
    default:
      return expr;
  }
  IR_ITER cur_node = tree->Insert_temp_node(opr_node);
  return cur_node;
}

IR_ITER Opt_lower_expr(IR_ITER expr, UINT32 index_in_parent, PU_INFO *func, FILE_MANAGER *file,
                       IR_LEVEL level, COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  if (tree->Number_of_children(expr) >= 0) {
    // visit child
    for (UINT32 ch_id = 0; ch_id < tree->Number_of_children(expr); ch_id++) {
      IR_ITER ch = tree->Get_operand(expr, ch_id);
      if (tree->Node(expr)->Opcode() == OPC_BLOCK) {
        Opt_lower_stmt(ch, func, file, level, conf);
      } else {
        IR_ITER res = Opt_lower_expr(ch, ch_id, func, file, level, conf);
        if (tree->Internal_tree().is_valid(ch)) {
          if (ch != res) {
            tree->Replace_recursive(ch, res);
          }
        }
      }
    }
  }
  // This is done on every level.
  if (OPCODE_is_bin_arith(tree->Get_node(expr)->Opcode()) &&
      conf.Opt_enabled(OPT_KIND_ARITH)) {
    // Check if lowerable,
    IR_ITER res = Opt_prpgate_const_bin_op(expr, tree, func, file, level, conf);
    if (res != expr) {
      return res;
    }
  }
  if (OPCODE_operator(tree->Get_node(expr)->Opcode()) == OPR_MOD &&
      (level == LEVEL_MID || level == LEVEL_HIGH || level == LEVEL_VHIGH)) {
    return Opt_lower_mod_op(expr, tree);
  }
  if (OPCODE_operator(tree->Get_node(expr)->Opcode()) == OPR_DIV &&
     (level == LEVEL_MID || level == LEVEL_HIGH || level == LEVEL_VHIGH)) {
    return Opt_lower_div_op(expr, tree);
  }
  if (OPCODE_operator(tree->Get_node(expr)->Opcode()) == OPR_LAND &&
      level == LEVEL_LOW) {
    // Lower
    /*
     * FALSEBR LABEL X
     *   EXPR0 (maybe LAND)
     *     LAND
     *       EXPR1
     *       EXPR2
     *     EXPR 3
     *  ->
     *
     *  FALSEBR LABEL X
     *    EXPR 1
     *
     *  FALSEBR LABEL X
     *    EXPR 0 (maybe LAND)
     *      EXPR 2
     *      EXPR 3
     * */
    IR_ITER lhs = tree->Get_operand(expr, 0);
    IR_ITER rhs = tree->Get_operand(expr, 1);
    IR_ITER use_stmt = tree->Get_parent_in_block(expr);
    IR_ITER block = tree->Get_parent(use_stmt);
    AssertThat(tree->Node(use_stmt)->Opcode() == OPC_FALSEBR,
               ("Not in a falsebr, it it a %s. ",
                 OPCODE_name(tree->Node(use_stmt)->Opcode())));
    IRNODE_IDX new_false_br = tree->Create_node(OPC_FALSEBR);
    tree->Node(new_false_br)->Set_label_num(tree->Node(use_stmt)->Get_label_num());
    IR_ITER new_false_br_stmt = tree->Insert_before(use_stmt, new_false_br);
    IR_ITER expr_lhs = tree->Set_operand(new_false_br_stmt, 0, new_false_br);
    IR_ITER expr_rhs = tree->Set_operand(new_false_br_stmt, 1, new_false_br);
    expr_lhs = tree->Replace_recursive(expr_lhs, lhs);
//    IR_ITER new_expr = tree->Set_operand(expr, 1, new_false_br);
    expr_rhs = tree->Replace_recursive(expr_rhs, rhs);
    return expr_rhs;
  }
  if (OPCODE_operator(tree->Get_node(expr)->Opcode()) == OPR_BIOR &&
      level == LEVEL_LOW) {
    // Lower
    /*
     * FALSEBR LABEL X
     *   EXPR0 (maybe LAND)
     *     BIOR
     *       EXPR1
     *       EXPR2
     *     EXPR 3
     *  ->
     *
     *  TRUEBR LABEL 6
     *    EXPR 1
     *
     *  FALSEBR LABEL X
     *    EXPR 0 (maybe LAND)
     *      EXPR 2
     *      EXPR 3
     *  LABEL 6
     * */
    IR_ITER lhs = tree->Get_operand(expr, 0);
    IR_ITER rhs = tree->Get_operand(expr, 1);
    IR_ITER use_stmt = tree->Get_parent_in_block(expr);
    IR_ITER block = tree->Get_parent(use_stmt);
    AssertThat(tree->Node(use_stmt)->Opcode() == OPC_FALSEBR,
               ("Not in a falsebr, it it a %s. ",
                 OPCODE_name(tree->Node(use_stmt)->Opcode())));
    IRNODE_IDX new_true_br = tree->Create_node(OPC_TRUEBR);
    IRNODE_IDX after_label = tree->Create_node(OPC_LABEL);
    LABEL_IDX  label3_id = 0;
    const IR_ITER &next_sib = tree->Internal_tree().next_sibling(expr);
    if (tree->Internal_tree().is_valid(next_sib) &&
        tree->Node(next_sib)->Opcode() == OPC_LABEL) {
      label3_id = tree->Node(next_sib)->Get_label_num();
    } else {
      char name_buf[1024];
      sprintf(name_buf, ".L_%d_tbr_%llu", func->proc_sym, *expr);
      STR_IDX    lname3      = File()->Save_string(name_buf);
      IRNODE_IDX label3_node = tree->Create_node(OPC_LABEL);
      label3_id = File()->Create_label(lname3,LABEL_ADDR_SAVED,
                                              LKIND_DEFAULT);
      tree->Node(label3_node)->Set_label_num(label3_id);
      IR_ITER new_label = tree->Insert_after(use_stmt, label3_node);
    }
    tree->Node(new_true_br)->Set_label_num(label3_id);
    IR_ITER new_false_br_stmt = tree->Insert_before(use_stmt, new_true_br);
    IR_ITER expr_lhs = tree->Set_operand(new_false_br_stmt, 0, new_true_br);
    IR_ITER expr_rhs = tree->Set_operand(new_false_br_stmt, 1, new_true_br);
    expr_lhs = tree->Replace_recursive(expr_lhs, lhs);
    expr_rhs = tree->Replace_recursive(expr_rhs, rhs);
    return expr_rhs;
  }
  if (OPCODE_operator(tree->Get_node(expr)->Opcode()) == OPR_LDID &&
      conf.Opt_enabled(OPT_KIND_LDID_CONST)  &&
      level == LEVEL_HIGH) {
    // LDID optimization
    // TODO: ...
    ST_IDX sym = tree->Get_node(expr)->Get_symbol_idx();
    if (TY_flags(ST_ty(sym)) & TY_FLAG_CONST) {
      // this is doable.
      Is_Trace(Tracing(COMPONENT_GOPT, TRACE_INFO),
               (TFile, ("Missing opportunity of LDID-const convesion")));
    }
  }
  if (OPCODE_operator(tree->Get_node(expr)->Opcode()) == OPR_ARRAY &&
      level == LEVEL_MID) {
    IR_ITER temp = Opt_lower_array_expr(expr, tree);
    return temp;
  }
  if (OPCODE_operator(tree->Get_node(expr)->Opcode()) == OPR_COMMA &&
      (level == LEVEL_MID || level == LEVEL_LOW)) {
    IR_ITER comma_blk = tree->Get_operand(expr, 0);
    IR_ITER comma_ldid = tree->Get_operand(expr, 1);
    AssertThat(tree->Node(comma_ldid)->Opcode() == OPC_I4LDID,
               ("Must be a LDID or preg retval"));
    IR_ITER use_stmt = tree->Get_parent_in_block(expr);
    IR_ITER block = tree->Get_parent(use_stmt);
    comma_blk = tree->Get_operand(expr, 0);
    AssertThat(comma_blk.number_of_children() == 2,
               ("Should have only one call + 1 stid. node = %d, yet child count = %d", *comma_blk,
                comma_blk.number_of_children()));
    IR_ITER call_stmt = tree->Get_operand(comma_blk, 0);
    IR_ITER stid_stmt = tree->Get_operand(comma_blk, 1);
    AssertThat(tree->Node(call_stmt)->Opcode() == OPC_I4CALL,
               ("Should be a I4 call. node = %d", *call_stmt));
    AssertThat(tree->Node(stid_stmt)->Opcode() == OPC_I4STID,
               ("Should be a I4 STID. node = %d", *call_stmt));
    if (tree->Node(use_stmt)->Opcode() == OPC_IF ||
        tree->Node(use_stmt)->Opcode() == OPC_WHILE_DO) {
      // a conditional evaluation ...
      Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN),
               (TFile, "WARNING: The generated code ignores "
                       "conditional evaluation. on %lld\n", *use_stmt));
    }
    tree->Internal_tree().insert_subtree(use_stmt, call_stmt);
    tree->Internal_tree().insert_subtree(use_stmt, stid_stmt);
    IRNODE_IDX new_node = tree->Create_node(OPC_I4LDID);
    tree->Node(new_node)->Set_symbol_idx(tree->Node(comma_ldid)->Get_symbol_idx());
    tree->Node(new_node)->Set_preg_num(tree->Node(comma_ldid)->Get_preg_num());
    return tree->Insert_temp_node(new_node);
  }
  // Nothing to do.
  return expr;
}



/*
 * MOD
 *  A
 *  B
 * ->
 * SUB
 *   A
 *   MPY
 *    B
 *    DIV
 *     A
 *     B
 * */
IR_ITER Opt_lower_mod_op(const IR_ITER &expr, TREE *tree) {
  AssertThat(tree->Number_of_children(expr) == 2,
             ("Not a valid MOD operator, got kid = %d", tree->Number_of_children(
               expr)));
  IR_ITER lhs = tree->Get_operand(expr, 0);
  IR_ITER rhs = tree->Get_operand(expr, 1);
  IRNODE_IDX sub_node = tree->Create_node(OPC_I4I4SUB);
  IRNODE_IDX mpy_node = tree->Create_node(OPC_I4I4MPY);
  IRNODE_IDX div_node = tree->Create_node(OPC_I4I4DIV);
  IR_ITER sub_expr = tree->Insert_temp_node(sub_node);
  IR_ITER mpy_expr = tree->Set_operand(sub_expr, 0, mpy_node);
  IR_ITER new_a1 = tree->Internal_tree().insert_subtree(mpy_expr, lhs);
  IR_ITER div_expr = tree->Set_operand(mpy_expr, 0, div_node);
  IR_ITER new_b1 = tree->Internal_tree().insert_subtree(div_expr, rhs);
  IR_ITER new_a2_expr = tree->Set_operand(div_expr, 0, div_node);
  IR_ITER new_b2_expr = tree->Set_operand(div_expr, 1, div_node);
  IR_ITER new_a2 = tree->Replace_recursive(new_a2_expr, lhs);
  IR_ITER new_b2 = tree->Replace_recursive(new_b2_expr, rhs);
  return sub_expr;
}


IR_ITER Opt_lower_div_op(IR_ITER &expr, TREE *tree) {
  AssertThat(tree->Number_of_children(expr) == 2,
             ("Not a valid MOD operator, got kid = %d", tree->Number_of_children(
               expr)));
  IR_ITER lhs = tree->Get_operand(expr, 0);
  IR_ITER rhs = tree->Get_operand(expr, 1);
  IRNODE_IDX call_node = tree->Create_node(OPC_I4CALL);
  // Create a COMMA + block
  IRNODE_IDX comma_idx   = tree->Create_node(OPC_COMMA);
  IRNODE_IDX block_idx   = tree->Create_node(OPC_BLOCK);
  IRNODE_IDX stid_idx    = tree->Create_node(OPC_I4STID);
  IRNODE_IDX ldid_idx    = tree->Create_node(OPC_I4LDID);
  IRNODE_IDX ld_ret_node = tree->Create_node(OPC_I4LDID);
  IR_ITER ret_stmt = tree->Insert_temp_node(comma_idx);
  IR_ITER block_expr = tree->Set_operand(ret_stmt, 0, block_idx);
  IR_ITER call_stmt = tree->Insert_stmt_to_block(block_expr, call_node);
  IR_ITER stid_expr =  tree->Insert_stmt_to_block(block_expr, stid_idx);
  IR_ITER ldid_expr = tree->Set_operand(ret_stmt, 1, ldid_idx);
  IR_ITER ld_ret_expr = tree->Set_operand(stid_expr, 0, ld_ret_node);
  PREG_IDX preg = File()->Create_preg(File()->Save_string(".ret_medium"), 0);
  PREG_IDX preg_ret = File()->Create_preg(File()->Save_string(".return_val"), 1);
  tree->Node(ldid_expr)->Set_symbol_idx(File()->Get_preg_sym(MTYPE_I4, preg));
  tree->Node(ldid_expr)->Set_preg_num(preg);
  tree->Node(stid_expr)->Set_symbol_idx(File()->Get_preg_sym(MTYPE_I4, preg));
  tree->Node(stid_expr)->Set_preg_num(preg);
  tree->Node(ld_ret_expr)->Set_symbol_idx(File()->Get_preg_sym(MTYPE_I4, preg));
  tree->Node(ld_ret_expr)->Set_preg_num(preg_ret);

  ST_IDX sym = File()->Find_symbol_by_name("__aeabi_idiv");
  tree->Node(call_node)->Set_symbol_idx(sym);
  IR_ITER temp_lhs = tree->Set_operand(call_stmt, 0, call_node);
  IR_ITER new_lhs = tree->Internal_tree().insert_subtree_after(temp_lhs, lhs);
  IR_ITER new_rhs = tree->Internal_tree().insert_subtree_after(temp_lhs, rhs);
  tree->Remove_node_recursive(temp_lhs);
  return ret_stmt;
}


IR_ITER Opt_lower_array_expr(IR_ITER &expr, TREE *tree) {
/*
 * Lowering ARRAY opr
 *
 * EXPR is usually CONST.
 *
 *  ARR
 *    5
 *    6
 *    7
 *    8
 *
 *    1
 *    2
 *    3
 *    4
 *
 *    *((1 * 6 + 2 ) * 7 + 3) * 8 + 4) * 4

 * ARRAY
 *   LDA
 *   EXPR size_dim1
 *   EXPR size_dim2
 *   ..
 *   EXPR size_dimn
 *   EXPR ofst_dim1
 *   EXPR ofst_dim2
 *   ..
 *   EXPR ofst_dimn
 * ->
 *mul
 * 4
 * add
 *  ofst_4
 *  mul
 *   size_4
 *   add
 *    ofst_3
 *     mul
 *      size_3
 *      add
 *       ofst_2
 *       mul
 *        size_2
 *        afst1
 *
 *  add
 *    ofst_last
 *    mul
 *      size last
 *      ofst 2
 *  add
 *    lda
 *    mul
 * */
  AssertThat((tree->Number_of_children(expr) - 1) % 2 == 0,
   ("not an even number of exprs for dimension/ofst in ARRAY."));
  AssertThat((tree->Number_of_children(expr) - 1) / 2 > 0,
   ("There should be at least one dimension in ARRAY."));
  UINT32        dims           = (tree->Number_of_children(expr) - 1) / 2;
  IR_ITER       temp           = tree->Insert_temp_node(tree->Create_node(OPC_I4I4ADD));
  IR_ITER       mul_ty_sz      = tree->Set_operand(temp, 0, tree->Create_node(OPC_I4I4MPY));
  IR_ITER       mul_const_sz   = tree->Set_operand(mul_ty_sz, 0, tree->Create_node(OPC_I4CONST));
  tree->Node(mul_const_sz)->Set_const_val(TY_size(MTYPE_to_ty(MTYPE_I4)));

  IR_ITER       lda_node       = tree->Get_operand(expr, 0);
  IR_ITER       new_lda_node   = tree->Internal_tree(). // temp's first param.
    insert_subtree(mul_ty_sz, lda_node); // move the LDA of first address, mul's 1st op.

  IR_ITER       last_add       = tree->Set_operand(mul_ty_sz, 1, tree->Create_node(OPC_I4I4ADD));
  last_add  = tree->Set_operand(mul_ty_sz, 1, tree->Create_node(OPC_I4I4ADD));
  // ADD
  //  LDA
  //  MUL
  //   CONST 4
  //   ADD <- last add
  //
  //   ADD <- last add
  //     - OFST level n
  //     - MUL 0 if ofst is the last ?, or MUL + size
        //   - size_expr
        //   - ADD
        //  c0
        //  c1
        //  co0
        //  co1
  for (INT32  cur_dim = 0; cur_dim < dims; cur_dim++) {
    INT32 dim_last_cnt = dims - cur_dim - 1; // ... 4,3,2,1,0
    IR_ITER temp_mul = tree->Set_operand(last_add, 0,
                                         tree->Create_node(OPC_I4I4MPY));
    IR_ITER temp_dim_size = tree->Set_operand(temp_mul, 0, tree->Create_node(OPC_I4I4ADD));
    IR_ITER dim_ofst = tree->Internal_tree().
      insert_subtree(temp_mul,
                           tree->Get_operand(
                             expr,
                             1 +
                             dims +
                             dim_last_cnt)); // move the size expr
    IR_ITER dim_size = tree->Internal_tree().
      insert_subtree(temp_dim_size,
                           tree->Get_operand(
                             expr,
                             1 +
                             dim_last_cnt)); // move the ofst expr
    last_add = temp_dim_size;
  }

  AssertThat(tree->Node(last_add)->Opcode() == OPC_I4I4ADD,
             ("Should be an ADD op. = %d", *last_add));
  // Move the last MPY to its parent, which used to be an ADD.
  tree->Node(last_add)->Set_opcode(OPC_I4CONST);
  tree->Node(last_add)->Set_const_val(0);

  Is_Trace(Tracing(COMPONENT_GOPT, TRACE_DATA),
           (TFile, "%sLowering ARRAY\n%s", DBAR, DBAR));
  if(Tracing(COMPONENT_GOPT, TRACE_DATA)) {
    tree->Print_recursive(TFile);
  }
  Is_Trace(Tracing(COMPONENT_GOPT, TRACE_DATA),
           (TFile, "%sEnd of lowering ARRAY\n%s", DBAR, DBAR));
  return temp;
}

IR_ITER Opt_lower_stmt(IR_ITER stmt, PU_INFO *func, FILE_MANAGER *file,
                       IR_LEVEL level, COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  char *name_buf = new char[128];
  if (tree->Number_of_children(stmt) >= 0) {
    // visit child
    for (UINT32 ch_id = 0; ch_id < tree->Number_of_children(stmt); ch_id++) {
      IR_ITER ch = tree->Get_operand(stmt, ch_id);
      IR_ITER res = Opt_lower_expr(ch, ch_id, func, file, level, conf);
      if (ch != res) {
        tree->Replace_recursive(ch, res);
      }
    }
  }
  switch (OPCODE_operator(tree->Get_node(stmt)->Opcode())) {
    case OPR_GOTO_OUT: {
      /*
       * WHILE_DO
      // EQ
      //  BLOCK
      //   IF ..
      //    THEN
              GOTO_OUT break;
            ELSE
              GOTO_OUT continue;
      ->
       LABEL3
       WHILE_DO
         EQ
         BLOCK
          IF
           THEN
            GOTO LABEL 4
           ELSE
            GOTO LABEL 3
       LABEL 4
       */
      IR_ITER while_stmt = tree->Get_parent_region(stmt);
      IR_ITER label3_stmt;
      sprintf(name_buf, ".L_%d_3_%llu", func->proc_sym, *stmt);
      STR_IDX    lname3       = File()->Save_string(name_buf);
      IRNODE_IDX label3_node = tree->Create_node(OPC_LABEL);
      LABEL_IDX  label3_id    = File()->Create_label(lname3,
                                                    LABEL_ADDR_SAVED,
                                                    LKIND_DEFAULT);

      label3_stmt = tree->Insert_before(while_stmt, label3_node);
      tree->Get_node(label3_stmt)->Set_label_num(label3_id);

      IR_ITER label4_stmt;
      sprintf(name_buf, ".L_%d_4_%llu", func->proc_sym, *stmt);
      STR_IDX    lname4       = File()->Save_string(name_buf);
      IRNODE_IDX label4_node = tree->Create_node(OPC_LABEL);
      LABEL_IDX  label4_id    = File()->Create_label(lname4,
                                                     LABEL_ADDR_SAVED,
                                                     LKIND_DEFAULT);

      label4_stmt = tree->Insert_after(while_stmt, label4_node);
      tree->Get_node(label4_stmt)->Set_label_num(label4_id);

      IRNODE_IDX goto_node = tree->Create_node(OPC_GOTO);
      if(tree->Node(stmt)->Get_label_num() == GOTO_OUT_BREAK){
        tree->Node(goto_node)->Set_label_num(label4_id);
      } else if(tree->Node(stmt)->Get_label_num() == GOTO_OUT_CONTINUE){
        tree->Node(goto_node)->Set_label_num(label3_id);
      } else {
        AssertThat(false,
                   ("Condition not impl. label = %d",
                     tree->Node(stmt)->Get_label_num()));
      }
      IR_ITER goto_stmt = tree->Insert_after(stmt, goto_node);
      tree->Remove_node_recursive(stmt);
      stmt = goto_stmt;
      break;
    }
    case OPR_WHILE_DO: {
      if (level != LEVEL_MID) break;
      stmt = Opt_lower_while_do(stmt, func, tree, name_buf);
      break;
    }
    case OPR_IF: {
      if (level != LEVEL_MID) break;
      stmt = Opt_lower_if_stmt(stmt, func, tree, name_buf);
      break;
    }
    case OPR_TRUEBR:
    case OPR_FALSEBR: {
      // Add NE 0 if there is only one kid
      if (level != LEVEL_VLOW) break;
      OPCODE opc = tree->Node(tree->Get_operand(stmt, 0))->Opcode();
      OPERATOR opr = OPCODE_operator(opc);
      switch (opr) {
        case OPR_NE:
        case OPR_EQ:
        case OPR_LT:
        case OPR_LE:
        case OPR_GT:
        case OPR_GE:
          break;
        default:
          // Not a valid opr, need
          AssertThat(tree->Number_of_children(stmt) == 1, ("not 1 child in true/falsebr"));
          IR_ITER cond = tree->Get_operand(stmt, 0);
          IRNODE_IDX new_ne = tree->Create_node(OPC_I4I4NE);
          IRNODE_IDX const_zero = tree->Create_node(OPC_I4CONST);
          IR_ITER temp_ne = tree->Insert_temp_node(new_ne);
          IR_ITER temp_zero = tree->Set_operand(temp_ne, 0, const_zero);
          IR_ITER new_cond = tree->Internal_tree().insert_subtree_after(temp_zero, cond);
          IR_ITER new_pos = tree->Replace_recursive(cond, temp_ne);
          break;
      }
    }
    default: {
      // do nothing about them.
    }
  }
  delete[] name_buf;
  return stmt;
  // Replace expr if necessary
}

IR_ITER &
Opt_lower_while_do(IR_ITER &stmt, const PU_INFO *func, TREE *tree,
                   char *name_buf) {
  /*
   // WHILE_DO
   //  EQ
   //  BLOCK
   //   ...stmts...

    transform to :

    LABEL 1
    FALSEBR LABEL 2
     EQ
    ... stmts ...
    GOTO LABEL 1
    LABEL 2
   */
  IR_ITER label1_stmt;
  sprintf(name_buf, ".L_%d_1_%llu", func->proc_sym, *stmt);
  STR_IDX    lname       = File()->Save_string(name_buf);
  IRNODE_IDX label1_node = tree->Create_node(OPC_LABEL);
  LABEL_IDX  label_id    = File()->Create_label(lname,
                                            LABEL_ADDR_SAVED,
                                            LKIND_DEFAULT);

  label1_stmt = tree->Insert_before(stmt, label1_node);
  tree->Get_node(label1_stmt)->Set_label_num(label_id);

  IRNODE_IDX identifier_node = tree->Create_node(OPC_GOTO);
  tree->Node(identifier_node)->Set_label_num(label_id);
  IR_ITER goto_stmt = tree->Insert_after(stmt, identifier_node);

  IR_ITER label2_stmt;
  sprintf(name_buf, ".L_%d_2_%llu", func->proc_sym, *stmt);
  STR_IDX    lname2      = File()->Save_string(name_buf);
  IRNODE_IDX label2_node = tree->Create_node(OPC_LABEL);
  LABEL_IDX label2_id = File()->Create_label(lname2,
                                             LABEL_ADDR_SAVED,
                                             LKIND_DEFAULT);

  label2_stmt = tree->Insert_after(goto_stmt, label2_node);
  tree->Get_node(label2_stmt)->Set_label_num(label2_id);

  /*
* WHILE_DO
*  EQ
*  BLOCK
*   STMT1
*   STMT2
*   ...
*   STMTn
* GOTO
*
* ->
*
* WHILE_DO
*   EQ
*   BLOCK
*    STMT1
*    ...
*    STMTn
* STMT1
* STMT2
* ...
* STMTn
* GOTO
*
*/
  // Move the contents the in while block to before the GOTO stmt.
  IR_ITER while_block = tree->Get_operand(stmt, 1);
  UINT32 n_stmts = tree->Number_of_children(while_block);
  for(UINT32 i = 0; i < n_stmts; i++) {
    IR_ITER orig = tree->Get_operand(while_block, i);
    tree->Internal_tree().insert_subtree(goto_stmt, orig);
  }
  tree->Remove_node_recursive(while_block);
  tree->Node(stmt)->Set_opcode(OPC_FALSEBR);
  tree->Node(stmt)->Set_label_num(label2_id);
  return stmt;
}

IR_ITER &Opt_lower_if_stmt(IR_ITER &stmt, const PU_INFO *func, TREE *tree,
                           char *name_buf) {//Move stmts to parent
  IR_ITER par = tree->Get_parent(stmt);
  AssertThat(tree->Node(par)->Opcode() == OPC_BLOCK, ("Must be a block node containing if stmt."));
  IR_ITER then = tree->Get_operand(stmt, 1);
  IR_ITER else_blk = tree->Get_operand(stmt, 2);

  // Create the else label
  sprintf(name_buf, ".L_%d_else_%llu", func->proc_sym, *stmt);
  STR_IDX    lname      = File()->Save_string(name_buf);
  LABEL_IDX  else_lidx  = File()->Create_label(lname, 0, LKIND_DEFAULT);
  IRNODE_IDX lnode_idx  = tree->Create_node(OPC_LABEL);
  tree->Node(lnode_idx)->Set_label_num(else_lidx);
  IR_ITER else_label = tree->Insert_after(stmt, lnode_idx);

  // Create end label
  sprintf(name_buf, ".L_%d_end_%llu", func->proc_sym, *stmt);
  lname                   = File()->Save_string(name_buf);
  LABEL_IDX end_lidx      = File()->Create_label(lname, 0, LKIND_DEFAULT);
  lnode_idx               = tree->Create_node(OPC_LABEL);
  tree->Node(lnode_idx)->Set_label_num(end_lidx);
  IR_ITER temp_end = tree->Insert_after(else_label, lnode_idx);

  // Move the kids
  UINT32 n_then = tree->Number_of_children(then);
  for(UINT32 i = 0; i < n_then; i++) {
    IR_ITER orig = tree->Get_operand(then, i);
    tree->Internal_tree().insert_subtree(else_label, orig);
  }
  IRNODE_IDX goto_ndoe = tree->Create_node(OPC_GOTO);
  tree->Node(goto_ndoe)->Set_label_num(end_lidx);
  tree->Internal_tree().insert(else_label, goto_ndoe);

  UINT32 n_else = tree->Number_of_children(else_blk);
  for(UINT32 i = 0; i < n_else; i++) {
    IR_ITER orig = tree->Get_operand(else_blk, i);
    tree->Internal_tree().insert_subtree(temp_end, orig);
  }
  tree->Node(stmt)->Set_label_num(else_lidx);
  tree->Node(stmt)->Set_opcode(OPC_FALSEBR);
  tree->Remove_node_recursive(then);
  tree->Remove_node_recursive(else_blk);
  return stmt;
}

void Opr_lower_function(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                        COMPILER_CONFIG &conf) {
  File()->Scopes()->Goto_function(func->proc_sym);
  // Lower each node in the tree
  TREE *tree = func->entry;
  IR_ITER body = tree->Get_operand(tree->Get_root(), TREE_SEQ_BODY);
  if (tree->Number_of_children(body) <= 0) {
    Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
             (TFile, "There is no statement in the body, skip lowering\n"));
    return;
  }
  // Lowering nodes in the ir tree
  for (UINT32 stmt_idx = 0; stmt_idx < tree->Number_of_children(body); stmt_idx++) {
    IR_ITER stmt = tree->Get_operand(body, stmt_idx);
    IR_ITER lower_result = Opt_lower_stmt(stmt, func, file, level, conf);
    if (lower_result == nullptr) {
      // remove current node.
      Is_Trace(Tracing(COMPONENT_VHO, TRACE_OPTIONS),
               (TFile, "Removing stmt during lowering : node-id = %llu \n", *stmt));
      tree->Remove_node_recursive(stmt);
      stmt_idx --;
      continue;
    }
    if (stmt != lower_result) {
      tree->Replace_recursive(stmt, lower_result);
    }
  }
}
