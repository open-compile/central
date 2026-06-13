// =============================================================================
// CLAUDE-MARKER  STATUS: 新建 (Step 1 起步)
// =============================================================================
// 二进制 IR Dump / Load 文件格式的低层 I/O 层
// 设计参考 plans/calm-hatching-ladybug.md，ELF 风格 (尾部 section table)
//
// Step 1 范围：仅提供 IR_WRITER / IR_READER 的原语 (u8/16/32/64 + blob + str)。
// Step 2 起会在此基础上加 Header / Section table / Sub-image 簿记。
// =============================================================================

#ifndef _OCC_IR_IO_H_
#define _OCC_IR_IO_H_

#include "host.h"
#include "basic.h"
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <utility>

// -----------------------------------------------------------------------------
// 文件常量 (Step 2 后正式启用，先在此声明便于 forward 引用)
// -----------------------------------------------------------------------------
static const UINT32 IR_FILE_MAGIC          = 0x31425249; // "IRB1" (LE)
static const UINT16 IR_FILE_FORMAT_VERSION = 1;
static const UINT16 IR_FILE_FLAG_LITTLE_ENDIAN = 0x0001;
static const UINT16 IR_FILE_FLAG_HAS_STRTAB    = 0x0002;

// 文件级 section kind
enum SECTION_KIND : UINT16 {
  SK_INVALID        = 0,
  SK_STRTAB         = 1,
  SK_TY_TAB         = 2,
  SK_TYLIST_TAB     = 3,
  SK_ARB_TAB        = 4,
  SK_ST_GLOBAL      = 5,
  SK_LABEL_GLOBAL   = 6,
  SK_PREG_GLOBAL    = 7,
  SK_INITO_GLOBAL   = 8,
  SK_PU_TAB         = 9,
  SK_PU_INFO_TAB    = 10,
  SK_FUNC_TAB       = 11,
  SK_FUNC_BLOB      = 12,
  // Phase 2 预留
  SK_TCON_TAB       = 16,
  SK_FILE_INFO_TAB  = 17,
  SK_SSA_GLOBAL     = 32,
  SK_CGIR_GLOBAL    = 48,
  SK_DEBUG_INFO     = 64,
};

// 函数 sub-image 内部的 section kind
enum SUBSECTION_KIND : UINT16 {
  SUBSK_INVALID     = 0,
  SUBSK_LOCAL_ST    = 1,
  SUBSK_LOCAL_LABEL = 2,
  SUBSK_LOCAL_PREG  = 3,
  SUBSK_LOCAL_INITO = 4,
  SUBSK_IRNODES     = 5,
  SUBSK_TREE_TOPO   = 6,
  // Phase 2 预留
  SUBSK_LOCAL_SSA   = 16,
  SUBSK_LOCAL_CGIR  = 17,
};

// -----------------------------------------------------------------------------
// SECTION_ENTRY  (24 字节)
// -----------------------------------------------------------------------------
struct SECTION_ENTRY {
  UINT16 kind;       // SECTION_KIND or SUBSECTION_KIND
  UINT16 version;    // per-section version
  UINT32 flags;      // 保留
  UINT64 offset;     // 绝对文件偏移 (sub-image 内则为 sub-image 起始的相对偏移)
  UINT64 size;       // 字节数
  SECTION_ENTRY() : kind(0), version(0), flags(0), offset(0), size(0) {}
};

// -----------------------------------------------------------------------------
// IR_WRITER  --  低层写出器
// -----------------------------------------------------------------------------
class IR_WRITER {
private:
  FILE *_fp;
  bool  _own_fp;             // 是否由本对象持有 fp，析构时要 close
  // file-level section 簿记 (Step 2 启用)
  std::vector<SECTION_ENTRY> _sections;
  bool   _in_section;
  UINT16 _cur_kind;
  UINT16 _cur_version;
  UINT64 _cur_section_start;
  // sub-section 状态 (subimage 内独立的一套，与外层 section 互不干扰)
  bool   _in_sub_section;
  UINT16 _cur_sub_kind;
  UINT16 _cur_sub_version;
  UINT64 _cur_sub_section_start;
  // sub-image 簿记 (Step 8 启用)
  struct SUBIMAGE_FRAME {
    UINT64 start_offset;                    // sub-image 在文件中的起始偏移
    std::vector<SECTION_ENTRY> sub_sections; // 内部 section 列表
  };
  std::vector<SUBIMAGE_FRAME> _subimage_stack;
  // header 偏移 (Finish 回填用)
  UINT64 _header_offset;
  bool   _header_written;

public:
  // ---- 构造 / 析构 ----
  IR_WRITER();
  explicit IR_WRITER(const char *path);   // 打开文件, "wb"
  explicit IR_WRITER(FILE *fp);           // 接管已存在 fp，不会 close
  ~IR_WRITER();

  bool Open(const char *path);
  void Close();
  FILE *Fp() const { return _fp; }
  bool Is_open() const { return _fp != NULL; }

  // ---- 原语 (固定宽度 + LE) ----
  void Write_u8 (UINT8  v);
  void Write_u16(UINT16 v);
  void Write_u32(UINT32 v);
  void Write_u64(UINT64 v);
  void Write_i8 (INT8   v) { Write_u8 ((UINT8 ) v); }
  void Write_i16(INT16  v) { Write_u16((UINT16) v); }
  void Write_i32(INT32  v) { Write_u32((UINT32) v); }
  void Write_i64(INT64  v) { Write_u64((UINT64) v); }
  void Write_blob(const void *p, size_t n);
  // u32 length + raw bytes (NOT null-terminated)
  void Write_str(const char *s);
  void Write_str(const std::string &s) { Write_str(s.c_str()); }
  // 原始 strtab 缓冲区写出 (u64 len + bytes)
  void Write_strtab_buffer(const char *buf, UINT64 used);

  UINT64 Tell();
  void   Pad_to_alignment(UINT32 align);

  // ---- Section 簿记 (Step 2 起启用) ----
  void Write_header();
  void Begin_section(UINT16 kind, UINT16 version);
  void End_section();
  void Finish();    // 写 section table, 回填 header

  // ---- Sub-image 簿记 (Step 8 起启用) ----
  void Begin_subimage();
  void End_subimage();
  void Begin_sub_section(UINT16 kind, UINT16 version);
  void End_sub_section();

private:
  void Write_raw_at(UINT64 off, const void *p, size_t n); // 回填用 (fseek)
};

// -----------------------------------------------------------------------------
// IR_READER  --  低层读取器
// -----------------------------------------------------------------------------
class IR_READER {
private:
  FILE *_fp;
  bool  _own_fp;
  // 文件级 section 索引 (Read_header 后填好)
  std::vector<SECTION_ENTRY> _sections;
  bool   _header_loaded;
  UINT32 _file_format_version;
  UINT16 _flags;
  UINT32 _target_arch_id;
  UINT64 _source_file_strtab_idx;
  UINT64 _file_size;

  // sub-image 上下文
  struct SUBIMAGE_FRAME {
    UINT64 start_offset;
    UINT64 size;
    UINT64 saved_pos;         // 进入前的位置
    std::vector<SECTION_ENTRY> sub_sections;
  };
  std::vector<SUBIMAGE_FRAME> _subimage_stack;

public:
  IR_READER();
  explicit IR_READER(const char *path);
  explicit IR_READER(FILE *fp);
  ~IR_READER();

  bool Open(const char *path);
  void Close();
  FILE *Fp() const { return _fp; }
  bool Is_open() const { return _fp != NULL; }

  // ---- 原语 ----
  UINT8  Read_u8();
  UINT16 Read_u16();
  UINT32 Read_u32();
  UINT64 Read_u64();
  INT8   Read_i8 () { return (INT8 ) Read_u8 (); }
  INT16  Read_i16() { return (INT16) Read_u16(); }
  INT32  Read_i32() { return (INT32) Read_u32(); }
  INT64  Read_i64() { return (INT64) Read_u64(); }
  void   Read_blob(void *p, size_t n);
  // 与 Write_str 对应；out_len 可为 NULL
  std::string Read_str();
  void Read_strtab_buffer(char **out_buf, UINT64 *out_used);

  UINT64 Tell();
  void   Seek(UINT64 off);
  void   Pad_to_alignment(UINT32 align);

  // ---- Section 索引 (Step 2 起启用) ----
  void Read_header();
  bool   Has_section(UINT16 kind) const;
  bool   Seek_section(UINT16 kind);     // true = 命中
  UINT16 Section_version(UINT16 kind) const;
  UINT64 Section_size   (UINT16 kind) const;
  UINT64 Section_offset (UINT16 kind) const;
  UINT16 File_format_version() const { return _file_format_version; }
  UINT16 File_flags()          const { return _flags; }

  // ---- Sub-image (Step 8 起启用) ----
  void Enter_subimage(UINT64 abs_offset, UINT64 size);
  void Leave_subimage();
  bool   Has_sub_section(UINT16 kind) const;
  bool   Seek_sub_section(UINT16 kind);
  UINT16 Sub_section_version(UINT16 kind) const;
  UINT64 Sub_section_size(UINT16 kind) const;

private:
  static const SECTION_ENTRY *Find_entry(
      const std::vector<SECTION_ENTRY> &v, UINT16 kind);
};

// -----------------------------------------------------------------------------
// 高层入口  (Step 9 接通)
// -----------------------------------------------------------------------------
class FILE_MANAGER;

// stage 名: "fe" / "opt-high" / "opt-mid" / "opt-after-ssa" /
//          "opt-low" / "opt-vlow" / "opt-cgir" / "pre-cg"
struct COMPILER_CONFIG;
void Maybe_dump_ir(const char *stage, COMPILER_CONFIG &conf, FILE_MANAGER *fm);
void Dump_ir_file(FILE_MANAGER *fm, const char *path);
void Load_ir_file(FILE_MANAGER *fm, const char *path);

#endif // _OCC_IR_IO_H_
