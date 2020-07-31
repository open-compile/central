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
  // Optimizations on High IR

  Opt_lower(File(), LEVEL_MID, conf);
  Opt_verify(File(), LEVEL_HIGH, conf);
  // Optimizations on Mid IR, SSA, DCE, CSE ...

  Opt_lower(File(), LEVEL_LOW, conf);
  Opt_verify(File(), LEVEL_LOW, conf);
  // Optimizations done in low IR, not much though

  Opt_lower(File(), LEVEL_VLOW, conf);
  Opt_verify(File(), LEVEL_VLOW, conf);

  // To CGIR
  Opt_lower(File(), LEVEL_CGIR, conf);
  Opt_verify(File(), LEVEL_CGIR, conf);

  if(Tracing(COMPONENT_BE, TRACE_OPTIONS)) {
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
    case OPR_LDID: {
      break;
    }
    case OPR_ILOAD: {
      break;
    }
    case OPR_DIV: {
      AssertThat(tree->Number_of_children(expr) == 2, ("not 2 operands"));
      IR_ITER divisor = tree->Get_operand(expr, 0);
      AssertThat(tree->Node(divisor)->Opcode() != OPC_I4CONST ||
                 tree->Node(divisor)->Get_const_val() != 0, ("cannot divide by zero."));
      break;
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
        MTYPE_ID stid_type = OPCODE_desc(tree->Get_node(stmt)->Opcode());
        AssertThat(OPCODE_rtype(tree->Get_node(expr_val)->Opcode()) == stid_type, ("The stid's operand should have same type"));
        Opt_verify_expr(expr_val, stmt, func, file, level, conf);
        break;
      }
      case OPR_ISTORE: {
        AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in ISTORE, 1 expected, got %d", tree->Number_of_children(stmt)));
        IR_ITER expr_val = tree->Get_operand(stmt, 0);
        Opt_verify_expr(expr_val, stmt, func, file, level, conf);
      }
      case OPR_IF: {
        AssertThat(level < LEVEL_MID, ("IF should not be present in level %d", level));
        AssertThat(tree->Number_of_children(stmt) == 3, ("Incorrect number of kid in WHILE_DO, 2 expected, got %d", tree->Number_of_children(stmt)));

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
        AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in WHILE_DO, 2 expected, got %d", tree->Number_of_children(stmt)));
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
        AssertThat(tree->Node(stmt)->Get_label_num() == GOTO_OUT_BREAK, ("Not correct label_num"));
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
Opt_lower_bin_op(IR_ITER expr, TREE *tree, PU_INFO *func, FILE_MANAGER *file,
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
             ("Incorrect type used in op = add"));
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
    default:
      return expr;
  }
  IR_ITER cur_node = tree->Insert_temp_node(opr_node);
  return cur_node;
}

IR_ITER Opt_lower_expr(IR_ITER expr, PU_INFO *func, FILE_MANAGER *file,
                       IR_LEVEL level, COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  if (tree->Number_of_children(expr) >= 0) {
    // visit child
    for (UINT32 ch_id = 0; ch_id < tree->Number_of_children(expr); ch_id++) {
      IR_ITER ch = tree->Get_operand(expr, ch_id);
      if (tree->Node(expr)->Opcode() == OPC_BLOCK) {
        Opt_lower_stmt(ch, func, file, level, conf);
      } else {
        IR_ITER res = Opt_lower_expr(ch, func, file, level, conf);
        if (ch != res) {
          tree->Replace_recursive(ch, res);
        }
      }
    }
  }
  // This is done on every level.
  if (OPCODE_is_bin_arith(tree->Get_node(expr)->Opcode()) &&
      conf.Opt_enabled(OPT_KIND_ARITH)) {
    // Check if lowerable,
    IR_ITER res = Opt_lower_bin_op(expr, tree, func, file, level, conf);
    if (res != expr) {
      return res;
    }
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
    /*
     * Lowering ARRAY opr
     *
     * EXPR is usually CONST.
     *
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
     * ADD
     *  MUL
     *    EXPR size_dim1
     *    EXPR ofst_dim1
     *  ADD
     *    MUL
     *    ADD
     *      MUL
     *      ADD
     *        MUL
     *
     * */
    AssertThat((tree->Number_of_children(expr) - 1) % 2 == 0,
               ("not an even number of exprs for dimension/ofst in ARRAY."));
    AssertThat((tree->Number_of_children(expr) - 1) / 2 > 0,
               ("There should be at least one dimension in ARRAY."));
    UINT32      dims           = (tree->Number_of_children(expr) - 1) / 2;
    IRNODE_IDX  final_add      = tree->Create_node(OPC_I4I4ADD);
    IR_ITER     temp           = tree->Insert_temp_node(final_add);
    IRNODE_IDX  offst_calc_add = tree->Create_node(OPC_I4I4ADD);
    IR_ITER     last_add       = tree->Set_operand(temp, 0, offst_calc_add);
    IR_ITER     dim_size       = tree->Internal_tree().
      insert_subtree(last_add,
                     tree->Get_operand(
                       expr,
                       0)); // move the LDA of first address.
    for (UINT32 cur_dim        = 0; cur_dim < dims; cur_dim++) {
      IRNODE_IDX mul      = tree->Create_node(OPC_I4I4MPY);
      IR_ITER    temp_mul = tree->Set_operand(last_add, 0, mul);
      IR_ITER    dim_temp = tree->Set_operand(temp_mul, 0, 0);
      IR_ITER    dim_size = tree->Internal_tree().
        insert_subtree_after(dim_temp,
                             tree->Get_operand(
                               expr,
                               1 +
                               cur_dim)); // move the size expr
      IR_ITER    dim_ofst = tree->Internal_tree().
        insert_subtree_after(dim_size,
                             tree->Get_operand(
                               expr,
                               1 +
                               dims +
                               cur_dim)); // move the ofst expr
      tree->Remove_node_recursive(dim_temp);
      IRNODE_IDX next_add = tree->Create_node(OPC_I4I4ADD);
      last_add = tree->Set_operand(last_add, 1, next_add);
    }

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
  // Nothing to do.
  return expr;
}

IR_ITER Opt_lower_stmt(IR_ITER stmt, PU_INFO *func, FILE_MANAGER *file,
                       IR_LEVEL level, COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  char *name_buf = new char[128];
  if (tree->Number_of_children(stmt) >= 0) {
    // visit child
    for (UINT32 ch_id = 0; ch_id < tree->Number_of_children(stmt); ch_id++) {
      IR_ITER ch = tree->Get_operand(stmt, ch_id);
      IR_ITER res = Opt_lower_expr(ch, func, file, level, conf);
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
      AssertThat(false, ("OPR_GOTO_OUT not implemented."));
      break;
    }
    case OPR_WHILE_DO: {
      IR_ITER expr = tree->Get_operand(stmt, 0);
      IR_ITER lower_result = Opt_lower_expr(expr, func, file, level, conf);
      if (expr != lower_result) {
        tree->Replace_recursive(expr, lower_result);
      }
      IR_ITER block_content = tree->Get_operand(stmt, 1);
      for (UINT32 i = 0; i < block_content.number_of_children(); i++) {
        IR_ITER child_stmt = tree->Get_operand(block_content, i);
        Opt_lower_stmt(child_stmt, func, file, level, conf);
      }
      // Only go through the following if in a MIDDLE IR.
      if (level != LEVEL_MID) {
        break;
      }
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
      break;
    }
    case OPR_IF: {
      IR_ITER block_content = tree->Get_operand(stmt, 1);
      for (UINT32 i = 0; i < block_content.number_of_children(); i++) {
        IR_ITER child_stmt = tree->Get_operand(block_content, i);
        Opt_lower_stmt(child_stmt, func, file, level, conf);
      }
      block_content = tree->Get_operand(stmt, 2);
      for (UINT32 i = 0; i < block_content.number_of_children(); i++) {
        IR_ITER child_stmt = tree->Get_operand(block_content, i);
        Opt_lower_stmt(child_stmt, func, file, level, conf);
      }
      if (level != LEVEL_MID) break;
      //Move stmts to parent
      IR_ITER par = tree->Get_parent_block(stmt);
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
      break;
    }
    default: {
      // do nothing about them.
    }
  }
  delete[] name_buf;
  return stmt;
  // Replace expr if necessary
}

void Opr_lower_function(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                        COMPILER_CONFIG &conf) {
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
