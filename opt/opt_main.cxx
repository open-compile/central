#include "basic.h"
#include "host.h"
#include "options.h"
#include "be_export.h"
#include "file_util.h"
#include <fstream>
#include <ir.h>
#include "symtab.h"
#include "opt_main.h"
#include "cgir.h"
#include "tn.h"
#include <vector>
#include <map>

using std::vector;
using std::map;

CGIR *_cgir_opt = nullptr;

INT32 BE_MAIN_NAME(INT32 argc, char **argv) {
  AssertThat(argc > 1, ("not enough arguments"));
  return 0;
}

CGIR *Cgir() {
  if (_cgir_opt == nullptr) {
    _cgir_opt = new CGIR();
  }
  return _cgir_opt;
}

void CG_process_func(FILE_MANAGER *file, COMPILER_CONFIG &config) {
  // Convert OCIR to CGIR, saving the CGIR in file
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    // Iterate over each pu_info (functions), dump each of the function
    PU_INFO *pu_info = file->Tables()->Pu_info()->Get(it);
    if (pu_info->proc_sym != 0) {
      Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_OPTIONS),
               (TFile, "Converting function to CGIR for pu_info_id = %u\n", it));
      Cgir()->CG_Init(&pu_info->scope);
      if (Tracing(COMPONENT_CG_CONV, TRACE_DATA)) {
        Cgir()->Print(pu_info->proc_sym, TFile);
      }
    } else {
      AssertThat(false, ("Incomoplete pu_infoo for PU_INFO_IDX = %u, or %0#x", it, it));
    }
  }
}

INT32 CG_full_process(COMPILER_CONFIG &conf) {
  REGISTER_Begin();	/* initialize the register package */
  Init_Dedicated_TNs ();
  // Convert OCIR to CGIR
  CG_process_func(File(), conf);
  // Run emitting of assembly code.
  FILE *output_assembly_file = fopen(conf.output_file.c_str(), "w+");
  if (!output_assembly_file) {
    Comp_Failure("Cannot open output file to write = %s",
                 conf.output_file.c_str());
  }
  Emit_section_data(output_assembly_file, File());
  Emit_section_code(output_assembly_file, File());
  if (fclose(output_assembly_file) != 0) {
    Comp_Failure("Cannot close output file to write = %s",
                 conf.output_file.c_str());
  }
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

  // Local and Global register allocation
  // Instruction scheduling etc.,
  Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
           (TFile, "Writing assembly to %s\n", conf.output_file.c_str()));
  AssertThat(conf.output_file.size() > 0, ("Incorrect output file name"));
  if (File_exists(conf.output_file)) {
    // delete the file if it exists
    Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS),
             (TFile, "Removing old output file under %s\n", conf.output_file.c_str()));
    if (remove(conf.output_file.c_str()) != 0) {
      Comp_Failure("Cannot delete file : %s", conf.output_file.c_str());
    }
  }
  CG_full_process(conf);
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
        AssertThat(level <= LEVEL_CGIR, ("STID should not be present in level %d", level));
        AssertThat(tree->Number_of_children(stmt) == 1, ("Incorrect number of kid in STID, 1 expected, got %d", tree->Number_of_children(stmt)));
        IR_ITER expr_val = tree->Get_operand(stmt, 0);
        MTYPE_ID stid_type = OPCODE_desc(tree->Get_node(stmt)->Opcode());
        AssertThat(OPCODE_rtype(tree->Get_node(expr_val)->Opcode()) == stid_type, ("The stid's operand should have same type"));
        break;
      }
      default: {
        AssertThat(false, ("Opcode: %s should not be in the body", tree->Get_node(stmt)->OPCODE_name(tree->Get_node(stmt)->Opcode())));
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

void Emit_section_code(FILE *out, FILE_MANAGER *file) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (TFile, "%sEmitting section: code\n%s", DBAR, DBAR));
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Debugging info enabled, writing file-level code section\n"));
  fprintf(out, ".text\n\n");
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    // Iterate over each pu_info (functions), dump each of the function
    PU_INFO *pu_info = file->Tables()->Pu_info()->Get(it);
    if (pu_info->proc_sym != 0) {
      // Valid pu_info.
      Emit_function(pu_info, out, file);
    } else {
      Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Skip PU_INFO for id = %d, due to unknown proc_symid = %d \n", it, pu_info->proc_sym));
    }
  }
}

void Emit_function(PU_INFO *func, FILE *out, FILE_MANAGER *file) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Emitting function ST_IDX = %d, name = %s \n",  func->proc_sym, ST_name(func->proc_sym)));
  file->Scopes()->Goto_function(func->proc_sym);
  // Inside the function now, emitting all symtab info
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Function has %d non-trivial symbols\n", file->Tables()->Sym()->Length(&(func->scope)) - 1));
  for (UINT32 i = 1; i < file->Tables()->Sym()->Length(&(func->scope)); i++) {
    ST_IDX one = (i << 8) | LOCAL_SYMTAB;
    fprintf(out, "# Id: %0#x, Symbol : %s, Type: %d \n", one, ST_name(one), ST_ty(one));
  }
  // Dump the instructions
  const char *func_name = ST_name(func->proc_sym);
  fprintf(out, ".global %s\n", func_name);
  fprintf(out, "%s: \n", func_name);
  Emit_tree(func, func->entry, out, file);
  // TODO: Use CGIR's emission instead.
}

void Emit_tree(PU_INFO *func, TREE *tree, FILE *out, FILE_MANAGER *file) {
  IR_ITER it = tree->Get_root();
  fprintf(out, "\tstr\tfp, [sp, #-4]!\n");
  fprintf(out, "\tadd\tfp, sp, #0\n");
  AssertThat(tree->Get_node(it)->Opcode() == OPC_FUNC_ENTRY, ("Incorrect root opcode"));
  // Get the function body.
  IR_ITER body = tree->Get_operand(it, TREE_SEQ_BODY);
  IRTREE &irtree = tree->Internal_tree();
  UINT32 stmt_count = tree->Number_of_children(body);
  map<ST_IDX, UINT32> temp_labels;
  // Generate all statements in the function-level body block
  for (UINT32 i = 0; i < stmt_count; i++) {
    IR_ITER one_stmt = tree->Get_operand(body, i);
    IRNODE_IDX one_stmt_id = *one_stmt;
    IRNODE *node = tree->Get_node(one_stmt_id);
    switch (OPCODE_operator(node->Opcode())) {
      case OPR_STID: {
        // generate memory access
        IR_ITER expr = tree->Get_operand(one_stmt, 0);
        AssertThat(tree->Get_node(expr)->Opcode() == OPC_I4CONST,
                   ("Not implemented expr to generate assembly for"));
        if (temp_labels.find(node->Get_symbol_idx()) == temp_labels.end()) {
          temp_labels.insert(
            std::make_pair(node->Get_symbol_idx(), temp_labels.size()));
        }
        AssertThat(temp_labels.find(node->Get_symbol_idx()) !=
                   temp_labels.end(), ("Cannot locate correct entry in map"));
        UINT32 temp_label_id = temp_labels.find(node->Get_symbol_idx())->second;
        fprintf(out, "# [IRNODE:%llu] I4STID, sym = %s, stidx = %0#x, "
                     "temp_label_id = %u  \n",
                     one_stmt_id, ST_name(node->Get_symbol_idx()),
                     node->Get_symbol_idx(), temp_label_id);
        fprintf(out, "\tldr %s, .TL%s_%u\n", "r2", ST_name(func->proc_sym), temp_label_id);
        fprintf(out, "\tmov %s, #%d\n", "r3", (INT32) tree->Get_node(expr)->Get_const_val());
        fprintf(out, "\tstr %s, [%s]\n", "r3", "r2");
        break;
      }
      case OPR_LABEL: {
        // generate memory access
        fprintf(out, "# [IRNODE:%llu] LABEL\n", one_stmt_id);
        fprintf(out, "%s%llu:\n", "label_", one_stmt_id);
        break;
      }
      default: {
        fprintf(out, "# [IRNODE:%llu] Skip stmt with opcode = %s\n", one_stmt_id, node->OPCODE_name(node->Opcode()));
      }
    }
  }
  fprintf(out, ".%s_end:\n", ST_name(func->proc_sym));
  // Finishing function
  fprintf(out, "\tadd\tsp, fp, #0\n");
  fprintf(out, "\tldr\tfp, [sp], #4\n");
  fprintf(out, "\tbx\tlr\n");

  // Dumping temp labels
  fprintf(out, "# Dumping temp labels : total = %lu \n", temp_labels.size());
  for (auto local_temp_it : temp_labels) {
    fprintf(out, ".TL%s_%u:\t.word %s\n", ST_name(func->proc_sym), local_temp_it.second, ST_name(local_temp_it.first));
  }
}

INT32 Emit_section_data(FILE *out, FILE_MANAGER *manager) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (TFile, "%sEmitting section: data\n%s", DBAR, DBAR));
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Debugging info enabled, writing file-level data section\n"));
  fprintf(out, ".section data\n\n");
  for (UINT32 it = 1; it < manager->Tables()->Sym()->Length(); it++) {
    ST_IDX new_idx = (it << 8) + 1;
    if (ST_st(new_idx) != NULL && ST_st(new_idx)->sym_class == SYM_CLASS_VAR) {
      Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# # Variable ST_IDX = %d, name = %s \n",  new_idx, ST_name(new_idx)));
      fprintf(out, "%s: \n", ST_name(new_idx));
      fprintf(out, ".word 0\n");
    }
  }
  return 0;
}