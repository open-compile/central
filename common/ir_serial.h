// =============================================================================
// CLAUDE-MARKER  STATUS: 新建
// =============================================================================
// 数据结构 ↔ 字节流的 encoder / decoder 函数声明
//
// 设计：每个 SYMTAB / TREE 元素结构对应一组 Encode_X / Decode_X，
//       它们只调用 IR_WRITER / IR_READER 的原语，不知道 section 结构。
//
// 注：Decode_X 返回 bool；当遇到不识别的 section version 时返回 false，
//     调用者可决定 fallback 或报错。
// =============================================================================

#ifndef _OCC_IR_SERIAL_H_
#define _OCC_IR_SERIAL_H_

#include "ir_io.h"
#include "host.h"
#include "consts.h"

class TY;
class ST;
struct PU;
struct PU_INFO;
struct LABEL;
struct PREG;
struct ARB;
struct TYLIST;
struct INITV;
class  INITO;
class  IRNODE;
class  TREE;
class  FILE_SYMTAB;
class  FILE_MANAGER;

// -----------------------------------------------------------------------------
// 单元素 encoder / decoder
// -----------------------------------------------------------------------------
void Encode_TY     (IR_WRITER &w, const TY     &t);
bool Decode_TY     (IR_READER &r, TY     &out, UINT16 ver);

void Encode_TYLIST (IR_WRITER &w, const TYLIST &t);
bool Decode_TYLIST (IR_READER &r, TYLIST &out, UINT16 ver);

void Encode_ARB    (IR_WRITER &w, const ARB    &a);
bool Decode_ARB    (IR_READER &r, ARB    &out, UINT16 ver);

void Encode_PU     (IR_WRITER &w, const PU     &p);
bool Decode_PU     (IR_READER &r, PU     &out, UINT16 ver);

void Encode_PU_INFO(IR_WRITER &w, const PU_INFO &p);   // 不含 sub-image
bool Decode_PU_INFO(IR_READER &r, PU_INFO &out, UINT16 ver);

void Encode_ST     (IR_WRITER &w, const ST    &s);
bool Decode_ST     (IR_READER &r, ST    &out, UINT16 ver);

void Encode_LABEL  (IR_WRITER &w, const LABEL &l);
bool Decode_LABEL  (IR_READER &r, LABEL &out, UINT16 ver);

void Encode_PREG   (IR_WRITER &w, const PREG  &p);
bool Decode_PREG   (IR_READER &r, PREG  &out, UINT16 ver);

void Encode_INITV  (IR_WRITER &w, const INITV &v);
bool Decode_INITV  (IR_READER &r, INITV &out, UINT16 ver);

void Encode_INITO  (IR_WRITER &w, INITO &o);  // 不 const，因为 INITO 没 const Size/Value
bool Decode_INITO  (IR_READER &r, INITO &out, UINT16 ver);

void Encode_IRNODE (IR_WRITER &w, const IRNODE &n);
bool Decode_IRNODE (IR_READER &r, IRNODE &out, UINT16 ver);

// -----------------------------------------------------------------------------
// Section-level (复合) encoder / decoder
// -----------------------------------------------------------------------------
// 全局表
void Encode_global_TY   (IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_TY   (IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_TYLIST(IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_TYLIST(IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_ARB  (IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_ARB  (IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_PU   (IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_PU   (IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_PU_INFO(IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_PU_INFO(IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_ST   (IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_ST   (IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_LABEL(IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_LABEL(IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_PREG (IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_PREG (IR_READER &r, FILE_SYMTAB *tab);
void Encode_global_INITO(IR_WRITER &w, FILE_SYMTAB *tab);
void Decode_global_INITO(IR_READER &r, FILE_SYMTAB *tab);

// 函数 sub-image  (Step 8)
void Encode_function_subimage(IR_WRITER &w, FILE_MANAGER *fm,
                              PU_INFO_IDX pu_info_idx);
void Decode_function_subimage(IR_READER &r, FILE_MANAGER *fm,
                              PU_INFO_IDX pu_info_idx,
                              UINT64 abs_off, UINT64 size);

// 各 section 的 version 常量 (per-section 可独立 bump，与 file_format_version 解耦)
static const UINT16 SECVER_STRTAB        = 1;
static const UINT16 SECVER_TY            = 1;
static const UINT16 SECVER_TYLIST        = 1;
static const UINT16 SECVER_ARB           = 1;
static const UINT16 SECVER_PU            = 1;
static const UINT16 SECVER_PU_INFO       = 1;
static const UINT16 SECVER_ST            = 1;
static const UINT16 SECVER_LABEL         = 1;
static const UINT16 SECVER_PREG          = 1;
static const UINT16 SECVER_INITO         = 1;
static const UINT16 SECVER_FUNC_TAB      = 1;
static const UINT16 SECVER_IRNODES       = 1;
static const UINT16 SECVER_TREE_TOPO     = 1;

#endif // _OCC_IR_SERIAL_H_
