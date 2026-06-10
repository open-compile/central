//
// ============================================================================
// CLAUDE-MARKER  STATUS: KEEP (data structure)
// ============================================================================
// 文件作用: OPT_STAB  — ST_IDX → 紧凑 AUX_ID 的映射, 加版本表 (ver_tab)
// 关键类:
//   - OPT_VAR    : 包含 st_idx / aux_id / mtype / ver_count / def_bbs (set)
//   - VER_ENTRY  : 单个版本, 包含 def_stmt / def_phi / def_bb
//   - OPT_STAB   : Add_var / Lookup / New_version / Ver / Var / Print
// 关键函数:
//   - OPT_STAB::Add_var — lazy 加新 aux
//   - OPT_STAB::New_version — push 新版本
//   - OPT_STAB::Print / Print_verbose
// 重写提示: 数据结构完整可保留; 若要做 sparse versioning (per-BB def) 可加
//           一个 BitSet<MAX_VER> 或者把 ver_tab 改成 per-BB 的 map。
// ============================================================================

#ifndef OCC_OPT_STAB_H
#define OCC_OPT_STAB_H

#include "basic.h"
#include "tree.h"
#include "symtab.h"
#include "opt_coderep.h"
#include <vector>
#include <set>
#include <map>

class STMTREP;
class PHI_NODE;

// 每个原始变量在 OPT_STAB 中的条目
struct OPT_VAR {
  ST_IDX     st_idx;
  AUX_ID     aux_id;
  MTYPE_ID   mtype;
  UINT32     ver_count;          // 已分配版本数
  std::set<SSABB *> def_bbs;     // 定义所在 BB 集合
  OPT_VAR() : st_idx(0), aux_id(0), mtype(MTYPE_UNKNOWN), ver_count(0) {}
};

// (AUX_ID, version) -> 唯一条目
struct VER_ENTRY {
  AUX_ID     aux_id;
  UINT32     version;
  STMTREP   *def_stmt;            // 二选一
  PHI_NODE  *def_phi;            // 二选一
  SSABB     *def_bb;
  CODEREP   *coderep;             // 此版本对应的 CODEREP
  VER_ENTRY()
    : aux_id(0), version(0), def_stmt(nullptr), def_phi(nullptr),
      def_bb(nullptr), coderep(nullptr) {}
};

class OPT_STAB {
public:
  OPT_STAB() : _next_aux(1) {}

  // 注册/查找变量
  AUX_ID Add_var(ST_IDX st, MTYPE_ID mtype);
  AUX_ID Lookup(ST_IDX st) const;
  BOOL   Is_known(ST_IDX st) const { return _st_to_aux.count(st) > 0; }

  // 分配新版本号（返回新版本号）
  UINT32 New_version(AUX_ID aux);

  // 访问变量 / 版本信息
  OPT_VAR  &Var(AUX_ID aux);
  const OPT_VAR &Var(AUX_ID aux) const;
  VER_ENTRY &Ver(AUX_ID aux, UINT32 ver);
  const VER_ENTRY &Ver(AUX_ID aux, UINT32 ver) const;

  UINT32 Num_vars() const { return (UINT32)_vars.size(); }
  UINT32 Num_versions(AUX_ID aux) const;
  BOOL   Ver_exists(AUX_ID aux, UINT32 ver) const;

  // 重置
  void Reset();

  // 打印
  void Print(FILE *f = stderr) const;
  void Print_verbose(FILE *f = stderr) const;

private:
  AUX_ID                                 _next_aux;
  std::vector<OPT_VAR>                   _vars;        // indexed by AUX_ID
  std::map<ST_IDX, AUX_ID>               _st_to_aux;
  // _ver_tab[aux] 是该变量的所有版本
  std::vector<std::vector<VER_ENTRY>>    _ver_tab;
};

#endif //OCC_OPT_STAB_H
