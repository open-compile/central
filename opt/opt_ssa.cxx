#include <vector>
#include <map>
#include "basic.h"
#include "host.h"
#include "options.h"
#include "be_export.h"
#include "file_util.h"
#include <fstream>
#include <ir.h>
#include "symtab.h"
#include "opt_ssa.h"
#include "opt_main.h"
#include "opt_basic.h"


// Single instance for the program to use, for now.
SSA_COMPOSITE *_ssair_opt = nullptr;
SSA_COMPOSITE *SSAMon() {
  if (_ssair_opt == nullptr) {
    _ssair_opt = new SSA_COMPOSITE();
  }
  return _ssair_opt;
}

class SSA_IR_GRAPH {
private:
  vector<IRNODE> _global_nodes; // currently store all global nodes in one vector. TODO: move it with each func.
public:
};

// Print SSANODE info, as in IRNODE_IDX
void SSANODE::Print(FILE *file) {
  SSAIR *ssair = SSAMon()->SSA_ir();
  // ....
}

void SSAIR::Print(FILE *file) {
  // Print by functions.
  map<ST_IDX, SSA_CFG *> &trees = this->Trees();
  fprintf(file, "%sPrinting the complete SSA-IR with %lu functions \n%s",
          DBAR, trees.size(), DBAR);
  for (auto iter : trees) {
    AssertThat(ST_st(iter.first) != nullptr, ("invalid ST_IDX in the ssa-ir map"));
    fprintf(file, "%sPrinting the SSAIR's function : name = %s, sym = 0x%08x\n%s",
            DBAR, ST_name(iter.first), iter.first, DBAR);
    iter.second->Print(file);
  }
}

void SSAIR::Print(ST_IDX sym, FILE *file) {
  // Print a functions detail.
  AssertThat(ST_st(sym) != nullptr, ("invalid function ST_IDX to print in SSA-IR"));
  fprintf(file, "%sPrinting the SSAIR's function with sym : name = %s, sym = 0x%08x\n%s",
          DBAR, ST_name(sym), sym, DBAR);
  Get_function(sym)->Print(file);
}


/**
 * Setting up SSA preparation,
 * @param cfg ssa module
 * @param sym Current function's PU symbol
 */
void SSAIR::Goto_function(ST_IDX sym) {
  if(_trees.find(sym) == _trees.end()) {
    _trees[sym] = new SSA_CFG();
  }
  if (_ssa_codemap.find(sym) == _ssa_codemap.end()) {
    _ssa_codemap[sym] = new CODE_STORE();
  }
  if (_ssa_symtab.find(sym) == _ssa_symtab.end()) {
    _ssa_symtab[sym] = new OPT_SYMTAB();
  }
  _current = _trees[sym];
  _current_sym = sym;
  _current_tree = PU_INFO_pu_info(File()->Tables()->Get_pu_info_by_st_idx(sym))->entry;
  _current_codemap = _ssa_codemap[sym];
  _current_symtab = _ssa_symtab[sym];
  AssertThat(_current_codemap != nullptr, ("Code store is not created properly for sym = %d...", sym));
  AssertThat(_current_symtab != nullptr, ("Opt Symtab is not created properly for sym = %d...", sym));
}


void SSA_COMPOSITE::Collect_defs_bb() {
  // collect defs BB, dominance frontiers. ....
}

void SSA_COMPOSITE::SSA_Rename() {
  // rename by ssa rules.
}

BOOL SSA_COMPOSITE::Verify_stack(OPT_SYMTAB *symtab) {
  return TRUE;
}



/**
 * Starting to convert function to SSA form.
 * @param scope
 * @param lvl
 */
void SSA_COMPOSITE::Construct_function(ST_IDX func_sym, SCOPE *scope, UINT32 ir_lvl) {
  Is_Trace(Tracing(COMPONENT_SSA_CONV, TRACE_INVOCATION),
           (TFile, "SSA_COMPOSITE::Construct_function\n"));
  File()->Scopes()->Goto_function(func_sym);
  AssertThat(func_sym != 0, ("Incorrect function symbol idx = 0x%08x", func_sym));
  SSA_ir()->Goto_function(func_sym);

  SSA_CFG    *cfg = SSA_ir()->Cfg(); // Cfg must be ready to use.
  OPT_SYMTAB *symtab = SSA_ir()->Current_symtab(); // Symbol table must be ready.
  CODE_STORE *codestore = SSA_ir()->Code_store(); // code store to convert codepod into

  // Creating function's entry chi.
  // symtab->Create_entry_chi();

  // For every variable, find out where it is defined.
  Collect_defs_bb();

  if (Tracing(COMPONENT_SSA_CONV, TRACE_DEBUG)) {
    Is_Trace(Tracing(COMPONENT_SSA_CONV, TRACE_DEBUG), (TFile, "OPT symtab after defs bb collection\n"));
    symtab->Print(TFile);
  }
  // For every BBs, setup the Phi_list.
  //  FOR_ALL_ELEM (bb, cfg_iter, Init(cfg))
  //    bb->Set_phi_list(CXX_NEW(PHI_LIST(bb), mem_pool));
  //
  SSA_Rename();

  // Verify the rename has completed
  AssertThat(Verify_stack(symtab), ("ssa: stack is non-empty"));

  // Do IR to SSA-IR conversion
  Is_Trace(Tracing(COMPONENT_SSA_CONV, TRACE_INVOCATION),
           (TFile, "Finishing IR to SSA construction...\n"));
}

void Opt_build_ssa(PU_INFO *pu, FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  SSAMon()->SSA_ir();
  // build SSA on a specific function.
  // Construct SSA
  Is_Trace(Tracing(COMPONENT_SSA_CONV, TRACE_DEBUG),
           (TFile, "Converting function to SSA-IR for pu_info_id = %u\n", pu->pu_idx));
  File()->Scopes()->Goto_function(pu->Proc_sym());
  SSAMon()->Construct_function(pu->Proc_sym(), &pu->scope, (UINT32) level); // Expansion
  if (Tracing(COMPONENT_SSA_CONV, TRACE_DATA)) {
    // Printing the cgir exapnsion result.
    SSAMon()->SSA_ir()->Print(pu->Proc_sym(), TFile);
  }
}

void Opt_destruct_ssa(PU_INFO *pu_info, FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  // Convert SSA form of a function to
  // Convert back to IR tree node....
}

void Opt_build_ssa_all(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  SSAMon()->Init();
  // Lower IR constructs
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    // Iterate over each pu_info (functions), dump each of the function
    PU_INFO *pu_info = file->Tables()->Pu_info()->Get(it);
    if (pu_info->proc_sym != 0) {
      Opt_build_ssa(pu_info, file, level, config);
    } else {
      AssertThat(false, ("Incomoplete pu_infoo for PU_INFO_IDX = %u, or %0#x", it, it));
    }
  }
}