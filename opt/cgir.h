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
#include "cfg_common.h"

#define POS_INVALID -1

using std::vector;
using std::map;
using std::set;
using IR_TN_MAP = std::unordered_map<IRNODE_IDX, TN*>;
using TN_IR_MAP = std::unordered_map<TN*, IRNODE_IDX>;

template <typename NODE_TYPE> class CFG_BB_BASE;
template <typename NODE_TYPE> class CG_CFG_BB_BASE;

template <typename NODE_TYPE>
class CG_CFG_BASE : public CFG_BASE<NODE_TYPE, CG_CFG_BB_BASE<NODE_TYPE>> {
private:
  map<TN_IDX, ST_IDX>              _recal_map;
  map<TN_IDX, INT64>               _exceed_map;
public:
  map<TN_IDX, INT64> & Get_exceed_map() {
    return _exceed_map;
  }
  map<TN_IDX, ST_IDX> & Get_recalibrate_map() {
    return _recal_map;
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


template <typename NODE_TYPE>
class CG_CFG_BB_BASE : public CFG_BB_BASE<NODE_TYPE> {
private:
  vector<CG_REVISIT_ITEM<NODE_TYPE> > _work_list;
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
};

// BB definitions
typedef CG_CFG_BB_BASE<CGOP>              CGBB;
typedef vector<CGBB *>                    CGBB_VECTOR;
typedef typename vector<CGBB *>::iterator CGBB_ITER;
typedef CG_CFG_BASE<CGOP>                 CG_CFG;



extern std::vector<TN *>              _global_tn_vec;

class CGIR {
private:
  // CGIR_Table
  map<ST_IDX, CG_CFG *>        trees;
  map<ST_IDX, DATA_LAYOUT*>    layout;
  map<TN_IDX, UINT32>         _tn_freq_map;
  map<TN_IDX, vector<UINT64> >_tn_live_range;
  map<PREG_IDX, TN_IDX>        _preg_to_tn;
  IR_TN_MAP                    ir_to_tn_map;
  TN_IR_MAP                    tn_to_ir_map;
  // Memory Layout
  CG_FRAME_SECT                sections[8];
  CG_CFG                     *_current;
  ST_IDX                      _current_sym;
  DATA_LAYOUT                *_current_layout;
  TREE                        *tree;
  vector<CGOP *>               _last_created;
public:
  CG_CFG       *Get_function(ST_IDX func_sym) {
    if(trees.find(func_sym) == trees.end()) {
      trees[func_sym] = new CG_CFG();
    }
    return trees[func_sym];
  }

  // Start the cg transformation of a function.
  void          CG_Expand(SCOPE *scope);        // Initialize the CG stuff
  void          IR_to_CGIR(ST_IDX func_sym);
  void          Data_layout(SCOPE *scope);    // Do data layout
  UINT32        Add_prolog(UINT32 bb);
  UINT32        Add_epilog(UINT32 bb);


  // Expanding a funciton, handle all kinds of VHIR stmt / expr.
  void          Handle_STID(IR_ITER stmt, CFG_BB_IDX cur_bb);
  TN           *Handle_LDID(IR_ITER stmt, CFG_BB_IDX cur_bb, TN *pTn);
  TN           *Handle_ILOAD(IR_ITER stmt, CFG_BB_IDX cur_bb, TN *target_res);
  TN           *Handle_ISTORE(IR_ITER stmt, CFG_BB_IDX cur_bb);
  TN           *Handle_LDA(IR_ITER expr, CFG_BB_IDX cur_bb, TN *target_res);
  void          Handle_ret_val(IR_ITER stmt, CFG_BB_IDX cur_bb);
  void          Handle_Entry(IR_ITER entry, CFG_BB_IDX cur_bb);
  CFG_BB_IDX    Handle_goto(IR_ITER stmt, CFG_BB_IDX cur_bb);
  CFG_BB_IDX    Handle_call(IR_ITER stmt, CFG_BB_IDX cur_bb, CFG_BB_IDX next_bb);
  void          Handle_ret(IR_ITER, CFG_BB_IDX cur_bb);
  // Expanding stuff, such as expression
  TN           *Expand_Expr(IR_ITER entry, IR_ITER parent, CFG_BB_IDX cur_bb, TN *result);

  // CG-tuple creation
  void          Exp_op(UINT32 expr_id, OPCODE opcode, CFG_BB_IDX cur_bb, TN *result, TN *op1, TN *op2, TN *op3,
                       VARIANT variant, CGOP **ops);
  void          Set_current_cgir(CG_CFG *cgir, ST_IDX sym);
  CG_CFG       *Cfg() { return _current; }
  void          Local_register_allocate(PU_INFO *info);
  void          Print(FILE *file = stderr);
  void          Print(ST_IDX sym, FILE *file = stderr);
  inline  void  Exp_op0(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, CGOP **ops)              {  Exp_op(e, c,bb,r,NULL,NULL,NULL,V_NONE,ops); }
  inline  void  Exp_op1(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, TN *o1, CGOP **ops)           {  Exp_op(e, c,bb,r,o1,NULL,NULL,V_NONE,ops); }
  inline  void  Exp_op1v(UINT32 e, OPCODE c, CFG_BB_IDX bb, TN *r, TN *o1, VARIANT v, CGOP **ops)        {  Exp_op(e, c,bb,r,o1,NULL,NULL,v,ops); }
  inline  void  Exp_op2(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, TN *o1, TN *o2, CGOP **ops)        {  Exp_op(e, c,bb,r,o1,o2,NULL,V_NONE,ops); }
  inline  void  Exp_op2v(UINT32 e, OPCODE c, CFG_BB_IDX bb, TN *r, TN *o1, TN *o2,VARIANT v, CGOP **ops)     {  Exp_op(e, c,bb,r,o1,o2,NULL,v,ops); }
  inline  void  Exp_op3(UINT32 e, OPCODE c,  CFG_BB_IDX bb, TN *r, TN *o1, TN *o2, TN *o3, CGOP **ops)     {  Exp_op(e, c,bb,r,o1,o2,o3,V_NONE,ops); }
  inline  void  Exp_op3v(UINT32 e, OPCODE c, CFG_BB_IDX bb, TN *r, TN *o1, TN *o2, TN *o3, VARIANT v, CGOP **ops)  {  Exp_op(e, c,bb,r,o1,o2,o3,v,ops); }
  void          Exp_LDST(OPCODE opc,
                         MTYPE_ID mtype, TN *src_res_tn,
                         TN *base_tn,
                         ST_IDX sym, INT64 ofst_val,
                         IR_ITER node, CFG_BB_IDX bb_idx,
                         VARIANT variant);

  // Emitting result.
  void          Emit_label(PU_INFO *func, FILE *out, UINT32 label_idx);
  void          Emit_operand(CGOP *oper, CGOPR_KIND k, UINT32 ch_id, FILE*out);
  void          Emit_tree(PU_INFO *func, FILE *out, FILE_MANAGER *file);

  VARIANT       Memop_Variant(IR_ITER iterator);


  // Symbol related access utility.
  CGOPC_INFO   *Get_cg_opc_info(CGOPC cgopc);
  LABEL_IDX     Get_addr_label(ST_IDX sym);
  BOOL          CGOPC_is_ldst(CGOPC cgopc);

  // Data layout
  void          Set_current_layout(DATA_LAYOUT *pLayout) { _current_layout = pLayout; }
  DATA_LAYOUT  *Layout() {
    AssertThat(_current_layout != NULL, ("layout is null"));
    return _current_layout;
  }

  // Utility functions
  CGOPC         Get_branch_cond(IR_ITER cond, BOOL is_true_br);
  void          Add_store_formals(IR_ITER entry, CFG_BB_IDX bb);

  // PREG, TN related ....
  TN           *PREG_to_TN (TY_IDX preg_ty, PREG_NUM preg_num);
  TN           *PREG_to_ST_TN(ST_IDX sym_idx, PREG_NUM preg_num);

  // Temporary Node related...
  vector<TN *>  &Get_tn_table() {  return _global_tn_vec;  };
  TN_IDX        Get_TN_from_symbol(ST_IDX sym);
  TN_IDX        Get_TN_by_ir_node(IR_ITER node, CFG_BB_IDX cur_bb);
  UINT32        TN_tab_size();
  void          Recalibrate_offset(PU_INFO *pInfo);

  // TN ir mapping
  TN           *Get_tn_by_ir(IRNODE_IDX ir) {
    if (ir_to_tn_map.find(ir) != ir_to_tn_map.end()) {
      return ir_to_tn_map[ir];
    }
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INFO),
             (TFile, "IR not found in ir_to_tn_map, ir = %lld", ir));
    return nullptr;
  }
  IRNODE_IDX    Get_ir_by_tn(TN *tn) {
    if (tn_to_ir_map.find(tn) != tn_to_ir_map.end()) {
      return tn_to_ir_map[tn];
    }
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_INFO),
             (TFile, "TN not found in tn_to_ir, tn = 0x%016llx", (UINT64) tn));
    return 0;
  }

  // Register Allocation
  void          Allocate_registers();
  void          Spill_tn(TN_IDX tid, TN *tn);
  UINT32        Count_needed_register(CGOP *oper, UINT32 cgop_id,
                               CGOPR_KIND kind, UINT32 cur_bb, UINT8 opr_pos);
  void          Process_spill_op(CGOP *oper, CGOPR_KIND kind,
                        UINT32 cur_bb, UINT32 opnd, BOOL is_write);
};


#endif //OCC_CGIR_H
