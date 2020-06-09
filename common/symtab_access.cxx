//
// Created by xc5 on 2020/6/9.
//
#include "symtab.h"

#define OCC_SYMTAB_ACCESS_DEF_MODE

#include "symtab_access.h"

#undef OCC_SYMTAB_ACCESS_DEF_MODE

template<typename IDX, typename T, typename TABLE>
T *Symtab_get(IDX idx) {
  AssertThat(File()->Tables()->Get_table((T *) NULL) != NULL,
             ("table should not be NULL"));
  AssertThat(File()->Tables()->Get_table((T *) NULL)->Get(idx) != NULL,
             ("item should not be NULL"));
  return File()->Tables()->Get_table((T *) NULL)->Get(idx);
}

template<typename IDX, typename T, typename TABLE>
const char *Symtab_name(IDX idx) {
  T *item = Symtab_get<IDX, T, TABLE>(idx);
  AssertThat(item != NULL, ("item should not be NULL"));
  AssertThat(item->name_idx > 0, ("name_idx found zero on item"));
  AssertThat(STR_str(item->name_idx) != NULL, ("string table returned null"));
  return STR_str(item->name_idx);
}

template<typename IDX, typename T, typename TABLE>
BOOL Symtab_is_flags_set(IDX idx, UINT64 flag_opt) {
  T *item = Symtab_get<IDX, T, TABLE>(idx);
  AssertThat(item != NULL, ("item should not be NULL"));
  return !!(item->flags & flag_opt);
}

//================================================================================
// Defining IDX -> T* functions, using Table->Get(IDX)
//================================================================================
template ST *Symtab_get<ST_IDX, ST, ST_TABLE>(ST_IDX idx);

template TY *Symtab_get<TY_IDX, TY, TY_TABLE>(TY_IDX idx);

template ARB *Symtab_get<ARB_IDX, ARB, ARB_TABLE>(ARB_IDX idx);

template PU *Symtab_get<PU_IDX, PU, PU_TABLE>(PU_IDX idx);

template PU_INFO *
Symtab_get<PU_INFO_IDX, PU_INFO, PU_INFO_TABLE>(PU_INFO_IDX idx);

template TYLIST *Symtab_get<TYLIST_IDX, TYLIST, TYLIST_TABLE>(TYLIST_IDX idx);

template const char *Symtab_get<STR_IDX, const char, STR_TABLE>(STR_IDX idx);


//================================================================================
// Defining IDX -> const char * functions, using STR Table, and a STR_IDX
//================================================================================
template const char *Symtab_name<ST_IDX, ST, ST_TABLE>(ST_IDX idx);

template const char *Symtab_name<TY_IDX, TY, TY_TABLE>(TY_IDX idx);

//================================================================================
// Defining IDX -> BOOL functions, using bit level AND op
//================================================================================
template BOOL
Symtab_is_flags_set<TY_IDX, TY, TY_TABLE>(TY_IDX idx, UINT64 flag_opt);

template BOOL
Symtab_is_flags_set<ARB_IDX, ARB, ARB_TABLE>(ARB_IDX idx, UINT64 flag_opt);


/******************************************************************************
 * SYMTAB_ACCESS
 ******************************************************************************/

template<typename IDX, typename T>
IDX GLOBAL_SYMTAB_ACCESS<IDX, T>::Add() {
  return tab.Add();
}

template<typename IDX, typename T>
T *GLOBAL_SYMTAB_ACCESS<IDX, T>::Get(IDX idx) {
  return tab[idx];
}

/******************************************************************************
 * Related Tables (Two-level tables)
 ******************************************************************************/

/**
 * Add an empty entry in the table
 * @param level GLOBAL_SYMTAB/LOCAL_SYMTAB
 * @return IDX, the result index in the table
 */
template<typename IDX, class T, TABLE_KIND KIND>
IDX RELATED_SYMTAB_ACCESS<IDX, T, KIND>::Add(UINT8 level) {
  if (level <= GLOBAL_SYMTAB) {
    return tab.Add() << 8;
  } else {
    AssertThat(level == LOCAL_SYMTAB,
               ("Currently, we'd only support LOCAL_SYMTAB and GLOBAL_SYMTAB,"
                " level given = %d", level));
    IDX base = Scoped_table()->Add();
    return level & (base << 8);
  }
}

template<typename IDX, class T, TABLE_KIND KIND>
T *RELATED_SYMTAB_ACCESS<IDX, T, KIND>::Get(IDX idx) {
  UINT8 first_level = idx & 0xff;
  UINT32 second = (idx >> 8) & 0xffffff;
  if (first_level <= GLOBAL_SYMTAB) {
    return tab[second];
  } else {
    return Scoped_table()->Get(second);
  }
}

template<>
GROWING_TABLE<ST_IDX, ST> *
RELATED_SYMTAB_ACCESS<ST_IDX, ST, TABLE_KIND_ST>::Scoped_table() {
  return File()->Tables()->Scope()->Current()->getStTab();
}

template<>
GROWING_TABLE<PREG_IDX, PREG> *
RELATED_SYMTAB_ACCESS<PREG_IDX, PREG, TABLE_KIND_PREG>::Scoped_table() {
  return File()->Tables()->Scope()->Current()->getPregTab();
};

template<>
GROWING_TABLE<LABEL_IDX, LABEL> *
RELATED_SYMTAB_ACCESS<LABEL_IDX, LABEL, TABLE_KIND_LABEL>::Scoped_table() {
  return File()->Tables()->Scope()->Current()->getLabelTab();
};

template<>
GROWING_TABLE<INITO_IDX, INITO> *
RELATED_SYMTAB_ACCESS<INITO_IDX, INITO, TABLE_KIND_INITO>::Scoped_table() {
  return File()->Tables()->Scope()->Current()->getInitoTab();
};

/**
 * These must be put after the definition of above functions
 * within GLOBAL_SYMTAB_ACCESS. and RELATED_SYMTAB_ACCESS
 */
template
class GLOBAL_SYMTAB_ACCESS<TY_IDX, TY>;

template
class GLOBAL_SYMTAB_ACCESS<ARB_IDX, ARB>;

template
class GLOBAL_SYMTAB_ACCESS<TYLIST_IDX, TYLIST>;

template
class GLOBAL_SYMTAB_ACCESS<PU_IDX, PU>;

template
class RELATED_SYMTAB_ACCESS<ST_IDX, ST, TABLE_KIND_ST>;

template
class RELATED_SYMTAB_ACCESS<PREG_IDX, PREG, TABLE_KIND_PREG>;

template
class GLOBAL_SYMTAB_ACCESS<PU_INFO_IDX, PU_INFO>;

template
class GLOBAL_SYMTAB_ACCESS<STR_IDX, const char>;