// =============================================================================
// CLAUDE-MARKER  STATUS: 新建
// =============================================================================
// 数据结构 ↔ 字节流 encoder / decoder 实现
//
// 设计要点:
//   - 所有 IDX 字段一律按 32-bit (UINT32) 写出, level 已经合并在低 8 bit 中
//   - 位字段 (ST.attr/sym_class 等) 显式 pack 到 u16, 不依赖 C++ 编译器布局
//   - INITV: kind 分支 + variant 一律 pad 到固定 stride (16B), 便于 random-access
//   - IRNODE: 72B POD 直冗 (v1)；v2 想精简只需 bump SECVER_IRNODES
// =============================================================================

#include "ir_serial.h"
#include "symtab.h"
#include "tree.h"
#include "tree_util.h"

#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// 辅助：ftell / 跨 IDX 的对齐工具
// -----------------------------------------------------------------------------
static inline UINT32 Pack_st_bitfields(const ST &s) {
  // ST 5 个 4-bit 位字段共 20 bit -> 装到一个 u32 里 (留 12 bit 备用)
  UINT32 v = 0;
  v |= ((UINT32) s.attr          & 0xf) <<  0;
  v |= ((UINT32) s.sym_class     & 0xf) <<  4;
  v |= ((UINT32) s.storage_class & 0xf) <<  8;
  v |= ((UINT32) s.export_class  & 0xf) << 12;
  v |= ((UINT32) s.tls_model     & 0xf) << 16;
  return v;
}

static inline void Unpack_st_bitfields(UINT32 v, ST &s) {
  s.attr          = (v >>  0) & 0xf;
  s.sym_class     = (SYM_CLASS  )((v >>  4) & 0xf);
  s.storage_class = (SYM_SCLASS )((v >>  8) & 0xf);
  s.export_class  = (SYM_ECLASS )((v >> 12) & 0xf);
  s.tls_model     = (ST_TLS_MODEL)((v >> 16) & 0xf);
}

// =============================================================================
//                                  TY
// =============================================================================
void Encode_TY(IR_WRITER &w, const TY &t) {
  w.Write_u64(t.size);
  w.Write_u8 ((UINT8 ) t.kind);
  w.Write_u8 ((UINT8 ) t.mtype);
  w.Write_u16((UINT16) t.flags);
  w.Write_i32(t.align);
  // u1 / u2 是 4B union，按 u32 写
  UINT32 u1v = (UINT32) t.u1.tylist;   // 取最大字段 (TYLIST_IDX/ARB_IDX 都是 UINT32)
  w.Write_u32(u1v);
  w.Write_u32((UINT32) t.name_idx);
  UINT32 u2v = (UINT32) t.u2.pu_flags;
  w.Write_u32(u2v);
  w.Write_u32((UINT32) t.vtable);
}

bool Decode_TY(IR_READER &r, TY &out, UINT16 ver) {
  if (ver != SECVER_TY) return false;
  out.size  = r.Read_u64();
  out.kind  = (TY_KIND ) r.Read_u8();
  out.mtype = (MTYPE_ID) r.Read_u8();
  out.flags = (TY_FLAG ) r.Read_u16();
  out.align = r.Read_i32();
  UINT32 u1v = r.Read_u32();
  out.u1.tylist = u1v;
  out.name_idx = r.Read_u32();
  UINT32 u2v = r.Read_u32();
  out.u2.pu_flags = u2v;
  out.vtable = r.Read_u32();
  return true;
}

// =============================================================================
//                                 TYLIST
// =============================================================================
void Encode_TYLIST(IR_WRITER &w, const TYLIST &t) {
  w.Write_u32((UINT32) t.ty_id);
}

bool Decode_TYLIST(IR_READER &r, TYLIST &out, UINT16 ver) {
  if (ver != SECVER_TYLIST) return false;
  out.ty_id = r.Read_u32();
  return true;
}

// =============================================================================
//                                  ARB
// =============================================================================
void Encode_ARB(IR_WRITER &w, const ARB &a) {
  w.Write_u16((UINT16) a.flags);
  w.Write_u16(a.dimension);
  w.Write_u32(a.dummy_padding);
  // u1/u2/u3 都是 8B union
  w.Write_i32(a.u1.lbnd_val);
  w.Write_u32((UINT32) a.u1.var.unused);
  w.Write_i32(a.u2.ubnd_val);
  w.Write_u32((UINT32) a.u2.var.unused);
  w.Write_i32(a.u3.stride_val);
  w.Write_u32((UINT32) a.u3.var.unused);
}

bool Decode_ARB(IR_READER &r, ARB &out, UINT16 ver) {
  if (ver != SECVER_ARB) return false;
  out.flags = (ARB_FLAGS) r.Read_u16();
  out.dimension = r.Read_u16();
  out.dummy_padding = r.Read_u32();
  out.u1.lbnd_val = r.Read_i32();
  out.u1.var.unused = r.Read_u32();
  out.u2.ubnd_val = r.Read_i32();
  out.u2.var.unused = r.Read_u32();
  out.u3.stride_val = r.Read_i32();
  out.u3.var.unused = r.Read_u32();
  return true;
}

// =============================================================================
//                                  PU
// =============================================================================
void Encode_PU(IR_WRITER &w, const PU &p) {
  w.Write_u32((UINT32) p.prototype);
  w.Write_u32((UINT32) p.base_class);
  w.Write_u32(p.flags);
  w.Write_u32((UINT32) p.pu_info_idx);
}

bool Decode_PU(IR_READER &r, PU &out, UINT16 ver) {
  if (ver != SECVER_PU) return false;
  out.prototype  = r.Read_u32();
  out.base_class = r.Read_u32();
  out.flags      = r.Read_u32();
  out.pu_info_idx= r.Read_u32();
  return true;
}

// =============================================================================
//                                PU_INFO
// =============================================================================
// PU_INFO 写出时不含 sub-image (TREE/scope) 数据；那些走 sub-image 表
void Encode_PU_INFO(IR_WRITER &w, const PU_INFO &p) {
  w.Write_u32((UINT32) p.pu_info_idx);
  w.Write_u32((UINT32) p.pu_idx);
  w.Write_u32((UINT32) p.proc_sym);
}

bool Decode_PU_INFO(IR_READER &r, PU_INFO &out, UINT16 ver) {
  if (ver != SECVER_PU_INFO) return false;
  out.pu_info_idx = r.Read_u32();
  out.pu_idx      = r.Read_u32();
  out.proc_sym    = r.Read_u32();
  // entry / scope 由 sub-image 重建
  return true;
}

// =============================================================================
//                                  ST
// =============================================================================
void Encode_ST(IR_WRITER &w, const ST &s) {
  w.Write_u32((UINT32) s.name_idx);
  w.Write_u32(Pack_st_bitfields(s));
  w.Write_u32((UINT32) s.type);
  w.Write_u32((UINT32) s.pu);
  w.Write_u32(s.pad);
  w.Write_u32(s.offset);
  w.Write_u32((UINT32) s.base_idx);
  w.Write_u32((UINT32) s.inito_idx);
}

bool Decode_ST(IR_READER &r, ST &out, UINT16 ver) {
  if (ver != SECVER_ST) return false;
  out.name_idx = r.Read_u32();
  UINT32 bitf  = r.Read_u32();
  Unpack_st_bitfields(bitf, out);
  out.type     = r.Read_u32();
  out.pu       = r.Read_u32();
  out.pad      = r.Read_u32();
  out.offset   = r.Read_u32();
  out.base_idx = r.Read_u32();
  out.inito_idx= r.Read_u32();
  return true;
}

// =============================================================================
//                                LABEL
// =============================================================================
void Encode_LABEL(IR_WRITER &w, const LABEL &l) {
  w.Write_u32((UINT32) l.name_idx);
  // flags: 24 bit, kind: 8 bit  -> pack to u32
  UINT32 v = ((UINT32)(l.kind & 0xff)) | ((l.flags & 0xffffff) << 8);
  w.Write_u32(v);
  w.Write_u32((UINT32) l.temp_sym);
}

bool Decode_LABEL(IR_READER &r, LABEL &out, UINT16 ver) {
  if (ver != SECVER_LABEL) return false;
  out.name_idx = r.Read_u32();
  UINT32 v = r.Read_u32();
  out.kind  = (LABEL_KIND)(v & 0xff);
  out.flags = (v >> 8) & 0xffffff;
  out.temp_sym = r.Read_u32();
  return true;
}

// =============================================================================
//                                 PREG
// =============================================================================
void Encode_PREG(IR_WRITER &w, const PREG &p) {
  w.Write_u32((UINT32) p.name_idx);
  w.Write_u32(p.desire_reg_num);
}

bool Decode_PREG(IR_READER &r, PREG &out, UINT16 ver) {
  if (ver != SECVER_PREG) return false;
  out.name_idx       = r.Read_u32();
  out.desire_reg_num = r.Read_u32();
  return true;
}

// =============================================================================
//                                 INITV
// =============================================================================
//
// INITV 一律 pad 到固定 stride: 16B body + 4B head = 20B 总长
// head: u16 kind, u16 repeat1
// body: 16B (按 kind 写实际字段, 剩余补零)
//
// 这样保证任意 INITO 数组步长固定, 便于 random access.
// =============================================================================

static const UINT32 INITV_BODY_BYTES = 16;

void Encode_INITV(IR_WRITER &w, const INITV &v) {
  w.Write_u16((UINT16) v.kind);
  w.Write_u16(v.repeat1);
  UINT64 start = w.Tell();
  switch (v.kind) {
    case INITVKIND_SYMOFF:
      w.Write_u32((UINT32) v.u.sto.st);
      w.Write_i32(v.u.sto.ofst);
      w.Write_u64(0);  // pad
      break;
    case INITVKIND_LABEL:
      w.Write_u32((UINT32) v.u.lab.lab);
      w.Write_i16((INT16) v.u.lab.flags);
      w.Write_u16((UINT16) v.u.lab.mtype);
      w.Write_u64(0);
      break;
    case INITVKIND_VAL:
    case INITVKIND_ZERO:
    case INITVKIND_ONE:
      // tcval struct: u32 tc(or mtype) + u32 repeat2  (8B); pad 8B
      w.Write_u32((UINT32) v.u.tcval.u.tc);
      w.Write_u32(v.u.tcval.repeat2);
      w.Write_u64(0);
      break;
    case INITVKIND_BLOCK:
      w.Write_u32((UINT32) v.u.blk.blk);
      w.Write_i32(v.u.blk.flags);
      w.Write_i32(v.u.blk.unused);
      w.Write_u32(0);
      break;
    case INITVKIND_PAD:
      w.Write_i32(v.u.pad.pad);
      w.Write_i32(v.u.pad.unused);
      w.Write_u64(0);
      break;
    case INITVKIND_UNK:
    default:
      w.Write_i64(v.u.c_val);
      w.Write_u64(0);
      break;
  }
  // 对齐到 INITV_BODY_BYTES
  UINT64 written = w.Tell() - start;
  AssertThat(written <= INITV_BODY_BYTES,
             ("Encode_INITV: body too big %llu", (unsigned long long) written));
  while (written < INITV_BODY_BYTES) { w.Write_u8(0); written++; }
}

bool Decode_INITV(IR_READER &r, INITV &out, UINT16 ver) {
  if (ver != SECVER_INITO) return false;
  memset(&out, 0, sizeof(INITV));
  out.kind    = (INITVKIND) r.Read_u16();
  out.repeat1 = r.Read_u16();
  UINT64 start = r.Tell();
  switch (out.kind) {
    case INITVKIND_SYMOFF: {
      out.u.sto.st   = r.Read_u32();
      out.u.sto.ofst = r.Read_i32();
      (void) r.Read_u64();
      break;
    }
    case INITVKIND_LABEL: {
      out.u.lab.lab   = r.Read_u32();
      out.u.lab.flags = r.Read_i16();
      out.u.lab.mtype = (mTYPE_ID) r.Read_u16();
      (void) r.Read_u64();
      break;
    }
    case INITVKIND_VAL:
    case INITVKIND_ZERO:
    case INITVKIND_ONE: {
      out.u.tcval.u.tc   = r.Read_u32();
      out.u.tcval.repeat2= r.Read_u32();
      (void) r.Read_u64();
      break;
    }
    case INITVKIND_BLOCK: {
      out.u.blk.blk    = r.Read_u32();
      out.u.blk.flags  = r.Read_i32();
      out.u.blk.unused = r.Read_i32();
      (void) r.Read_u32();
      break;
    }
    case INITVKIND_PAD: {
      out.u.pad.pad    = r.Read_i32();
      out.u.pad.unused = r.Read_i32();
      (void) r.Read_u64();
      break;
    }
    case INITVKIND_UNK:
    default: {
      out.u.c_val = r.Read_i64();
      (void) r.Read_u64();
      break;
    }
  }
  UINT64 read = r.Tell() - start;
  AssertThat(read <= INITV_BODY_BYTES,
             ("Decode_INITV: body too big %llu", (unsigned long long) read));
  // 跳过 padding
  while (read < INITV_BODY_BYTES) { (void) r.Read_u8(); read++; }
  return true;
}

// =============================================================================
//                                 INITO
// =============================================================================
void Encode_INITO(IR_WRITER &w, INITO &o) {
  w.Write_u32((UINT32) o.Get_sym());
  UINT32 cnt = o.Size();
  w.Write_u32(cnt);
  for (UINT32 i = 0; i < cnt; i++) {
    Encode_INITV(w, *o.Value(i));
  }
}

bool Decode_INITO(IR_READER &r, INITO &out, UINT16 ver) {
  if (ver != SECVER_INITO) return false;
  ST_IDX st = r.Read_u32();
  out.Set_sym(st);
  UINT32 cnt = r.Read_u32();
  for (UINT32 i = 0; i < cnt; i++) {
    INITV v;
    if (!Decode_INITV(r, v, ver)) return false;
    INITV_IDX nv = out.Add_value();
    *out.Value(nv) = v;
  }
  return true;
}

// =============================================================================
//                                 IRNODE
// =============================================================================
//
// IRNODE 是 72B POD (extra1=8B union, extra3=16B union 等 + opcode)
// v1 直接按 struct 字段顺序写出 union 的最大字段:
//   u32 opcode
//   8B  extra1 (按最大组合写: u32 c1 + u32 c2)
//   ?   extra3 (16B union: 写两个 u64)
// =============================================================================

void Encode_IRNODE(IR_WRITER &w, const IRNODE &n) {
  w.Write_u32((UINT32) n.opcode);
  // extra1: 8B union; 把 union 当成 2 个 u32 + 备用 8B (原 struct 是 8B 总,
  //   c1.combined 4B + c2.combined 4B), 但是为了保证 element_size 也保持值,
  //   element_size 是 INT64 = 8B, 与上面 8B 重叠 -> 这里直接按 8B 字节流
  UINT64 extra1_view = 0;
  memcpy(&extra1_view, &n.extra1, sizeof(UINT64));
  w.Write_u64(extra1_view);
  // extra3: 16B union -> 直接 2 个 u64
  UINT64 e3a = 0, e3b = 0;
  const UINT8 *p = (const UINT8 *) &n.extra3;
  memcpy(&e3a, p,     sizeof(UINT64));
  memcpy(&e3b, p + 8, sizeof(UINT64));
  w.Write_u64(e3a);
  w.Write_u64(e3b);
}

bool Decode_IRNODE(IR_READER &r, IRNODE &out, UINT16 ver) {
  if (ver != SECVER_IRNODES) return false;
  memset(&out, 0, sizeof(IRNODE));
  out.opcode = (OPCODE) r.Read_u32();
  UINT64 extra1_view = r.Read_u64();
  memcpy(&out.extra1, &extra1_view, sizeof(UINT64));
  UINT64 e3a = r.Read_u64();
  UINT64 e3b = r.Read_u64();
  UINT8 *p = (UINT8 *) &out.extra3;
  memcpy(p,     &e3a, sizeof(UINT64));
  memcpy(p + 8, &e3b, sizeof(UINT64));
  return true;
}

// =============================================================================
//                       Section-level Encoders / Decoders
// =============================================================================
//
// 这些函数假设调用方已经 Begin_section / Begin_sub_section, 自己只负责字段;
// section 簿记和 version 由更高层 (Dump_ir_file/Load_ir_file) 控制.
// =============================================================================

// ---- 通用宏：iterate global 表 ----
#define ENCODE_GLOBAL_TAB(W, TAB_ACCESS, ENC_FN)                            \
  do {                                                                      \
    UINT32 n = (TAB_ACCESS)->Length();                                      \
    (W).Write_u32(n);                                                       \
    for (UINT32 i = 0; i < n; i++) ENC_FN((W), *((TAB_ACCESS)->Get(i)));    \
  } while (0)

#define DECODE_GLOBAL_TAB(R, TAB_ACCESS, DEC_FN, VER)                       \
  do {                                                                      \
    UINT32 n = (R).Read_u32();                                              \
    /* 表的 idx 0 已由 FILE_SYMTAB ctor 创建; 其他 entries 用 Add 后用 Set 覆盖 */ \
    while ((TAB_ACCESS)->Length() < n) (TAB_ACCESS)->Add();                 \
    for (UINT32 i = 0; i < n; i++) {                                        \
      auto *obj = (TAB_ACCESS)->Get(i);                                     \
      if (!DEC_FN((R), *obj, (VER))) return;                                \
    }                                                                       \
  } while (0)

// ---- TY ----
void Encode_global_TY(IR_WRITER &w, FILE_SYMTAB *tab) {
  ENCODE_GLOBAL_TAB(w, tab->Ty(), Encode_TY);
}
void Decode_global_TY(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_TY_TAB);
  if (ver == 0) ver = SECVER_TY;
  DECODE_GLOBAL_TAB(r, tab->Ty(), Decode_TY, ver);
}

// ---- TYLIST ----
void Encode_global_TYLIST(IR_WRITER &w, FILE_SYMTAB *tab) {
  ENCODE_GLOBAL_TAB(w, tab->Tylist(), Encode_TYLIST);
}
void Decode_global_TYLIST(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_TYLIST_TAB);
  if (ver == 0) ver = SECVER_TYLIST;
  DECODE_GLOBAL_TAB(r, tab->Tylist(), Decode_TYLIST, ver);
}

// ---- ARB ----
void Encode_global_ARB(IR_WRITER &w, FILE_SYMTAB *tab) {
  ENCODE_GLOBAL_TAB(w, tab->Arb(), Encode_ARB);
}
void Decode_global_ARB(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_ARB_TAB);
  if (ver == 0) ver = SECVER_ARB;
  DECODE_GLOBAL_TAB(r, tab->Arb(), Decode_ARB, ver);
}

// ---- PU ----
void Encode_global_PU(IR_WRITER &w, FILE_SYMTAB *tab) {
  ENCODE_GLOBAL_TAB(w, tab->Pu(), Encode_PU);
}
void Decode_global_PU(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_PU_TAB);
  if (ver == 0) ver = SECVER_PU;
  DECODE_GLOBAL_TAB(r, tab->Pu(), Decode_PU, ver);
}

// ---- PU_INFO ----
void Encode_global_PU_INFO(IR_WRITER &w, FILE_SYMTAB *tab) {
  ENCODE_GLOBAL_TAB(w, tab->Pu_info(), Encode_PU_INFO);
}
void Decode_global_PU_INFO(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_PU_INFO_TAB);
  if (ver == 0) ver = SECVER_PU_INFO;
  // PU_INFO 不能 Add() 再 Set, 因为 PU_INFO 含 SCOPE; 直接 Add 出对象再覆盖字段
  PU_INFO_TABLE *t = tab->Pu_info();
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add();
  for (UINT32 i = 0; i < n; i++) {
    PU_INFO *p = t->Get(i);
    if (!Decode_PU_INFO(r, *p, ver)) return;
  }
}

// ---- ST (global only) ----
void Encode_global_ST(IR_WRITER &w, FILE_SYMTAB *tab) {
  // ST 是 RELATED_SYMTAB_ACCESS, 这里只写全局部分 (内部 tab)
  // 注意: t->Get(i) 把 i 当作 (raw<<8)|level 解码, 不能直接传 raw 计数
  // 用 Begin/End 迭代器跳过这层 IDX 编码
  ST_TABLE *t = tab->Sym();
  UINT32 n = t->Length();
  w.Write_u32(n);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) Encode_ST(w, *((ST*)(*it)));
}
void Decode_global_ST(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_ST_GLOBAL);
  if (ver == 0) ver = SECVER_ST;
  ST_TABLE *t = tab->Sym();
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add(GLOBAL_SYMTAB);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) {
    ST *s = (ST*)(*it);
    if (!Decode_ST(r, *s, ver)) return;
  }
}

// ---- LABEL (global) ----
void Encode_global_LABEL(IR_WRITER &w, FILE_SYMTAB *tab) {
  LABEL_TABLE *t = tab->Label();
  UINT32 n = t->Length();
  w.Write_u32(n);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) Encode_LABEL(w, *((LABEL*)(*it)));
}
void Decode_global_LABEL(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_LABEL_GLOBAL);
  if (ver == 0) ver = SECVER_LABEL;
  LABEL_TABLE *t = tab->Label();
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add(GLOBAL_SYMTAB);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) {
    LABEL *l = (LABEL*)(*it);
    if (!Decode_LABEL(r, *l, ver)) return;
  }
}

// ---- PREG (global) ----
void Encode_global_PREG(IR_WRITER &w, FILE_SYMTAB *tab) {
  PREG_TABLE *t = tab->Preg();
  UINT32 n = t->Length();
  w.Write_u32(n);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) Encode_PREG(w, *((PREG*)(*it)));
}
void Decode_global_PREG(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_PREG_GLOBAL);
  if (ver == 0) ver = SECVER_PREG;
  PREG_TABLE *t = tab->Preg();
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add(GLOBAL_SYMTAB);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) {
    PREG *p = (PREG*)(*it);
    if (!Decode_PREG(r, *p, ver)) return;
  }
}

// ---- INITO (global) ----
void Encode_global_INITO(IR_WRITER &w, FILE_SYMTAB *tab) {
  INITO_TABLE *t = tab->Inito();
  UINT32 n = t->Length();
  w.Write_u32(n);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) Encode_INITO(w, *((INITO*)(*it)));
}
void Decode_global_INITO(IR_READER &r, FILE_SYMTAB *tab) {
  UINT16 ver = r.Section_version(SK_INITO_GLOBAL);
  if (ver == 0) ver = SECVER_INITO;
  INITO_TABLE *t = tab->Inito();
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add(GLOBAL_SYMTAB);
  auto it = t->Begin();
  for (UINT32 i = 0; i < n; i++, ++it) {
    INITO *o = (INITO*)(*it);
    if (!Decode_INITO(r, *o, ver)) return;
  }
}

// =============================================================================
//                       Function sub-image encoder / decoder
// =============================================================================

static void Encode_local_table_ST(IR_WRITER &w, GROWING_TABLE<ST_IDX, ST> *t) {
  UINT32 n = t->Length();
  w.Write_u32(n);
  for (UINT32 i = 0; i < n; i++) Encode_ST(w, *(t->Get(i)));
}
static void Decode_local_table_ST(IR_READER &r, GROWING_TABLE<ST_IDX, ST> *t,
                                  UINT16 ver) {
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add();
  for (UINT32 i = 0; i < n; i++) {
    ST *s = t->Get(i);
    if (!Decode_ST(r, *s, ver)) return;
  }
}

static void Encode_local_table_LABEL(IR_WRITER &w,
                                     GROWING_TABLE<LABEL_IDX, LABEL> *t) {
  UINT32 n = t->Length();
  w.Write_u32(n);
  for (UINT32 i = 0; i < n; i++) Encode_LABEL(w, *(t->Get(i)));
}
static void Decode_local_table_LABEL(IR_READER &r,
                                     GROWING_TABLE<LABEL_IDX, LABEL> *t,
                                     UINT16 ver) {
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add();
  for (UINT32 i = 0; i < n; i++) {
    LABEL *l = t->Get(i);
    if (!Decode_LABEL(r, *l, ver)) return;
  }
}

static void Encode_local_table_PREG(IR_WRITER &w,
                                    GROWING_TABLE<PREG_IDX, PREG> *t) {
  UINT32 n = t->Length();
  w.Write_u32(n);
  for (UINT32 i = 0; i < n; i++) Encode_PREG(w, *(t->Get(i)));
}
static void Decode_local_table_PREG(IR_READER &r,
                                    GROWING_TABLE<PREG_IDX, PREG> *t,
                                    UINT16 ver) {
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add();
  for (UINT32 i = 0; i < n; i++) {
    PREG *p = t->Get(i);
    if (!Decode_PREG(r, *p, ver)) return;
  }
}

static void Encode_local_table_INITO(IR_WRITER &w,
                                     GROWING_TABLE<INITO_IDX, INITO> *t) {
  UINT32 n = t->Length();
  w.Write_u32(n);
  for (UINT32 i = 0; i < n; i++) Encode_INITO(w, *(t->Get(i)));
}
static void Decode_local_table_INITO(IR_READER &r,
                                     GROWING_TABLE<INITO_IDX, INITO> *t,
                                     UINT16 ver) {
  UINT32 n = r.Read_u32();
  while (t->Length() < n) t->Add();
  for (UINT32 i = 0; i < n; i++) {
    INITO *o = t->Get(i);
    if (!Decode_INITO(r, *o, ver)) return;
  }
}

// ---- TREE：IRNODES + 拓扑 ----
static void Encode_irnodes(IR_WRITER &w, TREE *tree_obj) {
  // 通过 Get_node(i) 间接读 _ir_elem_tab 的项
  // 我们没有 Length() API, 但可通过 Internal_tree() 来查看; 这里用直接 friend-ish
  // 走法：Tree 需要暴露 _ir_elem_tab.size()。先用 Internal_tree 推算最大 idx
  // -> 安全做法：写入所有 node, idx 从 0 开始连续
  // _ir_elem_tab 大小 == 在 tree.cxx 中 Create_node 累加, 这里假设全部都用过
  // 提供一个 helper: Get_node(i) 若 i<size 就返回非空; 我们要 size:
  IRTREE &irt = tree_obj->Internal_tree();
  // _ir_elem_tab 大小 >= irt 中所有 node_idx 的最大 + 1
  UINT64 max_idx = 0;
  bool has = false;
  for (auto it = irt.begin(); it != irt.end(); ++it) {
    if (!has || (UINT64)(*it) > max_idx) { max_idx = (UINT64)(*it); has = true; }
  }
  UINT32 count = has ? (UINT32)(max_idx + 1) : 0;
  // 但更稳的做法：把 _ir_elem_tab 全部 dump (不少于 max_idx+1)
  // 由于 TREE 没 expose size, 我们采用 max_idx+1 作为 count
  w.Write_u32(count);
  for (UINT32 i = 0; i < count; i++) {
    IRNODE *n = tree_obj->Get_node((IRNODE_IDX) i);
    if (n != NULL) {
      Encode_IRNODE(w, *n);
    } else {
      IRNODE empty;
      Encode_IRNODE(w, empty);
    }
  }
}

static void Decode_irnodes(IR_READER &r, TREE *tree_obj, UINT16 ver) {
  UINT32 count = r.Read_u32();
  // 通过 Create_node 逐个分配 idx, 然后 Decode 覆盖
  for (UINT32 i = 0; i < count; i++) {
    IRNODE_IDX idx = tree_obj->Create_node();
    IRNODE *n = tree_obj->Get_node(idx);
    if (!Decode_IRNODE(r, *n, ver)) return;
  }
}

// 拓扑: pre-order list of (parent_topo_pos, node_idx)
//       parent_topo_pos == 0xFFFFFFFF 表示 root
static void Encode_tree_topo(IR_WRITER &w, TREE *tree_obj) {
  IRTREE &irt = tree_obj->Internal_tree();
  // 收集 pre-order 顺序
  std::vector<IR_PRE_ITER> order;
  for (auto it = irt.begin(); it != irt.end(); ++it) order.push_back(it);
  UINT32 edge_count = (UINT32) order.size();
  w.Write_u32(edge_count);
  // root: 第一个 begin() 即根
  UINT32 root_node_idx = 0;
  if (edge_count > 0) root_node_idx = (UINT32)(*order[0]);
  w.Write_u32(root_node_idx);
  // 写每条 (parent_topo_pos, node_idx)
  // 为找 parent_topo_pos: 用 map<iter -> pos>
  // 由于 iterator 不可哈希, 我们用 map<node*, pos> (tree_node 内部指针)
  // tree_util::tree<T>::iterator_base 暴露 .node 指针
  // -> 借助 IRTREE::pre_order_iterator, 我们用其 .node 字段
  std::vector<void*> node_ptrs(edge_count, NULL);
  for (UINT32 i = 0; i < edge_count; i++) node_ptrs[i] = (void *) order[i].node;
  for (UINT32 i = 0; i < edge_count; i++) {
    auto parent_iter = irt.parent(order[i]);
    UINT32 parent_pos = 0xFFFFFFFFu;
    if (irt.is_valid(parent_iter)) {
      // 找 parent 在 order 中的位置
      void *pp = (void *) parent_iter.node;
      for (UINT32 j = 0; j < i; j++) {
        if (node_ptrs[j] == pp) { parent_pos = j; break; }
      }
    }
    w.Write_u32(parent_pos);
    w.Write_u32((UINT32)(*order[i]));
  }
}

static void Decode_tree_topo(IR_READER &r, TREE *tree_obj, UINT16 ver) {
  (void) ver;
  UINT32 edge_count = r.Read_u32();
  UINT32 root_node_idx = r.Read_u32();
  IRTREE &irt = tree_obj->Internal_tree();
  irt.clear();
  std::vector<IR_ITER> by_pos;
  by_pos.reserve(edge_count);
  for (UINT32 i = 0; i < edge_count; i++) {
    UINT32 parent_pos = r.Read_u32();
    UINT32 node_idx   = r.Read_u32();
    if (parent_pos == 0xFFFFFFFFu) {
      // root
      AssertThat(i == 0,
                 ("Decode_tree_topo: non-leading root at pos %u", i));
      AssertThat(node_idx == root_node_idx,
                 ("Decode_tree_topo: root mismatch %u vs %u",
                  node_idx, root_node_idx));
      IR_ITER it = irt.set_head((IRNODE_IDX) node_idx);
      by_pos.push_back(it);
    } else {
      AssertThat(parent_pos < by_pos.size(),
                 ("Decode_tree_topo: bad parent_pos %u", parent_pos));
      IR_ITER pit = by_pos[parent_pos];
      IR_ITER nit = irt.append_child(pit, (IRNODE_IDX) node_idx);
      by_pos.push_back(nit);
    }
  }
  AssertThat(irt.size() == edge_count,
             ("Decode_tree_topo: size mismatch %lu vs %u",
              irt.size(), edge_count));
}

// ---- Sub-image 整体 ----
void Encode_function_subimage(IR_WRITER &w, FILE_MANAGER *fm,
                              PU_INFO_IDX pu_info_idx) {
  PU_INFO *pu = fm->Tables()->Pu_info()->Get(pu_info_idx);
  AssertThat(pu != NULL, ("Encode_function_subimage: null pu_info"));
  // 4 张 local 表
  w.Begin_sub_section(SUBSK_LOCAL_ST, SECVER_ST);
  Encode_local_table_ST(w, pu->scope.st_tab);
  w.End_sub_section();

  w.Begin_sub_section(SUBSK_LOCAL_LABEL, SECVER_LABEL);
  Encode_local_table_LABEL(w, pu->scope.label_tab);
  w.End_sub_section();

  w.Begin_sub_section(SUBSK_LOCAL_PREG, SECVER_PREG);
  Encode_local_table_PREG(w, pu->scope.preg_tab);
  w.End_sub_section();

  w.Begin_sub_section(SUBSK_LOCAL_INITO, SECVER_INITO);
  Encode_local_table_INITO(w, pu->scope.inito_tab);
  w.End_sub_section();

  // IR nodes
  if (pu->entry != NULL) {
    w.Begin_sub_section(SUBSK_IRNODES, SECVER_IRNODES);
    Encode_irnodes(w, pu->entry);
    w.End_sub_section();

    w.Begin_sub_section(SUBSK_TREE_TOPO, SECVER_TREE_TOPO);
    Encode_tree_topo(w, pu->entry);
    w.End_sub_section();
  }
}

void Decode_function_subimage(IR_READER &r, FILE_MANAGER *fm,
                              PU_INFO_IDX pu_info_idx,
                              UINT64 abs_off, UINT64 size) {
  r.Enter_subimage(abs_off, size);
  PU_INFO *pu = fm->Tables()->Pu_info()->Get(pu_info_idx);
  // scope.Init 已经创建空 4 张 local 表 (上层调用 SCOPE::Init(proc_sym))
  // 如果还没 init, 就强制走一次:
  if (pu->scope.st_tab == NULL) pu->scope.Init(pu->Proc_sym());

  if (r.Seek_sub_section(SUBSK_LOCAL_ST)) {
    UINT16 ver = r.Sub_section_version(SUBSK_LOCAL_ST);
    Decode_local_table_ST(r, pu->scope.st_tab, ver);
  }
  if (r.Seek_sub_section(SUBSK_LOCAL_LABEL)) {
    UINT16 ver = r.Sub_section_version(SUBSK_LOCAL_LABEL);
    Decode_local_table_LABEL(r, pu->scope.label_tab, ver);
  }
  if (r.Seek_sub_section(SUBSK_LOCAL_PREG)) {
    UINT16 ver = r.Sub_section_version(SUBSK_LOCAL_PREG);
    Decode_local_table_PREG(r, pu->scope.preg_tab, ver);
  }
  if (r.Seek_sub_section(SUBSK_LOCAL_INITO)) {
    UINT16 ver = r.Sub_section_version(SUBSK_LOCAL_INITO);
    Decode_local_table_INITO(r, pu->scope.inito_tab, ver);
  }
  if (pu->entry == NULL) {
    pu->entry = new TREE();
    // 不调用 Initialize: Initialize 会预创建 FUNC_ENTRY/BLOCK 三个节点, 与
    // 后面 Decode_irnodes 顺序冲突
  }
  if (r.Seek_sub_section(SUBSK_IRNODES)) {
    UINT16 ver = r.Sub_section_version(SUBSK_IRNODES);
    // 重置: 清空 IRNODE 数组 + 内部 tree topology, 让 Decode 从空开始
    delete pu->entry;
    pu->entry = new TREE();   // 不 Initialize
    Decode_irnodes(r, pu->entry, ver);
  }
  if (r.Seek_sub_section(SUBSK_TREE_TOPO)) {
    UINT16 ver = r.Sub_section_version(SUBSK_TREE_TOPO);
    Decode_tree_topo(r, pu->entry, ver);
  }
  r.Leave_subimage();
}
