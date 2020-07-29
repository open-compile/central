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

#define POS_INVALID -1

using std::vector;
using std::map;
using std::set;
using IR_TN_MAP = std::unordered_map<IRNODE_IDX, TN*>;
using TN_IR_MAP = std::unordered_map<TN*, IRNODE_IDX>;

template <typename NODE_TYPE> class CFG_BB_BASE;
template <typename NODE_TYPE>
class CFG_BASE {
public:
  typedef CFG_BB_BASE<NODE_TYPE> BB_TYPE;
  typedef vector<BB_TYPE *> BB_VECTOR;
  typedef typename vector<BB_TYPE *>::iterator BB_ITER;
private:
  map<CFG_BB_IDX, set<CFG_BB_IDX> > edges;
  BB_VECTOR                         bb_list;
public:
  // iterators, accesses
  BB_ITER Begin() { return bb_list.begin(); }
  BB_ITER End()   { return bb_list.end();   }
  UINT32  Size()  { return bb_list.size();  }

  CFG_BB_IDX Add_bb() {
    CFG_BB_IDX idx = bb_list.size();
    bb_list.push_back(new BB_TYPE(idx));
    return idx;
  }
  BB_TYPE *Node(CFG_BB_IDX idx) {
    AssertThat(bb_list.size() > idx,
               ("Incorrect BB idx = %u, where size = %lu", idx, bb_list.size()));
    return bb_list.at(idx);
  }
  BB_TYPE *BB(CFG_BB_IDX idx) { return Node(idx); };
  void Add_succ(CFG_BB_IDX from, CFG_BB_IDX to) {
    if (edges.find(from) == edges.end()) {
      edges.insert(std::make_pair(from, set<CFG_BB_IDX>()));
    }
    edges.find(from)->second.insert(to);
  }
  BOOL Is_succ(CFG_BB_IDX from, CFG_BB_IDX to) {
    if (edges.find(from) == edges.end()) {
      return false;
    }
    set<CFG_BB_IDX> &second = edges.find(from)->second;
    return second.find(to) == second.end();
  }

  void Build_cfg(TREE *tree, PU_INFO *pu_info) {
    // Building the CFG
    // Seperate the tree, into different cfg bbs.
  }

  void  Print(FILE *file = stderr);
  CFG_BB_IDX Add_bb(INT pred);
};

// BB definitions
typedef CFG_BB_BASE<CGOP> CGBB;
typedef vector<CGBB *> CGBB_VECTOR;
typedef typename vector<CGBB *>::iterator CGBB_ITER;

template<typename NODE_TYPE>
class CGTODO_ITEM {
private:
  NODE_TYPE *target;
  NODE_TYPE *from;
  BOOL       put_before;
public:
  CGTODO_ITEM(NODE_TYPE *targ, NODE_TYPE *org, BOOL before) {
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

template<typename NODE_TYPE>
class CFG_BB_BASE {
private:
  typedef CFG_BB_BASE<NODE_TYPE>                      BB_TYPE;
  typedef std::list<BB_TYPE *>                        BB_LIST;

  typedef typename BB_LIST::iterator bb_iterator;
  typedef typename BB_LIST::const_iterator const_bb_iterator;

  typedef typename BB_LIST::iterator dom_iterator;
  typedef typename BB_LIST::const_iterator const_dom_iterator;

  typedef typename std::vector<NODE_TYPE *>::iterator stmt_iterator;
  typedef typename std::vector<NODE_TYPE *>::const_iterator const_stmt_iterator;

  vector<NODE_TYPE *> _stmts;
  vector< CGTODO_ITEM<NODE_TYPE> > _work_list;
  UINT32              _flags = 0;
  UINT32              _label_id = 0;
  BB_TYPE            *_idom  = nullptr;  // immediate dominator on CFG
  BB_TYPE            *_ipdom = nullptr;  // immediate dominator on RCFG
  BB_LIST             _preds;  // list of predesessors
  BB_LIST             _succs;  // list of successors
  BB_LIST             _dom_list;   // bb list dominated immediately on CFG
  BB_LIST             _pdom_list;  // bb list dominated immediately on RCFG
  BB_LIST             _df_list;   // dominance frontier on CFG
  BB_LIST             _cd_list;   // control dependence
  UINT32              _id;        // unique id

public:
  UINT32   Get_id() const { return _id; }
  explicit CFG_BB_BASE(UINT32 block_id) {
    _id = block_id;
    _flags = 0;
    _idom = nullptr;
  }
  NODE_TYPE *Get_op_by_id(CGOP_IDX idx) {
    return _stmts[idx];
  }
  CFG_STMT_IDX Add_stmt(NODE_TYPE *node) {
    CFG_STMT_IDX idx = _stmts.size();
    _stmts.push_back(node);
    return idx;
  }
  void Set_flag(BB_FLAG flag) {
    _flags |= flag;
  }
  void Clear_flag(BB_FLAG flag) {
    _flags &= (~flag);
  }
  NODE_TYPE *Get_stmt(CFG_STMT_IDX stmt_idx) {
    AssertThat(_stmts.size() > stmt_idx,
               ("Stmt list must be larger than idx = %d, size = %d",
                stmt_idx, _stmts.size()));
    return _stmts.at(stmt_idx);
  }
  const_stmt_iterator First_stmt() const   { return _stmts.begin();   }
  const_stmt_iterator Last_stmt()  const   { return _stmts.end();     }
  BOOL                Is_empty()   const   { return _stmts.empty();   }
  const_stmt_iterator Next_stmt(stmt_iterator stmt) const  { return stmt + 1; }
  const_stmt_iterator Prev_stmt(stmt_iterator stmt) const  { return stmt - 1; }

  void Add_pred(BB_TYPE *pred) {
    bb_iterator it = std::find(_preds.begin(), _preds.end(), pred);
    AssertThat(it == _preds.end(), ("pred has been added"));
    if (it == _preds.end())
      _preds.push_back(pred);
  }
  void Add_succ(BB_TYPE *succ) {
    bb_iterator it = std::find(_succs.begin(), _succs.end(), succ);
    AssertThat(it == _succs.end(), ("succ has been added"));
    if (it == _succs.end())
      _succs.push_back(succ);
  }
  bb_iterator Remove_pred(BB_TYPE *pred) {
    bb_iterator it = std::find(_preds.begin(), _preds.end(), pred);
    AssertThat(it != _preds.end(), ("Can not find pred"));
    return _preds.erase(it);
  }
  void Remove_all_preds() {
    _preds.clear();
  }
  bb_iterator Remove_succ(BB_TYPE *succ) {
    bb_iterator it = std::find(_succs.begin(), _succs.end(), succ);
    AssertThat(it != _preds.end(), ("Can not find succ"));
    return _succs.erase(it);
  }
  void Remove_all_succs() {
    _succs.clear();
  }

  INT32 Get_preds_count() const { return _preds.size(); }
  INT32 Get_succs_count() const { return _succs.size(); }

  const BB_TYPE *Get_pred(INT index) const {
    AssertThat(index >= 0 && index < _preds.size(), ("index out of bounds"));
    const_bb_iterator it = Pred_begin();
    for (int i=0; i < index; ++i)
      ++it;
    return *it;
  }
  const BB_TYPE *Get_succ(INT index) const {
    AssertThat(index >= 0 && index < _succs.size(), ("index out of bounds"));
    const_bb_iterator it = Succ_begin();
    for (int i=0; i < index; ++i)
      ++it;
    return *it;
  }
  INT32 Pred_pos(BB_TYPE *pred) {
    INT32 pos = 0;
    for (const_bb_iterator it = Pred_begin();
         it != Pred_end();
         ++it) {
      if (*it == pred)
        return pos;
      ++pos;
    }
    return POS_INVALID;
  }
  INT32 Succ_pos(BB_TYPE *succ) {
    INT32 pos = 0;
    for (const_bb_iterator it = Succ_begin();
         it != Succ_end();
         ++it) {
      if (*it == succ)
        return pos;
      ++pos;
    }
    return POS_INVALID;
  }

  // dominator and post-dominator related methods
  BB_TYPE *Get_idom() const {
    AssertThat(Get_preds_count() == 0 || _idom != NULL,
            ("idom is NULL. DOM is not built?"));
    return _idom;
  }
  void Set_idom(BB_TYPE *node)  { _idom = node;  }
  BB_TYPE *Get_ipdom() const {
    AssertThat(Get_succs_count() == 0 || _ipdom != NULL,
            ("ipdom is NULL. PDOM is not built?"));
    return _ipdom;
  }
  void Set_ipdom(BB_TYPE *node) { _ipdom = node; }
  BOOL Dominate(BB_TYPE *node) {
    AssertThat(node != NULL, ("node is NULL"));
    do {
      if (node == this)
        return TRUE;
      else
        node = node->Get_idom();
    } while(node != NULL);
    return FALSE;
  }
  BOOL Post_dominate(BB_TYPE *node) {
    AssertThat(node != NULL, ("node is NULL"));
    do {
      if (node == this)
        return TRUE;
      else
        node = node->Get_ipdom();
    } while (node != NULL);
    return FALSE;
  }

  // iterators for preds/succs
  bb_iterator Pred_begin() { return _preds.begin(); }
  bb_iterator Pred_end()   { return _preds.end();   }
  const_bb_iterator Pred_begin() const { return _preds.begin(); }
  const_bb_iterator Pred_end() const { return _preds.end(); }
  bb_iterator Succ_begin() { return _succs.begin(); }
  bb_iterator Succ_end()   { return _succs.end();   }
  const_bb_iterator Succ_begin() const { return _succs.begin(); }
  const_bb_iterator Succ_end() const { return _succs.end(); }

  // iterators for dom/pdom
  dom_iterator Dom_begin()  { return _dom_list.begin(); }
  dom_iterator Dom_end()    { return _dom_list.end();   }
  const_dom_iterator Dom_begin() const { return _dom_list.begin(); }
  const_dom_iterator Dom_end() const   { return _dom_list.end();   }
  dom_iterator Pdom_begin()  { return _pdom_list.begin(); }
  dom_iterator Pdom_end()    { return _pdom_list.end();   }
  const_dom_iterator Pdom_begin() const { return _pdom_list.begin(); }
  const_dom_iterator Pdom_end() const   { return _pdom_list.end();   }

  // iterators for df/cd
  bb_iterator Df_begin()  { return _df_list.begin(); }
  bb_iterator Df_end()    { return _df_list.end();   }
  const_bb_iterator Df_begin() const { return _df_list.begin(); }
  const_bb_iterator Df_end() const   { return _df_list.end();   }
  bb_iterator Cd_begin()  { return _cd_list.begin(); }
  bb_iterator Cd_end()    { return _cd_list.end();   }
  const_bb_iterator Cd_begin() const { return _cd_list.begin(); }
  const_bb_iterator Cd_end() const   { return _cd_list.end();   }


  // if dom is TRUE, _idom is returned
  BB_TYPE* get_idom(bool dom) {
    return (dom) ? _idom : _ipdom;
  }
  // if dom is TRUE, _idom is set to node
  void set_idom(BB_TYPE* node, bool dom) {
    if (dom)
      _idom = node;
    else
      _ipdom = node;
  }
  // if dom is TRUE, clear _dom_list, otherwise, clear _pdom_list
  void clear_dom_list(bool dom) {
    if (dom) {
      _dom_list.clear();
    }
    else {
      _pdom_list.clear();
    }
  }
  // if dom is TRUE, add node to _dom_list, otherwist, add to _pdom_list
  void add_to_dom_list(BB_TYPE* node, bool dom) {
    if (dom)
      _dom_list.push_back(node);
    else
      _pdom_list.push_back(node);
  }
  // if df is TRUE, add node to _df_list, otherwise, add to _cd_list
  void add_to_df_list(BB_TYPE* node, bool df) {
    AssertThat(node != NULL, ("node is NULL"));
    if (df)
      _df_list.push_back(node);
    else
      _cd_list.push_back(node);
  }

  // iterators for preds/succs, if fwd is TRUE, _succs is visited
  bb_iterator bb_begin(BOOL fwd) { return (fwd) ? _succs.begin() : _preds.begin(); }
  bb_iterator bb_end(BOOL fwd)   { return (fwd) ? _succs.end() : _preds.end();     }
  const_bb_iterator bb_begin(BOOL fwd) const { return (fwd) ? _succs.begin() : _preds.begin(); }
  const_bb_iterator bb_end(BOOL fwd) const   { return (fwd) ? _succs.end() : _preds.end();     }

  // iterators for dom/pdom, if dom is TRUE, _dom_list is visited
  dom_iterator dom_begin(BOOL dom) {
    if (dom) {
      return _dom_list.begin();
    }
    else {
      return _pdom_list.begin();
    }
  }
  dom_iterator dom_end(BOOL dom) {
    if (dom) {
      return _dom_list.end();
    }
    else {
      return _pdom_list.end();
    }
  }
  const_dom_iterator dom_begin(BOOL dom) const {
    if (dom) {
      return _dom_list.begin();
    }
    else {
      return _pdom_list.begin();
    }
  }
  const_dom_iterator dom_end(BOOL dom) const {
    if (dom) {
      return _dom_list.end();
    }
    else {
      return _pdom_list.end();
    }
  }

  // iterators for df/cd, if df is TRUE, _df_list is visited
  bb_iterator df_begin(BOOL df) { return (df) ? _df_list.begin() : _cd_list.begin(); }
  bb_iterator df_end(BOOL df)   { return (df) ? _df_list.end() : _cd_list.end();       }
  const_bb_iterator df_begin(BOOL df) const { return (df) ? _df_list.begin() : _cd_list.begin(); }
  const_bb_iterator df_end(BOOL df) const   { return (df) ? _df_list.end() : _cd_list.end();       }
  void  Print(FILE *file = stderr);
  UINT32 Get_flags() const { return _flags; }
  UINT32 Get_label_id() const { return _label_id; }
  void Set_label_id(UINT32 labelId) { _label_id = labelId; }
  void Move_stmt_to_after(NODE_TYPE *position, NODE_TYPE *from);
  void Move_stmt_to_before(NODE_TYPE *position, NODE_TYPE *from);
  CGOP *Last_real_stmt();
  vector<CGTODO_ITEM<NODE_TYPE>> &Get_work_list() { return _work_list; }
};

typedef CFG_BB_BASE<CGOP>    CGBB;
typedef CFG_BASE<CGOP>       CG_CFG;

class CGIR {
private:
  // CGIR_Table
  map<ST_IDX, CG_CFG *>        trees;
  map<ST_IDX, DATA_LAYOUT*>    layout;
  map<TN_IDX, UINT32>         _tn_freq_map;
  map<TN_IDX, vector<UINT32> >_tn_live_range;
  IR_TN_MAP                    ir_to_tn_map;
  TN_IR_MAP                    tn_to_ir_map;
  // Memory Layout
  CG_FRAME_SECT                sections[8];
  CG_CFG                     *_current;
  ST_IDX                      _current_sym;
  DATA_LAYOUT                *_current_layout;
  TREE                        *tree;
public:
  CG_CFG    *Get_function(ST_IDX func_sym) {
    if(trees.find(func_sym) == trees.end()) {
      trees[func_sym] = new CG_CFG();
    }
    return trees[func_sym];
  }
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
  void          CG_Expand(SCOPE *scope);        // Initialize the CG stuff
  void          Data_layout(SCOPE *scope);    // Do data layout
  void          IR_to_CGIR(ST_IDX func_sym);
  void          Handle_STID(IR_ITER stmt, CFG_BB_IDX cur_bb);
  TN           *Handle_LDID(IR_ITER stmt, CFG_BB_IDX cur_bb, TN *pTn);
  void          Handle_ret_val(IR_ITER stmt, CFG_BB_IDX cur_bb);
  void          Handle_Entry(IR_ITER entry, CFG_BB_IDX cur_bb);
  TN           *Expand_Expr(IR_ITER entry, IR_ITER parent, CFG_BB_IDX cur_bb, TN *result);
  TN           *PREG_to_TN (TY_IDX preg_ty, PREG_NUM preg_num);
  TN           *PREG_to_ST_TN(ST_IDX sym_idx, PREG_NUM preg_num);

  void          Exp_op(OPCODE opcode, TN *result, TN *op1, TN *op2, TN *op3,
                       VARIANT variant, CGOP *ops);
  void          Set_current_cgir(CG_CFG *cgir, ST_IDX sym);
  CG_CFG       *Cfg() { return _current; }
  TN_IDX        Get_TN_from_symbol(ST_IDX sym);
  TN_IDX        Get_TN_by_ir_node(IR_ITER node, CFG_BB_IDX cur_bb);
  void          Local_register_allocate(PU_INFO *info);
  void          Print(FILE *file = stderr);
  void          Print(ST_IDX sym, FILE *file = stderr);
  inline  void  Exp_op0(OPCODE c, TN *r, CGOP *ops)              {  Exp_op(c,r,NULL,NULL,NULL,V_NONE,ops); }
  inline  void  Exp_op1(OPCODE c, TN *r, TN *o1, CGOP *ops)           {  Exp_op(c,r,o1,NULL,NULL,V_NONE,ops); }
  inline  void  Exp_op1v(OPCODE c, TN *r, TN *o1, VARIANT v, CGOP *ops)        {  Exp_op(c,r,o1,NULL,NULL,v,ops); }
  inline  void  Exp_op2(OPCODE c, TN *r, TN *o1, TN *o2, CGOP *ops)        {  Exp_op(c,r,o1,o2,NULL,V_NONE,ops); }
  inline  void  Exp_op2v(OPCODE c, TN *r, TN *o1, TN *o2,VARIANT v, CGOP *ops)     {  Exp_op(c,r,o1,o2,NULL,v,ops); }
  inline  void  Exp_op3(OPCODE c, TN *r, TN *o1, TN *o2, TN *o3, CGOP *ops)     {  Exp_op(c,r,o1,o2,o3,V_NONE,ops); }
  inline  void  Exp_op3v(OPCODE c, TN *r, TN *o1, TN *o2, TN *o3, VARIANT v, CGOP *ops)  {  Exp_op(c,r,o1,o2,o3,v,ops); }

  void          Emit_tree(PU_INFO *func, FILE *out, FILE_MANAGER *file);

  VARIANT Memop_Variant(IR_ITER iterator);

  void Exp_LDST(OPCODE opc,
                MTYPE_ID mtype, TN *src_res_tn, ST_IDX sym, INT64 ofst_val, CFG_BB_IDX bb_idx,
                VARIANT variant);
  void Exp_Ldst (
        OPCODE opcode,
        TN *tn,
        ST_IDX sym,
        INT64 ofst,
        BOOL indirect_call,
        BOOL is_store,
        BOOL is_load,
        CFG_BB_IDX ops,
        VARIANT variant);

  void Emit_label(PU_INFO *func, FILE *out, UINT32 label_idx);
  void Emit_operand(CGOP *oper, CGOPR_KIND k, UINT32 ch_id, FILE*out);
  CGOPC_INFO *Get_cg_opc_info(CGOPC cgopc);
  LABEL_IDX Get_addr_label(ST_IDX sym);
  BOOL CGOPC_is_ldst(CGOPC cgopc);
  void Set_current_layout(DATA_LAYOUT *pLayout) { _current_layout = pLayout; }
  DATA_LAYOUT *Layout() {
    AssertThat(_current_layout != NULL, ("layout is null"));
    return _current_layout;
  }
  UINT32 Count_needed_register(CGOP *oper, CGOPR_KIND kind, UINT32 cur_bb, UINT8 opr_pos);
  void Process_spill_op(CGOP *oper, CGOPR_KIND kind,
                        UINT32 cur_bb, UINT32 opnd, BOOL is_write);
};


#endif //OCC_CGIR_H
