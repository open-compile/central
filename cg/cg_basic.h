//
// Created by xc5 on 2020/7/24.
//

#ifndef OCC_CG_BASIC_H
#define OCC_CG_BASIC_H

#include "basic.h"

enum CGOPC {
#define CGOPDEF(enum_name, nres, nopr, is_w, opr1, opr2, opr3, ins_name, opk)   \
  enum_name,
#include "cg_opc.h"
#undef CGOPDEF
};


enum CGOPC_KIND {
  CGOPK_LDST,
  CGOPK_UBR,
  CGOPK_CBR,
  CGOPK_NONE,
};

struct CGOPC_INFO {
  const char *name;
  CGOPC opcode;
  UINT8 n_res;  // num of results
  UINT8 n_oprs; // num of operands
  BOOL write_to_rd;
  CGOPR_KIND op1;
  CGOPR_KIND op2;
  CGOPR_KIND op3;
  const char *ins_token;
  CGOPC_KIND opk;

  BOOL isWriteToRd() const {
    return write_to_rd;
  }

  void isWriteToRd(BOOL isWriteToRd) {
    write_to_rd = isWriteToRd;
  }

  const char *getName() const {
    return name;
  }

  void setName(const char *name) {
    CGOPC_INFO::name = name;
  }

  CGOPC getOpcode() const {
    return opcode;
  }

  void setOpcode(CGOPC opcode) {
    CGOPC_INFO::opcode = opcode;
  }

  UINT8 getNRes() const {
    return n_res;
  }

  void setNRes(UINT8 nRes) {
    n_res = nRes;
  }

  UINT8 getNOprs() const {
    return n_oprs;
  }

  void setNOprs(UINT8 nOprs) {
    n_oprs = nOprs;
  }

  CGOPR_KIND getOp1() const {
    return op1;
  }

  void setOp1(CGOPR_KIND op1) {
    CGOPC_INFO::op1 = op1;
  }

  CGOPR_KIND getOp2() const {
    return op2;
  }

  void setOp2(CGOPR_KIND op2) {
    CGOPC_INFO::op2 = op2;
  }

  CGOPR_KIND getOp3() const {
    return op3;
  }

  void setOp3(CGOPR_KIND op3) {
    CGOPC_INFO::op3 = op3;
  }

  const char *getInsToken() const {
    return ins_token;
  }

  void setInsToken(const char *insToken) {
    ins_token = insToken;
  }

  CGOPC_KIND getOpk() const {
    return opk;
  }

  void setOpk(CGOPC_KIND opk) {
    CGOPC_INFO::opk = opk;
  }
};


UINT8 ISA_OPCODE_results(CGOPC cgopc);

UINT8 ISA_OPCODE_operands(CGOPC cgopc);

const char *ISA_OPCODE_name(CGOPC cgopc);

class CG_FRAME_SECT {
public:
  const char * name;
  UINT32       section_id;
  UINT32       section_size;
  UINT32       section_offset;
};

typedef UINT32 CG_OPRAND;

// CGOP flags.
enum CGOPF {
  CGOPF_NONE  =  0x0000,
  CGOPF_SPILL =  0x0001,
};

class CGOP {
private:
  CGOPC        opcode;         // assign, ... ...
  UINT8        results;
  UINT8        operands;

  // Other related info
  CFG_BB_IDX   bb;
  UINT32       _flags = 0;        // flags related to the CGOP
  UINT32       index_in_bb;   // index inside the BB, unique in BB
  UINT16       variant;
  UINT16       spill_generated;

  // unroll related
  CFG_BB_IDX   unroll_bb;
  UINT32       orig_id;
  UINT8        which_unroll;
  UINT32       res_opnd[5];
  UINT32       _tree_node_id;

public:
  CGOP (CGOPC opc, CFG_BB_IDX bb_idx,
        UINT32 op1, UINT32 op2,
        UINT32 op3, UINT32 op4) {
    opcode = opc;
    results = ISA_OPCODE_results(opc);
    operands = ISA_OPCODE_operands(opc);
    bb = bb_idx;
    index_in_bb = 0;
    spill_generated = 0;
    res_opnd[0] = op1;
    res_opnd[1] = op2;
    res_opnd[2] = op3;
    res_opnd[3] = op4;
    res_opnd[4] = 0;
    _tree_node_id = 0;
    _flags = 0;
  }
  CGOP (CGOPC opc, UINT32 tree_node_id, CFG_BB_IDX bb_idx,
        UINT32 op1, UINT32 op2,
        UINT32 op3, UINT32 op4) :
        CGOP (opc, bb_idx, op1, op2, op3, op4) {
    _tree_node_id = tree_node_id;
  }

  CGOPC getOpcode() const {
    return opcode;
  }

  UINT8 getResults() const {
    return results;
  }

  UINT8 getOperands() const {
    return operands;
  }

  CFG_BB_IDX getBb() const {
    return bb;
  }

  UINT32 getIndexInBb() const {
    return index_in_bb;
  }

  const UINT32 *getResOpnd() const {
    return res_opnd;
  }

  void setResOpnd(UINT8 id, UINT32 opr) {
    res_opnd[id] = opr;
  }

  void setFlags(UINT32 flags) {
    _flags |= flags;
  }

  void setIndexInBb(UINT32 indexInBb) {
    index_in_bb = indexInBb;
  }

  void setVariant(UINT16 variant) {
    CGOP::variant = variant;
  }

  void setUnrollBb(CFG_BB_IDX unrollBb) {
    unroll_bb = unrollBb;
  }

  void setOrigId(UINT32 origId) {
    orig_id = origId;
  }

  void setWhichUnroll(UINT8 whichUnroll) {
    which_unroll = whichUnroll;
  }
  void Print(FILE * file = stderr);
  UINT32 getFlags() { return _flags; };
  UINT32 getTreeNodeId() const { return _tree_node_id; }
  UINT16 getVariant() const { return variant; }

  UINT16 getSpillGenerated() const {
    return spill_generated;
  }

  void setSpillGenerated(UINT16 spillGenerated) {
    spill_generated = spillGenerated;
  }
};


#endif //OCC_CG_BASIC_H
