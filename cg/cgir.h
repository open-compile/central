//
// Created by xc5 on 2020/7/13.
//

#ifndef OCC_CGIR_H
#define OCC_CGIR_H

#include "basic.h"
#include "symtab.h"
#include "consts.h"
#include "tree.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include "register.h"
#include "tree_util.h"
#include "data_layout.h"
#include "cg_basic.h"
#include "cg_variant.h"
#include "tn.h"
#include <list>
#include <set>
#include "cfg_common.h"

#define POS_INVALID -1
#define MAX_LRA_ITER_ATTEMPT 1024000

using std::vector;
using std::map;
using std::set;
using IR_TN_MAP = std::unordered_map<IRNODE_IDX, TN*>;
using TN_IR_MAP = std::unordered_map<TN *, IRNODE_IDX>;
using TN_LIVERANGE = std::pair<UINT32, UINT32>;  // (start_instruction, end_instruction)


typedef set<TN_IDX>    TN_SET;
typedef vector<TN_SET> TN_SET_VEC;

template <typename NODE_TYPE> class CFG_BB_BASE;
template <typename NODE_TYPE> class CG_CFG_BB_BASE;

// forward decl.
class CGIR_BUILDER;

template <typename NODE_TYPE>
class CG_CFG_BASE : public CFG_BASE<NODE_TYPE, CG_CFG_BB_BASE<NODE_TYPE>> {
private:
  map<TN_IDX, ST_IDX>              _recal_map;
  map<TN_IDX, INT64>               _exceed_map;
  CFG_BB_EDGES_STORE               _preds;
public:
  map<TN_IDX, INT64> & Get_exceed_map() {
    return _exceed_map;
  }
  map<TN_IDX, ST_IDX> & Get_recalibrate_map() {
    return _recal_map;
  }
  void                  Set_preds(const CFG_BB_EDGES_STORE &given) {
    _preds = given;
  }
  CFG_BB_EDGES_STORE &Get_preds() { 
    return _preds; 
  }
  CFG_BB_EDGES       &Preds(CFG_BB_IDX idx) { 
    AssertThat(idx < _preds.size(), ("Index exceed Preds's container size, idx = %u, size = %u",
                                idx, _preds.size()));
    return _preds.at(idx); 
  }
  CFG_BB_EDGES_STORE &Get_succs() { 
    return this->Internal_edges();
  }
  CFG_BB_EDGES       &Succs(CFG_BB_IDX idx) { 
    UINT32 edge_cnt = this->Internal_edges().size();
    AssertThat(idx < edge_cnt, ("Index exceed Edges(Succs)'s container size, idx = %u, size = %u",
                                idx, edge_cnt));
    return this->Internal_edges().at(idx); 
  }
  void                Print_cg_cfg(FILE *file) {
    // Print CG specific info
    fprintf(file, "%sPrinting CG control flow graph, of size : %d\n%s",
            DBAR, this->Size(), DBAR);
    for (UINT32 i = 0; i < this->Size(); i++) {
      this->Node(i)->Print_detail(file);
    }
  }
};

/**
 * *
 *  Recording some information that requires a second pass
 *  for recalculation, e.g. offset ....
 * @tparam NODE_TYPE CGOP i.e.
 * This is used for filling the gap in case of some IR opcode that's not implemented right now.
 */
template<typename NODE_TYPE>
class   CG_REVISIT_ITEM {
private:
  NODE_TYPE *target;
  NODE_TYPE *from;
  BOOL       put_before;
public:
  CG_REVISIT_ITEM(NODE_TYPE *targ, NODE_TYPE *org, BOOL before) {
    target = targ;
    from = org;
    put_before = before;
  }
  NODE_TYPE *getTarget() const {
    return target;
  }

  void setTarget(NODE_TYPE *tg) {
    target = tg;
  }

  NODE_TYPE *getFrom() const {
    return from;
  }

  void setFrom(NODE_TYPE *fr) {
    from = fr;
  }

  BOOL getPutBefore() const {
    return put_before;
  }

  void setPutBefore(BOOL putBefore) {
    put_before = putBefore;
  }
};

extern void Print_tn_set(FILE *file, const TN_SET &tns);

extern void Print_tn_set_vec(FILE *file, const TN_SET_VEC &tnvecs);

template <typename NODE_TYPE>
class CG_CFG_BB_BASE : public CFG_BB_BASE<NODE_TYPE> {
private:
  vector<CG_REVISIT_ITEM<NODE_TYPE> > _work_list;
  TN_SET                         _defs;
  TN_SET                         _uses;
  TN_SET_VEC                     _stmt_defs;
  TN_SET_VEC                     _stmt_uses;
public:
  explicit CG_CFG_BB_BASE(UINT32 block_id)
    : CFG_BB_BASE<NODE_TYPE>(block_id),
      _work_list() {
  }
  CG_CFG_BB_BASE(UINT32 block_id, const vector<CG_REVISIT_ITEM<NODE_TYPE>> &work_list)
    : CFG_BB_BASE<NODE_TYPE>(block_id),
      _work_list(work_list) {
  }
  vector<CG_REVISIT_ITEM<NODE_TYPE>> &Get_work_list() { return _work_list; }
  TN_SET  &Defs() { return _defs; }
  TN_SET  &Uses() { return _uses; }
  void          Setup_stmt_def_use() {
    UINT32 stmt_count = this->Get_stmt_count();
    _stmt_defs.clear();
    _stmt_uses.clear();
    _stmt_defs.resize(stmt_count);
    _stmt_uses.resize(stmt_count);
  }
  BOOL          Has_stmt_def_use() {
    UINT32 stmt_count = this->Get_stmt_count();
    return _stmt_defs.size() == stmt_count && _stmt_uses.size() == stmt_count;
  }
  TN_SET  &Stmt_defs(UINT32 idx) { 
    UINT32 stmt_count = this->Get_stmt_count();
    AssertThat(idx < stmt_count && stmt_count == _stmt_defs.size(), (
      "Incorrect stmt defs or idx, idx = %u, stmt_cnt = %u, stmt_defs.size = %u",
      idx, stmt_count, _stmt_defs.size()));
    return _stmt_defs[idx]; 
  }
  TN_SET  &Stmt_uses(UINT32 idx) { 
    UINT32 stmt_count = this->Get_stmt_count();
    AssertThat(idx < stmt_count && stmt_count == _stmt_uses.size(), (
      "Incorrect stmt uses or idx, idx = %u, stmt_cnt = %u, stmt_uses.size = %u",
      idx, stmt_count, _stmt_uses.size()));
    return _stmt_uses[idx]; 
  }
  void Print_detail(FILE *file) {
    UINT32 stmt_cnt = this->Get_stmt_count();
    BOOL has_stmt_def_use =
      (_stmt_defs.size() == stmt_cnt && _stmt_uses.size() == stmt_cnt);

    // Print basic info.
    this->Print_basic(file);

    // def use info.
    fprintf(file, "#  BB defs: ");
    Print_tn_set(file, Defs());
    fprintf(file, ", BB uses: ");
    Print_tn_set(file, Uses());
    if (!has_stmt_def_use) {
      fprintf(file, "  [stmt def/use unavailable: stmt_cnt=%u defs=%lu uses=%lu]",
              stmt_cnt, _stmt_defs.size(), _stmt_uses.size());
    }
    fprintf(file, "\n");

    // stmt print
    fprintf(file, "#  --------- with %u statements inside  --------- \n", stmt_cnt);
    for (UINT32 i = 0; i < stmt_cnt; i++) {
      fprintf(file, "#    stmt[%u] defs: ", i);
      if (has_stmt_def_use) {
        Print_tn_set(file, _stmt_defs[i]);
      } else {
        fprintf(file, "?");
      }
      fprintf(file, ", uses: ");
      if (has_stmt_def_use) {
        Print_tn_set(file, _stmt_uses[i]);
      } else {
        fprintf(file, "?");
      }
      fprintf(file, "\n");
      this->Get_stmt(i)->Print(file);
    }
  }
};

// BB definitions
typedef CG_CFG_BB_BASE<CGOP>              CGBB;
typedef vector<CGBB *>                    CGBB_VECTOR;
typedef typename vector<CGBB *>::iterator CGBB_ITER;
typedef CG_CFG_BASE<CGOP>                 CG_CFG;
typedef CG_CONV_EXTRAINFO<CGOP, CGBB>     CG_CONV_INFO;
typedef map<TN_IDX, INT32>                TN_FREQ_MAP;

extern std::vector<TN *>              _global_tn_vec;

class CGIR {
private:
  // CGIR_Table
  map<ST_IDX, CG_CFG *>        trees;
  map<ST_IDX, DATA_LAYOUT*>    _layouts;
  map<PREG_IDX, TN_IDX>        _preg_to_tn;
  IR_TN_MAP                    _ir_to_tn_map;
  TN_IR_MAP                    _tn_to_ir_map;
  TN_FREQ_MAP                  _use_cnts;

  // Memory Layout
  CG_FRAME_SECT                sections[8];
  CG_CFG                     *_current;
  ST_IDX                      _current_sym;
  DATA_LAYOUT                *_current_layout;
  TREE                        *_current_tree;
  TN_IDX                       _func_tn_begin;

public:
  CG_CFG       *Get_cg_cfg(ST_IDX func_sym) {
    if(trees.find(func_sym) == trees.end()) {
      trees[func_sym] = new CG_CFG();
    }
    return trees[func_sym];
  }
  map<ST_IDX, DATA_LAYOUT*> &Layouts() {
    return _layouts;
  }
  // Start the cg transformation of a function.
  void          Set_current_cgir(CG_CFG *cfg, ST_IDX sym);
  TREE         *Current_tree() {
    AssertThat(_current_tree != nullptr, ("current tree must be set first."));
    return _current_tree;
  };

  ST_IDX        Current_func_sym()  {  return _current_sym;  }
  CG_CFG       *Cfg() { return _current; }
  void          Print(FILE *file = stderr);
  void          Print(ST_IDX sym, FILE *file = stderr);
  void          Print_cfg_detail(ST_IDX sym, FILE *file = stderr);
  void          Print_cfg_graph(ST_IDX sym, FILE *file = stderr);
  void          Print_tn_table(ST_IDX sym, FILE *file = stderr,
                               BOOL function_only = TRUE);

  TN_FREQ_MAP  &TN_freq_map() { return _use_cnts; } 

  // Data layout
  void          Set_current_layout(DATA_LAYOUT *pLayout) { _current_layout = pLayout; }
  DATA_LAYOUT  *Layout() {
    AssertThat(_current_layout != NULL, ("layout is null"));
    return _current_layout;
  }

  // PREG, TN related ....
  TN           *PREG_to_TN (TY_IDX preg_ty, PREG_NUM preg_num);
  TN           *PREG_to_ST_TN(ST_IDX sym_idx, PREG_NUM preg_num);

  // Creating empty TN.
  TN           *Gen_TN();
  TN_IDX        Gen_TN(MTYPE_ID mtype);
  TN           *TN_tn(TN_IDX tn_idx);
  void          Check_TN_Vec_Size();
  void          Cleanup_function_TN();
  void          Start_function_TN();
  TN_SET        Stmt_defs(CGOP *cgop);
  TN_SET        Stmt_uses(CGOP *cgop);

  // Temporary Node related...
  vector<TN *> &Get_tn_table() {  return _global_tn_vec;  };
  TN_IDX        Get_TN_from_symbol(ST_IDX sym);
  TN_IDX        Get_TN_by_ir_node(IR_ITER node, CFG_BB_IDX cur_bb);
  UINT32        TN_tab_size();
  void          Recalibrate_offset(PU_INFO *pInfo);

  // TN ir mapping
  TN           *Get_tn_by_ir(IRNODE_IDX ir) {
    if (_ir_to_tn_map.find(ir) != _ir_to_tn_map.end()) {
      return _ir_to_tn_map[ir];
    }
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INFO),
             (TFile, "IR not found in ir_to_tn_map, ir = %lld", ir));
    return nullptr;
  }
  IRNODE_IDX    Get_ir_by_tn(TN *tn) {
    if (_tn_to_ir_map.find(tn) != _tn_to_ir_map.end()) {
      return _tn_to_ir_map[tn];
    }
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INFO),
             (TFile, "TN not found in tn_to_ir, tn = 0x%016llx", (UINT64) tn));
    return 0;
  }
};

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
  void            Print(FILE *file = stderr);
};

/**
 * Register allocator
 */
class CG_REG_ALLOC {
private:
  CGIR                       *_cgir            = nullptr;
  CGIR_BUILDER               *_builder         = nullptr;
  map<TN_IDX, UINT32>         _tn_freq_map;
  map<TN_IDX, vector<UINT64> >_tn_live_range;
public:
  void Init(CGIR *cg, CGIR_BUILDER *bd) {
    _cgir = cg;
    _builder = bd;
    AssertThat(cg != nullptr, ("Cgir must be a valid CGIR"));
    AssertThat(bd != nullptr, ("Cgbuilder must be a valid one"));
  }
  CGIR *Cgir() {
    AssertThat(_cgir != nullptr,
      ("Cgir is not initialized in Live range analysis"));
    return _cgir;
  }
  CG_CFG       *Cfg() {
    CG_CFG  *cfg = Cgir()->Cfg();
    AssertThat(cfg != nullptr, ("Couldn't get cfg from cgir."));
    return cfg;
  }
  CGIR_BUILDER *Builder() {
    AssertThat(_builder != nullptr,
      ("Builder is not initialized in Live range analysis"));
    return _builder;
  }
  // Register Allocation
  // 入口: 由 CG_process_funcs 调用, 串起整个 RA pipeline
  void          Register_allocate(PU_INFO *info);

  // TODO(register-allocator): 真正 RA 流水线应该拆成以下几步 (每步是一个独立函数):
  //
  //   step1  Build_Interference_Graph(IFG&)          ← 建 IFG
  //          输入: live_range (从 CG_LIVE_RANGE::Analyze_live_range 得到)
  //          输出: IFG (节点=TN, 边=live-range-overlap)
  //
  //   step2  Chaitin_Simplify(IFG&, stack)            ← 启发式压栈
  //          把 degree < K 的节点压栈 (Simplify)
  //          选 cost/degree 最低的节点压栈 (Potential-Spill)
  //
  //   step3  Chaitin_Select(stack, IFG, coloring)     ← 反向染色
  //          弹栈, 给每个节点分配 K 种颜色之一
  //          不够色就 spill
  //
  //   step4  Spill_Rewrite(spilled, CGIR&)            ← 改 CGOP
  //          对每个 spilled TN, 在 def 前插 STR, use 后插 LDR
  //          然后回到 step1 (IFG 变了!)
  //
  //   step5  Rematerialize(CGIR&)                     ← 重物化
  //          如果 tn 是 const / lda 等可便宜重算的, 不 spill, 在 use 处重算
  //
  //   step6  Frame_Allocation(CGIR&, DATA_LAYOUT&)    ← 算栈帧
  //          给 spilled TN 分配 [sp, #off] 槽位
  //
  // 当前实现:
  //   - 没有 IFG (count-uses 只做频次)
  //   - 用 naive linear scan (next_register++)
  //   - spill 是 "寄存器用光就全 spill" 的简化版
  //   - 缺 step5 (remat) 和 step6 的精确计算
  //
  // 详细 spec 见 docs/cg.spec.md §4

  // 当前唯一入口: 把上面的 step1..step6 全揉在一起做 linear scan
  void          Allocate_registers();
  void          Spill_tn(TN_IDX tid, TN *tn);
  UINT32        Count_needed_register(CGOP *oper, UINT32 cgop_id,
                               CGOPR_KIND kind, UINT32 cur_bb, UINT8 opr_pos);
  void          Process_spill_op(CGOP *oper, CGOPR_KIND kind,
                        UINT32 cur_bb, UINT32 opnd, BOOL is_write);
  // 调试输出
  void          Print_freq_map(FILE *file = stderr);
  void          Print_live_range(FILE *file = stderr);
};


/**
 * CGIR's builder utility.
 */
class CGIR_BUILDER {
private:
  CGIR                        *_cgir = nullptr;
  TREE                        *_tree  = nullptr;
  vector<CGOP *>               _spill_related;
  BOOL                         _spill_recording = false;
public:
  void          Init(CGIR *cg) {
    _cgir = cg;
    AssertThat(cg != nullptr, ("Cgir must be a valid CGIR"));
  }
  CGIR         *Cgir() {
    AssertThat(_cgir != nullptr,
      ("Cgir is not initialized in Live range analysis"));
    return _cgir;
  }
  CG_CFG       *Cfg() {
    CG_CFG  *cfg = Cgir()->Cfg();
    AssertThat(cfg != nullptr, ("Couldn't get cfg from cgir."));
    return cfg;
  }
  TREE *Tree() {
    return Cgir()->Current_tree();
  }

  // Data layout
  void          Data_layout(SCOPE *scope);    // Do data layout
  VARIANT       Memop_Variant(IR_ITER iterator);

  // Expanding a funciton, handle all kinds of VHIR stmt / expr.
  void          Handle_stid     (IR_ITER stmt, CFG_BB_IDX cur_bb);
  TN           *Handle_ldid     (IR_ITER stmt, CFG_BB_IDX cur_bb, TN *pTn);
  TN           *Handle_iload    (IR_ITER stmt, CFG_BB_IDX cur_bb, TN *target_res);
  TN           *Handle_istore   (IR_ITER stmt, CFG_BB_IDX cur_bb);
  TN           *Handle_lda      (IR_ITER expr, CFG_BB_IDX cur_bb, TN *target_res);
  void          Handle_ret_val  (IR_ITER stmt, CFG_BB_IDX cur_bb, CG_CONV_INFO &builder);
  void          Handle_func_body(IR_ITER enti, CFG_BB_IDX cur_bb);
  void          Handle_goto     (IR_ITER stmt, CFG_BB_IDX cur_bb, CFG_BB_IDX next_bb, CG_CONV_INFO &conv_info);
  void          Handle_call     (IR_ITER stmt, CFG_BB_IDX cur_bb, CFG_BB_IDX next_bb);
  void          Handle_ret      (IR_ITER stmt, CFG_BB_IDX cur_bb, CG_CONV_INFO &builder);

  // Expanding stuff, such as expression
  TN           *Expand_expr     (IR_ITER entry, IR_ITER parent, CFG_BB_IDX cur_bb, TN *result);
  void          Exp_op(UINT32 expr_id, OPCODE opcode, CFG_BB_IDX cur_bb, TN *result, TN *op1, TN *op2, TN *op3,
                       VARIANT variant, CGOP **ops);
  inline  void  Exp_op0(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, CGOP **ops)              {  Exp_op(e, c,bb,r,NULL,NULL,NULL,V_NONE,ops); }
  inline  void  Exp_op1(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, TN *o1, CGOP **ops)           {  Exp_op(e, c,bb,r,o1,NULL,NULL,V_NONE,ops); }
  inline  void  Exp_op1v(UINT32 e, OPCODE c, CFG_BB_IDX bb, TN *r, TN *o1, VARIANT v, CGOP **ops)        {  Exp_op(e, c,bb,r,o1,NULL,NULL,v,ops); }
  inline  void  Exp_op2(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, TN *o1, TN *o2, CGOP **ops)        {  Exp_op(e, c,bb,r,o1,o2,NULL,V_NONE,ops); }
  inline  void  Exp_op2v(UINT32 e, OPCODE c, CFG_BB_IDX bb, TN *r, TN *o1, TN *o2,VARIANT v, CGOP **ops)     {  Exp_op(e, c,bb,r,o1,o2,NULL,v,ops); }
  inline  void  Exp_op3(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, TN *o1, TN *o2, TN *o3, CGOP **ops)     {  Exp_op(e, c,bb,r,o1,o2,o3,V_NONE,ops); }
  inline  void  Exp_op3v(UINT32 e, OPCODE c, CFG_BB_IDX bb, TN *r, TN *o1, TN *o2, TN *o3, VARIANT v, CGOP **ops)  {  Exp_op(e, c,bb,r,o1,o2,o3,v,ops); }
  void          Exp_load_store(OPCODE opc,
                         MTYPE_ID mtype, TN *src_res_tn,
                         TN *base_tn,
                         ST_IDX sym, INT64 ofst_val,
                         IR_ITER node, CFG_BB_IDX bb_idx,
                         VARIANT variant);
  // Utility functions
  CGOPC         Get_branch_cond(IR_ITER cond, BOOL is_true_br);
  void          Add_store_formals(IR_ITER entry, CFG_BB_IDX bb);

  void          CG_convert_function(SCOPE *scope);        // Initialize the CG stuff
  CFG_BB_IDX    Add_prolog(CFG_BB_IDX bb);
  CFG_BB_IDX    Add_epilog(CFG_BB_IDX bb);

  void          Add_to_spill_related(CGOP *op) {
    if (_spill_recording)
      _spill_related.push_back(op);
  };
  vector<CGOP *>& Get_spill_related() {
    return _spill_related;
  }
  void          Enable_spill_recording() {
    _spill_recording = true;
    _spill_related.clear();
  }
  void          Disable_spill_recording() {
    _spill_recording = false;
    _spill_related.clear();
  }
  void          Build_pred_succ();
  void          Build_def_use();
};

class CG_EMITTER {
private:
  CGIR *_cgir = nullptr;
  CGIR_BUILDER *_builder = nullptr;
public:
  void Init(CGIR *cg, CGIR_BUILDER *cgir_builder) {
    _cgir = cg;
    _builder = cgir_builder;
    AssertThat(cg != nullptr, ("Cgir must be a valid CGIR"));
  }
  CGIR *Cgir() {
    AssertThat(_cgir != nullptr,
      ("Cgir is not initialized in Live range analysis"));
    return _cgir;
  }
  TREE *Tree() {
    return Cgir()->Current_tree();
  }
  DATA_LAYOUT *Layout() {
    return Cgir()->Layout();
  }
  CGIR_BUILDER *Builder() {
    AssertThat(_builder != nullptr,
      ("Builder is not initialized in Live range analysis"));
    return _builder;
  }
  // Emitting result.
  void          Emit_label(FILE *out, UINT32 label_idx);
  void          Emit_operand(CGOP *oper, CGOPR_KIND k, UINT32 ch_id, FILE*out);
  void          Emit_tree(ST_IDX func_sym, FILE *out, FILE_MANAGER *file);
};


/**
 * The Entire CG class to access.
 */
class CG_COMPOSITE {
private:
  CGIR            *_cgir = nullptr;
  CG_EMITTER       _emitter;
  CGIR_BUILDER     _builder;
  CG_LIVE_RANGE     _lra;
  CG_REG_ALLOC      _reg_alloc;
public:
  void Init() {
    _cgir = new CGIR();
    AssertThat(_cgir != nullptr, ("Cgir must be a valid CGIR"));
    _builder.Init(_cgir);
    _emitter.Init(_cgir, &_builder);
    _reg_alloc.Init(_cgir, &_builder);
    _lra.Init(_cgir);
  }
  CGIR *Cgir() {
    AssertThat(_cgir != nullptr,
      ("Cgir is not initialized in Live range analysis"));
    return _cgir;
  }
  CG_EMITTER     &Emitter() {
    return _emitter;
  }
  CGIR_BUILDER   &Builder() {
    return _builder;
  }
  CG_REG_ALLOC   &Reg_alloc() { return _reg_alloc; }
  CG_LIVE_RANGE  &Lra()       { return _lra;       }
  void          CG_convert_function(SCOPE *scope);        // Initialize the CG stuff
};


// Utility Functions
LABEL_IDX     Get_addr_label(ST_IDX sym);
// Symbol related access utility.
CGOPC_INFO   *Get_cg_opc_info(CGOPC cgopc);
BOOL          CGOPC_is_ldst(CGOPC cgopc);

#endif //OCC_CGIR_H
