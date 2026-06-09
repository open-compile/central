#ifndef OCC_OPT_CODEREP_H
#define OCC_OPT_CODEREP_H

#include "basic.h"
#include "consts.h"
#include "tree.h"
#include "symtab.h"
#include "opt_basic.h"
#include <vector>

// 紧凑的变量 id 编号（OPT_STAB 内部）
typedef UINT32 AUX_ID;
typedef UINT32 VER_NUM;

// CODEREP: SSA 表达式节点
// 对应原始 IR 表达式子树的"值"
enum CR_KIND : UINT8 {
  CK_CONST = 1,  // 整数常数
  CK_LDA   = 2,  // 变量地址
  CK_VAR   = 3,  // SSA 版本化的具名变量
  CK_IVAR  = 4,  // 间接加载
  CK_OP    = 5,  // 表达式操作符
};

class STMTREP;     // 前置声明
class PHI_NODE;    // 前置声明

class CODEREP {
public:
  CR_KIND    _kind;
  MTYPE_ID   _mtype;   // 值类型
  UINT32     _id;      // 唯一 id（debug 用）

  union {
    INT64    _const_val;

    struct {
      ST_IDX _lda_sym;
      INT32  _lda_offset;
    } _lda;

    struct {
      AUX_ID  _aux_id;
      UINT32  _version;
      STMTREP  *_def_stmt;  // 二选一
      PHI_NODE *_def_phi;   // 二选一
    } _var;

    struct {
      CODEREP  *_base;
      INT32     _offset;
      TY_IDX    _ty;
    } _ivar;

    struct {
      OPCODE    _opc;
      UINT8     _num_kids;
      CODEREP  *_kids[2];
    } _op;
  };

  // 构造函数
  CODEREP() : _kind(CK_CONST), _mtype(MTYPE_UNKNOWN), _id(0) {
    memset(this, 0, sizeof(CODEREP));
  }

  // 类型判断
  BOOL Is_const() const { return _kind == CK_CONST; }
  BOOL Is_lda()   const { return _kind == CK_LDA;   }
  BOOL Is_var()   const { return _kind == CK_VAR;   }
  BOOL Is_ivar()  const { return _kind == CK_IVAR;  }
  BOOL Is_op()    const { return _kind == CK_OP;    }

  // 访问器
  INT64     Const_val() const { AssertThat(Is_const(), ("not a const")); return _const_val; }
  AUX_ID    Aux_id()    const { AssertThat(Is_var(),   ("not a var"));   return _var._aux_id; }
  UINT32    Version()   const { AssertThat(Is_var(),   ("not a var"));   return _var._version; }
  STMTREP  *Def_stmt()  const { AssertThat(Is_var(),   ("not a var"));   return _var._def_stmt; }
  PHI_NODE *Def_phi()   const { AssertThat(Is_var(),   ("not a var"));   return _var._def_phi; }
  CODEREP  *Opnd(UINT32 i) const {
    AssertThat(Is_op() && i < _op._num_kids, ("bad opnd idx"));
    return _op._kids[i];
  }
  UINT8     Num_kids() const { AssertThat(Is_op(), ("not an op")); return _op._num_kids; }
  OPCODE    Op()       const { AssertThat(Is_op(), ("not an op")); return _op._opc; }

  // 修改 def 指针
  void Set_def_stmt(STMTREP *s) { AssertThat(Is_var(), ("not a var")); _var._def_stmt = s; _var._def_phi = nullptr; }
  void Set_def_phi(PHI_NODE *p) { AssertThat(Is_var(), ("not a var")); _var._def_phi  = p; _var._def_stmt = nullptr; }

  // 工厂方法
  static CODEREP *Make_const(INT64 val, MTYPE_ID mtype);
  static CODEREP *Make_lda(ST_IDX sym, INT32 offset, MTYPE_ID mtype);
  static CODEREP *Make_var(AUX_ID aux, UINT32 ver, MTYPE_ID mtype);
  static CODEREP *Make_ivar(CODEREP *base, INT32 off, TY_IDX ty, MTYPE_ID mtype);
  static CODEREP *Make_op(OPCODE opc, CODEREP *k0, CODEREP *k1, MTYPE_ID mtype);

  void Print(FILE *f = stderr) const;
};


// PHI_NODE: 在 BB 头部的 SSA 合并函数
class PHI_NODE {
public:
  AUX_ID    _aux_id;
  UINT32    _result_ver;
  SSABB    *_bb;
  vector<UINT32>    _opnd_vers;     // 与 bb->_preds 一一对应
  CODEREP  *_result_cr;
  vector<CODEREP *> _opnd_crs;      // rename 阶段后填充

  BOOL      _live;        // DCE 标记
  BOOL      _dead;        // 不可达
  STMTREP  *_as_copy;     // out-of-SSA 时生成的 STID 模板

  PHI_NODE()
    : _aux_id(0), _result_ver(0), _bb(nullptr),
      _result_cr(nullptr), _live(FALSE), _dead(FALSE), _as_copy(nullptr) {}

  void Print(FILE *f = stderr) const;
};


// 简单的内存池，按函数分配一个，函数结束整体释放
class CR_POOL {
public:
  CR_POOL() : _next_id(1) {}
  ~CR_POOL() { for (auto *p : _crs) delete p; }

  CODEREP *Alloc() {
    CODEREP *cr = new CODEREP();
    cr->_id = _next_id++;
    _crs.push_back(cr);
    return cr;
  }

  PHI_NODE *Alloc_phi() {
    PHI_NODE *p = new PHI_NODE();
    _phis.push_back(p);
    return p;
  }

  STMTREP *Alloc_stmtrep();

  void Reset() {
    for (auto *p : _crs)   delete p;
    for (auto *p : _phis)  delete p;
    for (auto *p : _stmts) delete p;
    _crs.clear();
    _phis.clear();
    _stmts.clear();
    _next_id = 1;
  }

private:
  UINT32 _next_id;
  std::vector<CODEREP *>  _crs;
  std::vector<PHI_NODE *> _phis;
  std::vector<STMTREP *>  _stmts;
};

extern CR_POOL *Cr_pool();

#endif //OCC_OPT_CODEREP_H
