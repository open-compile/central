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
  OPERATOR_UNKNOTREE = 0,
  OPERATOR_FIRST = 1,
  OPR_ABS = 1,
  OPR_ADD = 2,
  OPR_AGOTO = 3,
  OPR_ALTENTRY = 4,
  OPR_ARRAY = 5,
  OPR_ARRAYEXP = 6,
  OPR_ARRSECTION = 7,
  OPR_ASHR = 8,
  OPR_ASSERT = 9,
  OPR_BACKWARD_BARRIER = 10,
  OPR_BAND = 11,
  OPR_BIOR = 12,
  OPR_BLOCK = 13,
  OPR_BNOR = 14,
  OPR_BNOT = 15,
  OPR_BXOR = 16,
  OPR_CALL = 17,
  OPR_CAND = 18,
  OPR_CASEGOTO = 19,
  OPR_CEIL = 20,
  OPR_CIOR = 21,
  OPR_COMMA = 22,
  OPR_COMMENT = 23,
  OPR_COMPGOTO = 24,
  OPR_PAIR = 25,
  OPR_CONST = 26,
  OPR_CSELECT = 27,
  OPR_CVT = 28,
  OPR_CVTL = 29,
  OPR_DIV = 30,
  OPR_DIVREM = 31,
  OPR_DO_LOOP = 32,
  OPR_DO_WHILE = 33,
  OPR_EQ = 34,
  OPR_EVAL = 35,
  OPR_EXC_SCOPE_BEGIN = 36,
  OPR_EXC_SCOPE_END = 37,
  OPR_FALSEBR = 38,
  OPR_FLOOR = 39,
  OPR_FORWARD_BARRIER = 40,
  OPR_FUNC_ENTRY = 41,
  OPR_GE = 42,
  OPR_GOTO = 43,
  OPR_GT = 44,
  OPR_HIGHMPY = 45,
  OPR_HIGHPART = 46,
  OPR_ICALL = 47,
  OPR_IDNAME = 48,
  OPR_IF = 49,
  OPR_ILDA = 50,
  OPR_ILDBITS = 51,
  OPR_ILOAD = 52,
  OPR_ILOADX = 53,
  OPR_SECONDPART = 54,
  OPR_INTCONST = 55,
  OPR_INTRINSIC_CALL = 56,
  OPR_INTRINSIC_OP = 57,
  OPR_IO = 58,
  OPR_IO_ITEM = 59,
  OPR_ISTBITS = 60,
  OPR_ISTORE = 61,
  OPR_ISTOREX = 62,
  OPR_LABEL = 63,
  OPR_LAND = 64,
  OPR_LDA = 65,
  OPR_LDBITS = 66,
  OPR_LDID = 67,
  OPR_LE = 68,
  OPR_LIOR = 69,
  OPR_LNOT = 70,
  OPR_LOOP_INFO = 71,
  OPR_LOWPART = 72,
  OPR_LSHR = 73,
  OPR_LT = 74,
  OPR_MADD = 75,
  OPR_MAX = 76,
  OPR_MAXPART = 77,
  OPR_MIN = 78,
  OPR_MINMAX = 79,
  OPR_MINPART = 80,
  OPR_MLOAD = 81,
  OPR_MOD = 82,
  OPR_MPY = 83,
  OPR_MSTORE = 84,
  OPR_MSUB = 85,
  OPR_NE = 86,
  OPR_NEG = 87,
  OPR_NMADD = 88,
  OPR_NMSUB = 89,
  OPR_OPTPARM = 90,
  OPR_OPT_CHI = 91,
  OPR_OPT_RESERVE2 = 92,
  OPR_PAREN = 93,
  OPR_PARM = 94,
  OPR_PICCALL = 95,
  OPR_PRAGMA = 96,
  OPR_PREFETCH = 97,
  OPR_PREFETCHX = 98,
  OPR_RCOMMA = 99,
  OPR_FIRSTPART = 100,
  OPR_RECIP = 101,
  OPR_REGION = 102,
  OPR_REGION_EXIT = 103,
  OPR_REM = 104,
  OPR_RETURN = 105,
  OPR_RETURN_VAL = 106,
  OPR_RND = 107,
  OPR_RSQRT = 108,
  OPR_SELECT = 109,
  OPR_SHL = 110,
  OPR_SQRT = 111,
  OPR_STBITS = 112,
  OPR_STID = 113,
  OPR_SUB = 114,
  OPR_SWITCH = 115,
  OPR_TAS = 116,
  OPR_TRAP = 117,
  OPR_TRIPLET = 118,
  OPR_TRUEBR = 119,
  OPR_TRUNC = 120,
  OPR_VFCALL = 121,
  OPR_WHERE = 122,
  OPR_WHILE_DO = 123,
  OPR_XGOTO = 124,
  OPR_XMPY = 125,
  OPR_XPRAGMA = 126,
  OPR_AFFIRM = 127,
  OPR_ALLOCA = 128,
  OPR_DEALLOCA = 129,
  OPR_LDMA = 130,
  OPR_ASM_STMT = 131,
  OPR_ASM_EXPR = 132,
  OPR_ASM_INPUT = 133,
  OPR_RROTATE = 134,
  OPR_LDA_LABEL = 135,
  OPR_GOTO_OUT = 136,
  OPR_EXTRACT_BITS = 137,
  OPR_COMPOSE_BITS = 138,
  OPERATOR_LAST = 138,
  // Don't add operators here, if you need extra operators, discuss it with @Jason
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

/**
 * Utility function to get a IRNODE&
 * @return
 */
//IRNODE &TNode(IR_TREE_ELEM elem);

#endif // _OCC_COMMON_TREE_H_
#pragma clang diagnostic pop