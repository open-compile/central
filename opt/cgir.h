//
// Created by xc5 on 2020/7/13.
//

#ifndef OCC_CGIR_H
#define OCC_CGIR_H

#include "basic.h"
#include "symtab.h"
#include <vector>
#include "register.h"
#include "data_layout.h"

using std::vector;
typedef UINT32 CGBB_IDX;
typedef UINT32 TN_IDX;

enum CGOPC {
  // Memory, data transfer
  CGOPC_MOV,
  CGOPC_STR  ,
  CGOPC_LDR  ,
  // Control
  CGOPC_LEAVE,
  CGOPC_CALL ,
  CGOPC_BR,
  CGOPC_B, // branch as well
  CGOPC_BEQ,
  CGOPC_BNE,
  CGOPC_BGE,
  CGOPC_BLT,
  CGOPC_BGT,
  CGOPC_BLE,
  // Arithmetic
  CGOPC_ADD,
  CGOPC_MUL,
  CGOPC_SUBS,
};

class CG_FRAME_SECT {
public:
  const char * name;
  UINT32       section_id;
  UINT32       section_size;
  UINT32       section_offset;
};

struct CG_OPRAND {
  union {
    UINT32  tn;
    UINT32  immediate;
  };
};

class CGOP {
private:
  CGOPC        opcode;         // assign, ... ...
  UINT8        results;
  UINT8        operands;
  TN_IDX       res_ops;   // operands

  // Other related info
  CGBB_IDX     bb;
  UINT32       flags;         // flags related to the CGOP
  UINT32       index_in_bb;   // index inside the BB, unique in BB
  UINT16       variant;

  // unroll related
  CGBB_IDX     unroll_bb;
  UINT32       orig_id;
  UINT8        which_unroll;
};

class CGBB {
  UINT32 block_id;
};

typedef std::vector<CGOP> CGIR_TREE;

class CGIR {
private:
  // Memory Layout
  CG_FRAME_SECT sections[8];
  // CGIR_Table
  CGIR_TREE     tree;
  DATA_LAYOUT   layout;
public:
  void          CG_Init(SCOPE *scope);        // Initialize the CG stuff
  void          Data_layout(SCOPE *scope);    // Do data layout
};

#endif //OCC_CGIR_H
