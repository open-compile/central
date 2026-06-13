//
// Created by xc5 on 2020/6/8.
//
#include "basic.h"
#include "ir.h"
#include "symtab.h"
#include "file_util.h"
#include "ir_io.h"
#include "ir_serial.h"
#include <stdio.h>
#include <unistd.h>

void File_extension_test();
void Tree_test();
void IR_io_primitives_test();
void IR_io_section_test();
void IR_io_strtab_test();
void IR_io_irnode_topo_test();

INT32 main() {

  const char *name = "abc";
  // File()->Create_ir_file("some.ir");

  File();
  Is_Trace(TRUE,
           (TFile, "[Symtab] Print testing\n"));
  File_extension_test();

  // ============================================================
  // IR I/O testing  (Step 1-7)
  // ============================================================
  IR_io_primitives_test();
  IR_io_section_test();
  IR_io_strtab_test();
  IR_io_irnode_topo_test();

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
  STR_IDX anon_array = File()->Save_string("_my_int_array");

  std::vector<ARB *> bound_vec;
  // Creating int[30][50][70]
  ARB_IDX arb_idx =
    File()->Create_array_bound_const(30, MTYPE_size(MTYPE_I4), 3, ARB_FIRST_DIMEN);
  ARB_IDX mid = File()->Create_array_bound_const(50, MTYPE_size(MTYPE_I4), 2, 0);
  ARB_IDX last = File()->Create_array_bound_const(70, MTYPE_size(MTYPE_I4), 1, ARB_LAST_DIMEN);

  TY_IDX array_ty = File()->Create_array_ty(anon_array, TY_FLAG_INTERNAL,
                                            ty_i4, last);
  TY_IDX array_arr_ty = File()->Create_array_ty(anon_array, TY_FLAG_INTERNAL,
                                                array_ty, mid);
  TY_IDX basic_array_ty = File()->Create_array_ty(anon_array, TY_FLAG_INTERNAL,
                                                  array_arr_ty, arb_idx);

  ARB_IDX one_arb = TY_arb(basic_array_ty);
  AssertThat(one_arb == arb_idx, ("Incorrect saving"));
  AssertThat(ARB_flags(one_arb) & ARB_CONST_UBND, ("Incorrect arb flag, not const upper bound"));

  AssertThat(ARB_dimension(one_arb) == 3, ("Incorrect arb dimension"));
  AssertThat(ARB_ubnd_val(one_arb) == 30, ("Incorrect arb upper-bound val"));

  AssertThat(ARB_dimension(one_arb + 1) == 2, ("Incorrect arb dimension"));
  AssertThat(ARB_ubnd_val(one_arb + 1) == 50, ("Incorrect arb dimension"));

  AssertThat(ARB_dimension(one_arb + 2) == 1, ("Incorrect arb dimension"));
  AssertThat(ARB_ubnd_val(one_arb + 2) == 70, ("Incorrect arb dimension"));

  // Dummy access
  TY_ty(basic_array_ty);

  // ============================================================
  // Symbol testing
  // ============================================================
  STR_IDX str_idx = File()->Save_string("some_basic_var");
  STR_IDX str_foo2_idx = File()->Save_string("_fooo2");
  ST_IDX basic_sym = File()->Create_var(str_idx, ty_i4, GLOBAL_SYMTAB,
                                        SYMC_FILE_STATIC, SYME_PREEMPTIBLE,
                                        SYM_CLASS_VAR);


  ST_IDX func_sym = File()->Create_var(str_foo_idx, basic_func_ty,
                                       GLOBAL_SYMTAB, SYMC_EXTERN,
                                       SYME_PREEMPTIBLE,
                                       SYM_CLASS_FUNC);

  ST_IDX func2_sym = File()->Create_var(str_foo2_idx, basic_func_ty,
                                       GLOBAL_SYMTAB, SYMC_EXTERN,
                                       SYME_PREEMPTIBLE,
                                       SYM_CLASS_FUNC);

  // ============================================================
  // Function testing
  // ============================================================
  File()->Create_function(func_sym, basic_func_ty);
  File()->Create_function(func2_sym, basic_func_ty);
  Tree_test();
  File()->Print(TFile);
  Is_Trace(TRUE, (TFile, "All Testing Passed\n"));
}

void Tree_test() {
  IR_ITER iter = Tree()->Get_root();
  Tree()->Print_recursive(stderr);
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

// =============================================================================
//  IR I/O test cases  (incremental tests for ir_io.cxx / ir_serial.cxx)
// =============================================================================

static const char *IR_IO_TMP1 = "/tmp/_occ_ir_io_test1.bin";
static const char *IR_IO_TMP2 = "/tmp/_occ_ir_io_test2.bin";
static const char *IR_IO_TMP3 = "/tmp/_occ_ir_io_test3.bin";
static const char *IR_IO_TMP4 = "/tmp/_occ_ir_io_test4.bin";

// ----------------------------------------------------------------------------
// Step 1: 原语 round-trip
// ----------------------------------------------------------------------------
void IR_io_primitives_test() {
  Is_Trace(TRUE, (TFile, "[IR_IO] primitives_test BEGIN\n"));
  {
    IR_WRITER w(IR_IO_TMP1);
    AssertThat(w.Is_open(), ("Cannot open tmp file %s", IR_IO_TMP1));
    w.Write_u8(0x42);
    w.Write_u16(0xBEEF);
    w.Write_u32(0xDEADBEEF);
    w.Write_u64(0x0123456789ABCDEFULL);
    w.Write_i32(-12345);
    w.Write_i64(-987654321LL);
    w.Write_str("hello-occ");
    const char tail[] = {1,2,3,4,5,6};
    w.Write_blob(tail, sizeof(tail));
    w.Close();
  }
  {
    IR_READER r(IR_IO_TMP1);
    AssertThat(r.Is_open(), ("Cannot open tmp file %s", IR_IO_TMP1));
    AssertThat(r.Read_u8()  == 0x42,            ("u8 mismatch"));
    AssertThat(r.Read_u16() == 0xBEEF,          ("u16 mismatch"));
    AssertThat(r.Read_u32() == 0xDEADBEEF,      ("u32 mismatch"));
    AssertThat(r.Read_u64() == 0x0123456789ABCDEFULL, ("u64 mismatch"));
    AssertThat(r.Read_i32() == -12345,          ("i32 mismatch"));
    AssertThat(r.Read_i64() == -987654321LL,    ("i64 mismatch"));
    std::string s = r.Read_str();
    AssertThat(s == "hello-occ", ("str mismatch: '%s'", s.c_str()));
    char buf[6] = {0};
    r.Read_blob(buf, 6);
    AssertThat(buf[0]==1 && buf[5]==6, ("blob mismatch"));
    r.Close();
  }
  unlink(IR_IO_TMP1);
  Is_Trace(TRUE, (TFile, "[IR_IO] primitives_test PASS\n"));
}

// ----------------------------------------------------------------------------
// Step 2: Header + Section table
// ----------------------------------------------------------------------------
void IR_io_section_test() {
  Is_Trace(TRUE, (TFile, "[IR_IO] section_test BEGIN\n"));
  {
    IR_WRITER w(IR_IO_TMP2);
    w.Write_header();
    w.Begin_section(SK_STRTAB, 1);
    w.Write_strtab_buffer("AAA\0BBB\0", 8);
    w.End_section();
    w.Begin_section(SK_TY_TAB, 1);
    w.Write_u32(0xCAFEBABE);
    w.End_section();
    w.Finish();
    w.Close();
  }
  {
    IR_READER r(IR_IO_TMP2);
    r.Read_header();
    AssertThat(r.File_format_version() == IR_FILE_FORMAT_VERSION,
               ("file format version mismatch"));
    AssertThat(r.Has_section(SK_STRTAB), ("no STRTAB section"));
    AssertThat(r.Has_section(SK_TY_TAB), ("no TY_TAB section"));
    AssertThat(!r.Has_section(SK_INITO_GLOBAL), ("phantom INITO section"));

    AssertThat(r.Seek_section(SK_STRTAB), ("seek STRTAB failed"));
    char *buf = NULL; UINT64 used = 0;
    r.Read_strtab_buffer(&buf, &used);
    AssertThat(used == 8 && buf != NULL, ("strtab readback wrong"));
    AssertThat(memcmp(buf, "AAA\0BBB\0", 8) == 0, ("strtab content"));
    free(buf);

    AssertThat(r.Seek_section(SK_TY_TAB), ("seek TY_TAB failed"));
    AssertThat(r.Read_u32() == 0xCAFEBABE, ("TY_TAB content"));
    r.Close();
  }
  unlink(IR_IO_TMP2);
  Is_Trace(TRUE, (TFile, "[IR_IO] section_test PASS\n"));
}

// ----------------------------------------------------------------------------
// Step 3: STRTAB encode/decode (用 FILE_SYMTAB::Save_string + 复原 buffer)
// ----------------------------------------------------------------------------
void IR_io_strtab_test() {
  Is_Trace(TRUE, (TFile, "[IR_IO] strtab_test BEGIN\n"));
  // 模拟 strtab 数据 (空缓冲, 然后写一些)
  const char *strings[] = {
    "alpha", "beta", "gamma", "delta", "epsilon",
    "zeta", "eta", "theta", "iota", "kappa"
  };
  // 直接用 File()->Save_string 累积
  std::vector<STR_IDX> idxs;
  for (size_t i = 0; i < sizeof(strings)/sizeof(strings[0]); i++) {
    idxs.push_back(File()->Save_string(strings[i]));
  }
  // 现在 dump strtab 部分
  // 注: Save_string 把 buf 存在 FILE_SYMTAB::internal_str_tab_buffer
  // 这里直接通过 STR_str() 验证
  for (size_t i = 0; i < idxs.size(); i++) {
    const char *got = STR_str(idxs[i]);
    AssertThat(strcmp(got, strings[i]) == 0,
               ("strtab roundtrip[%zu] '%s' vs '%s'", i, got, strings[i]));
  }
  Is_Trace(TRUE, (TFile, "[IR_IO] strtab_test (in-memory) PASS\n"));
}

// ----------------------------------------------------------------------------
// Step 7: IRNODE encode + 小 TREE 拓扑 round-trip
// ----------------------------------------------------------------------------
void IR_io_irnode_topo_test() {
  Is_Trace(TRUE, (TFile, "[IR_IO] irnode_topo_test BEGIN\n"));

  // 构造手工 mini-tree: entry → block → return
  TREE *mini = new TREE();
  IRNODE_IDX entry = mini->Create_node(OPC_FUNC_ENTRY);
  IRNODE_IDX blk   = mini->Create_node(OPC_BLOCK);
  IRNODE_IDX ret   = mini->Create_node(OPC_RETURN);
  IR_ITER root_it = mini->Set_root(entry);
  IR_ITER blk_it  = mini->Add_child(root_it, blk);
  /* IR_ITER ret_it  = */ mini->Add_child(blk_it, ret);
  // 给 entry 节点放一个 sym idx 验证字段保存
  mini->Get_node(entry)->Set_symbol_idx(0xAABB);
  mini->Get_node(blk)->Set_const_val(0x1122334455667788ULL);

  // dump
  {
    IR_WRITER w(IR_IO_TMP4);
    w.Write_header();
    w.Begin_section(SK_FUNC_BLOB, 1);
    w.Begin_subimage();
    w.Begin_sub_section(SUBSK_IRNODES, SECVER_IRNODES);
    // 直接 encode 全部 _ir_elem_tab (3 个)
    UINT32 count = 3;
    w.Write_u32(count);
    for (UINT32 i = 0; i < count; i++) {
      Encode_IRNODE(w, *(mini->Get_node((IRNODE_IDX) i)));
    }
    w.End_sub_section();

    w.Begin_sub_section(SUBSK_TREE_TOPO, SECVER_TREE_TOPO);
    {
      IRTREE &irt = mini->Internal_tree();
      std::vector<IR_PRE_ITER> order;
      for (auto it = irt.begin(); it != irt.end(); ++it) order.push_back(it);
      UINT32 ec = (UINT32) order.size();
      w.Write_u32(ec);
      w.Write_u32((UINT32)(*order[0]));
      std::vector<void*> nps(ec, NULL);
      for (UINT32 i = 0; i < ec; i++) nps[i] = (void *) order[i].node;
      for (UINT32 i = 0; i < ec; i++) {
        auto pit = irt.parent(order[i]);
        UINT32 pp = 0xFFFFFFFFu;
        if (irt.is_valid(pit)) {
          void *pp_ptr = (void *) pit.node;
          for (UINT32 j = 0; j < i; j++) if (nps[j] == pp_ptr) { pp = j; break; }
        }
        w.Write_u32(pp);
        w.Write_u32((UINT32)(*order[i]));
      }
    }
    w.End_sub_section();

    w.End_subimage();
    w.End_section();
    w.Finish();
    w.Close();
  }

  // load
  {
    IR_READER r(IR_IO_TMP4);
    r.Read_header();
    AssertThat(r.Has_section(SK_FUNC_BLOB), ("no FUNC_BLOB"));
    UINT64 fb_off = r.Section_offset(SK_FUNC_BLOB);
    UINT64 fb_sz  = r.Section_size(SK_FUNC_BLOB);
    r.Enter_subimage(fb_off, fb_sz);
    AssertThat(r.Has_sub_section(SUBSK_IRNODES), ("no IRNODES"));
    AssertThat(r.Has_sub_section(SUBSK_TREE_TOPO), ("no TREE_TOPO"));

    TREE *t2 = new TREE();
    AssertThat(r.Seek_sub_section(SUBSK_IRNODES), ("seek IRNODES"));
    UINT32 cnt = r.Read_u32();
    AssertThat(cnt == 3, ("expected 3 irnodes, got %u", cnt));
    for (UINT32 i = 0; i < cnt; i++) {
      IRNODE_IDX idx = t2->Create_node();
      IRNODE *n = t2->Get_node(idx);
      AssertThat(Decode_IRNODE(r, *n, SECVER_IRNODES), ("decode irnode"));
    }
    AssertThat(t2->Get_node((IRNODE_IDX)0)->opcode == OPC_FUNC_ENTRY,
               ("opcode 0 mismatch"));
    AssertThat(t2->Get_node((IRNODE_IDX)0)->Get_symbol_idx() == 0xAABB,
               ("entry sym mismatch: %x", t2->Get_node((IRNODE_IDX)0)->Get_symbol_idx()));
    AssertThat(t2->Get_node((IRNODE_IDX)1)->opcode == OPC_BLOCK,
               ("opcode 1 mismatch"));
    AssertThat(t2->Get_node((IRNODE_IDX)1)->Get_const_val() == 0x1122334455667788ULL,
               ("block const mismatch"));
    AssertThat(t2->Get_node((IRNODE_IDX)2)->opcode == OPC_RETURN,
               ("opcode 2 mismatch"));

    AssertThat(r.Seek_sub_section(SUBSK_TREE_TOPO), ("seek TREE_TOPO"));
    UINT32 ec = r.Read_u32();
    AssertThat(ec == 3, ("topo edge_count %u", ec));
    UINT32 root_idx = r.Read_u32();
    AssertThat(root_idx == 0, ("root_idx %u", root_idx));
    IRTREE &irt = t2->Internal_tree();
    irt.clear();
    std::vector<IR_ITER> by_pos;
    for (UINT32 i = 0; i < ec; i++) {
      UINT32 pp = r.Read_u32();
      UINT32 ni = r.Read_u32();
      if (pp == 0xFFFFFFFFu) {
        IR_ITER it = irt.set_head((IRNODE_IDX) ni);
        by_pos.push_back(it);
      } else {
        IR_ITER pit = by_pos[pp];
        IR_ITER nit = irt.append_child(pit, (IRNODE_IDX) ni);
        by_pos.push_back(nit);
      }
    }
    AssertThat(irt.size() == 3, ("topo size %lu", irt.size()));
    // 验证拓扑: root=entry, child=block, child-of-block=return
    IR_ITER root2 = t2->Get_root();
    AssertThat(*root2 == 0, ("root *0"));
    AssertThat(t2->Number_of_children(root2) == 1, ("root children"));
    IR_ITER blk2 = irt.child(root2, 0);
    AssertThat(*blk2 == 1, ("blk2 *1"));
    AssertThat(t2->Number_of_children(blk2) == 1, ("blk2 children"));
    IR_ITER ret2 = irt.child(blk2, 0);
    AssertThat(*ret2 == 2, ("ret2 *2"));
    r.Leave_subimage();
    r.Close();
    delete t2;
  }

  delete mini;
  unlink(IR_IO_TMP4);
  Is_Trace(TRUE, (TFile, "[IR_IO] irnode_topo_test PASS\n"));
}
