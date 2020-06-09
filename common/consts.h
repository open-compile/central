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

#endif //OCC_CONSTS_H
