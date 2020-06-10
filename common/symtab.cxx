#include "symtab.h"
#include "basic.h"

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
  {  MTYPE_BS,      KIND_SCALAR, TY_FLAG_INTERNAL, 0, "MTYPE_A8",      0 },
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


/******************************************************************************
 * File Symtab
 ******************************************************************************/

void FILE_SYMTAB::Print() {
  this->Ty()->Get(0)->Print(stderr);
  //  Is_Trace(Tracing(COMPONENT_BE, TRACE_INVOCATION),
  //           ("PU st_idx: %d\n", ));
}

PU_INFO_IDX FILE_SYMTAB::Get_pu_info_by_st_idx(ST_IDX func) {
  return Sym()->Get(func)->u2.pu;
}

void FILE_SYMTAB::Initialize() {
  // Add internal types from MTYPE_to_TY_table
  UINT32 total_size = sizeof(MTYPE_to_TY_table);
  UINT32 single_size = sizeof(MTYPE_to_TY_table[0]);
  UINT32 count = total_size / single_size;
  AssertThat(MTYPE_COUNT == count, ("MType table is incomplete or incorrect"));
  for (UINT16 cnt = 0; cnt < count; cnt ++) {
    STR_IDX str_idx = Save_string(MTYPE_to_TY_table[cnt].name);
    TY_IDX ty_idx = Ty()->Add();
    TY *ty = TY_ty(ty_idx);
    ty->name_idx = str_idx;
    ty->mtype = (MTYPE_ID) MTYPE_to_TY_table[cnt].mtype;
    ty->flags = MTYPE_to_TY_table[cnt].flag;
    ty->size = MTYPE_to_TY_table[cnt].size;
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
  STR_IDX str_idx = File()->Tables()->Str()->Add();
  File()->Tables()->Str()->Set(str_idx, (const char *) internal_str_tab_buffer + used_size_of_buffer);
  used_size_of_buffer += needed;
  return str_idx;
}

template<typename IDX, typename T>
GROWING_TABLE<IDX, T> *Get_by_idx(IDX idx) {

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

PU_INFO::PU_INFO() {
  bzero(this, sizeof(PU_INFO));
}

FILE_INFO::FILE_INFO() {
  bzero(this, sizeof(FILE_INFO));
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
  SCOPE * scope = new SCOPE;
  scope->Init(tables->Sym()->Get(pu->proc_sym));
  _current_function = scope;
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
      fprintf(f, "SCALAR (%s)\n", MTYPE_name(mtype));
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
  pu_info->proc_sym = func;
  pu_info->pu_idx = pu_idx;
  // Init st_pu
  ST *st = ST_st(func);
  st->u2.pu = pu_idx;
  File()->Scopes()->Goto_function(func);
  return pu_info_idx;
}

void FILE_MANAGER::Finish_creating_function(ST_IDX func, PU_INFO_IDX pu_info) {
  AssertThat(FALSE, ("Not implemented"));
}

void FILE_MANAGER::Open_ir_file(const char *file_name) {
  AssertThat(FALSE, ("Not implemented"));
}

void FILE_MANAGER::Create_ir_file(const char *file_name) {
  AssertThat(FALSE, ("Not implemented"));
}

void FILE_MANAGER::Write_data_to_file(const char *file_name) {
  AssertThat(FALSE, ("Not implemented"));
}

ST_IDX FILE_MANAGER::Create_var(STR_IDX str, TY_IDX idx, UINT8 level,
                              SYM_SCLASS sclass,
                              SYM_ECLASS eclass,
                              SYM_CLASS symclass) {
  AssertThat(level == GLOBAL_SYMTAB || level == LOCAL_SYMTAB, ("level not recognized"));
  AssertThat(level != LOCAL_SYMTAB || Scopes()->Current() != NULL, ("scope is not ready, please enter a function first"));
  ST_IDX sym = Tables()->Sym()->Add(level);
  ST *st = ST_st(sym);
  st->name_idx = str;
  st->u2.type = idx;
  st->export_class = eclass;
  st->sym_class = symclass;
  st->storage_class = sclass;
  st->st_idx = sym;
  st->attr = (SYM_ATTR) 0;
  return sym;
}

STR_IDX FILE_MANAGER::Save_string(const char *string) {
  return Tables()->Save_string(string);
}

TY_IDX
FILE_MANAGER::Create_array_ty(STR_IDX string, UINT64 size, MTYPE_ID mtype,
                              TY_FLAG ty_flag, TY_IDX element_type, ARB_IDX arb) {
  TY_IDX tyidx = Tables()->Ty()->Add();
  TY *ty = TY_ty(tyidx);
  ty->name_idx = string;
  ty->kind = KIND_ARRAY;
  ty->Set_etype(element_type);
  AssertThat(ARB_arb(arb)->dimension == 1, ("Multi dimension array not supported"));
  ty->size = ARB_arb(arb)->Ubnd_val() * TY_size(element_type);
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
  tylist_idx = File()->Tables()->Tylist()->Add();
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
