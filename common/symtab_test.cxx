//
// Created by xc5 on 2020/6/8.
//
#include "basic.h"
#include "ir.h"
#include "symtab.h"
#include "file_util.h"

void File_extension_test();

INT32 main() {

  const char *name = "abc";
  // File()->Create_ir_file("some.ir");

  File();

  File_extension_test();

  // ============================================================
  // Type testing
  // ============================================================
  TY_IDX ty_i4 = MTYPE_to_ty(MTYPE_I4);
  Is_Trace(TRUE,
           (TFile, "[Type testing] ty_i4 = %0#x, (%d)\n", ty_i4, ty_i4 >> 8));
  TY *ty_i4_obj = TY_ty(ty_i4);
  ty_i4_obj->Print(TFile);
  TY_IDX ty_v = MTYPE_to_ty(MTYPE_V);
  TY_IDX random_ty = File()->Tables()->Ty()->Add();
  STR_IDX str_foo_idx = File()->Save_string("_foo");
  std::vector<TY_IDX> *param_ret_vec = new std::vector<TY_IDX>;
  param_ret_vec->push_back(ty_i4);
  param_ret_vec->push_back(ty_v);
  TY_IDX basic_func_ty = File()->Create_func_ty(str_foo_idx, 0, MTYPE_V,
                                                (TY_FLAG) 0, *param_ret_vec);
  TY_ty(basic_func_ty)->Print(TFile);

  // ============================================================
  // Symbol testing
  // ============================================================
  STR_IDX str_idx = File()->Save_string(name);
  ST_IDX basic_sym = File()->Create_var(str_idx, ty_i4, GLOBAL_SYMTAB,
                                        SYMC_EXTERN, SYME_EXTERNAL,
                                        SYM_CLASS_VAR);


  ST_IDX func_sym = File()->Create_var(str_foo_idx, basic_func_ty,
                                       GLOBAL_SYMTAB, SYMC_EXTERN,
                                       SYME_EXTERNAL,
                                       SYM_CLASS_FUNC);

  // ============================================================
  // Function testing
  // ============================================================
  File()->Create_function(func_sym, basic_func_ty);

  Is_Trace(TRUE, (TFile, "All Testing Passed\n"));
}

void File_extension_test() {

  // File Utility test
  std::string fn = "a.sy.c", prefn = "a.sy.c";
  File_change_extension(fn, "s");
  AssertThat(fn == "a.sy.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "a.s";
  File_change_extension(fn, "s");
  AssertThat(fn == "a.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "a.sas";
  File_change_extension(fn, "s");
  AssertThat(fn == "a.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "a.c";
  File_change_extension(fn, "s");
  AssertThat(fn == "a.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/n/b/a.c";
  File_change_extension(fn, "s");
  AssertThat(fn == "/n/b/a.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/n/b/a.syu";
  File_change_extension(fn, "s");
  AssertThat(fn == "/n/b/a.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/n/b/a.yu.sas";
  File_change_extension(fn, "s");
  AssertThat(fn == "/n/b/a.yu.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/n/b/a.syu.sas";
  File_change_extension(fn, "s");
  AssertThat(fn == "/n/b/a.syu.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/n/b/a.s";
  File_change_extension(fn, "s");
  AssertThat(fn == "/n/b/a.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/a.v/b/p";
  File_change_extension(fn, "s");
  AssertThat(fn == "/a.v/b/p.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/a.v/b.s/p";
  File_change_extension(fn, "s");
  AssertThat(fn == "/a.v/b.s/p.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/a.v/b.m/p";
  File_change_extension(fn, "s");
  AssertThat(fn == "/a.v/b.m/p.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));


  fn = prefn = "/a.v/b/pancbcsbscu19k202100+?>P>>P<O<P.,>:!*!@&.,.^#$.,.$($*(!)A/a";
  File_change_extension(fn, "s");
  AssertThat(fn ==
             "/a.v/b/pancbcsbscu19k202100+?>P>>P<O<P.,>:!*!@&.,.^#$.,.$($*(!)A/a.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/a.v/b/.git";
  File_change_extension(fn, "s");
  AssertThat(fn == "/a.v/b/.git.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/a.v/b/.git.n";
  File_change_extension(fn, "s");
  AssertThat(fn == "/a.v/b/.git.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/a.v/b/.git.snp";
  File_change_extension(fn, "s");
  AssertThat(fn == "/a.v/b/.git.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

  fn = prefn = "/a.v/b/.git.n.p";
  File_change_extension(fn, "s");
  AssertThat(fn == "/a.v/b/.git.n.s",
             ("Failed - expected: %s, real: %s", prefn.c_str(), fn.c_str()));

}
