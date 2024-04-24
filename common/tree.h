#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedGlobalDeclarationInspection"
#ifndef _OCC_COMMON_TREE_H_
#define _OCC_COMMON_TREE_H_
/**
 *  Copyright SZU Compiler Team 2020 
 *  Author : @Guanting.Lu
 * 
 *  This is where most of the important definitions are presented
 *  we define the tree for containing the semantic content of the program we are about to compile
 *  
 *  Data Stuctures
 *  OIR
 *   TREE
 *   TY_TABLE
 *   ST_TABLE
 *   PREG_TABLE
 *   LABEL_TABLE
 *   ... 
 *  
 *  
 * 
 * */
#include "basic.h"
#include "consts.h"
#include "tree_util.h"
#include <memory.h>

enum OPERATOR {
#define OCIR_OPR(opr, opr_int, lhs, rhs, br, mem, cnst, commut) opr = opr_int,
#include "opr_base.h"
  // Don't add operators here, if you need extra operators, discuss it with @Jason
#undef OCIR_OPR
};

#define RTYPE(x) (x<<8)
#define DESC(x)  (x<<14)

#define MTYPE_AA MTYPE_A4

enum OPCODE {
// Define the logic here
#define OCIR_OPC(opc_enum, opr, mtype_res, mtype_desc) \
  opc_enum = opr + RTYPE(mtype_res) + DESC(mtype_desc),
// Use the def table
#include "opc_base.h"
#undef OCIR_OPC
};

typedef struct {
  OPCODE    opc;
  OPERATOR  opr;
  MTYPE_ID  res;
  MTYPE_ID  desc;
  const char *name;
} OPCODE_INFO;

typedef struct {
  OPERATOR    opr;
  INT32       opr_intval;
  INT32       num_lhs;
  INT32       num_rhs;
  INT32       is_branch;
  INT32       is_mem_access;
  INT32       is_const_exp;
  INT32       is_commutative; // being able to treat a+b as b+a
  const char *name;
} OPERATOR_INFO;

enum REGION_KIND{
  REGION_KIND_1 = 1,
};

enum INTRINSIC {
  INTRN_I4EXPEXPR = 1,
};

enum GOTO_OUT_LABELS {
  GOTO_NONE = 0,
  GOTO_OUT_BREAK = 1,
  GOTO_OUT_CONTINUE = 2,
};

class IRNODE {
public:
  OPCODE opcode;
  union {
    struct {
      union {
        TREE_OFFSET load_offset; // load offset or preg num.
        TREE_OFFSET lda_offset;
        TREE_OFFSET store_offset;
        TREE_OFFSET idname_offset;
        INT32       num_entries; /* used by computed goto statements; may be used by regions */
        INT32       label_number;
        UINT32      call_flag;
        UINT32      if_flag;
        struct {
          UINT16 trip_est;
          UINT16 loop_depth;
        }           li;
        struct {
          UINT16 pragma_extra_flag;
          UINT16 pragma_extra_id;
        }           pragma;
        struct {
          REGION_KIND rg_kind: 4;
          UINT32      rg_id: 28;
        }           region;
      } c1;
      union {
        ST_IDX    st_idx;  /* for ldid/stid/lda  */
        TY_IDX    ty;    /* for all types except lda,ldid,stid */
        INTRINSIC intrinsic; // For intrinsic operations, not used right now.
        UINT32    prefetching_extra_flag; // For prefetching.
        UINT32    loop_extra_flag;
        INT32     the_last_label;  /* end of switch */
        INITO_IDX region_filter_table;  // for region
        UINT32    label_level; /* nest level for target of goto_outer_block */
      } c2;
    } combined;
    TREE_ESIZE	    element_size;
  } extra1;
  union {
    struct {
      TY_IDX ty;    /* ty used for lda,ldid,stid,iload */
    }          ty_fields;
    IRNODE_IDX kids[2];
    INT64      const_val;
    struct {
      UINT32 num_inputs;
      UINT32 num_clobbers;
    }          asm_fields;
    struct {
      IRNODE_IDX dummy2;
      UINT32     label_flag;
    }          label_flag_fields;
    union {
      INT64 pragma_info64;
      struct {
        union {
          IRNODE_IDX dummy3;
          INT32      pragma_arg1;
        };
        union {
          INT32 pragma_arg2;
          struct {
            UINT32 pragma_asm_opnd_num: 8;
//            PREG_NUM pragma_asm_copyout_preg : 24;
          }     asm_pragma;
        };
      }     up1;
      struct {
        INT16 pragma_pad1;
        INT8  pragma_distr_type;
        INT8  pragma_index;
        INT32 pragma_preg;
      }     up2;
    }          pragma;
  } extra3;
  IRNODE() {
    memset(this, 0, sizeof(IRNODE));
  };
  IRNODE(OPCODE op) {
    memset(this, 0, sizeof(IRNODE));
    opcode = op;
  }
  // Move/Copy construction
  IRNODE(const IRNODE &op) {
    memcpy(this, &op, sizeof(IRNODE));
  }
  IRNODE(IRNODE &op) {
    memcpy(this, &op, sizeof(IRNODE));
  }
  IRNODE_IDX &Opnd(UINT32 pos);
  void Print(FILE *f);
  OPCODE &Opcode() { return opcode; }
  void Set_opcode(OPCODE opc) { opcode = opc; }

  void Set_symbol_idx(ST_IDX sym) { extra1.combined.c2.st_idx = sym; };
  ST_IDX Get_symbol_idx() { return extra1.combined.c2.st_idx; };

  void Set_load_offset(TREE_OFFSET ofst) { extra1.combined.c1.load_offset = ofst; };
  TREE_OFFSET Get_load_offset() { return extra1.combined.c1.load_offset; };

  void Set_type_idx(TY_IDX sym) { extra1.combined.c2.ty = sym; };
  TY_IDX Get_type_idx() { return extra1.combined.c2.ty; };

  void Set_const_val(UINT64 i) { extra3.const_val = i; }
  UINT64 Get_const_val() { return extra3.const_val; }

  UINT32 Get_preg_num()  { return extra1.combined.c1.load_offset; }
  void Set_preg_num(PREG_IDX preg_num)  { extra1.combined.c1.load_offset = preg_num; }

  UINT32 Get_label_num()  { return extra1.combined.c1.label_number; }
  void Set_label_num(LABEL_IDX label_n)  { extra1.combined.c1.label_number = label_n; }

  UINT32 Get_goto_out_level() { return extra1.combined.c2.label_level; }
  void Set_goto_out_level(UINT32 lvl) {  extra1.combined.c2.label_level = lvl; }
};

typedef IRNODE_IDX IR_TREE_ELEM;

using IRTREE = tree<IR_TREE_ELEM>;
using IR_ITER = tree<IR_TREE_ELEM>::iterator;
using IR_PRE_ITER = tree<IR_TREE_ELEM>::pre_order_iterator;
using IR_POST_ITER = tree<IR_TREE_ELEM>::post_order_iterator;
using IR_LEAF_ITER = tree<IR_TREE_ELEM>::leaf_iterator;
using IR_SIBLING_ITER = tree<IR_TREE_ELEM>::sibling_iterator;

class TREE{
  IRTREE irtree;
  std::vector<IRNODE> _ir_elem_tab;
public:
  TREE() : irtree() {
    Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
             (TFile, "Creating TREE, size = %lu\n", irtree.size()));
  }
  void Print_recursive(FILE *f);
  void Print();
  IRNODE *Get_node(IRNODE_IDX iridx);
  IRNODE *Get_node(IR_ITER ir_it) { return Get_node(*ir_it); } //alias
  IRNODE *Node(IR_ITER ir_it)     { return Get_node(ir_it);  } //alias
  IRNODE *Node(IRNODE_IDX ir_idx) { return Get_node(ir_idx); } //alias
  IRNODE_IDX Create_node();
  IRNODE_IDX Create_node(OPCODE opc);
  // Kid access
  IR_ITER Insert_stmt_to_block(IR_ITER block, IR_ITER child);
  IR_ITER Insert_stmt_to_block(IR_ITER block, IR_TREE_ELEM child);
  IR_ITER Add_child(IR_ITER parent, IR_TREE_ELEM child);
  // Operand access
  IR_ITER Set_operand(IR_ITER parent, UINT32 pos, IR_ITER operand);
  IR_ITER Set_operand(IR_ITER parent, UINT32 pos, IR_TREE_ELEM opnd);
  IR_ITER Get_operand(IR_ITER node_iter, UINT32 kid_pos);
  // Root
  IR_ITER Get_root();
  IR_ITER Set_root(IR_TREE_ELEM root_pos);
  // Global
  void Initialize();
  IR_ITER Insert_temp_node(IRNODE_IDX idx);
  IR_ITER End();
  IRTREE &Internal_tree();
  UINT32 Number_of_children(IR_ITER node);
  UINT32 Number_of_siblings(IR_ITER node);
  UINT32 Index(IR_ITER node);
  IR_ITER Replace_recursive(IR_ITER pos, IR_ITER from);
  IR_ITER Remove_node_recursive(IR_ITER pos); // return incremented iter
  IR_ITER Get_parent_in_block(IR_ITER stmt);
  IR_ITER Get_parent_region(IR_ITER stmt);
  IR_ITER Insert_after(IR_ITER position, IRNODE_IDX node);
  IR_ITER Insert_before(IR_ITER position, IRNODE_IDX node);
  IR_ITER Copy_recursive(IR_ITER position, IR_ITER from);
  IR_ITER Get_parent(IR_ITER expr);
};

TREE *Tree();
void Set_current_tree(TREE *current);
MTYPE_ID OPCODE_rtype(OPCODE opc); // get the return type part from the opcode.
MTYPE_ID OPCODE_desc(OPCODE opc); // get the descriptor type part from the opcode.
OPERATOR OPCODE_operator(OPCODE opc); // get operator(non-typed) from opcode(typed)
const char *OPCODE_name(OPCODE opcode);
BOOL OPCODE_is_const(OPCODE opc);
BOOL OPCODE_is_bin_arith(OPCODE opc);
const char *OPERATOR_name(OPERATOR opr);
const char *OPCODE_name(OPCODE opc);

/**
 * Utility function to get a IRNODE&
 * @return
 */
//IRNODE &TNode(IR_TREE_ELEM elem);

#endif // _OCC_COMMON_TREE_H_
#pragma clang diagnostic pop