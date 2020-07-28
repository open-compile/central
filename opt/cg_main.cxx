//
// Created by xc5 on 2020/7/24.
//
#include <file_util.h>
#include "options.h"
#include "cgir.h"
#include "cg_main.h"
#include "tn.h"

INLINE BOOL TR_EMIT() {
  return Tracing(CO_CG_EMIT, TRACE_EMIT_CORE);
}

// Single instance for the program to use, for now.
CGIR *_cgir_opt = nullptr;

CGIR *Cgir() {
  if (_cgir_opt == nullptr) {
    _cgir_opt = new CGIR();
  }
  return _cgir_opt;
}

/**
 * CG processing of one function
 * CG_Expand, Exapnsion,
 * @param file
 * @param config
 */
void CG_process_funcs(FILE_MANAGER *file, COMPILER_CONFIG &config) {
  // Convert OCIR to CGIR, saving the CGIR in file
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    // Iterate over each pu_info (functions), dump each of the function
    PU_INFO *pu_info = file->Tables()->Pu_info()->Get(it);
    if (pu_info->proc_sym != 0) {
      Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_OPTIONS),
               (TFile, "Converting function to CGIR for pu_info_id = %u\n", it));
      File()->Scopes()->Goto_function(pu_info->proc_sym);
      Cgir()->CG_Expand(&pu_info->scope); // Expansion
      Cgir()->Local_register_allocate(pu_info); // GRA/LRA
      if(Tracing(COMPONENT_CG, TRACE_DATA)) {
        // Printing the layout table.
        Cgir()->Layout()->Print(TFile);
      }
      if (Tracing(COMPONENT_CG_CONV, TRACE_DATA)) {
        // Printing the cgir exapnsion result.
        Cgir()->Print(pu_info->proc_sym, TFile);
      }
    } else {
      AssertThat(false, ("Incomoplete pu_infoo for PU_INFO_IDX = %u, or %0#x", it, it));
    }
  }
}

/**
 * CG Full processing, this is the only exported function to opt_main.cxx
 * @param conf
 * @return
 */
INT32 CG_full_process(COMPILER_CONFIG &conf) {

  // Local and Global register allocation
  // Instruction scheduling etc.,
  Is_Trace(Tracing(COMPONENT_CG, TRACE_OPTIONS),
           (TFile, "Writing assembly to %s\n", conf.output_file.c_str()));
  AssertThat(conf.output_file.size() > 0, ("Incorrect output file name"));
  if (File_exists(conf.output_file)) {
    // delete the file if it exists
    Is_Trace(Tracing(COMPONENT_CG, TRACE_OPTIONS),
             (TFile, "Removing old output file under %s\n", conf.output_file.c_str()));
    if (remove(conf.output_file.c_str()) != 0) {
      Comp_Failure("Cannot delete file : %s", conf.output_file.c_str());
    }
  }

  // This should only be run once.
  REGISTER_Begin();	/* initialize the register package */
  Init_Dedicated_TNs ();

  // Convert OCIR to CGIR
  CG_process_funcs(File(), conf);

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


/**
 * Emitting function, the @deprecated way
 * @deprecated
 * @param func
 * @param out
 * @param file
 */
void Emit_function(PU_INFO *func, FILE *out, FILE_MANAGER *file) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Emitting function ST_IDX = %d, name = %s \n",  func->proc_sym, ST_name(func->proc_sym)));
  file->Scopes()->Goto_function(func->proc_sym);
  // Inside the function now, emitting all symtab info
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Function has %d non-trivial symbols\n", file->Tables()->Sym()->Length(&(func->scope)) - 1));
  for (UINT32 i = 1; i < file->Tables()->Sym()->Length(&(func->scope)); i++) {
    ST_IDX one = (i << 8) | LOCAL_SYMTAB;
    fprintf(out, "# Id: 0x%08x, Symbol : %s, Type: %d \n", one, ST_name(one), ST_ty(one));
  }
  // Dump the instructions
  const char *func_name = ST_name(func->proc_sym);
  fprintf(out, ".global %s\n", func_name);
  fprintf(out, "%s: \n", func_name);
  // Letting Cgir to point to current function.
  Cgir()->Set_current_cgir(Cgir()->Get_function(func->proc_sym), func->proc_sym);
  Cgir()->Emit_tree(func, out, file);
  // TODO: Use CGIR's emission instead.
}

void CGIR::Emit_tree(PU_INFO *func, FILE *out, FILE_MANAGER *file) {
  IR_ITER it = tree->Get_root();
  AssertThat(tree->Get_node(it)->Opcode() == OPC_FUNC_ENTRY, ("Incorrect root opcode"));
  // Get the function body.
  map<ST_IDX, UINT32> temp_labels;
  CGIR *cgir = Cgir();
  // Generate all statements in the function-level body block
  UINT32 bb_cnt = cgir->Cfg()->Size();
  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cfg()->BB(i);

    // label to be set
    if (cgbb->Get_label_id() != 0) {
      Is_Trace(TR_EMIT(), (out, "#  ---  BB has a label: ---   \n"));
      Emit_label(func, out, cgbb->Get_label_id());
    }

    // Tracings
    if (TR_EMIT()) {
      fprintf(out, "#  --------- Processing BB : %d ---------  \n"
                   "#  --------- BB label = %d      ---------  \n"
                   "#  Pred: ", i, cgbb->Get_label_id());
      for (auto prd_id = cgbb->Pred_begin();
           prd_id != cgbb->Pred_end(); prd_id++) {
        fprintf(out, "%d ", (*prd_id)->Get_id());
      }
      fprintf(out, "\n#  Succ: ");
      for (auto prd_id = cgbb->Succ_begin();
           prd_id != cgbb->Succ_end(); prd_id++) {
        fprintf(out, "%d ", (*prd_id)->Get_id());
      }
      fprintf(out, "\n");
    }

    // function prologue
    // Get the flags, expat-adjust, function epilog
    if (cgbb->Get_flags() & BB_FLAG_ENTRY) {
      Is_Trace(TR_EMIT(), (out, "#  ---  function prologue ---   \n"));
      fprintf(out, "\tstr\tfp, [sp, #-4]!\n");
      fprintf(out, "\tadd\tfp, sp, #0\n");
      fprintf(out, "\tadd\tsp, sp, #%d\n", (-Layout()->Frame_final_size()));
    }
    // Get the flags, expat-adjust, function epilog
    if (cgbb->Get_flags() & BB_FLAG_EXIT) {
      // Finishing function
      Is_Trace(TR_EMIT(), (out, "#  ---  function epilog ---   \n"));
      fprintf(out, "\tadd\tsp, fp, #0\n");
      fprintf(out, "\tldr\tfp, [sp, #-4]\n");
    }
    Is_Trace(TR_EMIT(), (out, "#  -------- Begin Code ---------- \n"));
    // If there is a label to it, emit the label
    for (auto stmt_it = cgbb->First_stmt(); stmt_it != cgbb->Last_stmt(); stmt_it++) {
      CGOP *cgop = (*stmt_it);
      switch (cgop->getOpcode()) {
        default: {
          fprintf(out, "\t%s\t", Get_cg_opc_info(cgop->getOpcode())->ins_token);
          if (Get_cg_opc_info(cgop->getOpcode())->n_res >= 1) {
            Emit_operand(cgop, CGOPR_R, 0, out);
          }
          if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 1) {
            if (Get_cg_opc_info(cgop->getOpcode())->n_res >= 1) {
              // Printed operator before.
              fprintf(out, ", ");
            }
            if (CGOPC_is_ldst(cgop->getOpcode())) {
              fprintf(out, "[");
            }
            Emit_operand(cgop, CGOPR_R, 1, out);
          }
          if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 2) {
            fprintf(out, ", ");
            Emit_operand(cgop, CGOPR_R, 2, out);
          }
          if (CGOPC_is_ldst(cgop->getOpcode())) {
            fprintf(out, "]");
          }
          fprintf(out, "\n");
          break;
//          AssertThat(false,
//                     ("CG opcode = %d emission not implemented.",
//                       cgop->getOpcode()));
        }
      }
    }
  }
  // Dumping temp labels
  fprintf(out, "# Dumping temp labels : total = %lu \n", temp_labels.size());
  for (auto local_temp_it : temp_labels) {
    fprintf(out, ".TL%s_%u:\t.word %s\n", ST_name(func->proc_sym), local_temp_it.second, ST_name(local_temp_it.first));
  }
  LABEL_TABLE *tbl = File()->Tables()->Label();
  SCOPE *scope = File()->Scopes()->Current();
  for (UINT32 i = 1; i < tbl->Length(scope); i++) {
    LABEL_IDX lbl_idx = (i << 8) + LOCAL_SYMTAB;
    if (LABEL_label(lbl_idx)->Get_kind() != LKIND_RELOC) {
      continue;
    }
    const char *name = LABEL_name(lbl_idx);
    ST_IDX sym = LABEL_label(lbl_idx)->Get_temp_sym();
    AssertThat(sym != 0, ("There should be a valid global var to points to"));
    AssertThat(ST_sclass(sym) == SYMC_FILE_STATIC, ("This should be global-var"));
    fprintf(out, "%s:\t.word %s\n", name, ST_name(sym));
  }
}

INT32 Emit_section_data(FILE *out, FILE_MANAGER *manager) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (TFile, "%sEmitting section: data\n%s", DBAR, DBAR));
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Debugging info enabled, writing file-level data section\n"));
  fprintf(out, ".data\n\n");
  for (UINT32 it = 1; it < manager->Tables()->Sym()->Length(); it++) {
    ST_IDX new_idx = (it << 8) + 1;
    if (ST_st(new_idx) != NULL && ST_st(new_idx)->sym_class == SYM_CLASS_VAR) {
      Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# # Variable ST_IDX = 0x%08x, name = %s \n",  new_idx, ST_name(new_idx)));
      fprintf(out, "%s: \n", ST_name(new_idx));
      fprintf(out, ".word 0\n");
    }
  }
  return 0;
}