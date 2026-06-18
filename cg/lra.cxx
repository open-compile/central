//
// Created by xc5 on 2026/6/18.
// This is the IR definition for CG to use, i.e. CG-IR
//
#include "lra.h"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using std::unordered_map;
using std::unordered_set;
using std::vector;
#include <algorithm>

INLINE BOOL TR_LRA() {
  return Tracing(COMPONENT_CG_LRA, TRACE_DATA);
}

void Print_tn_set_named(FILE *file, const char *name, const TN_SET &tns) {
  fprintf(file, "%s={", name);
  Print_tn_set(file, tns);
  fprintf(file, "}");
}

static void Print_lra_separator(FILE *file, const char *title) {
  fprintf(file, "========== CG-LRA %s ==========\n", title);
}

static void Print_lra_bb_edges(FILE *file, const char *name, CGBB *bb) {
  fprintf(file, "%s=", name);
  if ((name[0] == 'p' && bb->Get_preds_count() == 0) ||
      (name[0] == 's' && bb->Get_succs_count() == 0)) {
    fprintf(file, "-");
    return;
  }

  BOOL first = TRUE;
  if (name[0] == 'p') {
    for (auto it = bb->Pred_begin(); it != bb->Pred_end(); ++it) {
      fprintf(file, "%sBB%u", first ? "" : ",", (*it)->Get_id());
      first = FALSE;
    }
  } else {
    for (auto it = bb->Succ_begin(); it != bb->Succ_end(); ++it) {
      fprintf(file, "%sBB%u", first ? "" : ",", (*it)->Get_id());
      first = FALSE;
    }
  }
}

static void Print_lra_bb_summary(FILE *file, CG_CFG *cfg, UINT32 bb,
                                 const TN_SET &live_in,
                                 const TN_SET &live_out) {
  CGBB *cgbb = cfg->BB(bb);
  fprintf(file, "BB%-3u stmts=%-3u ", bb, cgbb->Get_stmt_count());
  Print_lra_bb_edges(file, "preds", cgbb);
  fprintf(file, " ");
  Print_lra_bb_edges(file, "succs", cgbb);
  fprintf(file, " ");
  Print_tn_set_named(file, "defs", cgbb->Defs());
  fprintf(file, " ");
  Print_tn_set_named(file, "uses", cgbb->Uses());
  fprintf(file, " ");
  Print_tn_set_named(file, "in", live_in);
  fprintf(file, " ");
  Print_tn_set_named(file, "out", live_out);
  fprintf(file, "\n");
}


/**
 * Analyze function-level live range for CGIR;
 * @param info current function
 *
 * 标准 RA 算法流水线:
 *   ┌──────────────────────────┐
 *   │ Live Range Analysis (本函数) │  1. 反向 dataflow, 求每个 TN 的 [d, u]
 *   └──────────────────────────┘
 *              │ 输出: live_in[BB] / live_out[BB] / TN.live_range
 *              ▼
 *   ┌──────────────────────────┐
 *   │ Build Interference Graph  │  2. TN a ─ TN b iff live_range[a] ∩ live_range[b] ≠ ∅
 *   └──────────────────────────┘
 *              │ 输出: IFG (无向图)
 *              ▼
 *   ┌──────────────────────────┐
 *   │ Graph Coloring (Chaitin / │  3. K-coloring (K = #available regs); 不够则 spill
 *   │ Briggs / Linear Scan)     │
 *   └──────────────────────────┘
 *
 * 详细 spec 见 docs/cg.spec.md §3 / §4
 */
void CG_LIVE_RANGE::Analyze_BB_Level(CG_CFG *cfg, UINT32 bb_cnt) {
  // TODO(live-range): 当前是 stub; 真正的实现需要:
  //   1. 反向数据流分析 (Reverse Dataflow Analysis):
  //      - gen[BB] = TN defined in BB
  //      - kill[BB] = TN used in BB
  //      - live_in[BB] = use[BB] ∪ (live_out[BB] - def[BB])
  //      - live_out[BB] = ∪_{succ} live_in[succ]
  //   2. 走不动点直到稳定
  //   3. 给每个 TN 拼出 [first_def, last_use] 的区间
  // 见 cg.spec.md
  AssertThat(cfg != nullptr, ("CGIR cfg not initialized"));

  Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_DEBUG),
           (TFile, "[CG-LRA] BB-level analysis: bb_count=%u\n", bb_cnt));

  // Clean up data structure for re-calculation          
  _live_ins.clear();           
  _live_ins.resize(bb_cnt);

  _live_outs.clear();
  _live_outs.resize(bb_cnt);

  INT32 iter_cnt = 0;

  AssertThat(_live_ins.size()  == bb_cnt, 
             ("Incorrect size for _live_ins, size = %u, should be %u", 
              _live_ins.size(), bb_cnt));
  AssertThat(_live_outs.size() == bb_cnt, 
             ("Incorrect size for _live_outs, size = %u, should be %u", 
              _live_outs.size(), bb_cnt));

  // LRA Algorithm
  BOOL changed = TRUE;
  while(changed) {
    changed = FALSE;
    iter_cnt++;
    // reverse order
    for (INT32 cur_bb = (INT32)bb_cnt - 1; cur_bb >= 0; cur_bb--) {
      CFG_BB_EDGES  &succ     = cfg->Succs(cur_bb);
      UINT32         succ_cnt = succ.size();
      
      Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_DEBUG),
               (TFile, "[CG-LRA][iter %d] visit BB%d succ_count=%u\n",
                iter_cnt, cur_bb, succ_cnt));

      TN_SET old_in   = _live_ins[cur_bb]; // copy
      TN_SET old_out  = _live_outs[cur_bb]; // copy

      // live_out(n) = U for all succ, live_in(succ)
      // live_in(n)  = use U (live_out - def)
      _live_outs[cur_bb].clear();

      for (CFG_BB_EDGES::iterator it = succ.begin(); it != succ.end(); it++) {
        CFG_BB_IDX one_succ = *it;
        AssertThat(one_succ < bb_cnt, ("succ of cgbb is invalid cur_bb = %u, succ_bb = %u, total_bbs = %u", 
                                       cur_bb, one_succ, bb_cnt));
        _live_outs[cur_bb].insert(_live_ins[one_succ].begin(), _live_ins[one_succ].end());
      }
      _live_ins[cur_bb] = cfg->BB(cur_bb)->Uses(); // copy

      const TN_SET &defs = cfg->BB(cur_bb)->Defs();
      TN_SET second_half = _live_outs[cur_bb];
      for (TN_IDX d : defs) {
        second_half.erase(d);
      }
      _live_ins[cur_bb].insert(second_half.begin(), second_half.end());

      // Check if changed
      if (old_in != _live_ins[cur_bb] || old_out != _live_outs[cur_bb]) {
        changed = TRUE;
        if (Tracing(COMPONENT_CG_LRA, TRACE_DEBUG)) {
          fprintf(TFile, "[CG-LRA][iter %d] changed ", iter_cnt);
          Print_lra_bb_summary(TFile, cfg, cur_bb, _live_ins[cur_bb],
                               _live_outs[cur_bb]);
        }
      } 
    } // end for(BB)

    AssertThat(iter_cnt < MAX_LRA_ITER_ATTEMPT, ("LRA iteration loops indefinitely"));

  } // end while(changed)

  // After iteration
  if(Tracing(COMPONENT_CG_LRA, TRACE_DEBUG)) {
    fprintf(TFile, "[CG-LRA] BB-level fixed point reached in %d iterations\n",
            iter_cnt);
  }
  // visit all bb.
}


void CG_LIVE_RANGE::Analyze_live_range(PU_INFO *info) {
    AssertThat(info != nullptr, ("PU_INFO is null"));
    AssertThat(Cgir()->Cfg() != nullptr, ("CGIR cfg not initialized"));

    CGIR *cgir = Cgir();
    CG_CFG *cfg = cgir->Cfg();
    UINT32 bb_cnt = cgir->Cfg()->Size();

    Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_INFO),
             (TFile, "[CG-LRA] Analyze_live_range, cfgbb count = %u \n", bb_cnt));

    _cur_sym = info->Proc_sym();
    AssertThat(_cur_sym == Cgir()->Current_func_sym(), 
               ("Cfg and lra is out of sync"));

    // Phase 1: BB-level analysis (keep your existing code)
    Analyze_BB_Level(cfg, bb_cnt);
    
    // Phase 2: Statement-level analysis (NEW)
    Analyze_Stmt_Level(cfg, bb_cnt);
    
    // Phase 3: Build live ranges per TN (NEW)
    Build_live_ranges(cfg, bb_cnt);

    if (Tracing(COMPONENT_CG_LRA, TRACE_DEBUG)) {
      Dump_debug(TFile);
    }
}

void CG_LIVE_RANGE::Analyze_Stmt_Level(CG_CFG *cfg, UINT32 bb_cnt) {
  CGIR *cgir = Cgir();
  
  // Initialize statement-level structures
  _stmt_live_ins.clear();
  _stmt_live_ins.resize(bb_cnt);
  _stmt_live_outs.clear();
  _stmt_live_outs.resize(bb_cnt);
    
  for (UINT32 bb = 0; bb < bb_cnt; bb++) {
    CGBB *cur_bb = cfg->BB(bb);
    UINT32 stmt_count = cur_bb->Get_stmt_count();  // Number of statements
        
    _stmt_live_ins[bb].resize(stmt_count);
    _stmt_live_outs[bb].resize(stmt_count);
        
    if (stmt_count == 0) continue;
        
    // Start with BB live_out for the last statement
    TN_SET current_live = _live_outs[bb];  // From Phase 1
        
    // Walk backwards through statements
    for (INT32 stmt = (INT32)stmt_count - 1; stmt >= 0; stmt--) {
      CGOP *cur_stmt = cur_bb->Get_stmt(stmt);
            
      // Statement live-out
      _stmt_live_outs[bb][stmt] = current_live;
            
      // Statement live-in = uses ∪ (live_out - defs)
      TN_SET stmt_live_in = current_live;

      Is_Trace(TR_LRA(),
               (TFile, "Def-use builder: %s\n",
                       Get_cg_opc_info(cur_stmt->getOpcode())->ins_token));
          
      // Remove definitions (these are killed)
      const TN_SET &stmt_defs = cgir->Stmt_defs(cur_stmt);
      for (TN_IDX d : stmt_defs) {
        stmt_live_in.erase(d);
      }
            
      // Add uses (these become live)
      const TN_SET &stmt_uses = cgir->Stmt_uses(cur_stmt);
      stmt_live_in.insert(stmt_uses.begin(), stmt_uses.end());
            
      _stmt_live_ins[bb][stmt] = stmt_live_in;
            
      // This becomes live-out for previous statement
      current_live = stmt_live_in;
    }
        
    // Sanity check: live_in of first stmt should ⊆ BB live_in
    if (stmt_count > 0) {
      BOOL is_subset = std::includes(_live_ins[bb].begin(), _live_ins[bb].end(),
				     _stmt_live_ins[bb][0].begin(), _stmt_live_ins[bb][0].end());
      AssertThat(is_subset, ("Stmt-level live_in doesn't match BB-level, bb_idx = %d", cur_bb));
    }
  }
}



void CG_LIVE_RANGE::Build_live_ranges(CG_CFG *cfg, UINT32 bb_cnt) {
  CGIR *cgir = Cgir();
  _live_ranges.clear();
    
  // Temporary map: TN → live range info
  std::map<TN_IDX, TN_LIVE_RANGE> &range_map = _range_map;
  range_map.clear();
    
  for (UINT32 bb = 0; bb < bb_cnt; bb++) {
    CGBB *cur_bb = cfg->BB(bb);
    UINT32 stmt_count = cur_bb->Get_stmt_count();
        
    for (UINT32 stmt = 0; stmt < stmt_count; stmt++) {
      CGOP *cur_stmt = cur_bb->Get_stmt(stmt);
            
      // Track definitions (start of live range)
      const TN_SET &defs = cgir->Stmt_defs(cur_stmt);
      for (TN_IDX tn : defs) {
        auto it = range_map.find(tn);
        if (it == range_map.end()) {
          // First definition seen
          range_map[tn] = {tn, (INT32)bb, (INT32)stmt, 
                           (INT32)bb, (INT32)stmt, FALSE};
        } else {
          // Update first_def if this is earlier
          if (bb < it->second.first_def_bb ||
              (bb == it->second.first_def_bb && 
               stmt < it->second.first_def_stmt)) {
            it->second.first_def_bb = bb;
            it->second.first_def_stmt = stmt;
          }
        }
      }
            
      // Track uses from live-in set (end of live range)
      const TN_SET &live_in = _stmt_live_ins[bb][stmt];
      for (TN_IDX tn : live_in) {
        auto it = range_map.find(tn);
        if (it == range_map.end()) {
          // Used but not defined in this function (parameter)
          range_map[tn] = {tn, (INT32)bb, (INT32)stmt,
                           (INT32)bb, (INT32)stmt, FALSE};
        } else {
          // Update last_use if this is later
          if (bb > it->second.last_use_bb ||
              (bb == it->second.last_use_bb && 
               stmt > it->second.last_use_stmt)) {
            it->second.last_use_bb = bb;
            it->second.last_use_stmt = stmt;
          }
        }
      }
    }
        
    // Also check variables live-out of BB
    for (TN_IDX tn : _live_outs[bb]) {
      auto it = range_map.find(tn);
      if (it != range_map.end()) {
        // Variable lives to end of this BB
        if (bb > it->second.last_use_bb) {
          it->second.last_use_bb = bb;
          it->second.last_use_stmt = stmt_count - 1;  // Last stmt
        }
      }
    }
  }
    
  // Mark global variables
  for (auto& [tn, range] : range_map) {
    if (range.first_def_bb != range.last_use_bb) {
      range.is_global = TRUE;
    }
    _live_ranges.push_back(range);
  }
}


void CG_LIVE_RANGE::Analyze_live_range_flat(PU_INFO *info) {
  // ... setup code same as before ...

  AssertThat(info != nullptr, ("PU_INFO is null"));
  AssertThat(Cgir()->Cfg() != nullptr, ("CGIR cfg not initialized"));

  CGIR *cgir = Cgir();
  CG_CFG *cfg = cgir->Cfg();
  UINT32 bb_cnt = cgir->Cfg()->Size();

  Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_INFO),
           (TFile, "[CG-LRA] Analyze_live_range, cfgbb count = %u \n", bb_cnt));

  _cur_sym = info->Proc_sym();
  AssertThat(_cur_sym == Cgir()->Current_func_sym(),
             ("Cfg and lra is out of sync"));
  
  // Flatten all statements
  struct FlatPoint {
    UINT32 bb;
    UINT32 stmt;
    CGOP *stmt_ptr;
  };
    
  std::vector<FlatPoint> all_points;
  for (UINT32 bb = 0; bb < bb_cnt; bb++) {
    CGBB *cur_bb = cfg->BB(bb);
    for (UINT32 stmt = 0; stmt < cur_bb->Get_stmt_count(); stmt++) {
      CGOP *cur_stmt = cur_bb->Get_stmt(stmt);
      all_points.push_back({bb, stmt, cur_stmt});
    }
  }
    
  UINT32 total_points = all_points.size();
  vector<TN_SET> live_ins(total_points);
  vector<TN_SET> live_outs(total_points);
    
  // Build successor mapping for flat points
  std::vector<std::vector<UINT32>> successors(total_points);
  for (UINT32 i = 0; i < total_points; i++) {
    if (i + 1 < total_points) {
      // Check if next point is in same BB
      if (all_points[i].bb == all_points[i+1].bb) {
        successors[i].push_back(i + 1);
      } else {
        // End of BB - connect to successors' first statements
        UINT32 bb = all_points[i].bb;
        for (auto succ_bb : cfg->Succs(bb)) {
          // Find first point of succ_bb
          for (UINT32 j = 0; j < total_points; j++) {
            if (all_points[j].bb == succ_bb) {
              successors[i].push_back(j);
              break;
            }
          }
        }
      }
    }
  }
    
  // Now run dataflow on flat points (same algorithm)
  BOOL changed = TRUE;
  INT32 iter = 0;
  while (changed) {
    changed = FALSE;
    iter++;
        
    for (INT32 i = total_points - 1; i >= 0; i--) {
      TN_SET old_in = live_ins[i];
            
      // live_out = ∪ live_in(succ)
      TN_SET new_live_out;
      for (UINT32 succ : successors[i]) {
        new_live_out.insert(live_ins[succ].begin(), live_ins[succ].end());
      }
      live_outs[i] = new_live_out;
            
      // live_in = use ∪ (live_out - def)
      TN_SET new_live_in = live_outs[i];
      for (TN_IDX d : cgir->Stmt_defs(all_points[i].stmt_ptr)) {
        new_live_in.erase(d);
      }
      const TN_SET uses = cgir->Stmt_uses(all_points[i].stmt_ptr);
      new_live_in.insert(uses.begin(), uses.end());
            
      live_ins[i] = new_live_in;
            
      if (old_in != live_ins[i]) {
        changed = TRUE;
      }
    }
  }
  _flat_live_ins = live_ins;
  _flat_live_outs = live_outs;
}

void CG_LIVE_RANGE::Dump_debug(FILE *file) {
  if (!file) file = stderr;
  CGIR *cgir = Cgir();
  CG_CFG *cfg = cgir->Cfg();
  const char *func_name = ST_st(_cur_sym) ? ST_name(_cur_sym) : "<unknown>";

  Print_lra_separator(file, "SUMMARY");
  fprintf(file, "function=%s sym=0x%08x bb_count=%u live_ranges=%lu\n",
          func_name, _cur_sym, cfg ? cfg->Size() : 0, _live_ranges.size());

  Print_lra_separator(file, "BB LIVE SETS");
  if (!cfg) {
    fprintf(file, "<no cfg>\n");
  } else {
    for (UINT32 bb = 0; bb < cfg->Size(); ++bb) {
      const TN_SET empty;
      const TN_SET &live_in = (bb < _live_ins.size()) ? _live_ins[bb] : empty;
      const TN_SET &live_out = (bb < _live_outs.size()) ? _live_outs[bb] : empty;
      Print_lra_bb_summary(file, cfg, bb, live_in, live_out);
    }
  }

  Print_lra_separator(file, "TN LIVE RANGES");
  if (_live_ranges.empty()) {
    fprintf(file, "<none>\n");
  } else {
    fprintf(file, "%-8s %-12s %-12s %-8s\n", "tn", "first_def", "last_use",
            "scope");
    for (const TN_LIVE_RANGE &range : _live_ranges) {
      fprintf(file, "t%-7u BB%d:S%d      BB%d:S%d      %s\n",
              (UINT32)range.tn,
              range.first_def_bb, range.first_def_stmt,
              range.last_use_bb, range.last_use_stmt,
              range.is_global ? "global" : "local");
    }
  }

  Print_lra_separator(file, "STMT LIVE SETS");
  if (_stmt_live_ins.empty()) {
    fprintf(file, "<none>\n");
  } else {
    for (UINT32 bb = 0; bb < _stmt_live_ins.size(); ++bb) {
      fprintf(file, "BB%u\n", bb);
      UINT32 stmt_count = _stmt_live_ins[bb].size();
      for (UINT32 stmt = 0; stmt < stmt_count; ++stmt) {
        const char *op_name = "<stmt>";
        if (cfg && bb < cfg->Size() && stmt < cfg->BB(bb)->Get_stmt_count()) {
          CGOP *op = cfg->BB(bb)->Get_stmt(stmt);
          op_name = Get_cg_opc_info(op->getOpcode())->ins_token;
        }
        fprintf(file, "  S%-3u %-12s ", stmt, op_name);
        Print_tn_set_named(file, "in", _stmt_live_ins[bb][stmt]);
        fprintf(file, " ");
        Print_tn_set_named(file, "out", _stmt_live_outs[bb][stmt]);
        fprintf(file, "\n");
      }
    }
  }

  Print_lra_separator(file, "END");
}

void CG_LIVE_RANGE::Print(FILE *file) {
  Dump_debug(file);
}
