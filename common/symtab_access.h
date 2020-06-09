//
// Created by xc5 on 2020/6/8.
// This is directly included in symtab.h, symtab_access.cxx
//
#if (!defined(OCC_SYMTAB_ACCESS_H_DECL) && defined(OCC_SYMTAB_ACCESS_DECL_MODE)) || (!defined(OCC_SYMTAB_ACCESS_H_DEF) && defined(OCC_SYMTAB_ACCESS_DEF_MODE))
  #ifdef OCC_SYMTAB_ACCESS_DEF_MODE
    #define OCC_SYMTAB_ACCESS_H_DEF

    #if defined(OCC_SYMTAB_ACCESS_H_DECL)
    // symtab.cxx
    #undef DEF_MEMBER_ACCESS_FUNC
    #undef DEF_MEMBER_FLAG_FUNC
    #undef ACCESS_TEMP_EXTERN
    #endif
    #define ACCESS_TEMP_EXTERN

    /**
     * Utilities for accessing fields in specific data structure
     */
    #define DEF_MEMBER_ACCESS_FUNC(RESULT_TYPE, Name, T, ITEM_IDX, Field, IDX2PTR) \
    RESULT_TYPE Name(ITEM_IDX idx) { \
      T *struct_val = IDX2PTR(idx); \
      AssertThat(struct_val != NULL, ("symbol could not be null, idx = %0#x", idx)); \
      RESULT_TYPE item = struct_val->Field; \
      return item; \
    }

    /**
     * Get a flag field in the data structure, either zero, or one.
     */
    #define DEF_MEMBER_FLAG_FUNC(Name, T, ITEM_IDX, Field, FLAG_NUM, IDX2PTR) \
    BOOL Name(ITEM_IDX idx) { \
      T *item = IDX2PTR(idx); \
      BOOL is_flag_on = !!(item->Field & FLAG_NUM); \
      return is_flag_on; \
    }
  #else
    #define OCC_SYMTAB_ACCESS_H_DECL
    #define ACCESS_TEMP_EXTERN extern
    /**
     * Utilities for accessing fields in specific data structure
     */
    #define DEF_MEMBER_ACCESS_FUNC(RESULT_TYPE, Name, T, ITEM_IDX, Field, IDX2PTR) \
    extern RESULT_TYPE Name(ITEM_IDX idx);

    /**
     * Get a flag field in the data structure, either zero, or one.
     */
    #define DEF_MEMBER_FLAG_FUNC(Name, T, ITEM_IDX, Field, FLAG_NUM, IDX2PTR) \
    extern BOOL Name(ITEM_IDX idx);
  #endif //OCC_SYMTAB_ACCESS_H_DECL

#include "host.h"
#include "basic.h"
#define INLINE inline

//================================================================================
// Defining IDX -> T* functions, using Table->Get(IDX)
//================================================================================
template <typename IDX, typename T, typename TABLE> T * Symtab_get(IDX idx);
template <typename IDX, typename T, typename TABLE> const char * Symtab_name (IDX idx);
template <typename IDX, typename T, typename TABLE> BOOL Symtab_is_flags_set (IDX idx, UINT64 flag_opt);
extern template ST *        Symtab_get<ST_IDX, ST, ST_TABLE> (ST_IDX idx);
extern template TY *        Symtab_get<TY_IDX, TY, TY_TABLE> (TY_IDX idx);
extern template ARB *       Symtab_get<ARB_IDX, ARB, ARB_TABLE> (ARB_IDX idx);
extern template PU *        Symtab_get<PU_IDX, PU, PU_TABLE> (PU_IDX idx);
extern template PU_INFO *   Symtab_get<PU_INFO_IDX, PU_INFO, PU_INFO_TABLE> (PU_INFO_IDX idx);
extern template TYLIST *    Symtab_get<TYLIST_IDX, TYLIST, TYLIST_TABLE> (TYLIST_IDX idx);
extern template const char *Symtab_get<STR_IDX, const char, STR_TABLE> (STR_IDX idx);

#define ST_st(st_idx) Symtab_get<ST_IDX, ST, ST_TABLE>(st_idx)
#define TY_ty(idx) Symtab_get<TY_IDX, TY, TY_TABLE>(idx)
#define PU_pu(idx) Symtab_get<PU_IDX, PU, PU_TABLE>(idx)
#define ARB_arb(idx) Symtab_get<ARB_IDX, ARB, ARB_TABLE>(idx)
#define PU_INFO_pu_info(idx) Symtab_get<PU_INFO_IDX, PU_INFO, PU_INFO_TABLE>(idx)
#define TYLIST_tylist(idx) Symtab_get<TYLIST_IDX, TYLIST, TYLIST_TABLE>(idx)
#define STR_str(idx) Symtab_get<STR_IDX, const char, STR_TABLE>(idx)

//================================================================================
// Defining IDX -> const char * functions, using STR Table, and a STR_IDX
//================================================================================
extern template const char * Symtab_name<ST_IDX, ST, ST_TABLE> (ST_IDX idx);
extern template const char * Symtab_name<TY_IDX, TY, TY_TABLE> (TY_IDX idx);
#define TY_name(ty_idx) Symtab_name<TY_IDX, TY, TY_TABLE>(ty_idx)
#define ST_name(st_idx) Symtab_name<ST_IDX, ST, ST_TABLE>(st_idx)

//================================================================================
// Defining IDX -> BOOL functions, using bit level AND op
//================================================================================
extern template BOOL Symtab_is_flags_set<TY_IDX, TY, TY_TABLE> (TY_IDX idx, UINT64 flag_opt);
extern template BOOL Symtab_is_flags_set<ARB_IDX, ARB, ARB_TABLE> (ARB_IDX idx, UINT64 flag_opt);
#define TY_is_const(idx) Symtab_is_flags_set<TY_IDX, TY, TY_TABLE> (idx, TY_FLAG_CONST)
#define ARB_last_dimen(idx) Symtab_is_flags_set<ARB_IDX, ARB, ARB_TABLE> (idx, ARB_LAST_DIMEN)

DEF_MEMBER_ACCESS_FUNC(SYM_ECLASS, ST_eclass, ST, ST_IDX, export_class, ST_st);
DEF_MEMBER_ACCESS_FUNC(TY_IDX, ST_ty, ST, ST_IDX, u2.type, ST_st);
DEF_MEMBER_ACCESS_FUNC(PU_IDX, ST_pu, ST, ST_IDX, u2.type, ST_st);

DEF_MEMBER_ACCESS_FUNC(TY_FLAG, TY_flags, TY, TY_IDX, flags, TY_ty);
DEF_MEMBER_ACCESS_FUNC(UINT64, TY_size, TY, TY_IDX, size, TY_ty);
DEF_MEMBER_ACCESS_FUNC(TY_KIND, TY_kind, TY, TY_IDX, kind, TY_ty);
DEF_MEMBER_ACCESS_FUNC(INT32, TY_align, TY, TY_IDX, align, TY_ty);
DEF_MEMBER_ACCESS_FUNC(ARB_IDX, TY_arb, TY, TY_IDX, Arb(), TY_ty);

DEF_MEMBER_ACCESS_FUNC(UINT16, ARB_flags, ARB, ARB_IDX, flags, ARB_arb);
DEF_MEMBER_ACCESS_FUNC(UINT16, ARB_dimension, ARB, ARB_IDX, dimension, ARB_arb);
DEF_MEMBER_ACCESS_FUNC(INT32, ARB_lbnd_val, ARB, ARB_IDX, u1.lbnd_val, ARB_arb);
DEF_MEMBER_ACCESS_FUNC(INT32, ARB_ubnd_val, ARB, ARB_IDX, u2.ubnd_val, ARB_arb);
DEF_MEMBER_ACCESS_FUNC(ST_IDX, ARB_lbnd_var, ARB, ARB_IDX, u1.var.lbnd_var, ARB_arb);
DEF_MEMBER_ACCESS_FUNC(ST_IDX, ARB_ubnd_var, ARB, ARB_IDX, u2.var.ubnd_var, ARB_arb);

#endif