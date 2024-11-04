//
// Created by lugt on 24-4-17.
// This file contains code related to the creation of control flow graph(CFG)
// Control flow graph, Basic block, ....
//

#ifndef CFG_COMMON_H
#define CFG_COMMON_H

#include "basic.h"
#include "consts.h"
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>

using std::vector;
using std::map;
using std::set;
using std::list;

#define POS_INVALID -1

/**
 * Flags to mark on basic blocks
 */
enum BB_FLAG{
  BB_FLAG_ENTRY       = 0x0001,
  BB_FLAG_EXIT        = 0x0002,
  BB_FLAG_HANDLER     = 0x0004,
  BB_FLAG_CALL        = 0x0008,
  BB_FLAG_LABEL       = 0x0010,
  BB_FLAG_UNREACH     = 0x0020,
  BB_FLAG_SCHED       = 0x0040,
  BB_FLAG_SPILL       = 0x0080,
  BB_FLAG_LRA         = 0x0100,
};

// Define basic block
template <typename NODE_TYPE> class CFG_BB_BASE;

// Define CFG related type renames.
typedef UINT32                              CFG_BB_IDX;
typedef vector<set<CFG_BB_IDX>> CFG_BB_EDGES_STORE;
typedef set<CFG_BB_IDX>                     CFG_BB_EDGES;

// TODO: maybe we need a second template type to
// TODO: store additional NODE_TYPE related info once in each BB.
// Basic block or BB, and node, and arrow
// Control flow graph basic node;
template <typename NODE_TYPE, typename BB_TYPE>
class CFG_BASE {
public:
  typedef vector<BB_TYPE *>                    BB_VECTOR;
  typedef typename vector<BB_TYPE *>::iterator BB_ITER;
private:
  CFG_BB_EDGES_STORE                           _edges;
  BB_VECTOR                                    _bb_list;
public:
  // iterators, accesses
  BB_ITER Begin() { return _bb_list.begin(); }
  BB_ITER End()   { return _bb_list.end();   }
  UINT32  Size()  { return _bb_list.size();  }
  CFG_BB_EDGES  &Edges(CFG_BB_IDX from) {
    AssertThat(from <= _edges.size(), ("bb index out of bound of edge table"));
    CFG_BB_EDGES &pointee = _edges[from];
    return pointee;
  };
  // Add a BB with predecessor
  CFG_BB_IDX Add_bb(CFG_BB_IDX pred) {
    CFG_BB_IDX new_bb = Add_bb();
    this->BB(pred)->Add_succ(BB(new_bb));
    this->BB(new_bb)->Add_pred(BB(pred));
    return new_bb;
  }
  CFG_BB_IDX Add_bb() {
    CFG_BB_IDX idx = _bb_list.size();
    _bb_list.push_back(new BB_TYPE(idx));
    _edges.emplace_back();
    return idx;
  }
  BB_TYPE *Node(CFG_BB_IDX idx) {
    AssertThat(_bb_list.size() > idx,
               ("Incorrect BB idx = %u, where size = %lu", idx, _bb_list.size()));
    return _bb_list.at(idx);
  }
  BB_TYPE *BB(CFG_BB_IDX idx) { return Node(idx); };
  void Add_succ(CFG_BB_IDX from, CFG_BB_IDX to) { Edges(from).insert(to); }
  BOOL Is_succ(CFG_BB_IDX from, CFG_BB_IDX to) {
    CFG_BB_EDGES &edge = Edges(from);
    return edge.find(to) == edge.end();
  }
  void  Print(FILE *file = stderr)  {
    // Print ...
    fprintf(file, "%sPrinting control flow graph, of size : %d\n%s",
            DBAR, this->Size(), DBAR);
    for (UINT32 i = 0; i < this->Size(); i++) {
      this->Node(i)->Print(file);
    }
  }
};


// How to use DNA / RNA to connect IPA.
class IPA_CALLING_NODE {
private:
  // Should have DNA/RNA here.
  UINT32              _flags = 0;
  UINT32              _label_id = 0;

public:

};

class IPA_CALL_GRAPH {

};

/**
 * Basic block, as to contain stmt without any jumps,
 * a.k.a. all stmts must be executed in sequence from static compiler view.
 * @tparam NODE_TYPE
 */
template<typename NODE_TYPE>
class CFG_BB_BASE {
private:
  typedef CFG_BB_BASE<NODE_TYPE>                      BB_TYPE;
  typedef std::list<BB_TYPE *>                        BB_LIST;

  typedef typename BB_LIST::iterator                  bb_iterator;
  typedef typename BB_LIST::const_iterator            const_bb_iterator;
  typedef typename BB_LIST::iterator                  dom_iterator;
  typedef typename BB_LIST::const_iterator            const_dom_iterator;

  typedef typename std::vector<NODE_TYPE *>::iterator stmt_iterator;
  typedef typename std::vector<NODE_TYPE *>::const_iterator const_stmt_iterator;

  // Core vector for storing stmt in the basic block.
  vector<NODE_TYPE *> _stmts;

  UINT32              _flags = 0;
  UINT32              _label_id = 0;
  BB_LIST             _preds;  // list of predesessors
  BB_LIST             _succs;  // list of successors

  // Used for calculating SSA ....
  BB_TYPE            *_idom  = nullptr;  // immediate dominator on CFG
  BB_TYPE            *_ipdom = nullptr;  // immediate dominator on RCFG
  BB_LIST             _dom_list;   // bb list dominated immediately on CFG
  BB_LIST             _pdom_list;  // bb list dominated immediately on RCFG
  BB_LIST             _df_list;   // dominance frontier on CFG
  BB_LIST             _cd_list;   // control dependence
  UINT32              _id;        // unique id
  vector<UINT32>      _dedicated_regs;

public:
  UINT32   Get_id() const { return _id; }

  // Initiate cfg bb
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
  bb_iterator       Pred_begin()        { return _preds.begin();     }
  bb_iterator       Pred_end()          { return _preds.end();       }
  const_bb_iterator Pred_begin()  const { return _preds.begin();     }
  const_bb_iterator Pred_end()    const { return _preds.end();       }
  bb_iterator       Succ_begin()        { return _succs.begin();     }
  bb_iterator       Succ_end()          { return _succs.end();       }
  const_bb_iterator Succ_begin()  const { return _succs.begin();     }
  const_bb_iterator Succ_end()    const { return _succs.end();       }

  // iterators for dom/pdom
  dom_iterator       Dom_begin()        { return _dom_list.begin();  }
  dom_iterator       Dom_end()          { return _dom_list.end();    }
  const_dom_iterator Dom_begin()  const { return _dom_list.begin();  }
  const_dom_iterator Dom_end()    const { return _dom_list.end();    }
  dom_iterator       Pdom_begin()       { return _pdom_list.begin(); }
  dom_iterator       Pdom_end()         { return _pdom_list.end();   }
  const_dom_iterator Pdom_begin() const { return _pdom_list.begin(); }
  const_dom_iterator Pdom_end()   const { return _pdom_list.end();   }

  // iterators for df/cd
  bb_iterator        Df_begin()         { return _df_list.begin();   }
  bb_iterator        Df_end()           { return _df_list.end();     }
  const_bb_iterator  Df_begin()   const { return _df_list.begin();   }
  const_bb_iterator  Df_end()     const { return _df_list.end();     }

  // iterators for control dependence
  bb_iterator        Cd_begin()         { return _cd_list.begin();   }
  bb_iterator        Cd_end()           { return _cd_list.end();     }
  const_bb_iterator  Cd_begin()   const { return _cd_list.begin();   }
  const_bb_iterator  Cd_end()     const { return _cd_list.end();     }

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
  bb_iterator       bb_begin(BOOL fwd)       { return (fwd) ? _succs.begin() : _preds.begin(); }
  bb_iterator       bb_end(BOOL fwd)         { return (fwd) ? _succs.end() : _preds.end();     }
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
  bb_iterator       df_begin(BOOL df)       { return (df) ? _df_list.begin() : _cd_list.begin();   }
  bb_iterator       df_end(BOOL df)         { return (df) ? _df_list.end() : _cd_list.end();       }
  const_bb_iterator df_begin(BOOL df) const { return (df) ? _df_list.begin() : _cd_list.begin();   }
  const_bb_iterator df_end(BOOL df)   const { return (df) ? _df_list.end() : _cd_list.end();       }

  // Utility functions
  void Print_basic(FILE *out = stderr) {
    fprintf(out, "#  --------- Printing BB : %d, label = %d = 0x%04x  ---------  \n"
            "#  --------- (Pred: ", Get_id(), Get_label_id(), Get_label_id());
    for (auto prd_id = Pred_begin();
         prd_id != Pred_end(); prd_id++) {
      fprintf(out, "%d ", (*prd_id)->Get_id());
    }
    fprintf(out, ", Succ: ");
    for (auto prd_id = Succ_begin();
         prd_id != Succ_end(); prd_id++) {
      fprintf(out, "%d ", (*prd_id)->Get_id());
    }
    fprintf(out, ") --------- \n");
  }
  void              Print(FILE *file = stderr) {
    this->Print_basic(file);
    fprintf(file, "#  --------- with %lu statements inside  --------- \n", _stmts.size());
    for (UINT32 i = 0; i < _stmts.size(); i++) {
      // STMTs
      _stmts[i]->Print(file);
    }
  }

  UINT32            Get_flags()            const { return _flags; }
  // Create label index here.
  UINT32            Get_label_id()         const { return _label_id; }
  void              Set_label_id(UINT32 labelId) { _label_id = labelId; }
  void              Move_stmt_to_after(NODE_TYPE *position, NODE_TYPE *from) {
    BOOL flag = false;
    for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
      if (*it == position) {
        it++;
        AssertThat(it != _stmts.end(), ("This cannot be, otherwise we are moving [position] after [position]"));
        _stmts.insert(it, from);
        flag = true;
        break;
      }
    }
    AssertThat(flag, ("Cannot find the target stmt %p in stmts.", position));
    flag = false;
    for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
      if (*it == from) {
        if (!flag) {
          flag = true; // skip the first one.
          continue;
        }
        _stmts.erase(it);
        flag = true;
        break;
      }
    }
    AssertThat(flag, ("Cannot find the old value to delete."));
  }
  void              Move_stmt_to_before(NODE_TYPE *position, NODE_TYPE *from){
    BOOL flag = false;
    for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
      if (*it == position) {
        _stmts.insert(it, from);
        flag = true;
        break;
      }
    }
    AssertThat(flag, ("Cannot find the target stmt %p in stmts.", position));
    flag = false;
    for (typename vector<NODE_TYPE*>::iterator it = _stmts.begin(); it != _stmts.end(); it++) {
      if (*it == from) {
        if (!flag) {
          flag = true; // skip the first one.
          continue;
        }
        _stmts.erase(it);
        flag = true;
        break;
      }
    }
    AssertThat(flag, ("Cannot find the old value to delete."));
  }

  NODE_TYPE        *Last_real_stmt() {
    AssertThat(_stmts.size() > 0, ("No last exist"));
    return _stmts.back();
  };
  UINT32            Get_stmt_count() {
    return _stmts.size();
  }
  // vector<CGTODO_ITEM<NODE_TYPE>> &Get_work_list() { return _work_list; }
  // Register allocation related, this should not be here.
  void Dedicate_reg(UINT32 i) {
    _dedicated_regs.push_back(i);
  }
  vector<UINT32> &Get_dedicate_regs() {
    return _dedicated_regs;
  }
};


/**
 * Containing some extra info for label and goto conversion on CGIR creation.
 * saving which bb goto which label, for later on destructing BBs and generating final code.
 * @tparam NODE_TYPE
 * @tparam BB_TYPE
 */
template<typename NODE_TYPE, typename BB_TYPE>
class CG_CONV_EXTRAINFO {
private:
  CFG_BASE<NODE_TYPE, BB_TYPE>                *_cfg               = nullptr;
  map<LABEL_IDX, CFG_BB_IDX>                   _label_to_bb_map;
  map<CFG_BB_IDX, LABEL_IDX>                   _bb_goto_label;
  vector<CFG_BB_IDX>                           _revisit;  // the list to revisit when the labels, goto are all marked.
public:
  void Init(CFG_BASE<NODE_TYPE, BB_TYPE> *cfg) {
    AssertThat(cfg != nullptr, ("CFG cannot be null."));
    _cfg = cfg;
    _label_to_bb_map.clear();
    _bb_goto_label.clear();
    _revisit.clear();
  };
  void Bind_label_to_bb(LABEL_IDX label, CFG_BB_IDX bb_id) {
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
      (TFile, "[CFG-Builder] Marking up label %d = %04x at BB %d\n", label, label, bb_id));
    _label_to_bb_map.insert(std::make_pair(label, bb_id));
  }
  void Mark_goto_in_bb(CFG_BB_IDX bb, LABEL_IDX label) {
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
      (TFile, "[CFG-Builder] Marking up BB %d goto label %d = %04x\n", bb, label, label));
    _bb_goto_label.insert(std::make_pair(bb, label));
    _revisit.push_back(bb);
  }
  void Fixup_pred_succ() {
    if (!_cfg) {
      // cfg not ready.
      AssertThat(false, ("Call Init first before using fixup pred-succ utility. nullptr found"));
      return;
    }
    Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA), (TFile, "[CFG-Builder] Fixing up BB's pred / succ relations..\n"));
    for(auto it = _revisit.begin(); it != _revisit.end(); it++) {
      CFG_BB_IDX such_bb = *it;
      LABEL_IDX to_label = 0;
      if (_bb_goto_label.find(such_bb) != _bb_goto_label.end()) {
        to_label = _bb_goto_label[such_bb];
      } else {
        AssertThat(to_label != 0,
          ("Couldn't find BB %d in bb-goto-label map, map's size = %d",
          such_bb, _bb_goto_label.size()));
      }
      AssertThat(to_label != 0,
        ("The goto target label cannot be zero for BB = %d", such_bb));
      CFG_BB_IDX target_bb = 0;
      if (_label_to_bb_map.find(to_label) != _label_to_bb_map.end()) {
        target_bb = _label_to_bb_map[to_label];
      } else {
        AssertThat(target_bb != 0,
          ("Couldn't find target BB for label %d in label-bb map, map's size = %d",
          (INT32) to_label, _label_to_bb_map.size()));
      }
      AssertThat(target_bb != 0,
        ("The target bb cannot be zero for label = %d", to_label));
      AssertThat(such_bb < _cfg->Size() && target_bb < _cfg->Size(),
        ("The such_bb = %d, target_bb = %d, not be valid bb id, for label = %d", such_bb, target_bb, to_label));
      // pred: such_bb, succ: target_bb
      Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
        (TFile, "[CFG-Builder] Add relation BB %d -> %d, found via label = %04x..\n", such_bb, target_bb, to_label));
      _cfg->BB(such_bb)->Add_succ(_cfg->BB(target_bb));
      _cfg->BB(target_bb)->Add_pred(_cfg->BB(such_bb));
    }
  }
  void Unbind_all_labels() {
    _label_to_bb_map.clear();
  };
};

// We probably need a SSA based stuff;

#endif //CFG_COMMON_H