//
// Created by xc5 on 2020/6/8.
//
#include "basic.h"
#include "ir.h"
#include "symtab.h"


INT32 main() {

  const char *name = "abc";
  // File()->Create_ir_file("some.ir");

  File();

  // ============================================================
  // Type testing
  // ============================================================
  TY_IDX ty_i4 = MTYPE_to_ty(MTYPE_I4);
  Is_Trace(TRUE, (TFile, "[Type testing] ty_i4 = %0#x, (%d)\n", ty_i4, ty_i4 >> 8));
  TY *ty_i4_obj = TY_ty(ty_i4);
  ty_i4_obj->Print(TFile);

  TY_IDX random_ty = File()->Tables()->Ty()->Add();
  TYLIST_IDX tylist_idx = File()->Tables()->Tylist()->Add();
  TYLIST *tylist = TYLIST_tylist(tylist_idx);
  tylist->ty_id = ty_i4;
  tylist_idx = File()->Tables()->Tylist()->Add();
  tylist = TYLIST_tylist(tylist_idx);
  tylist->ty_id = 0;

  STR_IDX str_foo_idx = File()->Save_string("_foo");
  TY_IDX basic_func_ty = File()->Create_func_ty(str_foo_idx, 0, MTYPE_V, (TY_FLAG) 0, ty_i4, tylist_idx);

  // ============================================================
  // Symbol testing
  // ============================================================
  STR_IDX str_idx = File()->Save_string(name);
  ST_IDX basic_sym = File()->Create_var(str_idx, ty_i4, GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL,
                                        SYM_CLASS_VAR);


  ST_IDX func_sym = File()->Create_var(str_foo_idx, basic_func_ty, GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL,
                                        SYM_CLASS_FUNC);

  // ============================================================
  // Function testing
  // ============================================================
  File()->Create_function(func_sym, basic_func_ty);

  Is_Trace(TRUE, (TFile, "All Testing Passed\n"));
}
