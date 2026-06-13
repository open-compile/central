#include "symtab.h"
#include "tree.h"

// External use
static FILE_MANAGER *_current_file_manager = NULL;

// Use of symbols
// ST_name(st_idx);
// ST_ty(st_idx);
// TY_name(ty_idx);

/******************************************************************************
 * MTYPE access
 ******************************************************************************/
MTYPE_TAB MTYPE_to_TY_table[MTYPE_COUNT] = {
  {  MTYPE_UNKNOWN, KIND_SCALAR, TY_FLAG_INTERNAL, 0, "MTYPE_UNKNOWN", 0 },
  {  MTYPE_B,       KIND_SCALAR, TY_FLAG_INTERNAL, 1, "MTYPE_B",       0 },
  {  MTYPE_I1,      KIND_SCALAR, TY_FLAG_INTERNAL, 1, "MTYPE_I1",      0 },
  {  MTYPE_I2,      KIND_SCALAR, TY_FLAG_INTERNAL, 2, "MTYPE_I2",      0 },
  {  MTYPE_I4,      KIND_SCALAR, TY_FLAG_INTERNAL, 4, "MTYPE_I4",      0 },
  {  MTYPE_I8,      KIND_SCALAR, TY_FLAG_INTERNAL, 8, "MTYPE_I8",      0 },
  {  MTYPE_U1,      KIND_SCALAR, TY_FLAG_INTERNAL, 1, "MTYPE_U1",      0 },
  {  MTYPE_U2,      KIND_SCALAR, TY_FLAG_INTERNAL, 2, "MTYPE_U2",      0 },
  {  MTYPE_U4,      KIND_SCALAR, TY_FLAG_INTERNAL, 4, "MTYPE_U4",      0 },
  {  MTYPE_U8,      KIND_SCALAR, TY_FLAG_INTERNAL, 8, "MTYPE_U8",      0 },
  {  MTYPE_F4,      KIND_SCALAR, TY_FLAG_INTERNAL, 4, "MTYPE_F4",      0 },
  {  MTYPE_F8,      KIND_SCALAR, TY_FLAG_INTERNAL, 8, "MTYPE_F8",      0 },
  {  MTYPE_M,       KIND_STRUCT, TY_FLAG_INTERNAL, 0, "MTYPE_M",       0 },
  {  MTYPE_V,       KIND_VOID,   TY_FLAG_INTERNAL, 0, "MTYPE_V",       0 },
  {  MTYPE_A4,      KIND_SCALAR, TY_FLAG_INTERNAL, 4, "MTYPE_A4",      0 },
  {  MTYPE_A8,      KIND_SCALAR, TY_FLAG_INTERNAL, 8, "MTYPE_A8",      0 },
  {  MTYPE_BS,      KIND_SCALAR, TY_FLAG_INTERNAL, 0, "MTYPE_BS",      0 },
};

const char * MTYPE_name(MTYPE_ID mtype) {
    AssertThat(mtype >= 0 && mtype < MTYPE_COUNT, ("MTYPE id not valid : %d", mtype));
    AssertThat(MTYPE_to_TY_table[mtype].name != NULL, ("name is corrupted"));
  return MTYPE_to_TY_table[mtype].name;
}

UINT64 MTYPE_size(MTYPE_ID mtype) {
    AssertThat(mtype >= 0 && mtype < MTYPE_COUNT, ("MTYPE id not valid : %d", mtype));
    AssertThat(MTYPE_to_TY_table[mtype].size < 128, ("size is corrupted"));
  return MTYPE_to_TY_table[mtype].size;
}

TY_IDX MTYPE_to_ty(MTYPE_ID mtype) {
    AssertThat(mtype >= 0 && mtype < MTYPE_COUNT, ("MTYPE id not valid : %d", mtype));
    AssertThat(MTYPE_to_TY_table[mtype].ty_idx > 0, ("size is corrupted"));
  return MTYPE_to_TY_table[mtype].ty_idx;
};

TY_IDX MTYPE_kind(MTYPE_ID mtype) {
  AssertThat(mtype >= 0 && mtype < MTYPE_COUNT, ("MTYPE id not valid : %d", mtype));
  AssertThat(MTYPE_to_TY_table[mtype].kind != KIND_INVALID, ("mtype kind is corrupted"));
  return MTYPE_to_TY_table[mtype].kind;
};

/******************************************************************************
 * File Symtab
 ******************************************************************************/
void FILE_SYMTAB::Print(FILE *f) {
  fprintf(f, "\n%sProgram Units\n%s", DBAR, DBAR);
  this->Print_functions(f);
  // Global Print
  fprintf(f, "\n%sFile-level Symbol Table\n%s", DBAR, DBAR);
  this->Sym()->Print(f);
  fprintf(f, "\n%sFile-level Type Table\n%s", DBAR, DBAR);
  this->Ty()->Print(f);
  fprintf(f, "\n%sFile-level Tylist Table\n%s", DBAR, DBAR);
  this->Tylist()->Print(f);
  fprintf(f, "\n%sFile-level Array Bound Table\n%s", DBAR, DBAR);
  this->Arb()->Print(f);
  fprintf(f, "\n%sFile-level Program Unit Table\n%s", DBAR, DBAR);
  this->Pu()->Print(f);
  fprintf(f, "\n%sFile-level Program Unit Info Table\n%s", DBAR, DBAR);
  this->Pu_info()->Print(f);
  fprintf(f, "\n%sFile-level Inito Table\n%s", DBAR, DBAR);
  this->Inito()->Print(f);
  fprintf(f, "\n%sEnd of file level symtabs\n%s", DBAR, DBAR);

  //  Is_Trace(Tracing(COMPONENT_BE, TRACE_INVOCATION),
  //           ("PU st_idx: %d\n", ));
}

PU_INFO_IDX FILE_SYMTAB::Get_pu_info_by_st_idx(ST_IDX func) {
  return Sym()->Get(func)->Pu_info_idx();
}

void FILE_SYMTAB::Initialize() {
  // Add internal types from MTYPE_to_TY_table
  UINT32 total_size = sizeof(MTYPE_to_TY_table);
  UINT32 single_size = sizeof(MTYPE_to_TY_table[0]);
  UINT16 count = total_size / single_size;
  AssertThat(MTYPE_COUNT == count, ("MType table is incomplete or incorrect"));
  for (UINT16 cnt = 0; cnt < count; cnt ++) {
    STR_IDX str_idx = Save_string(MTYPE_to_TY_table[cnt].name);
    TY_IDX ty_idx = Ty()->Add();
    TY *ty = TY_ty(ty_idx);
    ty->name_idx = str_idx;
    ty->mtype = (MTYPE_ID) MTYPE_to_TY_table[cnt].mtype;
    ty->flags = MTYPE_to_TY_table[cnt].flag;
    ty->size = MTYPE_to_TY_table[cnt].size;
    ty->kind = MTYPE_to_TY_table[cnt].kind;
    MTYPE_to_TY_table[cnt].ty_idx = ty_idx;
  }
}

STR_IDX FILE_SYMTAB::Save_string(const char *string) {
  AssertThat(string != NULL, ("String should not be null"));
  if (internal_str_tab_buffer == NULL) {
    internal_str_tab_buffer = (char *) malloc(STR_TABLE_BLOCK_SIZE);
    AssertThat(internal_str_tab_buffer != NULL, ("Malloc failed"));
    allocated_size_of_buffer = STR_TABLE_BLOCK_SIZE;
    used_size_of_buffer = 1; // SKIPPING FIRST BYTE
  }
  UINT64 needed = strlen(string) + 1;
  if (used_size_of_buffer + needed < allocated_size_of_buffer) {
    allocated_size_of_buffer = used_size_of_buffer + needed + STR_TABLE_BLOCK_SIZE;
    internal_str_tab_buffer = (char *) realloc(internal_str_tab_buffer,
                                               allocated_size_of_buffer);
    AssertThat(internal_str_tab_buffer != NULL, ("Realloc failed"));
  }
  AssertThat(internal_str_tab_buffer != NULL, ("buffer can't be null"));
  AssertThat(used_size_of_buffer + needed < allocated_size_of_buffer, ("not enough space in the buffer"));
  memcpy((void *) (internal_str_tab_buffer + used_size_of_buffer), string, needed);
  STR_IDX str_idx = used_size_of_buffer;
  used_size_of_buffer += needed;
  return str_idx;
}

const char *FILE_SYMTAB::Get_string(STR_IDX idx) {
  AssertThat(idx > 0 && idx < allocated_size_of_buffer,
             ("Invalid string idx = %d, allocated = %d", idx, allocated_size_of_buffer));
  return internal_str_tab_buffer + idx;
}

// 用一段已存在的 buffer 替换当前 strtab; 接管所有权 (free 旧的)
void FILE_SYMTAB::Strtab_replace(char *new_buf, UINT64 new_used) {
  if (internal_str_tab_buffer != NULL) free(internal_str_tab_buffer);
  // 留出一个 block 的余量, 让后续 Save_string 有空间继续追加
  // (Save_string 仅在 used + needed < allocated 时 realloc; 若 allocated == used
  //  下次 Save_string 直接命中 line 125 的边界 assert)
  UINT64 alloc = new_used + STR_TABLE_BLOCK_SIZE;
  char *grown = (char *) malloc(alloc);
  AssertThat(grown != NULL, ("Strtab_replace: malloc failed"));
  if (new_used > 0 && new_buf != NULL) memcpy(grown, new_buf, new_used);
  if (new_buf != NULL) free(new_buf);
  internal_str_tab_buffer  = grown;
  used_size_of_buffer      = new_used;
  allocated_size_of_buffer = alloc;
}

void FILE_SYMTAB::Print_functions(FILE *f) {
  UINT32 cursor = 0;
  UINT32 total = Pu_info()->Length();
  SCOPE *old_current = File()->Scopes()->Current();
  for (cursor = 1; cursor < total; cursor++) {
    PU_INFO *pu_info = Pu_info()->Get(cursor);
    if (pu_info->proc_sym > 0) {
      File()->Scopes()->Goto_function(pu_info->proc_sym);
    }
    const char *func_name = pu_info->proc_sym > 0
                          ? ST_name(pu_info->proc_sym)
                          : "(incomplete function)";
    // Header — top-level per-function, gets the same DBAR treatment as the
    // file-level sections above.
    fprintf(f, "\n%sFunction %u: %s\n%s", DBAR, (unsigned) cursor, func_name, DBAR);
    // Sub-section labels are short inline tags — no banner per sub-section,
    // just blank-line separation. Keeps the per-function block clearly
    // delimited without stacking '=' bars on every nested label.
    fprintf(f, "\n  -- Tree --\n");
    if (pu_info->entry == NULL) {
      fprintf(f, " (NULL) \n");
    } else {
      pu_info->entry->Print_recursive(f);
    }
    fprintf(f, "\n  -- Symtab --\n");
    if (pu_info->scope.st_tab != NULL) {
      Sym()->Print(f, &(pu_info->scope));
    }
    fprintf(f, "\n  -- Labels --\n");
    if (pu_info->scope.label_tab != NULL) {
      Label()->Print(f, &(pu_info->scope));
    }
    fprintf(f, "\n  -- Preg --\n");
    if (pu_info->scope.preg_tab != NULL) {
      Preg()->Print(f, &(pu_info->scope));
    }
    fprintf(f, "\n  -- INITO --\n");
    if (pu_info->scope.inito_tab != NULL) {
      Inito()->Print(f, &(pu_info->scope));
    }
  }
  if (old_current && old_current->st_idx != 0) {
    File()->Scopes()->Goto_function(old_current->st_idx);
  }
}


/******************************************************************************
 * PU Related Utility
 ******************************************************************************/
PU *PU_INFO_Pu(PU_INFO_IDX pu_inf_idx) {
  ST_IDX proc_sym = File()->Tables()->Pu_info()->Get(pu_inf_idx)->proc_sym;
  AssertThat(proc_sym > 0, ("proc_sym should be > 0"));
  PU_IDX pu_idx = ST_pu(proc_sym);
  AssertThat(pu_idx > 0, ("pu_idx should be > 0"));
  return PU_pu(pu_idx);
}

void PU_INFO::Print_function_verbose(FILE *f) {
  // PU_INFO table slot 0 is a placeholder reserved by the GROWING_TABLE so that
  // valid PU_INFO_IDXes start at 1 (matching how the rest of the symtab treats
  // idx 0 as "null"). Print a one-line summary; this routine is called once
  // per slot under the "Dumping scope manager" banner, so a heavy bar-wrapped
  // banner here just stacks separators.
  if (pu_info_idx == 0 && pu_idx == 0 && proc_sym == 0) {
    fprintf(f, "  [%-4d] (Function_idx_0_placeholder)\n", 0);
    return;
  }
  AssertThat(pu_idx > 0 && pu_info_idx > 0 && proc_sym > 0,
             ("Incomplete function: pu_idx=%u pu_info_idx=%u proc_sym=0x%x",
              (unsigned) pu_idx, (unsigned) pu_info_idx, (unsigned) proc_sym));
  // Per-function scope tables and IR tree are printed in detail by
  // FILE_SYMTAB::Print_functions later in the same dump; we only emit a
  // summary line here.
  fprintf(f, "  [%-4d] Function %s (pu_idx=%u, proc_sym=0x%x)\n",
          (int) pu_info_idx,
          (proc_sym > 0) ? ST_name(proc_sym) : "(anon)",
          (unsigned) pu_idx, (unsigned) proc_sym);
}

FILE_INFO::FILE_INFO() {
  memset(this, 0,sizeof(FILE_INFO));
}

/******************************************************************************
 * Scope Manager
 ******************************************************************************/
/*
 * Get the current function
 */
SCOPE *SCOPE_MANAGER::Current() {
  return _current_function;
}

/**
 * Goto a specific function
 * @param func
 * @return
 */
BOOL SCOPE_MANAGER::Goto_function(ST_IDX func) {
  FILE_SYMTAB *tables = _file_manager->Tables();
  PU_INFO_IDX pu_info_idx = tables->Get_pu_info_by_st_idx(func);
  PU_INFO *pu = tables->Pu_info()->Get(pu_info_idx);
  // Setting up SCOPE*
  _current_function = &(pu->scope);
  // Setting up TREE*
  AssertThat(pu->entry != NULL, ("PU's entry is null, improper init"));
  Set_current_tree(pu->entry);
  return TRUE;
}

/**
 * Invoke for finishing up the creation of a function
 * @param func
 * @param func_info
 */
void SCOPE_MANAGER::Finish_function(ST_IDX func, PU_INFO_IDX func_info) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INVOCATION),
           (TFile, "Completing function = %s", ST_name(func)));
}

void SCOPE_MANAGER::Print(FILE *f) {
  fprintf(f, "\n%sScope Manager\n%s", DBAR, DBAR);
}


/******************************************************************************
 * TY structure (Type system)
 ******************************************************************************/
static void
Print_type_attributes (FILE *f, TY_IDX ty)
{
  if (ty <= 0)
    return;
  if (TY_is_const (ty))
    fprintf(f, "const ");
} // Print_type_attributes

static void
Print_TY_IDX_verbose (FILE *f, TY_IDX idx)
{
  Print_type_attributes (f, idx);
  if (idx > 0 && TY_ty (idx) == NULL) {
    fputs ("<NULL>", f);
    return;
  }
  const char *name = TY_name (idx) == NULL ? "(null-ty)" : TY_name (idx);
  fprintf (f, "%s (#%d) align %d", name, idx, TY_align(idx));
} // Print_TY_IDX_verbose

/**
 * Printing the content in the TY entry
 * @param f
 */
void TY::Print(FILE *f) const {
  AssertThat(f != NULL, ("Printing to null file"));
  fprintf (f, "%-14s:",
           this->name_idx ? STR_str(this->name_idx) : "(anon)");
  fprintf (f, " (f: 0x%04x", flags);
  fprintf (f, ")");
  fprintf (f, " size %lld %s: ",
           size, (mtype != 0) ? MTYPE_name (mtype) : "");

  switch (kind) {
    case KIND_SCALAR:
      fprintf(f, "SCALAR (%s)", MTYPE_name(mtype));
      break;

    case KIND_ARRAY:
      fputs("ARRAY of ", f);
      Print_TY_IDX_verbose(f, Etype());
      if (Arb() != 0) {
        ARB_IDX arb_idx = Arb();
        ARB *arb_info = ARB_arb(arb_idx);
        INT i, ndim;
        ndim = ARB_dimension(arb_idx);
        for (i = 0; i < ndim; i++) {
          fputs(" (", f);
          ARB_arb(arb_idx + i)->Print(f);
          fputc(')', f);
          // possible early exit for broken tables
          if (ARB_last_dimen(arb_idx + i)) break;
        }
      }
      fputc('\n', f);
      break;
    case KIND_FUNCTION:
      fprintf (f, "FUNCTION (f: 0x%04x)\n", Pu_flags());
      {
        TYLIST_IDX idx = Tylist();
        AssertThat(idx > 0, ("Tylist-idx should not be zero"));
        fprintf (f, "\treturns ");
        TYLIST *tylist_start = TYLIST_tylist(idx);
        if (tylist_start->ty_id != 0) {
          Print_TY_IDX_verbose (f, tylist_start->ty_id);
        }
        if (tylist_start->ty_id <= 0 || TY_ty(tylist_start->ty_id) == NULL) {
          fputc ('\n', f);
          break;
        }
        ++idx;
        while (TYLIST_tylist(idx) != NULL && TYLIST_tylist(idx)->ty_id != 0) {
          fputs ("\n\tparameter ", f);
          if (TYLIST_tylist(idx)->ty_id > 0) {
            Print_TY_IDX_verbose(f, TYLIST_tylist(idx)->ty_id);
          }
          ++idx;
        }
        fputc ('\n', f);
      }
      break;
    default:
      fprintf (f, "other");
      break;
  };
  fputc ('\n', f);
}


/******************************************************************************
 * File Manager
 ******************************************************************************/
FILE_MANAGER *File() {
  if (!_current_file_manager) {
    _current_file_manager = new FILE_MANAGER();
    _current_file_manager->Initialize();
  }
    AssertThat(_current_file_manager != NULL, ("Cannot find current file manager"));
  return _current_file_manager;
}

const char *STR_str(STR_IDX idx) {
  return File()->Tables()->Get_string(idx);
}

void FILE_MANAGER::Initialize() {
  Tables()->Initialize();
}

PU_INFO_IDX FILE_MANAGER::Create_function(ST_IDX func, TY_IDX prototype) {
  PU_IDX pu_idx = File()->Tables()->Pu()->Add();
  PU_INFO_IDX pu_info_idx = File()->Tables()->Pu_info()->Add();
  // Init pu
  PU *pu = PU_pu(pu_idx);
  pu->prototype = prototype;
  pu->pu_info_idx = pu_info_idx;
  // Init pu_info
  PU_INFO *pu_info = PU_INFO_pu_info(pu_info_idx);
  pu_info->Set_proc_sym(func);
  pu_info->pu_idx       = pu_idx;
  pu_info->pu_info_idx  = pu_info_idx;   // self-back-pointer; needed by
                                         // Print_function_verbose() and any
                                         // future consumer that walks PU_INFO
                                         // independently of the table index.
  // Init st_pu
  ST *st = ST_st(func);
  st->pu = pu_idx;
  st->type = prototype;
  // Init tree
  pu_info->entry = new TREE();
  pu_info->entry->Initialize();
  pu_info->scope.Init(func);
  // Initialize the scope structure, and put related info into it.
  File()->Scopes()->Goto_function(func);
  return pu_info_idx;
}

void FILE_MANAGER::Finish_creating_function(ST_IDX func) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO), (TFile, "Finishing creating func = %s", ST_name(func)));
}

void FILE_MANAGER::Open_ir_file(const char *file_name) {
  // 等价于 Load_ir_file: 从二进制 IR 文件恢复整个 FILE_MANAGER 状态.
  extern void Load_ir_file(FILE_MANAGER *, const char *);
  Load_ir_file(this, file_name);
}

void FILE_MANAGER::Create_ir_file(const char *file_name) {
  // 这个函数原本设计是 "open for writing"; 现在我们把整个 dump 都做掉.
  extern void Dump_ir_file(FILE_MANAGER *, const char *);
  Dump_ir_file(this, file_name);
}

void FILE_MANAGER::Write_data_to_file(const char *file_name) {
  // 与 Create_ir_file 等价: 把当前内存状态全量序列化到磁盘.
  extern void Dump_ir_file(FILE_MANAGER *, const char *);
  Dump_ir_file(this, file_name);
}

ST_IDX FILE_MANAGER::Create_var(STR_IDX str, TY_IDX idx, UINT8 level,
                              SYM_SCLASS sclass,
                              SYM_ECLASS eclass,
                              SYM_CLASS symclass) {
  AssertThat(level == GLOBAL_SYMTAB || level == LOCAL_SYMTAB, ("level not recognized"));
  AssertThat(level != LOCAL_SYMTAB || Scopes()->Current() != NULL, ("scope is not ready, please enter a function first"));
  ST_IDX sym = Tables()->Sym()->Add(level);
  AssertThat(sym > 0 && (sym & 0xff) == level, ("Incorrect var symidx generated = %u", sym));
  ST *st = ST_st(sym);
  st->name_idx = str;
  st->type = idx;
  st->export_class = eclass;
  st->sym_class = symclass;
  st->storage_class = sclass;
  st->attr = (SYM_ATTR) 0;
  return sym;
}

INITO_IDX FILE_MANAGER::Create_inito(ST_IDX st_idx, std::vector<INITV> &initvs, UINT8 level) {
  INITO_IDX inito_idx = Tables()->Inito()->Add(level);
  INITO *inito = INITO_inito(inito_idx);
  inito->Set_sym(st_idx);
  ST_st(st_idx)->setInitoIdx(inito_idx);
  for (int i = 0; i < initvs.size(); ++i) {
    inito->Add_value(st_idx, initvs[i]);
  }
  return inito_idx;
}

STR_IDX FILE_MANAGER::Save_string(const char *string) {
  return Tables()->Save_string(string);
}

TY_IDX
FILE_MANAGER::Create_array_ty(STR_IDX string, TY_FLAG ty_flag,
                              TY_IDX element_type, ARB_IDX arb) {
  TY_IDX tyidx = Tables()->Ty()->Add();
  TY *ty = TY_ty(tyidx);
  ty->name_idx = string;
  ty->kind = KIND_ARRAY;
  ty->u1.arb = arb;
  ty->Set_etype(element_type);
  AssertThat(ARB_arb(arb)->dimension == 1 || TY_kind(element_type) == KIND_ARRAY,
             ("Multi dimension array should have array elements, = %d",
               ARB_arb(arb)->dimension));
  if (ARB_flags(arb) & ARB_CONST_UBND) {
    ty->size = ARB_arb(arb)->Ubnd_val() * TY_size(element_type);
  } else {
    ty->size = 0;
  }
  return tyidx;
}

TY_IDX FILE_MANAGER::Create_func_ty(STR_IDX string, UINT64 size, MTYPE_ID mtype,
                                    TY_FLAG ty_flag, std::vector<TY_IDX> &ret_and_params) {
  TYLIST_IDX tylist_idx, initial;
  TYLIST *tylist;
  initial = tylist_idx = File()->Tables()->Tylist()->Add();
  AssertThat(initial > 0, ("Tylist id should be greater than zero"));
  for (std::vector<TY_IDX>::iterator it = ret_and_params.begin();
       it != ret_and_params.end();
       it++) {
    tylist = TYLIST_tylist(tylist_idx);
    tylist->ty_id = *it;
    tylist_idx = File()->Tables()->Tylist()->Add();
  }
  tylist = TYLIST_tylist(tylist_idx);
  tylist->ty_id = 0;

  TY_IDX tyidx = Tables()->Ty()->Add();
  TY *ty = TY_ty(tyidx);
  ty->kind = KIND_FUNCTION;
  ty->name_idx = string;
  ty->size = size;
  ty->mtype = mtype;
  ty->flags = ty_flag;
  ty->Set_tylist(initial);
  return tyidx;
}

void FILE_MANAGER::Print(FILE *f) {
  this->Scopes()->Print(f);
  // All functions
  for (GT_ITERATOR it  = Tables()->Pu_info()->Begin();
                   it != Tables()->Pu_info()->End();
                   it ++) {
    PU_INFO *pu_info = static_cast<PU_INFO *>(*it);
    pu_info->Print_function_verbose(f);
  }
  this->Tables()->Print(f);
}

ARB_IDX FILE_MANAGER::Create_array_bound_const(UINT64 ubnd_val, UINT64 stride_val, UINT32 dimen, UINT32 flag) {
  ARB_IDX arbnd = Tables()->Arb()->Add();
  flag |= ARB_CONST_UBND;
  ARB_arb(arbnd)->Init_const(ubnd_val, stride_val, dimen, flag);
  return arbnd;
}

ARB_IDX FILE_MANAGER::Create_array_bound_var(ST_IDX ubnd_var, UINT64 stride_val, UINT32 dimen, UINT32 flag) {
  ARB_IDX arbnd = Tables()->Arb()->Add();
  flag &= ~ARB_CONST_UBND;
  ARB_arb(arbnd)->Init_var(ubnd_var, stride_val, dimen, flag);
  return arbnd;
}

ST_IDX FILE_MANAGER::Find_symbol_by_name(const char *name) {
  // Find from locals table
  SCOPE *current = File()->Scopes()->Current();
  if (current != nullptr) {
    for (UINT32 sym_num = 0; sym_num < Tables()->Sym()->Length(current); sym_num++) {
      ST_IDX sym_idx = (sym_num << 8) | LOCAL_SYMTAB;
      if(ST_st(sym_idx)->name_idx > 0 &&
         strcmp(STR_str(ST_st(sym_idx)->name_idx), name) == 0) {
        return sym_idx;
      }
    }
  }
  // Find from global table
  for (UINT32 sym_num = 0; sym_num < Tables()->Sym()->Length(); sym_num++) {
    ST_IDX sym_idx = (sym_num << 8) | GLOBAL_SYMTAB;
    if(ST_st(sym_idx)->name_idx > 0 &&
       strcmp(STR_str(ST_st(sym_idx)->name_idx), name) == 0) {
      return sym_idx;
    }
  }
  return 0;
}

LABEL_IDX FILE_MANAGER::Create_label(STR_IDX name, UINT32 flags, LABEL_KIND k) {
  LABEL_IDX label_idx = Tables()->Label()->Add(LOCAL_SYMTAB);
  LABEL *lbl = LABEL_label(label_idx);
  lbl->Set_name_idx(name);
  lbl->Set_flags(flags);
  lbl->Set_kind(k);
  return label_idx;
}

LABEL_IDX FILE_MANAGER::Create_preg(STR_IDX preg_name, UINT32 desire_num) {
  PREG_IDX preg_idx = Tables()->Preg()->Add(LOCAL_SYMTAB);
  PREG *preg        = PREG_preg(preg_idx);
  preg->setDesireRegNum(desire_num);
  preg->setNameIdx(preg_name);
  return preg_idx;
}

LABEL_IDX FILE_MANAGER::Get_func_exit_label() {
  LABEL_TABLE *lbt = Tables()->Label();
  SCOPE *scope = Scopes()->Current();
  for (UINT32 i = 0; i < lbt->Length(scope); i++) {
    LABEL_IDX lbl = (i << 8) + LOCAL_SYMTAB;
    if (LABEL_label(lbl)->kind == LKIND_EXIT) {
      return lbl;
    }
  }
  ST_IDX st = scope->st_idx;
  char names[120];
  sprintf(names, ".L%d_endfunc", st);
  return Create_label(Save_string(names),
                      LABEL_FLAGS::LABEL_ADDR_SAVED, LKIND_EXIT);
}

ST_IDX FILE_MANAGER::Get_preg_sym(MTYPE_ID mt, PREG_IDX regid) {
  ST_TABLE    *symtab = File()->Tables()->Sym();
  TY_IDX ty = MTYPE_to_ty(mt);
  for (UINT32 i       = 0; i < symtab->Length(); i++) {
    ST_IDX sym = (i << 8) + GLOBAL_SYMTAB;
    if (ST_symclass(sym) == SYM_CLASS_PREG && ty == MTYPE_to_ty(mt)) {
      return sym; // Single one.
    }
  }
  ST_IDX sym = Tables()->Sym()->Add(GLOBAL_SYMTAB);
  AssertThat(sym > 0 && (sym & 0xff) == GLOBAL_SYMTAB, ("Incorrect var symidx generated = %u", sym));
  ST *st = ST_st(sym);
  st->name_idx = Save_string(".predef_preg");
  st->type          = ty;
  st->export_class  = SYME_INTERNAL;
  st->sym_class     = SYM_CLASS_PREG;
  st->storage_class = SYMC_UNKNOWN;
  st->offset        = regid;
  st->attr          = (SYM_ATTR) 0;
  return sym;
}

void SCOPE::Print(FILE *f) {
  if (st_idx <= 0) {
    fprintf(f, "[Scope] sym = %d, (dummy function)", st_idx);
    return;
  }
  fprintf(f, "[Scope] sym = %s\n", ST_name(st_idx));
}

void ST::Print(FILE *f, BOOL verbose) {
  fprintf(f, "%-10s    ", (name_idx > 0) ? STR_str(name_idx) : "(anon)");
  Print_storage_class(f);
  Print_details(f);
}

void ST::Print_storage_class(FILE *f) {
  switch (storage_class) {
    case SYMC_AUTO:
      fprintf(f, "%s", "local");
      break;
    case SYMC_FORMAL:
      fprintf(f, "%s", "formal");
      break;
    case SYMC_FILE_STATIC:
      fprintf(f, "%s", "file-static");
      break;
    case SYMC_EXTERN:
      fprintf(f, "%s", "extern (possibly syscall)");
      break;
    case SYMC_TEXT:
      fprintf(f, "%s", "text (program)");
      break;
    case SYMC_FUNC_STATIC:
      fprintf(f, "%s", "function static");
      break;
    case SYMC_GLOBAL_DEF:
      fprintf(f, "%s", "global, defined");
      break;
    case SYMC_GLOBAL_UNDEF:
      fprintf(f, "%s", "global, undefined");
      break;
    case SYMC_UNKNOWN:
      fprintf(f, "%s", "unknown");
      break;
    default:
      fprintf(f, "%s", "invalid storage class");
  }
}

void ST::Print_details(FILE *f) {
  switch (sym_class) {
    case SYM_CLASS_FUNC:
      fprintf(f, "  --> function = pu_id = [%d], prototype = [%d] \n", pu, type);
      break;
    case SYM_CLASS_BLOCK:
      fprintf(f, "  --> block \n");
      break;
    case SYM_CLASS_CONST:
      fprintf(f, "  --> const \n");
      break;
    case SYM_CLASS_NAME:
      fprintf(f, "  --> name \n");
      break;
    case SYM_CLASS_PREG:
      fprintf(f, "  --> preg, preg_idx = %d \n", this->offset);
      break;
    case SYM_CLASS_VAR:
      fprintf(f, "  --> var, of type [%d] \n", this->type);
      break;
    case SYM_CLASS_COUNT:
      fprintf(f, "  --> count \n");
      break;
    case SYM_CLASS_UNK:
      fprintf(f, "  -> unknown\n");
      break;
    default:
      fprintf(f, "  -> invalid sym_class\n");
      break;
  }
}

PU_INFO_IDX ST::Pu_info_idx() {
  AssertThat(this != NULL, ("Invalid this pointer"));
  AssertThat(this->sym_class == SYM_CLASS_FUNC, ("Trying to get pu_info_idx from non-func symbol = %s", STR_str(this->name_idx)));
  AssertThat(this->pu > 0, ("Invalid pu_idx in symbol = %d", pu));
  return PU_pu(this->pu)->pu_info_idx;
}

void TYLIST::Print(FILE *f) {
  fprintf(f, "(type = %d)\n", this->ty_id);
}

void LABEL::Print(FILE *f) const {
  fprintf(f, " Label name = %s, kind = %d, temp_sym = %d, flags = 0x%08x \n",
          name_idx > 0 ? STR_str(name_idx) : "(null)", kind, temp_sym, flags);
}

void INITO::Print(FILE *f) const {
  fprintf(f, " Inito for sym name = %s, init-values: %lu\n",
          st_idx > 0 ? ST_name(st_idx) : "(null)", val.size());
  for (UINT32 i = 0; i < val.size(); i++) {
    val[i].Print(f);
  }
}

void INITV::Print(FILE *file) const {
  if (kind == INITVKIND_VAL) {
    fprintf(file, " [IV] VAL(%d), value: %lld \n",
            kind, this->Val());
  } else if (kind == INITVKIND_PAD) {
    fprintf(file, " [IV] PADDING(%d), pad-length = %d \n",
            kind, this->Pad());
  }
}
