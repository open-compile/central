//
// Created by xc5 on 2020/6/9.
//

#ifndef OCC_CONSTS_H
#define OCC_CONSTS_H

#define GLOBAL_SYMTAB 1
#define LOCAL_SYMTAB 2
#define STR_TABLE_BLOCK_SIZE (1024 * 4)
#define OCC_STRINGIFY( name ) # name

typedef UINT32 STR_IDX; // string table index
typedef UINT32 ST_IDX; // symbol info idx
typedef UINT32 LABEL_IDX;
typedef UINT32 PREG_IDX;
typedef UINT32 TY_IDX; //type info idx
typedef UINT32 TYLIST_IDX; //type info list idx
typedef UINT32 PU_IDX; //program unit idx
typedef UINT32 TCON_IDX;
typedef UINT32 ARB_IDX;
typedef UINT32 INITV_IDX;
typedef UINT32 INITO_IDX;
typedef UINT32 PU_INFO_IDX;
typedef UINT8 SCOPE_IDX;
typedef UINT16 mTYPE_ID;
typedef UINT16 PREG_NUM;
typedef UINT32 TN_IDX;
typedef UINT32 CFG_BB_IDX;
typedef UINT32 CGOP_IDX;
typedef UINT32 CFG_STMT_IDX;

// Tree
typedef INT32  TREE_OFFSET;
typedef INT64  TREE_ESIZE;
typedef UINT64 IRNODE_IDX;


// Code generation related.

enum CGOPR_KIND {
  CGOPR_IMM = 1,
  CGOPR_R = 2,
  CGOPR_N = 3,
};

// Symbol Table Frame Work
enum SYM_ATTR {
  ST_CONST = 0,
  ST_ALLOCATED = 1,
};

// Must be smaller than 64
enum MTYPE_ID {
  MTYPE_UNKNOWN = 0,
  MTYPE_BEGIN = 1,
  MTYPE_B = 1,
  MTYPE_I1 = 2,
  MTYPE_I2 = 3,
  MTYPE_I4 = 4,
  MTYPE_I8 = 5,
  MTYPE_U1 = 6,
  MTYPE_U2 = 7,
  MTYPE_U4 = 8,
  MTYPE_U8 = 9,
  MTYPE_F4 = 10,
  MTYPE_F8 = 11,
  MTYPE_M = 12,
  MTYPE_V = 13,
  MTYPE_A4 = 14,
  MTYPE_A8 = 15,
  MTYPE_BS = 16,
  MTYPE_COUNT = 17,
};

enum SYM_SCLASS {
  SYMC_UNKNOWN      = 0,
  SYMC_AUTO         = 1,
  SYMC_FUNC_STATIC  = 2,
  SYMC_FILE_STATIC  = 3,
  SYMC_EXTERN       = 4,
  SYMC_GLOBAL_UNDEF = 5,
  SYMC_GLOBAL_DEF   = 6,
  SYMC_TEXT         = 7,
  SYMC_FORMAL       = 8,
};

typedef enum {
  SYME_UNKNOWN     = 0,
  SYME_INTERNAL    = 1,
  SYME_EXTERNAL    = 2,
  SYME_PREEMPTIBLE = 3,
} SYM_ECLASS;

enum SYM_CLASS {
  SYM_CLASS_UNK    = 0,
  SYM_CLASS_VAR    = 1,      // data variable
  SYM_CLASS_FUNC   = 2,      // addrress of a function.
  SYM_CLASS_CONST  = 3,      // constant value
  SYM_CLASS_PREG   = 4,      // pseudo register
  SYM_CLASS_BLOCK  = 5,      // base to a block of data
  SYM_CLASS_NAME   = 6,      // just hold an ST name
  SYM_CLASS_COUNT  = 7      // total number of classes
}; // SYM_CLASS

typedef enum {
  TLS_NONE = 0
} ST_TLS_MODEL;

enum TREE_SEQ {
  TREE_SEQ_BODY      = 1,
  TREE_SEQ_PRAGMA    = 0,
};

const char * const DBAR = "=======================================================\n";

#endif //OCC_CONSTS_H
