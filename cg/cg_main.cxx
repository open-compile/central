//
// Created by xc5 on 2020/7/24.
//
#include <file_util.h>
#include "options.h"
#include "cgir.h"
#include "cg_main.h"
#include "cg_pass.h"
#include "cg_composite.h"
#include "tn.h"
#include "timing.h"

INLINE BOOL TR_EMIT() {
  return Tracing(COMPONENT_CG_EMIT, TRACE_EMIT_CORE);
}

static void Trace_cg_timing(COMPILER_CONFIG &config, const char *stage,
                            const TIMING_SNAPSHOT &start) {
  if (config.timing) {
    Timing_trace_stage(TFile, stage, start, Timing_snapshot());
  }
}

// Single instance for the program to use, for now.
CG_COMPOSITE *_cgir_opt = nullptr;
CG_COMPOSITE *Cgmon() {
  if (_cgir_opt == nullptr) {
    _cgir_opt = new CG_COMPOSITE();
  }
  return _cgir_opt;
}


/**
 * CG processing of one function
 * CG_Expand, Exapnsion,
 * @param file
 * @param config
 */
void CG_process_funcs(FILE_MANAGER *file, COMPILER_CONFIG &config, FILE *outfile) {
  CGIR *main_cgir = Cgmon()->Cgir();
  
  CG_BUILD_PASS           build_cg_pass;
  CG_LIVE_RANGE_PASS      lra_pass;
  CG_REG_ALLOC_PASS       reg_alloc_pass;
  CG_FRAME_LAYOUT_PASS    layout_pass;
  CG_EMITTER_PASS         emit_pass;

  build_cg_pass.Init(Cgmon());
  lra_pass.Init(Cgmon());
  reg_alloc_pass.Init(Cgmon());
  layout_pass.Init(Cgmon());
  emit_pass.Init(Cgmon());

  // Setup emit output file.
  emit_pass.Set_outfile(outfile);

  // TODO(cg-pipeline): 当前是单 pass 串, 一次性走完 convert + alloc + layout + emit.
  //   真正的实现应该走 pass pipeline (见 cg.spec.md §1):
  //     Pass chain = [ Build → LiveRange → IFG → Color → SpillRewrite → FrameLayout → Emit ]
  //     每步可单独 Is_Trace dump; 可单独 disable / enable
  //   建议抽出一个 CG_PASS 抽象基类 + Run_passes 函数.
  // Convert OCIR to CGIR, saving the CGIR in file
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    // Iterate over each pu_info (functions), dump each of the function
    PU_INFO *pu_info = file->Tables()->Pu_info()->Get(it);
    if (pu_info->Proc_sym() == 0) {
      AssertThat(false, ("Incomoplete pu_info for PU_INFO_IDX = %u, or %0#x", it, it));
      return;
    }
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
    (TFile, "Converting function to CGIR for pu_info_id = %u\n", it));
    
    // 原逻辑：Cgmon()->CG_convert_function(&pu->scope)
    ST_IDX func_sym = pu_info->Proc_sym();
    File()->Scopes()->Goto_function(func_sym);
    AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
    
    // Setup current builder/emitter/lra/regalloc sutff.
    main_cgir->Start_function_TN();
    main_cgir->Get_cg_cfg(func_sym);
    CG_CFG    *function_cgir = main_cgir->Get_cg_cfg(func_sym);
    main_cgir->Set_current_cgir(function_cgir, func_sym);

    TIMING_SNAPSHOT cg_stage_start;

    // Build pass
    if (config.timing) cg_stage_start = Timing_snapshot();
    build_cg_pass.Run(pu_info);
    Trace_cg_timing(config, "cg-build", cg_stage_start);
    if (Tracing(COMPONENT_CG_CONV, TRACE_DATA)) {
      // Printing the cgir exapnsion result.
      main_cgir->Print(pu_info->proc_sym, TFile);
    }

    // TODO(pipeline-call): 真正 pipeline 应该分两步:
    //   1) LRA.Pre_alloc(): 调 Cgmon()->Lra().Analyze_live_range(pu_info)
    //                      建 IFG → 染色 → spill rewrite
    //   2) 调 Reg_alloc().Register_allocate(pu_info)  (只剩 layout)
    // 当前是直接调 Register_allocate, 内部走 naive linear scan.
    if (config.cg_cfg.enable_lra) {
      if (config.timing) cg_stage_start = Timing_snapshot();
      lra_pass.Run(pu_info);
      Trace_cg_timing(config, "lra", cg_stage_start);
      if (Tracing(COMPONENT_CG_LRA, TRACE_DATA)) {
        // Printing the lra results
        lra_pass.Print(TFile);
      }
      Cgmon()->Builder().Build_def_use();
    } else {
      Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_OPTIONS),
               (TFile, "Skipping CG LRA pass\n"));
    }

    if (config.cg_cfg.enable_regalloc) {
      if (config.timing) cg_stage_start = Timing_snapshot();
      reg_alloc_pass.Run(pu_info);
      Trace_cg_timing(config, "regalloc", cg_stage_start);
      Cgmon()->Builder().Build_def_use();
      if (Tracing(COMPONENT_CG_REGALLOC, TRACE_DATA)) {
        // Printing the lra results
        reg_alloc_pass.Print(TFile);
      }
    } else {
      Is_Trace(Tracing(COMPONENT_CG_REGALLOC, TRACE_OPTIONS),
               (TFile, "Skipping CG register allocation pass\n"));
    }
    
    // Frame Layout
    if (config.timing) cg_stage_start = Timing_snapshot();
    layout_pass.Run(pu_info);
    Trace_cg_timing(config, "layout", cg_stage_start);
    Cgmon()->Builder().Build_def_use();
    if(Tracing(COMPONENT_CG_LAYOUT, TRACE_DATA)) {
      // Printing the layout table.
      layout_pass.Print(TFile);
    }
    if (Tracing(COMPONENT_CG_CONV, TRACE_DATA)) {
      // Printing the cgir exapnsion result.
      main_cgir->Print(pu_info->proc_sym, TFile);
    }

    if (config.cg_cfg.dump_cfg_graph) {
      main_cgir->Print_cfg_graph(pu_info->Proc_sym(), TFile);
    }
    if (config.cg_cfg.dump_cfg) {
      main_cgir->Print_cfg_detail(pu_info->Proc_sym(), TFile);
    }
    if (config.cg_cfg.dump_tn) {
      main_cgir->Print_tn_table(pu_info->Proc_sym(), TFile, TRUE);
    }

    // Emit pass
    if (config.timing) cg_stage_start = Timing_snapshot();
    emit_pass.Run(pu_info);
    Trace_cg_timing(config, "emit", cg_stage_start);

    // Cleanup
    main_cgir->Cleanup_function_TN();
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
  Cgmon()->Init(); // Creating CG_COMPOSITE, initializing CGIR, EMITTER, BUILDER, REG_ALLOC ....
  REGISTER_Begin();	/* initialize the register package */
  Init_Dedicated_TNs ();

  // Run emitting of assembly code.
  FILE *output_assembly_file = fopen(conf.output_file.c_str(), "w+");
  if (!output_assembly_file) {
    Comp_Failure("Cannot open output file to write = %s",
                 conf.output_file.c_str());
  }
  // Emit starting prologue
  Emit_section_code_prologue(output_assembly_file, File());

  // Do CG on PUs, pu by pu.
  CG_process_funcs(File(), conf, output_assembly_file);

  // After all PU done, dump the data section and epilogues.
  Emit_section_code_epilogue(output_assembly_file, File());
  
  // Emit data section
  Emit_section_data(output_assembly_file, File());

  if (fclose(output_assembly_file) != 0) {
    Comp_Failure("Cannot close output file to write = %s",
                 conf.output_file.c_str());
  }
  return 0;
}



void Emit_section_code_prologue(FILE *out, FILE_MANAGER *file) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (TFile, "%sEmitting section: code prologue\n%s", DBAR, DBAR));
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# Debugging info enabled, writing file-level code section\n"));
  fprintf(out, ".text\n\n");
  fprintf(out, ".global __aeabi_idiv \n");
}

void Emit_section_code_epilogue(FILE *out, FILE_MANAGER *file) {
  Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# End of all functions, code .... \n"));
}

void CG_EMITTER::Emit_tree(ST_IDX func_sym, FILE *out, FILE_MANAGER *file) {
  IR_ITER it = Tree()->Get_root();
  AssertThat(Tree()->Get_node(it)->Opcode() == OPC_FUNC_ENTRY, ("Incorrect root opcode"));
  // Get the function body.
  map<ST_IDX, UINT32> temp_labels;
  CGIR *cgir = Cgir();
  // Generate all statements in the function-level body block
  UINT32 bb_cnt = cgir->Cfg()->Size();
  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cgir()->Cfg()->BB(i);
    // Tracings
    if (TR_EMIT()) {
      // Printing some BB level basic info before each basic block.
      cgbb->Print_basic(out);
    }
    // label to be set
    if (cgbb->Get_label_id() != 0) {
      Emit_label(out, cgbb->Get_label_id());
    }
    const int FUNC_PUSH_SIZE = 28 + 8;
    const int SP_EXTRA = FUNC_PUSH_SIZE - 4;
    // function prologue
    // Get the flags, expat-adjust, function epilog
    if (cgbb->Get_flags() & BB_FLAG_ENTRY) {
      Is_Trace(TR_EMIT(), (out, "#  ---  function prologue ---   \n"));
      if (Cgir()->Layout()->Get_local_pad_size() > (1 << 8)) {
        // This is a large stack.
        fprintf(out, "\tpush\t{fp, lr}\n" // 8bytes
                     "\tpush\t{r4-r10}\n"); // 28bytes

        fprintf(out, "\tmov\tr4, #%d\n", ((Layout()->Get_local_pad_size()) & 0xFFFF));
        if (((Layout()->Get_local_pad_size()  >> 16) & 0xFFFF) != 0 ) {
          fprintf(out, "\tmovt\tr4, #%d\n",
                  ((Layout()->Get_local_pad_size() >> 16) & 0xFFFF));
        }
        fprintf(out, "\tadd\tfp, sp, #%d\n"
                     "\tsub\tsp, sp, r4\n", SP_EXTRA);
      } else {
        fprintf(out, "\tpush\t{fp, lr}\n" // 8bytes
                     "\tpush\t{r4-r10}\n" // 28bytes
                     "\tadd\tfp, sp, #%d\n"
                     "\tsub\tsp, sp, #%d\n",
                SP_EXTRA,
                (Layout()->Get_local_pad_size()));
      }
    }
    // Get the flags, expat-adjust, function epilog
    if (cgbb->Get_flags() & BB_FLAG_EXIT) {
      // Finishing function
      Is_Trace(TR_EMIT(), (out, "#  ---  function epilog ---   \n"));
      fprintf(out, "\tsub\tsp, fp, #%d\n", SP_EXTRA);
      fprintf(out, "\tpop\t{r4-r10}\n"
                   "\tpop\t{fp, pc}\n");
    }
    Is_Trace(TR_EMIT(), (TFile, "Emit_code: Begin real stmt in BB(%d)\n", i));
    // If there is a label to it, emit the label
    for (auto stmt_it = cgbb->Begin_stmt(); stmt_it != cgbb->End_stmt(); stmt_it++) {
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
        }
      }
    }
  }
  // Dumping temp labels
  fprintf(out, "# Dumping temp labels : total = %lu \n", temp_labels.size());
  for (auto local_temp_it : temp_labels) {
    fprintf(out, ".TL%s_%u:\t.word %s\n", ST_name(func_sym), local_temp_it.second, ST_name(local_temp_it.first));
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
    ST_IDX new_idx = (it << 8) + GLOBAL_SYMTAB;
    if (ST_st(new_idx) != NULL && ST_st(new_idx)->sym_class == SYM_CLASS_VAR) {
      AssertThat(ST_sclass(new_idx) == SYMC_FILE_STATIC, ("These globals should be file-static. while %s isn't.", ST_name(new_idx)));
      Is_Trace(Tracing(COMPONENT_CG, TRACE_INFO), (out, "# # Variable ST_IDX = 0x%08x, name = %s \n",  new_idx, ST_name(new_idx)));
      fprintf(out, "%s: \n", ST_name(new_idx));
      TY_IDX ty = ST_ty(new_idx);
      AssertThat(ty != 0, ("Cannot find type of the symbol %s", ST_name(new_idx)));
      AssertThat(TY_size(ty) != 0,
        ("Cannot initialize a symbol that has "
         "incomplete type, sym = <%s, or 0x%08x>", ST_name(new_idx), new_idx));
      UINT32 size = TY_size(ty);
      UINT32 filled_size = 0;
      // Find INITO matching this.
      INITO_IDX inito_idx = ST_st(new_idx)->getInitoIdx();
      // Generate initv
      INITO *inito = INITO_inito(inito_idx);
      for (UINT32 i = 0; i < inito->Size(); i++) {
        if (inito->Value(i)->kind == INITVKIND_VAL) {
          fprintf(out, ".word %lld\n", inito->Value(i)->Val());
          filled_size += 4;
        } else if (inito->Value(i)->kind == INITVKIND_PAD) {
          fprintf(out, ".zero %lld\n", inito->Value(i)->Val());
          filled_size += inito->Value(i)->Val();
        }
      }
      if (size > filled_size) {
        fprintf(out, ".zero %u\n", size - filled_size);
      }
    }
  }
  return 0;
}
