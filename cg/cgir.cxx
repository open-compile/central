//
// Created by xc5 on 2020/7/13.
// This is the IR definition for CG to use, i.e. CG-IR
//
#include "cgir.h"
#include "cg_main.h"
#include "target_backend.h"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using std::unordered_map;
using std::unordered_set;
using std::vector;
#include <algorithm>

INLINE BOOL TR_CGIR() {
  return Tracing(COMPONENT_CG_LRA, TRACE_DATA);
}

TN *CGIR::PREG_to_TN(TY_IDX preg_ty, PREG_NUM preg_num) {
  AssertThat(preg_num != 0, ("Preg number should not be zero."));
  if (_preg_to_tn.find(preg_num) != _preg_to_tn.end()) {
    return TN_tn(_preg_to_tn[preg_num]);
  }
  PREG *preg = PREG_preg(preg_num);
  TN_IDX base_idx = 0;
  if (preg->getDesireRegNum() == 1) {
    // This is return val.
    TN *base = nullptr;
    if (Cgmon()->Target().abi.return_register_is_argument_zero) {
      base = Gen_Register_TN(ISA_REGISTER_CLASS_integer, MTYPE_size(MTYPE_I4));
      Set_TN_is_preallocated(base);
      Set_TN_register(base, 0);
    } else {
      base = Build_Dedicated_TN(REGISTER_CLASS_v0, REGISTER_v0,
                                MTYPE_size(MTYPE_I4));
    }
    base_idx = TN_tn_idx(base);
  } else {
    base_idx = Gen_TN(MTYPE_I4);
    Set_TN_is_gra_cannot_split(TN_tn(base_idx));
  }
  _preg_to_tn.insert(std::make_pair(preg_num, base_idx));
  return TN_tn(base_idx);
}

TN *CGIR::PREG_to_ST_TN(ST_IDX sym_idx, PREG_NUM preg_num) {
  AssertThat(false, ("PREG_to_ST_TN is not implemented"));
  return nullptr;
}

void Print_tn_set(FILE *file, const TN_SET &tns) {
  if (tns.empty()) {
    fprintf(file, "-");
    return;
  }
  UINT32 pos = 0;
  for (auto tn : tns) {
    fprintf(file, "t%u%s", (UINT32)tn, (++pos == tns.size()) ? "" : ",");
  }
}

void Print_tn_set_vec(FILE *file, const TN_SET_VEC &tnvecs) {
  UINT32 sets_cnt = tnvecs.size();
  if (tnvecs.empty()) {
    fprintf(file, "- [EMPTY-VEC]-");
    return;
  }
  fprintf(file, "[Vec] rows = %d => ", sets_cnt);
  for (UINT32 i = 0; i < sets_cnt; i++) {
    fprintf(file, "[Row=%u] ", i);
    const TN_SET &tns = tnvecs[i];
    if (tns.empty()) {
      fprintf(file, "-");
    }
    UINT32 pos = 0;
    for (auto tn : tns) {
      fprintf(file, "t%u%s", (UINT32)tn, (++pos == tns.size()) ? "" : ",");
    }
    if (i != sets_cnt - 1) fprintf(file, ", ");
  }
}


// for future compatibility with SIMD/SIMT
TN_SET CGIR::Stmt_defs(CGOP *cgop) {
  TN_SET defs = {};
  CGOPC_INFO *opc_info = Get_cg_opc_info(cgop->getOpcode());
  UINT32 res_count = opc_info->getNRes();
  CGOPR_KIND slot_kinds[3] = {
    opc_info->getOp1(), opc_info->getOp2(), opc_info->getOp3()
  };

  AssertThat(res_count  <= 1, ("operand count must be 0 or 1."));

  for (UINT32 slot = 0; slot < 3; ++slot) {
    if (slot_kinds[slot] == CGOPR_N) continue;
    // this is reading all non-zero operands by default here.
    TN_IDX cgoper = cgop->getResOpnd()[slot];
    TN    *tn     = this->TN_tn(cgoper);
    if (cgoper == 0) continue;

    // check if this is constant
        
    if (slot < res_count && opc_info->isWriteToRd()) {
      AssertThat((!TN_is_label(tn)) && (!TN_is_constant(tn)), ("Invalid tn on res %s", (cgop->Print(TFile), "")));
      defs.insert(cgoper);
    } else {
      // check if it is constant/label. these two need no allocation
      if (TN_is_constant(tn) || TN_is_label(tn)) {
	continue;
      }
      // TN_is_dedicated(tn) || TN_is_preallocated(tn) are going to be calculated as use.
      //uses.insert(cgoper);
    }
  }
  return defs;
}


TN_SET CGIR::Stmt_uses(CGOP *cgop) {
  TN_SET uses = {};
  CGOPC_INFO *opc_info = Get_cg_opc_info(cgop->getOpcode());
  UINT32 res_count = opc_info->getNRes();
  CGOPR_KIND slot_kinds[3] = {
    opc_info->getOp1(), opc_info->getOp2(), opc_info->getOp3()
  };

  AssertThat(res_count  <= 1, ("operand count must be 0 or 1."));

  for (UINT32 slot = 0; slot < 3; ++slot) {
    if (slot_kinds[slot] == CGOPR_N) continue;
    // this is reading all non-zero operands by default here.
    TN_IDX cgoper = cgop->getResOpnd()[slot];
    TN    *tn     = this->TN_tn(cgoper);
    if (cgoper == 0) continue;

    // check if this is constant
        
    if (slot < res_count && opc_info->isWriteToRd()) {
      AssertThat((!TN_is_label(tn)) && (!TN_is_constant(tn)), ("Invalid tn on res %s", (cgop->Print(TFile), "")));
      // def not use.
    } else {
      // check if it is constant/label. these two need no allocation
      if (TN_is_constant(tn) || TN_is_label(tn)) {
	continue;
      }
      // TN_is_dedicated(tn) || TN_is_preallocated(tn) are going to be calculated as use.
      uses.insert(cgoper);
    }
  }
  return uses;
}


/**
 * Setting up CGIR preparation,
 * @param cgir CGIR module
 * @param sym Current function's PU symbol
 */
void CGIR::Set_current_cgir(CG_CFG *cgir, ST_IDX sym) {
  _current = cgir;
  _current_sym = sym;
  _preg_to_tn.clear();
  _current_tree = PU_INFO_pu_info(File()->Tables()->Get_pu_info_by_st_idx(sym))->entry;
  Set_current_layout(Layouts()[sym]);
}

/**
 * Find the symbol represented by the TN.
 * @param sym
 * @return
 */
TN_IDX CGIR::Get_TN_from_symbol(ST_IDX sym) {
  TY_IDX ty_idx = ST_ty(sym);
  AssertThat(MTYPE_to_ty(MTYPE_I4) == ty_idx,
             ("Should be a I4 type, rather, it's %d ", ty_idx));
  return Gen_TN(MTYPE_I4);
}

/**
 * Find the TN mapped by the ir, in the map(cached)
 * @param node
 * @param cur_bb
 * @return
 */
TN_IDX CGIR::Get_TN_by_ir_node(IR_ITER node, CFG_BB_IDX cur_bb) {
  // TODO: This is only for testing. change this later.
  return Gen_TN(MTYPE_I4);
}

/**
 * Global register allocation
 * @param info Current function
 */
void CG_REG_ALLOC::Register_allocate(PU_INFO *info) {
  Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_INVOCATION),
           (TFile, "CGIR::Local_register_allocate \n"));
  AssertThat(info != nullptr, ("PU_INFO is null"));
  AssertThat(Cgir()->Cfg() != nullptr, ("CGIR cfg not initialized"));
  CGIR *cgir = this->Cgir();
  // Count registers needed.
  UINT32 i32_register_needed = 0;
  UINT32 bb_cnt = cgir->Cfg()->Size();
  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cfg()->BB(i);
    // Tracings
    Is_Trace(TR_CGIR(), (TFile, " --------- Processing BB : %d ---------  \n", i));
    _tn_freq_map.clear();
    // If there is a label to it, emit the label
    UINT32 stmt_id = 0;
    for (auto stmt_it = cgbb->Begin_stmt(); stmt_it != cgbb->End_stmt(); stmt_it++, stmt_id++) {
      CGOP *cgop = (*stmt_it);
      Is_Trace(TR_CGIR(),
               (TFile, "LRA: Processing op = %s\n",
                Get_cg_opc_info(cgop->getOpcode())->ins_token));
      // TODO(step1: count-uses): 这里是 step 1 — 扫描所有 CGOP, 统计每个 TN
      //   的 use/def 数, 并把它塞进 _tn_freq_map / _tn_live_range.
      //   真正的 RA 流水线要把这一步的结果:
      //     - "每个 TN 被 use 几次" → 用来算 spill cost (cost = uses / degree)
      //     - "每个 TN 在哪些 BB 里被 use" → 用来构造 IFG
      //   见 cg.spec.md §4.4
      if (Get_cg_opc_info(cgop->getOpcode())->n_res >= 1) {
        i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 0);
      }
      if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 1) {
        i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 1);
      }
      if (Get_cg_opc_info(cgop->getOpcode())->n_oprs >= 2) {
        i32_register_needed += Count_needed_register(cgop, stmt_id, CGOPR_R, i, 2);
      }
    }

    /*** LRA ***/
    Is_Trace(TR_CGIR(),
             (TFile, "Found %lu registers to allocate for \n", _tn_freq_map.size()));

    // Build interference graph.
    // TODO(step2: ifg-build): 在真正实现时, 这里应该:
    //   1. 先调 Cgir()->Reg_lra().Analyze_live_range(info) 算出每个 TN 的 live range
    //      (反向 dataflow, 见 cg.spec.md §3)
    //   2. 再建 IFG: TN a ─ TN b iff live_range[a] ∩ live_range[b] ≠ ∅
    //      数据结构: 一个 unordered_map<TN_IDX, vector<TN_IDX>> 或邻接矩阵
    //      (图小就用矩阵, 图大用 adjacency list)
    //   3. IFG 是无向图, 用于下一步 Chaitin-Briggs coloring
    //
    //   当前 _tn_freq_map 只是一个 use-频次表, 不能替代 IFG.
    //   没有 IFG, 你不知道两个 TN 是否冲突 → 给它们同色 = 错误.
    //
    //   真正的 LRA 需要:
    //   1. 调 Analyze_live_range 计算每个 TN 的 live range
    //   2. 建 IFG (节点=TN, 边=live-range 重叠)
    //   3. 走 Chaitin / Briggs 启发式 graph coloring
    //   4. 选 K 不到时做 spill
    //   现在是 naive linear scan, 见 cg.spec.md
    //   2. 构造 interference graph: TN a, b 干扰 iff 它们的 live range 重叠
    //   3. 走 Chaitin / Briggs 启发式 graph coloring
    //   4. 选 K 不到时做 spill
    // 现在是 naive linear scan, 见 cg.spec.md

    // This is actually global register allocation.
    //
    // TODO(step3: coloring): 当前是 naive linear scan: next_register 一直 ++.
    //   这是错的, 因为:
    //     - 没看 IFG, 可能把两个 live-range 重叠的 TN 分到同一寄存器 (互相覆盖)
    //     - spill 一刀切 (next_register >= 7 就全 spill), 没有 cost model
    //   正确做法: 在 step 2 建的 IFG 上跑 Chaitin-Briggs (见 cg.spec.md §4.2):
    //     1. Simplify 阶段: 把 degree < K 的节点压栈
    //     2. Spill 阶段: 选 cost/degree 最低的候选压栈
    //     3. Select 阶段: 弹栈, 给每个节点分配第一个可用颜色
    //   K 的取值 (ARMv7 AAPCS integer pool):
    //     - 全部 caller/callee save 可用: {r0,r1,r2,r3,r4,r5,r6,r7,r8,r10,r11} = 11 个
    //     - 严格 AAPCS: caller save {r0-r3} = 4, callee save {r4-r8,r10,r11} = 7
    //     - 调用约定: 跨 call 的 TN 优先用 callee save
    REGISTER_SET used = 0;
    REGISTER_SET_EmptyP(used);
    UINT32 next_register = 1; // Use r0 for return value
    for (auto tn_freq : _tn_freq_map) {
      TN_IDX tid = tn_freq.first;
      TN *tn = TN_tn(tid);
      AssertThat(tn != nullptr, ("tn is null for tid=%u", tid));
      AssertThat(TN_register(tn) == 0,
                 ("TN: %d should not be pre-allocated. cur register = %d",
                  tid, TN_register(tn)));
      // TODO(step3.1: 选色): 真正实现时, 这里应该:
      //   color = pick_color(tn, IFG, K, available_colors)
      //   而 pick_color 要看 tn 的邻居们 (degree) 都用了什么颜色
      // Allocate one-by-one
      vector<UINT32> &ded = Cfg()->BB(i)->Get_dedicate_regs();
      if (TN_is_gra_cannot_split(tn)){
        // TODO(step4.1: forced-spill): 这个 TN 不能切 live range (e.g. PREG),
        //   必须 spill. 真正的实现也要记录: 为啥不能切 (call 实参 / 特殊约束)
        // PREG, must spill here.
        Spill_tn(tid, tn);
      } else {
        if (next_register >= 7) {
          // TODO(step4.2: heuristic-spill): 当前 spill 是 "寄存器用光就全 spill".
          //   真正做法 (Chaitin spill heuristics, 见 cg.spec.md §4.4):
          //     cost(tn) = Σ_{op ∈ uses/defs} (10 ^ in_loop(op))  /  degree(tn)
          //     选 cost 最低 (spill 影响最小) 的节点 spill
          //   还应该考虑:
          //     - 跨 call 的 TN 用 callee-save (r4-r11) 优先, 这样 spill 少
          //     - 常数 TN 用 rematerialization, 不 spill
          //     - loop-invariant 优先放在 callee-save (跨 call 不丢)
          /* Spill all now. */
          Spill_tn(tid, tn);
        } else {
          Is_Trace(TR_CGIR(), (TFile, "LRA: Assigning reg %d to TN : %d\n", next_register, tid));
          Cfg()->BB(i)->Get_dedicate_regs().push_back(next_register);
          Set_TN_is_preallocated(tn);
          Set_TN_register_class(tn, ISA_REGISTER_CLASS_integer);
          Set_TN_register(tn, next_register);
          while (std::find(ded.begin(), ded.end(), next_register) != ded.end() &&
                 next_register < 8) {
            next_register++;
          }
        }
      }
    }
  }
  // Re-add spilling etc.,
  // Allocate all registers, mark spilling info.
  //
  // TODO(step5: spill-rewrite): coloring 阶段选出 spill 候选 TN 后,
  //   这一步要把它们"重写"为 load/store 序列:
  //     - 在 def 之前插 STR tn, [sp, #spill_offset]
  //     - 在 use 之后插 LDR tn, [sp, #spill_offset]
  //   重写后需要重新计算:
  //     - 因为新增了 CGOP, IFG 变了
  //     - 需要回到 step 2 重新做 IFG + coloring
  //   真正的 RA 是迭代的: coloring → spill → 改 CGOP → 回到 coloring, 直到收敛.
  //
  //   当前实现只对 TN_SPILL flag 已置位的 TN 做处理 (line ~1112),
  //   真正的实现应该是: 把 step 3 / 4 选出的 spill 候选都 rewrite 一遍.
  for (UINT32 i = 0; i < bb_cnt; i++) {
    CGBB *cgbb = Cfg()->BB(i);
    // Tracings
    Is_Trace(TR_CGIR(), (TFile, " --------- Post-LRA-Spill BB : %d ---------  \n", i));
    // If there is a label to it, emit the label
    UINT32 stmt_cnt = cgbb->Get_stmt_count();
    UINT32 stmt_id = 0;
    for (auto stmt_it = cgbb->Begin_stmt(); stmt_id  < stmt_cnt; stmt_id++) {
      CGOP *cgop = (*(cgbb->Begin_stmt() + stmt_id));
      Is_Trace(TR_CGIR(),
               (TFile, "Processing : %s\n", Get_cg_opc_info(cgop->getOpcode())->ins_token));
      if (cgop->getFlags() & CGOPF_SPILL) {
        continue;
      }
      if (Get_cg_opc_info(cgop->getOpcode())->getNRes() >= 1) {
        Process_spill_op(cgop, CGOPR_R, i, 0, Get_cg_opc_info(cgop->getOpcode())->isWriteToRd());
      }
      if (Get_cg_opc_info(cgop->getOpcode())->getNOprs() >= 1) {
        Process_spill_op(cgop, CGOPR_R, i, 1, false);
      }
      if (Get_cg_opc_info(cgop->getOpcode())->getNOprs() >= 2) {
        Process_spill_op(cgop, CGOPR_R, i, 2, false);
      }
    }
  }

  // for (UINT32 i = 0; i < bb_cnt; i++) {
  //   CGBB *cgbb = Cfg()->BB(i);
  //   auto work_list = cgbb->Get_work_list();
  //   for (auto work_item : work_list) {
  //     if (work_item.getPutBefore()) {
  //       cgbb->Move_stmt_to_before(work_item.getTarget(), work_item.getFrom());
  //     } else {
  //       cgbb->Move_stmt_to_after(work_item.getTarget(), work_item.getFrom());
  //     }
  //   }
  // }
}

void CG_REG_ALLOC::Spill_tn(TN_IDX tid, TN *tn) {// We could put the spill on r8.
// Now we have at least the R9 to process
  Is_Trace(TR_CGIR(),
           (TFile, "Need to spill the TN %d \n", tid));
  // Make TN = ... to Var(.spill) = ...
  Set_TN_register(tn, 0); // not allocating right now, wait for second pass.
  Set_TN_is_preallocated(tn);
  // Allocate spill space.
// Create var, create object space.
  ST_IDX sym = 0; //File()->Find_symbol_by_name(".spill");
  char   name_str[100];
  sprintf(name_str, ".spill_%d", tid);
  STR_IDX name = File()->Save_string(name_str);
  sym = File()->Create_var(name, MTYPE_to_ty(MTYPE_I4), LOCAL_SYMTAB,
                           SYMC_AUTO, SYME_INTERNAL, SYM_CLASS_VAR);
  Cgir()->Layout()->Allocate_object(sym); // re-allocate this.
  Set_TN_flags(tn, TN_SPILL);
  Set_TN_spill(tn, sym);
  Set_TN_register_class(tn, ISA_REGISTER_CLASS_integer);
}

// 调试输出: 把当前 _tn_freq_map 打到 FILE*; 无副作用
void CG_REG_ALLOC::Print_freq_map(FILE *file) {
  if (!file) file = stderr;
  fprintf(file, "%s CG_REG_ALLOC::_tn_freq_map (%lu entries) \n%s",
          DBAR, _tn_freq_map.size(), DBAR);
  for (auto &p : _tn_freq_map) {
    TN *tn = TN_tn(p.first);
    fprintf(file, "  TN %u : freq = %u, class = %d, reg = %d, dedicated = %d, prealloc = %d\n",
            p.first, p.second,
            TN_register_class(tn), TN_register(tn),
            TN_is_dedicated(tn), TN_is_preallocated(tn));
  }
  fprintf(file, "%s end \n%s", DBAR, DBAR);
}

// 调试输出: 把每个 TN 的 live range (bb_stmt 序列) 打到 FILE*
void CG_REG_ALLOC::Print_live_range(FILE *file) {
  if (!file) file = stderr;
  fprintf(file, "%s CG_REG_ALLOC::_tn_live_range (%lu entries) \n%s",
          DBAR, _tn_live_range.size(), DBAR);
  for (auto &p : _tn_live_range) {
    fprintf(file, "  TN %u : live points =", p.first);
    for (auto bs : p.second) {
      UINT32 bb = (UINT32)((bs >> 32) & 0xFFFFFFFF);
      UINT32 stmt = (UINT32)(bs & 0xFFFFFFFF);
      fprintf(file, " (bb=%u,stmt=%u)", bb, stmt);
    }
    fprintf(file, "\n");
  }
  fprintf(file, "%s end \n%s", DBAR, DBAR);
}

static UINT32 CGBB_stmt_count(CGBB *bb) {
  UINT32 count = 0;
  for (auto it = bb->Begin_stmt(); it != bb->End_stmt(); ++it) {
    count++;
  }
  return count;
}

static void Print_cgbb_flags(FILE *file, UINT32 flags) {
  BOOL any = FALSE;
#define PRINT_BB_FLAG(flag, name) \
  if (flags & flag) { fprintf(file, "%s%s", any ? "|" : "", name); any = TRUE; }
  PRINT_BB_FLAG(BB_FLAG_ENTRY, "entry");
  PRINT_BB_FLAG(BB_FLAG_EXIT, "exit");
  PRINT_BB_FLAG(BB_FLAG_HANDLER, "handler");
  PRINT_BB_FLAG(BB_FLAG_CALL, "call");
  PRINT_BB_FLAG(BB_FLAG_LABEL, "label");
  PRINT_BB_FLAG(BB_FLAG_UNREACH, "unreach");
  PRINT_BB_FLAG(BB_FLAG_SCHED, "sched");
  PRINT_BB_FLAG(BB_FLAG_SPILL, "spill");
  PRINT_BB_FLAG(BB_FLAG_LRA, "lra");
#undef PRINT_BB_FLAG
  if (!any) fprintf(file, "-");
}

static void Print_tn_compact(CGIR *cgir, TN_IDX tn_idx, FILE *file) {
  if (tn_idx == 0 || tn_idx >= cgir->TN_tab_size()) {
    fprintf(file, "-");
    return;
  }

  TN *tn = cgir->TN_tn(tn_idx);
  fprintf(file, "t%u", tn_idx);
  if (TN_is_constant(tn)) {
    if (TN_is_label(tn)) {
      LABEL_IDX lbl = TN_label(tn);
      fprintf(file, "<label:%s>", lbl ? LABEL_name(lbl) : "0");
    } else {
      fprintf(file, "<#%lld>", TN_value(tn));
    }
    return;
  }

  if (TN_is_register(tn)) {
    fprintf(file, "<r%d,c%d", TN_register(tn), TN_register_class(tn));
    if (TN_is_preallocated(tn)) fprintf(file, ",pre");
    if (TN_is_dedicated(tn)) fprintf(file, ",ded");
    if (TN_flags(tn) & TN_SPILL) fprintf(file, ",spill=0x%x", TN_spill(tn));
    fprintf(file, ">");
  }
}

static void Print_cgop_compact(CGIR *cgir, CGOP *op, FILE *file) {
  CGOPC_INFO *info = Get_cg_opc_info(op->getOpcode());
  CGOPR_KIND slot_kinds[3] = {
    info->getOp1(), info->getOp2(), info->getOp3()
  };
  fprintf(file, "    [%02u] %-12s tree=%u",
          op->getIndexInBb(), ISA_OPCODE_name(op->getOpcode()),
          op->getTreeNodeId());

  fprintf(file, "  slots:");
  BOOL any_slot = FALSE;
  for (UINT8 slot = 0; slot < 3; ++slot) {
    if (slot_kinds[slot] == CGOPR_N || op->getResOpnd()[slot] == 0) continue;
    fprintf(file, " %u=", slot);
    Print_tn_compact(cgir, op->getResOpnd()[slot], file);
    if (slot < op->getResults() && info->isWriteToRd()) {
      fprintf(file, "(def)");
    } else {
      fprintf(file, "(use)");
    }
    any_slot = TRUE;
  }
  if (!any_slot) fprintf(file, " -");

  if (op->getFlags() != 0) {
    fprintf(file, "  flags=0x%x", op->getFlags());
  }
  fprintf(file, "\n");
}

void CGIR::Print(FILE *file) {
  // Print by functions.
  fprintf(file, "%sPrinting the complete CGIR with %lu functions \n%s",
          DBAR, this->trees.size(), DBAR);
  for (auto iter : this->trees) {
    AssertThat(ST_st(iter.first) != nullptr, ("invalid ST_IDX in the cgir map"));
    fprintf(file, "%sPrinting the CGIR's function : name = %s, sym = 0x%08x\n%s",
            DBAR, ST_name(iter.first), iter.first, DBAR);
    iter.second->Print(file);
  }
}

void CGIR::Print_cfg_detail(ST_IDX sym, FILE *file) {
  AssertThat(ST_st(sym) != nullptr, ("invalid function ST_IDX to print in CGIR"));
  CG_CFG *cfg = Get_cg_cfg(sym);
  fprintf(file, "%sCG CFG detail: %s (sym=0x%08x), BBs=%u\n%s",
          DBAR, ST_name(sym), sym, cfg->Size(), DBAR);

  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    CGBB *bb = cfg->Node(i);
    LABEL_IDX lbl = bb->Get_label_id();
    fprintf(file, "BB%u", bb->Get_id());
    if (lbl != 0) fprintf(file, " label=%s(0x%04x)", LABEL_name(lbl), lbl);
    fprintf(file, " flags=");
    Print_cgbb_flags(file, bb->Get_flags());
    fprintf(file, " stmts=%u\n", CGBB_stmt_count(bb));

    fprintf(file, "  preds:");
    if (bb->Get_preds_count() == 0) fprintf(file, " -");
    for (auto it = bb->Pred_begin(); it != bb->Pred_end(); ++it) {
      fprintf(file, " BB%u", (*it)->Get_id());
    }
    fprintf(file, "\n");

    fprintf(file, "  succs:");
    if (bb->Get_succs_count() == 0) fprintf(file, " -");
    for (auto it = bb->Succ_begin(); it != bb->Succ_end(); ++it) {
      fprintf(file, " BB%u", (*it)->Get_id());
    }
    fprintf(file, "\n");
    fprintf(file, "  BB defs: ");
    Print_tn_set(file, bb->Defs());
    fprintf(file, ", BB uses: ");
    Print_tn_set(file, bb->Uses());
    fprintf(file, "\n");

    BOOL has_stmt_def_use = bb->Has_stmt_def_use();
    if (!has_stmt_def_use) {
      fprintf(file,
              "  stmt def/use unavailable: stmt cache is stale after CG rewrites\n");
    }

    UINT32 stmt_idx = 0;
    for (auto it = bb->Begin_stmt(); it != bb->End_stmt(); ++it, ++stmt_idx) {
      (*it)->setIndexInBb(stmt_idx);
      fprintf(file, "    def/use: [");
      if (has_stmt_def_use) {
        Print_tn_set(file, bb->Stmt_defs(stmt_idx));
      } else {
        fprintf(file, "?");
      }
      fprintf(file, "] / [");
      if (has_stmt_def_use) {
        Print_tn_set(file, bb->Stmt_uses(stmt_idx));
      } else {
        fprintf(file, "?");
      }
      fprintf(file, "]\n");
      Print_cgop_compact(this, *it, file);
    }
  }
  fprintf(file, "%sEnd CG CFG detail\n%s", DBAR, DBAR);
}

void CGIR::Print_cfg_graph(ST_IDX sym, FILE *file) {
  AssertThat(ST_st(sym) != nullptr, ("invalid function ST_IDX to print in CGIR"));
  CG_CFG *cfg = Get_cg_cfg(sym);
  fprintf(file, "%sCG CFG ASCII graph: %s (sym=0x%08x)\n%s",
          DBAR, ST_name(sym), sym, DBAR);

  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    CGBB *bb = cfg->Node(i);
    LABEL_IDX lbl = bb->Get_label_id();
    fprintf(file, "BB%u", bb->Get_id());
    if (lbl != 0) fprintf(file, "[%s]", LABEL_name(lbl));
    fprintf(file, " {stmts=%u, flags=", CGBB_stmt_count(bb));
    Print_cgbb_flags(file, bb->Get_flags());
    fprintf(file, "}\n");

    if (bb->Get_succs_count() == 0) {
      fprintf(file, "  `-> <exit>\n");
    } else {
      INT32 succ_pos = 0;
      for (auto it = bb->Succ_begin(); it != bb->Succ_end(); ++it, ++succ_pos) {
        const char *edge = (succ_pos + 1 == bb->Get_succs_count()) ? "`->" : "|->";
        fprintf(file, "  %s BB%u\n", edge, (*it)->Get_id());
      }
    }
  }

  fprintf(file, "\nEdges:\n");
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    CGBB *bb = cfg->Node(i);
    if (bb->Get_succs_count() == 0) {
      fprintf(file, "  BB%u -> <exit>\n", bb->Get_id());
      continue;
    }
    for (auto it = bb->Succ_begin(); it != bb->Succ_end(); ++it) {
      fprintf(file, "  BB%u -> BB%u\n", bb->Get_id(), (*it)->Get_id());
    }
  }
  fprintf(file, "%sEnd CG CFG ASCII graph\n%s", DBAR, DBAR);
}

void CGIR::Print_tn_table(ST_IDX sym, FILE *file, BOOL function_only) {
  AssertThat(ST_st(sym) != nullptr, ("invalid function ST_IDX to print in CGIR"));
  INT32 begin = function_only ? _func_tn_begin : 0;
  fprintf(file, "%sCG TN table: %s (sym=0x%08x), range=[%d,%u)\n%s",
          DBAR, ST_name(sym), sym, begin, TN_tab_size(), DBAR);
  for (INT32 i = TN_tab_size() - 1; i >= begin; --i) {
    TN *tn_obj = TN_tn(i);
    fprintf(file, "[TN %-8d, 0x%04x]  ", i, i);
    tn_obj->Print(file);
  }
  fprintf(file, "%sEnd CG TN table\n%s", DBAR, DBAR);
}

void CGIR::Print(ST_IDX sym, FILE *file) {
  // Print a functions detail.
  AssertThat(ST_st(sym) != nullptr, ("invalid function ST_IDX to print in CGIR"));
  fprintf(file, "%s Printing the CGIR's function with sym : name = %s, sym = 0x%08x\n%s",
          DBAR, ST_name(sym), sym, DBAR);
  Get_cg_cfg(sym)->Print_cg_cfg(file);

  if (Tracing(COMPONENT_CG_CONV, TRACE_DEBUG)) {
    Print_tn_table(sym, file, FALSE);
  } else if (Tracing(COMPONENT_CG_CONV, TRACE_DATA)) {
    Print_tn_table(sym, file, TRUE);
  }
}


OPCODE OPCODE_make_op(OPERATOR opr, MTYPE_ID res, MTYPE_ID desc) {
  return (OPCODE) (opr + RTYPE(res) + DESC(desc));
}

/**
 * Generating the LOAD/STORE instructions for LDID/STID/ILOAD/ISTORE...
 * @param opc
 * @param mtype
 * @param src_res_tn
 * @param base_tn
 * @param sym
 * @param ofst_val
 * @param bb_idx
 * @param variant
 */
void CGIR_BUILDER::Exp_load_store (
  OPCODE opc,
  MTYPE_ID mtype,
  TN *src_res_tn,
  TN *base, // only for ILOAD/ISTORE
  ST_IDX sym,
  INT64 ofst_val,
  IR_ITER node,
  CFG_BB_IDX bb_idx,
  VARIANT variant)
{
  UINT32 node_id = 0;
  if (node != nullptr) {
    node_id = *node;
  }
  TN *src_res  = src_res_tn;
  BOOL need_recalibrate = false;
  INT64 offset_from_base = ofst_val;
  CGOPC top = CGOPC_STR;
  Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
           (TFile, "Exp_LDST sym = %u, ofst = %lld\n", sym, ofst_val));
  if (base != nullptr) {
    // ISTORE or ILOAD case, where base and offset are known.
    // nothing to do.
  } else if (sym != 0 && ST_symclass(sym) == SYM_CLASS_PREG) {
    // Either return-val or just a intermediate number.
    PREG_IDX pgid = ofst_val;
    AssertThat(pgid != 0, ("Preg number should not be zero."));
    PREG *preg = PREG_preg(pgid);
    if (preg->getDesireRegNum() == 1) {
      // This is return val.
      base = Gen_Register_TN(ISA_REGISTER_CLASS_integer, MTYPE_size(MTYPE_I4));
      Set_TN_is_preallocated(base);
      Set_TN_register(base, 0);
      top = CGOPC_ADD;
    } else {
      // Just another TN.
      base = Cgir()->PREG_to_TN(ST_ty(sym), ofst_val);
      top = CGOPC_ADD;
    }
    if (opc == OPC_I4STID) {
      // Reverse the base and res_src.
      TN *mid = base;
      base = src_res;
      src_res = mid;
    }

    offset_from_base = 0;
  } else if (sym != 0 && ST_sclass(sym) != SYMC_AUTO && ST_sclass(sym) != SYMC_FORMAL) {
    // Create a LDR first
    base = TN_tn(Gen_TN(MTYPE_I4));
    LABEL_IDX lbl = Get_addr_label(sym);
    CGOP      *node = new CGOP(CGOPC_LDRLBL, node_id, bb_idx, TN_tn_idx(base),
                               TN_tn_idx(Gen_Label_TN(lbl, 0)), 0, 0);
    Cfg()->BB(bb_idx)->Add_stmt(node);
    _spill_related.push_back(node);
  } else {
    base = Build_Dedicated_TN(REGISTER_CLASS_sp,
                              REGISTER_sp,
                              MTYPE_size(MTYPE_I4));
    offset_from_base = Cgir()->Layout()->Get_sym_sp_ofst(sym);
    need_recalibrate = true;
  }
  AssertThat(base != nullptr, ("Base cannot be null here."));
  TN *ofst = Gen_Literal_TN(offset_from_base, 4);
  if (need_recalibrate) {
    Cfg()->Get_recalibrate_map().insert(std::make_pair(TN_tn_idx(ofst), sym));
  }
  if (top == CGOPC_ADD) {
    // do nothing, continue on.
  } else if (OPCODE_operator(opc) == OPR_STID) {
    top = CGOPC_STR;
  } else if (OPCODE_operator(opc) == OPR_LDID) {
    top = CGOPC_LDR;
  } else if (OPCODE_operator(opc) == OPR_ISTORE) {
    top = CGOPC_STR;
  } else if (OPCODE_operator(opc) == OPR_ILOAD) {
    top = CGOPC_LDR;
  } else {
    AssertThat(false, ("not impl ldst opcode = %s.", OPCODE_name(opc)));
  }
  AssertThat(TN_is_constant(ofst), ("Exp_LDST: Illegal offset TN"));
  if (!TN_has_value(ofst) || TN_value(ofst) < (1 << 8)) {
    AssertThat(src_res != nullptr && base != nullptr && ofst != nullptr, ("No operands should be null."));
    Cfg()->BB(bb_idx)->Add_stmt(
      new CGOP(top, node_id, bb_idx, TN_tn_idx(src_res), TN_tn_idx(base),
               TN_tn_idx(ofst), 0));
  } else {
    // Somehow calculate a middle number,
    TN *middle_base = TN_tn(Gen_TN(MTYPE_I4));
    CGOP *mov_1     = new CGOP(CGOPC_MOV, node_id, bb_idx,
                               TN_tn_idx(middle_base),
                               TN_tn_idx(
                                 Gen_Literal_TN((offset_from_base) & 0xFFFF,
                                                2)), 0, 0);
    Cfg()->BB(bb_idx)->Add_stmt(mov_1);
    _spill_related.push_back(mov_1);
    if (((offset_from_base >> 16) & 0xFFFF) != 0) {
      CGOP *mov2 = new CGOP(CGOPC_MOVT, node_id, bb_idx, TN_tn_idx(middle_base),
                            TN_tn_idx(
                              Gen_Literal_TN((offset_from_base >> 16) & 0xFFFF,
                                             2)), 0, 0);
      Cfg()->BB(bb_idx)->Add_stmt(mov2);
      _spill_related.push_back(mov2);
    }
    CGOP *add_op = new CGOP(CGOPC_ADD, node_id, bb_idx, TN_tn_idx(middle_base),
             TN_tn_idx(base), TN_tn_idx(middle_base), 0);
    _spill_related.push_back(add_op);
    Cfg()->BB(bb_idx)->Add_stmt(add_op);
    TN_value(ofst) = 0;
    Cfg()->Get_exceed_map().insert(std::make_pair(TN_tn_idx(ofst), offset_from_base));
    CGOP *real_op = new CGOP(top, node_id, bb_idx, TN_tn_idx(src_res),
                             TN_tn_idx(middle_base),
                             TN_tn_idx(ofst), 0);
    Cfg()->BB(bb_idx)->Add_stmt(real_op);
    _spill_related.push_back(real_op);
  }
}

void CGIR_BUILDER::Exp_op(UINT32 expr_id,
                  OPCODE opcode, CFG_BB_IDX cur_bb,
                  TN *result, TN *op1, TN *op2, TN *op3,
                  VARIANT variant, CGOP **ops) {
  AssertThat(ops != NULL, ("Nowhere to put results"));
  CGOPC cgop = CGOPC_NOP;
  switch (OPCODE_operator(opcode)) {
    case OPR_ADD: { cgop = CGOPC_ADD; break; }
    case OPR_MPY: { cgop = CGOPC_MUL; break; }
    case OPR_DIV: { cgop = CGOPC_DIV; break; }
    case OPR_SUB: { cgop = CGOPC_SUBS; break; }
    case OPR_LAND: { cgop = CGOPC_AND; break; }
    case OPR_BIOR: { cgop = CGOPC_ORR; break; }
    case OPR_LNOT: { cgop = CGOPC_MVN; break; }
    default: {
      AssertThat(false, ("Exp_op some opcode = %s not impl.", OPCODE_name(opcode)));
    }
  }
  AssertThat(result != NULL, ("Result should not be null."));
  AssertThat(op1 != NULL, ("OP1 should not be null."));
  if (op2 != nullptr) {
    AssertThat(op2 != NULL, ("OP2 should not be null."));
    CGOP *stmt_ins = new CGOP(cgop, expr_id, cur_bb, TN_tn_idx(result), TN_tn_idx(op1),
                              TN_tn_idx(op2), 0);
    Cfg()->BB(cur_bb)->Add_stmt(stmt_ins);
  } else {
    CGOP *stmt_ins = new CGOP(cgop, expr_id, cur_bb, TN_tn_idx(result), TN_tn_idx(op1), 0, 0);
    Cfg()->BB(cur_bb)->Add_stmt(stmt_ins);
  }
}

void CG_EMITTER::Emit_label(FILE *out, LABEL_IDX label_idx) {
  fprintf(out, "%s:\n", LABEL_name(label_idx));
}

void CG_EMITTER::Emit_operand(CGOP *oper, CGOPR_KIND kind, UINT32 ch_id, FILE* out) {
  AssertThat(ch_id < 4, ("operand count must be less than 4."));
  CG_OPRAND cgoper = oper->getResOpnd()[ch_id];
  AssertThat(cgoper != 0,
             ("Should not be empty, cgopc = %s", Get_cg_opc_info(
               oper->getOpcode())->getName()));
  if (CGOPR_R == kind) {
    TN *tn = TN_tn(cgoper);
    if (TN_is_symbol(tn)) {
      ST_IDX sym = TN_var(tn);
      AssertThat(ST_sclass(sym) == SYMC_FILE_STATIC,
                 ("this should only be used for static/global var."));
      // Generate labels for current func.
      fprintf(out, "%s ", LABEL_name(Get_addr_label(sym)));
    } else if (TN_is_label(tn)) {
      LABEL_IDX lbl = TN_label(tn);
      fprintf(out, "%s ", LABEL_name(lbl));
    } else if (TN_is_constant(tn)) {
      fprintf(out, "%s%lld ", Backend().Immediate_prefix(), TN_value(tn));
    } else if (TN_is_label(tn)) {
      fprintf(out, ".%s ", LABEL_name(TN_label(tn)));
    } else if (TN_is_dedicated(tn)) {
      UINT32 reg_id = TN_register(tn);
      if (TN_register_class(tn) == REGISTER_CLASS_ra &&
          reg_id == REGISTER_ra) {
        const std::string name = Backend().Dedicated_register_name(
            TARGET_REGISTER_ROLE::LINK_REGISTER);
        fprintf(out, "%s ", name.c_str());
      } else if (TN_register_class(tn) == REGISTER_CLASS_sp &&
                 reg_id == REGISTER_sp) {
        const std::string name = Backend().Dedicated_register_name(
            TARGET_REGISTER_ROLE::STACK_POINTER);
        fprintf(out, "%s", name.c_str());
      } else if (TN_register_class(tn) == REGISTER_CLASS_fp &&
                 reg_id == REGISTER_fp) {
        const std::string name = Backend().Dedicated_register_name(
            TARGET_REGISTER_ROLE::FRAME_POINTER);
        fprintf(out, "%s", name.c_str());
      } else if (TN_register_class(tn) == REGISTER_CLASS_v0 &&
                 reg_id == REGISTER_v0) {
        const std::string name = Backend().Dedicated_register_name(
            TARGET_REGISTER_ROLE::RETURN_VALUE);
        fprintf(out, "%s", name.c_str());
      } else {
        AssertThat(false, ("not implemented"));
      }
    } else {
      UINT32 reg_id = TN_register(tn);
      const std::string name = Backend().Integer_register_name(reg_id);
      fprintf(out, "%s ", name.c_str());
    }
  } else if (CGOPR_IMM == kind) {
    UINT32 val = TN_value(TN_tn(cgoper));
    fprintf(out, "%s%d ", Backend().Immediate_prefix(), val);
  } else {
    AssertThat(false, ("not implemented."));
  }
}

void CGOP::Print(FILE *file) {
  fprintf(file, "[CGOP] node = %d, opc = %s(%d), index:%d, res/opnd: [%u] [%u] [%u] [%u] \n",
          _tree_node_id, ISA_OPCODE_name(getOpcode()), getOpcode(), getIndexInBb(),
          (UINT32) res_opnd[0], (UINT32) res_opnd[1],
          (UINT32) res_opnd[2], (UINT32) res_opnd[3]);
}

CGOPC_INFO CGOPC_INFO_LIST[] = {
#define CGOPDEF(enum_name, nres, nopr, is_w, opr1, opr2, opr3, ins_name, kind)   \
  { #enum_name, enum_name, nres, nopr, is_w, opr1, opr2, opr3, ins_name, kind },
#include "cg_opc.h"
#undef CGOPDEF
};

CGOPC_INFO *Get_cg_opc_info(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return &(CGOPC_INFO_LIST[i]);
    }
  }
  AssertThat(false, ("Cannot find opcode in table = %d", cgopc));
  return &(CGOPC_INFO_LIST[0]);
}

LABEL_IDX Get_addr_label(ST_IDX sym) {
  char *targ = (char*) malloc(sizeof(".taddr_") + strlen(ST_name(sym)) + 100);
  sprintf(targ, ".taddr_%d_%s", File()->Scopes()->Current()->getSt(), ST_name(sym));
  for (UINT32 i       = 1; i < File()->Tables()->Label()->Length(File()->Scopes()->Current()); i++) {
    LABEL_IDX lbl = (LABEL_IDX) (i << 8) + LOCAL_SYMTAB;
    if (LABEL_label(lbl)->Get_temp_sym() == sym) {
      free(targ);
      return lbl;
    }
  }
  STR_IDX     name    = File()->Save_string(targ);
  LABEL_IDX   lbl_idx = File()->Create_label(name, LABEL_ADDR_PASSED &
                                             LABEL_ADDR_SAVED, LKIND_RELOC);
  LABEL_label(lbl_idx)->Set_temp_sym(sym);
  free(targ);
  return lbl_idx;
}

BOOL CGOPC_is_ldst(CGOPC cgopc) {
  return (Get_cg_opc_info(cgopc)->opk == CGOPK_LDST);
}

UINT32 CG_REG_ALLOC::Count_needed_register(CGOP *oper, UINT32 cgop_id,
                                   CGOPR_KIND kind, UINT32 cur_bb, UINT8 opr_pos) {
  if (kind != CGOPR_R) {
    Is_Trace(TR_CGIR(), (TFile, "Found kind != r, no need to allocate \n"));
    return 0;
  }
  CG_OPRAND cgoper;
  cgoper = oper->getResOpnd()[opr_pos];
  AssertThat(cgoper != 0, ("TN does not exist on CGOP_id %d, cur_bb = %d, opr_pos = %d",
             cgop_id, cur_bb, opr_pos));
  AssertThat(cgoper < Cgir()->Get_tn_table().size(),
             ("TN %d exceed the table length %d, on CGOP_id %d, cur_bb = %d, opr_pos = %d",
              cgoper, Cgir()->Get_tn_table().size(), cgop_id, cur_bb, opr_pos));
  TN *tn = TN_tn(cgoper);
  if (TN_is_symbol(tn) || TN_is_label(tn) || TN_is_preallocated(tn) ||
      TN_is_constant(tn)  || TN_is_dedicated(tn)) {
    if (TN_is_dedicated(tn) || TN_is_preallocated(tn)) {
      UINT32 reg_id = 1024;
      if (TN_register_class(tn) == REGISTER_CLASS_v0 &&
          TN_register(tn) == REGISTER_v0) {
        reg_id = 0;
      } else if (TN_is_preallocated(tn) && TN_register(tn) >= 0 && TN_register(tn) < 1024) {
        reg_id = TN_register(tn);
      }
      if (reg_id != 1024) {
        vector<UINT32> &ded = Cfg()->BB(cur_bb)->Get_dedicate_regs();
        if (std::find(ded.begin(), ded.end(), reg_id) == ded.end()) {
          ded.push_back(reg_id);
        }
      }
    }
    Is_Trace(TR_CGIR(), (TFile, "Found tn %d no need to allocate \n", cgoper));
    return 0;
  } else {
    // There is a need for R-A.
    Is_Trace(TR_CGIR(), (TFile, "Found tn %d to allocate \n", cgoper));
    AssertThat(TN_register(tn) == 0,
               ("TN: %d, Should not be allocated already. %d",
                cgoper, TN_register(tn)));
    TN_IDX tid = TN_tn_idx(tn);
    UINT32 old_freq = 0;
    if (_tn_freq_map.find(tid) != _tn_freq_map.end()) {
      old_freq = _tn_freq_map[tid];
    } else {
      _tn_freq_map.insert(std::make_pair(tid, 0));
      _tn_live_range.insert(std::make_pair(tid, vector<UINT64>()));
    }
    _tn_freq_map[tid] = old_freq + 1;
    UINT64 bb_stmt = (((UINT64) cur_bb) << 32l) & cgop_id;
    _tn_live_range[tid].push_back(bb_stmt);
    return 1;
  }
  return 0;
}

void CG_REG_ALLOC::Process_spill_op(CGOP *oper, CGOPR_KIND kind, UINT32 cur_bb,
                            UINT32 opnd, BOOL is_write) {
  Builder()->Enable_spill_recording();
  // Process a possible write that may create spilling.
  AssertThat(oper->getResOpnd()[opnd] != 0, ("incorrect tn found"));
  TN_IDX tid = oper->getResOpnd()[opnd];
  TN *tn = TN_tn(tid);
  if (TN_flags(tn) & TN_SPILL) {
    // There is a spill.
    TN *spill_tn = Gen_Register_TN(ISA_REGISTER_CLASS_integer, REG_SIZE_I);
    Set_TN_is_preallocated(spill_tn);
    Set_TN_flags(spill_tn, TN_SPILL_MEDIUM);
    UINT32 reg_num_to_use = opnd == 0 ? 8 : ((opnd == 1) ?  10 : 7);
    if (is_write) { // this is dependent on the oper currently we're visiting.
      Is_Trace(TR_CGIR(), (TFile, "Create store temp tn %d to r%d\n", TN_tn_idx(spill_tn), REGISTER_spill));
      // haven't allocated
      Is_Trace(TR_CGIR(), (TFile, "Spill tn %d to r%d\n", TN_tn_idx(tn), REGISTER_spill));
      Set_TN_register(spill_tn, reg_num_to_use);
      Builder()->Exp_load_store(OPC_I4STID, MTYPE_I4,
               spill_tn,
               nullptr,
               TN_spill(tn), 0,
               nullptr,
               cur_bb, V_BR_NONE);
      oper->setResOpnd(opnd, TN_tn_idx(spill_tn));
      vector <CGOP*> &sp_related = Builder()->Get_spill_related();
      for (INT32 i = sp_related.size() - 1; i >= 0; i--) {
        CGOP *rs = sp_related[i];
        if (rs->getOpcode() == CGOPC_ADD) {
          // Probably a too long stack, in this sense, we need to allocate middle base.
          TN_IDX middle_base = rs->getResOpnd()[0];
          TN *middle = TN_tn(middle_base);
          AssertThat(!TN_is_preallocated(middle) && !TN_is_dedicated(middle),
                     ("Tn must be clean. %d", middle_base));
          Set_TN_is_preallocated(middle);
          Set_TN_register(middle, 10);
        }
        rs->setFlags(CGOPF_SPILL);
        Cfg()->BB(cur_bb)->Get_work_list().push_back(
          CG_REVISIT_ITEM<CGOP>(oper, rs, false));
      }
    } else {
      Is_Trace(TR_CGIR(), (TFile, "Spill tn %d to r%d\n", TN_tn_idx(tn), reg_num_to_use));
      Is_Trace(TR_CGIR(), (TFile, "Create store temp tn %d to r%d\n", TN_tn_idx(spill_tn), reg_num_to_use));
      Set_TN_register(spill_tn, reg_num_to_use); // Making sure the two register are the same.
      Builder()->Exp_load_store(OPC_I4LDID, MTYPE_I4,
        spill_tn,
        nullptr,
        TN_spill(tn), 0, 0, cur_bb, V_BR_NONE);
      oper->setResOpnd(opnd, TN_tn_idx(spill_tn));
      vector<CGOP*> &sp_related = Builder()->Get_spill_related();
      for (UINT32 i = 0; i < sp_related.size(); i++) {
        CGOP *rs = sp_related[i];
        if (rs->getOpcode() == CGOPC_ADD) {
          // Probably a too long stack, in this sense, we need to allocate middle base.
          TN_IDX middle_base = rs->getResOpnd()[0];
          TN *middle = TN_tn(middle_base);
          AssertThat(!TN_is_preallocated(middle) && !TN_is_dedicated(middle),
                     ("Tn must be clean. %d", middle_base));
          Set_TN_is_preallocated(middle);
          Set_TN_register(middle, 0);
        }
        rs->setFlags(CGOPF_SPILL);
        Cfg()->BB(cur_bb)->Get_work_list().push_back(CG_REVISIT_ITEM<CGOP> (oper, rs, true));
      }
    }
  }
  Builder()->Disable_spill_recording();
}

UINT32 CGIR::TN_tab_size() {
  return _global_tn_vec.size();
}

void CGIR::Recalibrate_offset(PU_INFO *pInfo) {
  for (auto item : Cfg()->Get_recalibrate_map()) {
    TN *ofst_tn = TN_tn(item.first);
    INT64 new_value = Layout()->Get_sym_sp_ofst(item.second);
    if (Cfg()->Get_exceed_map().find(item.first) !=
        Cfg()->Get_exceed_map().end()) {
      // Oversided TN, use offset instead.
      INT64 orig_val = Cfg()->Get_exceed_map()[item.first];
      Set_TN_value(ofst_tn, new_value - orig_val);
    } else {
      AssertThat(TN_is_constant(ofst_tn), ("Not a constant TN to calibrate"));
      Set_TN_value(ofst_tn, new_value);
    }
  }
}

UINT8 ISA_OPCODE_results(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return CGOPC_INFO_LIST[i].n_res;
    }
  }
  AssertThat(false, ("CGOPC not found in table = %d", cgopc));
  return 0;
}

UINT8 ISA_OPCODE_operands(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return CGOPC_INFO_LIST[i].n_oprs;
    }
  }
  AssertThat(false, ("CGOPC not found in table = %d", cgopc));
  return 0;
}

const char *ISA_OPCODE_name(CGOPC cgopc) {
  UINT32 count = sizeof(CGOPC_INFO_LIST) / sizeof(CGOPC_INFO);
  for (UINT32 i = 0; i < count; i++) {
    if (CGOPC_INFO_LIST[i].opcode == cgopc) {
      return CGOPC_INFO_LIST[i].name;
    }
  }
  AssertThat(false, ("CGOPC not found in table = %d", cgopc));
  return nullptr;
}

template
class CFG_BB_BASE<CGOP>;  // CGBB

template
class CFG_BASE<CGOP, CG_CFG_BB_BASE<CGOP>>;     // CG_CFG
