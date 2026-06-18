#ifndef _OCC_LRA_H
#define _OCC_LRA_H

#include "basic.h"
#include "tn.h"
#include "cg_basic.h"
#include "cgir.h"
#include "graph.h"
#include "graph_coloring.h"

extern void Print_tn_set_named(FILE *file, const char *name, const TN_SET &tns);

struct TN_LIVE_RANGE {
    TN_IDX tn;
    INT32  first_def_bb;     // BB of first definition
    INT32  first_def_stmt;   // Statement index of first definition  
    INT32  last_use_bb;      // BB of last use
    INT32  last_use_stmt;    // Statement index of last use
    BOOL   is_global;         // Spans multiple BBs?
};

// Or alternatively, flat with mapping
struct StmtLiveness {
  CFG_BB_IDX bb;
  UINT32 stmt;
  TN_SET live_in;
  TN_SET live_out;
};

/**
 * Live range analyze
 */
class CG_LIVE_RANGE {
private:
  CGIR *_cgir = nullptr;
  ST_IDX                     _cur_sym;
  TN_SET_VEC                 _live_ins;
  TN_SET_VEC                 _live_outs;
  // NEW: Statement-level (2D vectors)
  vector<TN_SET_VEC>         _stmt_live_ins;   // [bb][stmt]
  vector<TN_SET_VEC>         _stmt_live_outs;  // [bb][stmt]
  vector<StmtLiveness>       _stmt_liveness;
  vector<TN_SET>             _flat_live_ins;
  vector<TN_SET>             _flat_live_outs;
  vector<TN_LIVE_RANGE>      _live_ranges;
  map<TN_IDX, TN_LIVE_RANGE> _range_map;
public:
  void Init(CGIR *cg) {
    _cur_sym = 0;
    _cgir    = cg;
    AssertThat(cg != nullptr, ("Cgir must be a valid CGIR"));
    _live_ins.clear();
    _live_outs.clear();
  }
  CGIR *Cgir() {
    AssertThat(_cgir != nullptr,
      ("Cgir is not initialized in Live range analysis"));
    return _cgir;
  }
  void            Analyze_live_range(PU_INFO *info);
  void            Analyze_BB_Level(CG_CFG *cfg, UINT32 bb_cnt);
  void            Analyze_Stmt_Level(CG_CFG *cfg, UINT32 bb_cnt);
  void            Analyze_live_range_flat(PU_INFO *info);
  void            Build_live_ranges(CG_CFG *cfg, UINT32 bb_cnt);
  // 调试输出: 把每个 TN 的 live range 打到 FILE*; 无副作用
  void            Dump_debug(FILE *file = stderr);
  void            Print(FILE *file = stderr);
};


#endif