// =============================================================================
// CLAUDE-MARKER  STATUS: 新建
// =============================================================================
// IR_WRITER / IR_READER 实现
//
// Step 1: 原语 (u8/16/32/64, blob, str, Tell, Seek)
// Step 2: Header + section table
// Step 3: STRTAB 编解码
// ... (Step 8 sub-image, Step 9 入口)
//
// 编码字节序：固定 little-endian，跨架构兼容。
// =============================================================================

#include "ir_io.h"
#include "ir_serial.h"
#include "symtab.h"
#include "tree.h"
#include "options.h"
#include <string.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------
//                                IR_WRITER
// -----------------------------------------------------------------------------
IR_WRITER::IR_WRITER()
    : _fp(NULL), _own_fp(false),
      _in_section(false), _cur_kind(0), _cur_version(0),
      _cur_section_start(0),
      _in_sub_section(false), _cur_sub_kind(0), _cur_sub_version(0),
      _cur_sub_section_start(0),
      _header_offset(0), _header_written(false) {}

IR_WRITER::IR_WRITER(const char *path)
    : _fp(NULL), _own_fp(false),
      _in_section(false), _cur_kind(0), _cur_version(0),
      _cur_section_start(0),
      _in_sub_section(false), _cur_sub_kind(0), _cur_sub_version(0),
      _cur_sub_section_start(0),
      _header_offset(0), _header_written(false) {
  Open(path);
}

IR_WRITER::IR_WRITER(FILE *fp)
    : _fp(fp), _own_fp(false),
      _in_section(false), _cur_kind(0), _cur_version(0),
      _cur_section_start(0),
      _in_sub_section(false), _cur_sub_kind(0), _cur_sub_version(0),
      _cur_sub_section_start(0),
      _header_offset(0), _header_written(false) {}

IR_WRITER::~IR_WRITER() {
  Close();
}

bool IR_WRITER::Open(const char *path) {
  AssertThat(_fp == NULL, ("IR_WRITER::Open: already open"));
  _fp = fopen(path, "wb");
  if (_fp == NULL) {
    Is_Trace(TRUE, (TFile, "[IR_WRITER] failed to open %s for write\n", path));
    return false;
  }
  _own_fp = true;
  return true;
}

void IR_WRITER::Close() {
  if (_fp != NULL && _own_fp) {
    fclose(_fp);
  }
  _fp = NULL;
  _own_fp = false;
}

// ---------- 原语 ----------
void IR_WRITER::Write_u8(UINT8 v) {
  AssertThat(_fp != NULL, ("IR_WRITER::Write_u8: file not open"));
  size_t r = fwrite(&v, 1, 1, _fp);
  AssertThat(r == 1, ("IR_WRITER::Write_u8: short write"));
}

void IR_WRITER::Write_u16(UINT16 v) {
  UINT8 buf[2];
  buf[0] = (UINT8)(v & 0xff);
  buf[1] = (UINT8)((v >> 8) & 0xff);
  size_t r = fwrite(buf, 1, 2, _fp);
  AssertThat(r == 2, ("IR_WRITER::Write_u16: short write"));
}

void IR_WRITER::Write_u32(UINT32 v) {
  UINT8 buf[4];
  buf[0] = (UINT8)(v & 0xff);
  buf[1] = (UINT8)((v >> 8) & 0xff);
  buf[2] = (UINT8)((v >> 16) & 0xff);
  buf[3] = (UINT8)((v >> 24) & 0xff);
  size_t r = fwrite(buf, 1, 4, _fp);
  AssertThat(r == 4, ("IR_WRITER::Write_u32: short write"));
}

void IR_WRITER::Write_u64(UINT64 v) {
  UINT8 buf[8];
  for (int i = 0; i < 8; i++) {
    buf[i] = (UINT8)((v >> (i * 8)) & 0xff);
  }
  size_t r = fwrite(buf, 1, 8, _fp);
  AssertThat(r == 8, ("IR_WRITER::Write_u64: short write"));
}

void IR_WRITER::Write_blob(const void *p, size_t n) {
  if (n == 0) return;
  AssertThat(_fp != NULL, ("IR_WRITER::Write_blob: file not open"));
  size_t r = fwrite(p, 1, n, _fp);
  AssertThat(r == n, ("IR_WRITER::Write_blob: short write %zu/%zu", r, n));
}

void IR_WRITER::Write_str(const char *s) {
  if (s == NULL) {
    Write_u32(0);
    return;
  }
  UINT32 len = (UINT32) strlen(s);
  Write_u32(len);
  if (len > 0) Write_blob(s, len);
}

void IR_WRITER::Write_strtab_buffer(const char *buf, UINT64 used) {
  Write_u64(used);
  if (used > 0 && buf != NULL) Write_blob(buf, (size_t) used);
}

UINT64 IR_WRITER::Tell() {
  AssertThat(_fp != NULL, ("IR_WRITER::Tell: file not open"));
  long pos = ftell(_fp);
  AssertThat(pos >= 0, ("IR_WRITER::Tell: ftell failed"));
  return (UINT64) pos;
}

void IR_WRITER::Pad_to_alignment(UINT32 align) {
  if (align <= 1) return;
  UINT64 pos = Tell();
  UINT64 aligned = (pos + align - 1) / align * align;
  while (pos < aligned) { Write_u8(0); pos++; }
}

void IR_WRITER::Write_raw_at(UINT64 off, const void *p, size_t n) {
  AssertThat(_fp != NULL, ("IR_WRITER::Write_raw_at: file not open"));
  long save = ftell(_fp);
  AssertThat(save >= 0, ("IR_WRITER::Write_raw_at: ftell save failed"));
  int rc = fseek(_fp, (long) off, SEEK_SET);
  AssertThat(rc == 0, ("IR_WRITER::Write_raw_at: fseek failed"));
  size_t w = fwrite(p, 1, n, _fp);
  AssertThat(w == n, ("IR_WRITER::Write_raw_at: short write"));
  rc = fseek(_fp, save, SEEK_SET);
  AssertThat(rc == 0, ("IR_WRITER::Write_raw_at: fseek restore failed"));
}

// ---------- Header / Section 簿记 (Step 2) ----------
void IR_WRITER::Write_header() {
  AssertThat(!_header_written, ("IR_WRITER::Write_header: already written"));
  _header_offset = Tell();
  // 40 字节 header；section_table_offset / section_count 先写 0，Finish 回填
  Write_u32(IR_FILE_MAGIC);
  Write_u16(IR_FILE_FORMAT_VERSION);
  Write_u16(IR_FILE_FLAG_LITTLE_ENDIAN | IR_FILE_FLAG_HAS_STRTAB);
  Write_u64(0);  // section_table_offset (回填)
  Write_u32(0);  // section_count (回填)
  Write_u32(0);  // target_arch_id
  Write_u64(0);  // source_file_strtab_idx
  Write_u64(0);  // reserved
  _header_written = true;
}

void IR_WRITER::Begin_section(UINT16 kind, UINT16 version) {
  AssertThat(!_in_section, ("IR_WRITER::Begin_section: already in section"));
  AssertThat(_subimage_stack.empty(),
             ("IR_WRITER::Begin_section: cannot begin file-section inside subimage; use Begin_sub_section"));
  _in_section = true;
  _cur_kind = kind;
  _cur_version = version;
  _cur_section_start = Tell();
}

void IR_WRITER::End_section() {
  AssertThat(_in_section, ("IR_WRITER::End_section: not in section"));
  UINT64 cur = Tell();
  SECTION_ENTRY e;
  e.kind    = _cur_kind;
  e.version = _cur_version;
  e.flags   = 0;
  e.offset  = _cur_section_start;
  e.size    = cur - _cur_section_start;
  _sections.push_back(e);
  _in_section = false;
}

void IR_WRITER::Finish() {
  AssertThat(!_in_section, ("IR_WRITER::Finish: section not closed"));
  AssertThat(_header_written, ("IR_WRITER::Finish: header not written"));
  AssertThat(_subimage_stack.empty(), ("IR_WRITER::Finish: subimage open"));

  UINT64 sec_table_off = Tell();
  for (size_t i = 0; i < _sections.size(); i++) {
    const SECTION_ENTRY &e = _sections[i];
    Write_u16(e.kind);
    Write_u16(e.version);
    Write_u32(e.flags);
    Write_u64(e.offset);
    Write_u64(e.size);
  }
  // 回填 header 中的 sec_table_off / count
  // header 的字节布局：u32 magic(0), u16 ver(4), u16 flags(6), u64 sec_off(8),
  //                   u32 sec_count(16), u32 arch(20), u64 src_strtab(24),
  //                   u64 reserved(32)  -> 40 字节
  UINT8 sec_off_bytes[8];
  UINT64 v = sec_table_off;
  for (int i = 0; i < 8; i++) sec_off_bytes[i] = (UINT8)((v >> (i * 8)) & 0xff);
  Write_raw_at(_header_offset + 8, sec_off_bytes, 8);

  UINT8 cnt_bytes[4];
  UINT32 cnt = (UINT32) _sections.size();
  for (int i = 0; i < 4; i++) cnt_bytes[i] = (UINT8)((cnt >> (i * 8)) & 0xff);
  Write_raw_at(_header_offset + 16, cnt_bytes, 4);

  fflush(_fp);
}

// ---------- Sub-image (Step 8) ----------
void IR_WRITER::Begin_subimage() {
  AssertThat(!_in_sub_section, ("IR_WRITER::Begin_subimage: still in sub-section"));
  // 注: 一个 sub-image 一定属于某个 file-level section (例如 SK_FUNC_BLOB),
  // 所以此时 _in_section 通常为 true; 不强制要求.
  SUBIMAGE_FRAME f;
  f.start_offset = Tell();
  _subimage_stack.push_back(f);
}

void IR_WRITER::End_subimage() {
  AssertThat(!_in_sub_section, ("IR_WRITER::End_subimage: sub-section open"));
  AssertThat(!_subimage_stack.empty(), ("IR_WRITER::End_subimage: no subimage"));
  SUBIMAGE_FRAME f = _subimage_stack.back();
  _subimage_stack.pop_back();
  // 在尾部写 sub section table，再写 16-byte trailer:
  //   u32 sub_count, u32 reserved, u64 sub_table_off (相对 sub-image 起点)
  UINT64 sub_table_off_abs = Tell();
  for (size_t i = 0; i < f.sub_sections.size(); i++) {
    const SECTION_ENTRY &e = f.sub_sections[i];
    Write_u16(e.kind);
    Write_u16(e.version);
    Write_u32(e.flags);
    Write_u64(e.offset);   // 相对 sub-image 起点
    Write_u64(e.size);
  }
  UINT64 sub_table_off_rel = sub_table_off_abs - f.start_offset;
  Write_u32((UINT32) f.sub_sections.size());
  Write_u32(0);
  Write_u64(sub_table_off_rel);
}

void IR_WRITER::Begin_sub_section(UINT16 kind, UINT16 version) {
  AssertThat(!_subimage_stack.empty(),
             ("IR_WRITER::Begin_sub_section: not inside a subimage"));
  AssertThat(!_in_sub_section,
             ("IR_WRITER::Begin_sub_section: already in sub-section"));
  _in_sub_section = true;
  _cur_sub_kind = kind;
  _cur_sub_version = version;
  _cur_sub_section_start = Tell();
}

void IR_WRITER::End_sub_section() {
  AssertThat(_in_sub_section, ("IR_WRITER::End_sub_section: not in sub-section"));
  AssertThat(!_subimage_stack.empty(),
             ("IR_WRITER::End_sub_section: subimage stack empty"));
  UINT64 cur = Tell();
  SUBIMAGE_FRAME &top = _subimage_stack.back();
  SECTION_ENTRY e;
  e.kind    = _cur_sub_kind;
  e.version = _cur_sub_version;
  e.flags   = 0;
  e.offset  = _cur_sub_section_start - top.start_offset;
  e.size    = cur - _cur_sub_section_start;
  top.sub_sections.push_back(e);
  _in_sub_section = false;
}

// -----------------------------------------------------------------------------
//                                IR_READER
// -----------------------------------------------------------------------------
IR_READER::IR_READER()
    : _fp(NULL), _own_fp(false), _header_loaded(false),
      _file_format_version(0), _flags(0), _target_arch_id(0),
      _source_file_strtab_idx(0), _file_size(0) {}

IR_READER::IR_READER(const char *path)
    : _fp(NULL), _own_fp(false), _header_loaded(false),
      _file_format_version(0), _flags(0), _target_arch_id(0),
      _source_file_strtab_idx(0), _file_size(0) {
  Open(path);
}

IR_READER::IR_READER(FILE *fp)
    : _fp(fp), _own_fp(false), _header_loaded(false),
      _file_format_version(0), _flags(0), _target_arch_id(0),
      _source_file_strtab_idx(0), _file_size(0) {}

IR_READER::~IR_READER() { Close(); }

bool IR_READER::Open(const char *path) {
  AssertThat(_fp == NULL, ("IR_READER::Open: already open"));
  _fp = fopen(path, "rb");
  if (_fp == NULL) {
    Is_Trace(TRUE, (TFile, "[IR_READER] failed to open %s\n", path));
    return false;
  }
  _own_fp = true;
  fseek(_fp, 0, SEEK_END);
  long n = ftell(_fp);
  AssertThat(n >= 0, ("IR_READER::Open: ftell failed"));
  _file_size = (UINT64) n;
  fseek(_fp, 0, SEEK_SET);
  return true;
}

void IR_READER::Close() {
  if (_fp != NULL && _own_fp) fclose(_fp);
  _fp = NULL;
  _own_fp = false;
}

UINT8 IR_READER::Read_u8() {
  UINT8 v = 0;
  size_t r = fread(&v, 1, 1, _fp);
  AssertThat(r == 1, ("IR_READER::Read_u8: short read"));
  return v;
}

UINT16 IR_READER::Read_u16() {
  UINT8 buf[2];
  size_t r = fread(buf, 1, 2, _fp);
  AssertThat(r == 2, ("IR_READER::Read_u16: short read"));
  return (UINT16)(buf[0] | (buf[1] << 8));
}

UINT32 IR_READER::Read_u32() {
  UINT8 buf[4];
  size_t r = fread(buf, 1, 4, _fp);
  AssertThat(r == 4, ("IR_READER::Read_u32: short read"));
  return (UINT32)buf[0] | ((UINT32)buf[1] << 8) |
         ((UINT32)buf[2] << 16) | ((UINT32)buf[3] << 24);
}

UINT64 IR_READER::Read_u64() {
  UINT8 buf[8];
  size_t r = fread(buf, 1, 8, _fp);
  AssertThat(r == 8, ("IR_READER::Read_u64: short read"));
  UINT64 v = 0;
  for (int i = 0; i < 8; i++) {
    v |= ((UINT64) buf[i]) << (i * 8);
  }
  return v;
}

void IR_READER::Read_blob(void *p, size_t n) {
  if (n == 0) return;
  size_t r = fread(p, 1, n, _fp);
  AssertThat(r == n, ("IR_READER::Read_blob: short read %zu/%zu", r, n));
}

std::string IR_READER::Read_str() {
  UINT32 len = Read_u32();
  if (len == 0) return std::string();
  std::string s;
  s.resize(len);
  Read_blob(&s[0], (size_t) len);
  return s;
}

void IR_READER::Read_strtab_buffer(char **out_buf, UINT64 *out_used) {
  UINT64 used = Read_u64();
  if (out_used) *out_used = used;
  if (used == 0) {
    if (out_buf) *out_buf = NULL;
    return;
  }
  char *buf = (char *) malloc((size_t) used);
  AssertThat(buf != NULL, ("IR_READER::Read_strtab_buffer: oom"));
  Read_blob(buf, (size_t) used);
  if (out_buf) *out_buf = buf;
}

UINT64 IR_READER::Tell() {
  long n = ftell(_fp);
  AssertThat(n >= 0, ("IR_READER::Tell: ftell failed"));
  return (UINT64) n;
}

void IR_READER::Seek(UINT64 off) {
  int r = fseek(_fp, (long) off, SEEK_SET);
  AssertThat(r == 0, ("IR_READER::Seek: fseek failed off=%llu", (unsigned long long) off));
}

void IR_READER::Pad_to_alignment(UINT32 align) {
  if (align <= 1) return;
  UINT64 pos = Tell();
  UINT64 aligned = (pos + align - 1) / align * align;
  if (aligned != pos) Seek(aligned);
}

void IR_READER::Read_header() {
  AssertThat(!_header_loaded, ("IR_READER::Read_header: already read"));
  Seek(0);
  UINT32 magic = Read_u32();
  AssertThat(magic == IR_FILE_MAGIC,
             ("IR_READER::Read_header: bad magic 0x%08x", magic));
  _file_format_version = Read_u16();
  _flags = Read_u16();
  UINT64 sec_table_off = Read_u64();
  UINT32 sec_count     = Read_u32();
  _target_arch_id      = Read_u32();
  _source_file_strtab_idx = Read_u64();
  (void) Read_u64();   // reserved

  // 读 section table
  Seek(sec_table_off);
  _sections.clear();
  for (UINT32 i = 0; i < sec_count; i++) {
    SECTION_ENTRY e;
    e.kind    = Read_u16();
    e.version = Read_u16();
    e.flags   = Read_u32();
    e.offset  = Read_u64();
    e.size    = Read_u64();
    _sections.push_back(e);
  }
  _header_loaded = true;
}

const SECTION_ENTRY *IR_READER::Find_entry(
    const std::vector<SECTION_ENTRY> &v, UINT16 kind) {
  for (size_t i = 0; i < v.size(); i++) {
    if (v[i].kind == kind) return &v[i];
  }
  return NULL;
}

bool IR_READER::Has_section(UINT16 kind) const {
  return Find_entry(_sections, kind) != NULL;
}

bool IR_READER::Seek_section(UINT16 kind) {
  const SECTION_ENTRY *e = Find_entry(_sections, kind);
  if (e == NULL) return false;
  Seek(e->offset);
  return true;
}

UINT16 IR_READER::Section_version(UINT16 kind) const {
  const SECTION_ENTRY *e = Find_entry(_sections, kind);
  return e ? e->version : 0;
}

UINT64 IR_READER::Section_size(UINT16 kind) const {
  const SECTION_ENTRY *e = Find_entry(_sections, kind);
  return e ? e->size : 0;
}

UINT64 IR_READER::Section_offset(UINT16 kind) const {
  const SECTION_ENTRY *e = Find_entry(_sections, kind);
  return e ? e->offset : 0;
}

void IR_READER::Enter_subimage(UINT64 abs_offset, UINT64 size) {
  SUBIMAGE_FRAME f;
  f.start_offset = abs_offset;
  f.size         = size;
  f.saved_pos    = Tell();
  // 读 sub trailer (尾部 16B)
  Seek(abs_offset + size - 16);
  UINT32 sub_count = Read_u32();
  (void) Read_u32();
  UINT64 sub_table_off_rel = Read_u64();
  // 读 sub section table
  Seek(abs_offset + sub_table_off_rel);
  for (UINT32 i = 0; i < sub_count; i++) {
    SECTION_ENTRY e;
    e.kind    = Read_u16();
    e.version = Read_u16();
    e.flags   = Read_u32();
    e.offset  = Read_u64();   // 相对 abs_offset
    e.size    = Read_u64();
    f.sub_sections.push_back(e);
  }
  _subimage_stack.push_back(f);
}

void IR_READER::Leave_subimage() {
  AssertThat(!_subimage_stack.empty(), ("IR_READER::Leave_subimage: empty"));
  SUBIMAGE_FRAME f = _subimage_stack.back();
  _subimage_stack.pop_back();
  Seek(f.saved_pos);
}

bool IR_READER::Has_sub_section(UINT16 kind) const {
  AssertThat(!_subimage_stack.empty(), ("Has_sub_section: not in subimage"));
  return Find_entry(_subimage_stack.back().sub_sections, kind) != NULL;
}

bool IR_READER::Seek_sub_section(UINT16 kind) {
  AssertThat(!_subimage_stack.empty(), ("Seek_sub_section: not in subimage"));
  const SUBIMAGE_FRAME &top = _subimage_stack.back();
  const SECTION_ENTRY *e = Find_entry(top.sub_sections, kind);
  if (e == NULL) return false;
  Seek(top.start_offset + e->offset);
  return true;
}

UINT16 IR_READER::Sub_section_version(UINT16 kind) const {
  AssertThat(!_subimage_stack.empty(), ("Sub_section_version: not in subimage"));
  const SECTION_ENTRY *e = Find_entry(_subimage_stack.back().sub_sections, kind);
  return e ? e->version : 0;
}

UINT64 IR_READER::Sub_section_size(UINT16 kind) const {
  AssertThat(!_subimage_stack.empty(), ("Sub_section_size: not in subimage"));
  const SECTION_ENTRY *e = Find_entry(_subimage_stack.back().sub_sections, kind);
  return e ? e->size : 0;
}

// -----------------------------------------------------------------------------
// Maybe_dump_ir / Dump_ir_file / Load_ir_file
// -----------------------------------------------------------------------------
//
// Dump 顺序:
//   header
//   STRTAB
//   全局 TY / TYLIST / ARB / PU / PU_INFO / ST / LABEL / PREG / INITO
//   FUNC_TAB        : (pu_info_idx, sub_offset, sub_size) 数组
//   FUNC_BLOB       : 所有函数 sub-image 连续区域
//   tail section table
//
// Load 顺序:
//   FILE_SYMTAB::Initialize() (内置 MTYPE 已填好)
//   读 STRTAB 覆盖 strtab
//   读各全局 section 覆盖 _ty_tab / _st_tab / ...
//   读 FUNC_TAB, 对每个 entry Enter_subimage 覆盖 PU_INFO
// -----------------------------------------------------------------------------

void Dump_ir_file(FILE_MANAGER *fm, const char *path) {
  AssertThat(fm != NULL, ("Dump_ir_file: null fm"));
  IR_WRITER w(path);
  AssertThat(w.Is_open(), ("Dump_ir_file: cannot open '%s' for write", path));

  FILE_SYMTAB *tab = fm->Tables();
  w.Write_header();

  // STRTAB
  w.Begin_section(SK_STRTAB, SECVER_STRTAB);
  w.Write_strtab_buffer(tab->Strtab_buffer(), tab->Strtab_used());
  w.End_section();

  // Globals
  w.Begin_section(SK_TY_TAB,        SECVER_TY);        Encode_global_TY    (w, tab); w.End_section();
  w.Begin_section(SK_TYLIST_TAB,    SECVER_TYLIST);    Encode_global_TYLIST(w, tab); w.End_section();
  w.Begin_section(SK_ARB_TAB,       SECVER_ARB);       Encode_global_ARB   (w, tab); w.End_section();
  w.Begin_section(SK_PU_TAB,        SECVER_PU);        Encode_global_PU    (w, tab); w.End_section();
  w.Begin_section(SK_PU_INFO_TAB,   SECVER_PU_INFO);   Encode_global_PU_INFO(w, tab); w.End_section();
  w.Begin_section(SK_ST_GLOBAL,     SECVER_ST);        Encode_global_ST    (w, tab); w.End_section();
  w.Begin_section(SK_LABEL_GLOBAL,  SECVER_LABEL);     Encode_global_LABEL (w, tab); w.End_section();
  w.Begin_section(SK_PREG_GLOBAL,   SECVER_PREG);      Encode_global_PREG  (w, tab); w.End_section();
  w.Begin_section(SK_INITO_GLOBAL,  SECVER_INITO);     Encode_global_INITO (w, tab); w.End_section();

  // FUNC_BLOB: 所有函数的 sub-image 一字排开;
  //   Sub-image 内偏移由 IR_WRITER 在 End_subimage 时写到 sub-table.
  //   每个 sub-image 我们暂时不在外层留索引信息 (Step 9 简化版),
  //   而是另起一个 SK_FUNC_TAB section 列出 (pu_info_idx, abs_off, sz).
  std::vector<UINT64> sub_off, sub_sz;
  std::vector<UINT32> sub_pu_info_idx;
  // 从 idx=1 开始 (idx=0 是占位)
  UINT32 pu_info_count = tab->Pu_info()->Length();

  w.Begin_section(SK_FUNC_BLOB, 1);
  for (UINT32 i = 1; i < pu_info_count; i++) {
    PU_INFO *pu = tab->Pu_info()->Get(i);
    if (pu == NULL) continue;
    if (pu->proc_sym == 0)   continue;   // 占位项
    if (pu->entry == NULL)   continue;   // 没有 IR
    UINT64 off_before = w.Tell();
    w.Begin_subimage();
    Encode_function_subimage(w, fm, (PU_INFO_IDX) i);
    w.End_subimage();
    UINT64 off_after = w.Tell();
    sub_off.push_back(off_before);
    sub_sz .push_back(off_after - off_before);
    sub_pu_info_idx.push_back(i);
  }
  w.End_section();

  // FUNC_TAB
  w.Begin_section(SK_FUNC_TAB, SECVER_FUNC_TAB);
  w.Write_u32((UINT32) sub_off.size());
  for (size_t i = 0; i < sub_off.size(); i++) {
    w.Write_u32(sub_pu_info_idx[i]);
    w.Write_u64(sub_off[i]);
    w.Write_u64(sub_sz [i]);
  }
  w.End_section();

  w.Finish();
  w.Close();
  Is_Trace(TRUE, (TFile, "[IR_IO] Dump_ir_file -> %s OK\n", path));
}

void Load_ir_file(FILE_MANAGER *fm, const char *path) {
  AssertThat(fm != NULL, ("Load_ir_file: null fm"));
  IR_READER r(path);
  AssertThat(r.Is_open(), ("Load_ir_file: cannot open '%s' for read", path));
  r.Read_header();
  FILE_SYMTAB *tab = fm->Tables();

  // 1) FILE_MANAGER 的 Initialize() 已在 File() 单例首次访问时跑过
  //    (symtab.cxx:351), 这里无需也不能再调一次 — 否则会重复种入 MTYPE
  //    -> TY/strtab, 把 _ty_tab 撑过 dump 的 count, 后续 decode 覆盖不到
  //    多出的项, 留下 dangling name_idx 触发 strtab 边界 assert.

  // 2) STRTAB 覆盖
  if (r.Seek_section(SK_STRTAB)) {
    char  *buf  = NULL;
    UINT64 used = 0;
    r.Read_strtab_buffer(&buf, &used);
    tab->Strtab_replace(buf, used);
  }

  // 3) 各全局 section
  if (r.Seek_section(SK_TY_TAB))      Decode_global_TY    (r, tab);
  if (r.Seek_section(SK_TYLIST_TAB))  Decode_global_TYLIST(r, tab);
  if (r.Seek_section(SK_ARB_TAB))     Decode_global_ARB   (r, tab);
  if (r.Seek_section(SK_PU_TAB))      Decode_global_PU    (r, tab);
  if (r.Seek_section(SK_PU_INFO_TAB)) Decode_global_PU_INFO(r, tab);
  if (r.Seek_section(SK_ST_GLOBAL))   Decode_global_ST    (r, tab);
  if (r.Seek_section(SK_LABEL_GLOBAL))Decode_global_LABEL (r, tab);
  if (r.Seek_section(SK_PREG_GLOBAL)) Decode_global_PREG  (r, tab);
  if (r.Seek_section(SK_INITO_GLOBAL))Decode_global_INITO (r, tab);

  // 4) FUNC_TAB + sub-images
  if (r.Seek_section(SK_FUNC_TAB)) {
    UINT32 n = r.Read_u32();
    std::vector<UINT32> pu_info_idxs(n);
    std::vector<UINT64> offs(n);
    std::vector<UINT64> szs(n);
    for (UINT32 i = 0; i < n; i++) {
      pu_info_idxs[i] = r.Read_u32();
      offs[i]         = r.Read_u64();
      szs [i]         = r.Read_u64();
    }
    for (UINT32 i = 0; i < n; i++) {
      Decode_function_subimage(r, fm, pu_info_idxs[i], offs[i], szs[i]);
    }
  }
  r.Close();
  Is_Trace(TRUE, (TFile, "[IR_IO] Load_ir_file <- %s OK\n", path));
}

void Maybe_dump_ir(const char *stage, COMPILER_CONFIG &conf, FILE_MANAGER *fm) {
  if (conf.dump_ir_stages.empty()) return;
  for (size_t i = 0; i < conf.dump_ir_stages.size(); i++) {
    const std::string &st = conf.dump_ir_stages[i].first;
    const std::string &p  = conf.dump_ir_stages[i].second;
    if (st != stage) continue;
    Dump_ir_file(fm, p.c_str());
    if (conf.dump_textual_after_dump) {
      std::string txt = p + ".txt";
      FILE *fp = fopen(txt.c_str(), "w");
      if (fp != NULL) { fm->Print(fp); fclose(fp); }
    }
  }
}
