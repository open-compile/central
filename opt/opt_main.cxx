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
        break;
      }
      case OPR_WHILE_DO: {
        AssertThat(level <= LEVEL_MID, ("WHILE_DO should not be present in level %d", level));
        AssertThat(tree->Number_of_children(stmt) == 2, ("Incorrect number of kid in WHILE_DO, 2 expected, got %d", tree->Number_of_children(stmt)));
        // Previous one should be label, next should be label
        break;
      }
      case OPR_RETURN_VAL: {
        AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in RETURN_VAL, 1 expected, got %d", tree->Number_of_children(stmt)));
        IR_ITER expr_val = tree->Get_operand(stmt, 0);
        MTYPE_ID ret_type = OPCODE_rtype(tree->Get_node(expr_val)->Opcode());
        AssertThat(MTYPE_I4 == ret_type, ("Should return mtype i4"));
        break;
      }
      case OPR_RETURN: {
        AssertThat(tree->Number_of_children(stmt) == 0, ("Incorrect number of kid in RETURN, 0 expected, got %d", tree->Number_of_children(stmt)));
        break;
      }
      default: {
        AssertThat(false, ("Opcode: %s should not be in the body", OPCODE_name(tree->Get_node(stmt)->Opcode())));
      }
    }
  }
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
  switch (OPCODE_operator(tree->Get_node(expr)->Opcode())) {
    case OPR_ADD: {
      UINT64 lhs = tree->Get_node(tree->Get_operand(expr, 0))->Get_const_val();
      UINT64 rhs = tree->Get_node(tree->Get_operand(expr, 1))->Get_const_val();
      AssertThat(OPCODE_rtype(tree->Get_node(expr)->Opcode()) == MTYPE_I4,
        ("Incorrect type used in op = add"));
      IRNODE_IDX opr_node = tree->Create_node(OPC_I4CONST);
      tree->Get_node(opr_node)->Set_const_val(lhs + rhs);
      IR_ITER cur_node = tree->Insert_temp_node(opr_node);
      return cur_node; // not optimizing anything
    }
    default:
      return expr;
  }
}

IR_ITER Opt_lower_expr(IR_ITER expr, PU_INFO *func, FILE_MANAGER *file,
                       IR_LEVEL level, COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  if (tree->Number_of_children(expr) >= 0) {
    // visit child
    for (UINT32 ch_id = 0; ch_id < tree->Number_of_children(expr); ch_id++) {
      IR_ITER ch = tree->Get_operand(expr, ch_id);
      IR_ITER res = Opt_lower_expr(ch, func, file, level, conf);
      if (ch != res) {
        tree->Replace_recursive(ch, res);
      }
    }
  }
  if (OPCODE_is_bin_arith(tree->Get_node(expr)->Opcode()) &&
      conf.Opt_enabled(OPT_KIND_ARITH) &&
      level == LEVEL_HIGH) {
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
  }
  // Nothing to do.
  return expr;
}

IR_ITER Opt_lower_stmt(IR_ITER stmt, PU_INFO *func, FILE_MANAGER *file,
                       IR_LEVEL level, COMPILER_CONFIG &conf) {
  TREE *tree = func->entry;
  if (OPCODE_operator(tree->Get_node(stmt)->Opcode()) == OPR_STID) {
    // optmize the child
    IR_ITER expr = tree->Get_operand(stmt, 0);
    IR_ITER lower_result = Opt_lower_expr(expr, func, file, level, conf);
    if (expr != lower_result) {
      tree->Replace_recursive(expr, lower_result);
    }
  }
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