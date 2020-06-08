//
// Created by xc5 on 2020/6/8.
//
#include "host.h"
#include "basic.h"
#if (!defined(OCC_SYMTAB_ACCESS_H_DECL) && defined(OCC_SYMTAB_ACCESS_DECL_MODE)) || (!defined(OCC_SYMTAB_ACCESS_H_DEF) && defined(OCC_SYMTAB_ACCESS_DEF_MODE))
  #ifdef OCC_SYMTAB_ACCESS_DEF_MODE
    #define OCC_SYMTAB_ACCESS_H_DEF

    #if defined(OCC_SYMTAB_ACCESS_H_DECL)
    // symtab.cxx
    #undef DEF_ACCESS_FUNC
    #undef DEF_MEMBER_ACCESS_FUNC
    #undef DEF_MEMBER_NAME_FUNC
    #undef DEF_MEMBER_FLAG_FUNC
    #endif
    /**
     * Getting the data item from the Table, by IDX
     * @param idx
     * @return
     */
    #define DEF_ACCESS_FUNC(T, ITEM_IDX, Name, TableAccessFunction) \
    T * Name ( ITEM_IDX idx ) { \
      T *item = File()->Tables()->TableAccessFunction()->Get(idx); \
      AssertThat(item != NULL, ("item cannot be null, idx = %0#x", idx)); \
      return item; \
    };

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
     * Getting symbol name by st_idx
     * @param idx
     * @return
     */
    #define DEF_MEMBER_NAME_FUNC(Name, T, ITEM_IDX, Field, IDX2PTR) \
    const char *Name(ITEM_IDX idx) { \
      T *item = IDX2PTR(idx); \
      STR_IDX str_idx = item->Field; \
      AssertThat(str_idx > 0, ("Str idx must be larger than zero, st_idx = %0#x, str_idx = %0#x", idx, str_idx)); \
      const char *c_str = File()->Tables()->Str()->Get(str_idx); \
      AssertThat(c_str != NULL, ("name must not be NULL, st_idx = %0#x, str_idx = %0#x", idx, str_idx)); \
      return c_str; \
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
    /**
     * Getting the data item from the Table, by IDX
     * @param idx
     * @return
     */
    #define DEF_ACCESS_FUNC(T, ITEM_IDX, Name, TableAccessFunction) \
    extern T * Name ( ITEM_IDX idx );

    /**
     * Utilities for accessing fields in specific data structure
     */
    #define DEF_MEMBER_ACCESS_FUNC(RESULT_TYPE, Name, T, ITEM_IDX, Field, IDX2PTR) \
    extern RESULT_TYPE Name(ITEM_IDX idx);

    /**
     * Getting symbol name by st_idx
     * @param idx
     * @return
     */
    #define DEF_MEMBER_NAME_FUNC(Name, T, ITEM_IDX, Field, IDX2PTR) \
    extern const char *Name(ITEM_IDX idx);

    /**
     * Get a flag field in the data structure, either zero, or one.
     */
    #define DEF_MEMBER_FLAG_FUNC(Name, T, ITEM_IDX, Field, FLAG_NUM, IDX2PTR) \
    extern BOOL Name(ITEM_IDX idx);
  #endif //OCC_SYMTAB_ACCESS_H_DECL

//================================================================================
// Defining IDX -> T* functions, using Table->Get(IDX)
//================================================================================
DEF_ACCESS_FUNC(PU, PU_IDX, PU_pu, Pu);
DEF_ACCESS_FUNC(TY, TY_IDX, TY_ty, Ty);
DEF_ACCESS_FUNC(TYLIST, TYLIST_IDX, TYLIST_tylist, Tylist);
DEF_ACCESS_FUNC(ARB, ARB_IDX, ARB_arb, Arb);
DEF_ACCESS_FUNC(ST, ST_IDX, ST_st, Sym);
DEF_ACCESS_FUNC(const char, STR_IDX, STR_str, Str);
DEF_ACCESS_FUNC(PU_INFO, PU_INFO_IDX, PU_INFO_pu_info, Pu_info);

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

//================================================================================
// Defining IDX -> const char * functions, using STR Table, and a STR_IDX
//================================================================================
DEF_MEMBER_NAME_FUNC(ST_name, ST, ST_IDX, name_idx, ST_st);
DEF_MEMBER_NAME_FUNC(TY_name, TY, TY_IDX, name_idx, TY_ty);

//================================================================================
// Defining IDX -> BOOL functions, using bit level AND op
//================================================================================
DEF_MEMBER_FLAG_FUNC(TY_is_const, TY, TY_IDX, flags, TY_FLAG_CONST, TY_ty);
DEF_MEMBER_FLAG_FUNC(ARB_last_dimen, ARB, ARB_IDX, flags, ARB_LAST_DIMEN, ARB_arb);

#endif