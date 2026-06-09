//
// Created by xc5 on 2024/11/4.
//

#ifndef OCC_OPT_BASIC_H
#define OCC_OPT_BASIC_H

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include "symtab.h"
#include "cfg_common.h"
#include "ir.h"

// OPT_STAB 在 opt_stab.h 中定义。这里前向声明以打破循环。
class OPT_STAB;

using std::vector;
using std::map;
using std::set;

enum IR_LEVEL {
  LEVEL_VHIGH = 1,
  LEVEL_HIGH  = 2,
  LEVEL_MID   = 3,
  LEVEL_LOW   = 4,
  LEVEL_VLOW  = 5,
  LEVEL_CGIR  = 6,
};

// Node declaration
class SSANODE {
private:
  IRNODE_IDX _node_idx;
public:
  void Print(FILE *file);
};

/**
 * Code storage
 */
class CODE_STORE {
  void Append_code() {
  }
  void Print(FILE * file) {};
};

class OPT_SYMTAB {
private:
  vector<ST_IDX> _ir_tree_syms;
public:
  void Print(FILE * file) {};
};

// Forward Decl.
template <typename NODE_TYPE> class CFG_BB_BASE;
template <typename NODE_TYPE> class SSA_CFG_BB_BASE;

template<typename NODE_TYPE, typename BB_TYPE>
class SSA_CONV_EXTRAINFO {
private:
};

template <typename NODE_TYPE>
class SSA_CFG_BASE : public CFG_BASE<NODE_TYPE, SSA_CFG_BB_BASE<NODE_TYPE>> {
private:
public:
  void Print(FILE *file = stderr)  {
    // Print ...
    fprintf(file, "%sPrinting control flow graph, of size : %d\n%s",
            DBAR, this->Size(), DBAR);
    for (UINT32 i = 0; i < this->Size(); i++) {
      this->Node(i)->Print(file);
    }
  }
};

template <typename NODE_TYPE>
class SSA_CFG_BB_BASE : public CFG_BB_BASE<NODE_TYPE> {
public:
  // PHI 列表（此 BB 头部的所有 phi 函数）
  std::vector<class PHI_NODE *>  _phi_list;
  // STMTREP 列表（SSA 化的语句）
  std::vector<class STMTREP *>   _stmtreps;

  void Add_phi(class PHI_NODE *p) { _phi_list.push_back(p); }
  void Add_stmtrep(class STMTREP *s) { _stmtreps.push_back(s); }

  SSA_CFG_BB_BASE(UINT32 block_id) : CFG_BB_BASE<NODE_TYPE>(block_id) {}
  SSA_CFG_BB_BASE() : CFG_BB_BASE<NODE_TYPE>() {}
};

// BB definitions
typedef SSA_CFG_BB_BASE<SSANODE>             SSABB;
typedef vector<SSABB *>                      SSABB_VECTOR;
typedef typename vector<SSABB *>::iterator   SSABB_ITER;
typedef SSA_CFG_BASE<SSANODE>                SSA_CFG;
typedef SSA_CONV_EXTRAINFO<SSANODE, SSABB>   SSA_CONV_INFO;


// Whole IR global data structure
class SSAIR {
private:
// CGIR_Table
  map<ST_IDX, SSA_CFG *>       _trees;
  map<ST_IDX, OPT_STAB *>      _ssa_symtab;
  map<ST_IDX, CODE_STORE *>    _ssa_codemap;
  SSA_CFG                     *_current         = nullptr;
  OPT_STAB                    *_current_symtab  = nullptr;
  CODE_STORE                  *_current_codemap = nullptr;
  ST_IDX                       _current_sym     = 0;
  TREE                        *_current_tree    = nullptr;
public:
  SSAIR();
  ~SSAIR();
  map<ST_IDX, SSA_CFG *>      &Trees() { return _trees; }
  map<ST_IDX, OPT_STAB *>     &Symtab_map() { return _ssa_symtab; }

  SSA_CFG       *Get_function(ST_IDX func_sym);  // 实现见 opt_ssa.cxx
  SSA_CFG       *Cfg() { return _current; }
  TREE         *Current_tree() {
    AssertThat(_current_tree != nullptr, ("current tree must be set first."));
    return _current_tree;
  };
  OPT_STAB     *Current_symtab() {
    AssertThat(_current_symtab != nullptr, ("Uninitialized current sym-table when accessed."));
    return _current_symtab;
  }
  CODE_STORE    *Code_store() {
    AssertThat(_current_codemap != nullptr, ("Uninitialized current code store when accessed."));
    return _current_codemap;
  };
  ST_IDX         Current_func_sym() {
    AssertThat(ST_st(_current_sym) != nullptr, ("Uninitialized current sym when accessed."));
    return _current_sym;
  };
  // Externally defined functions.
  void          Print(FILE *file = stderr);
  void          Print(ST_IDX sym, FILE *file = stderr);
  void          Goto_function(ST_IDX sym); // setup SSAIR
};

/**
 * SSA's builder utility.
 */
class SSA_BUILDER {
private:
  SSAIR                       *_ssair = nullptr;
  TREE                        *_tree  = nullptr;
public:
  void          Init(SSAIR *ssair) {
    _ssair = ssair;
    AssertThat(ssair != nullptr, ("SSA-ir must be a valid SSAIR object"));
  }
  SSAIR         *SSAir() {
    AssertThat(_ssair != nullptr,
               ("ssa-ir is not initialized in Live range analysis"));
    return _ssair;
  }
  SSA_CFG       *Cfg() {
    SSA_CFG  *cfg = SSAir()->Cfg();
    AssertThat(cfg != nullptr, ("Couldn't get cfg from cgir."));
    return cfg;
  }
  TREE *Tree() {
    return SSAir()->Current_tree();
  }
};

/**
 * The Entire CG class to access.
 */
class SSA_COMPOSITE {
private:
  SSAIR            *_ssair = nullptr;
  SSA_BUILDER       _builder;
public:
  void Init() {
    _ssair = new SSAIR();
    AssertThat(_ssair != nullptr, ("SSA-IR must be a valid SSA-IR"));
    _builder.Init(_ssair);
  }
  SSAIR         *SSA_ir() {
    AssertThat(_ssair != nullptr,
               ("SSA-IR is not initialized in Live range analysis"));
    return _ssair;
  }
  SSA_BUILDER   &SSA_builder() { return _builder; }
  void           Construct_function(ST_IDX func_sym, SCOPE *scope, UINT32 ir_lvl);        // Convert a function to SSA form.
  void           Gen_name(OPT_SYMTAB *opt_stab, UINT32 var_idx);
  void           Value_number(CODE_STORE *hstore, OPT_SYMTAB *opt_stab, SSABB *bb );
  void           Collect_defs_bb();
  void           Place_phi_node();
  void           SSA_Rename();
  void           SSA_Rename_rhs();
  BOOL           Verify_stack(OPT_SYMTAB *symtab);
  //, COPYPROP *copyprop, EXC *exc);
  void           Print(FILE*);
};

#endif //OCC_OPT_BASIC_H
