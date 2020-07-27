//
// Created by xc5 on 2020/7/24.
//

#ifndef OCC_CG_BASIC_H
#define OCC_CG_BASIC_H

#include "basic.h"

enum CGOPC {
#define CGOPDEF(enum_name, nres, nopr, opr1, opr2, opr3, ins_name, opk)   \
  enum_name,
#include "cg_opc.h"
#undef CGOPDEF
};

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
  CGOPR_KIND op1;
  CGOPR_KIND op2;
  CGOPR_KIND op3;
  const char *ins_token;
  CGOPC_KIND opk;

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

struct CG_OPRAND {
  union {
    UINT32  tn;
    UINT32  immediate;
  };
  operator int() {
    return tn;
  }
  CG_OPRAND(UINT32 tn_id) {
    tn = tn_id;
  }
  CG_OPRAND() {
    tn = 0;
  }
};

class CGOP {
private:
  CGOPC        opcode;         // assign, ... ...
  UINT8        results;
  UINT8        operands;
  TN_IDX       res_ops;   // operands

  // Other related info
  CFG_BB_IDX   bb;
  UINT32       flags;         // flags related to the CGOP
  UINT32       index_in_bb;   // index inside the BB, unique in BB
  UINT16       variant;

  // unroll related
  CFG_BB_IDX   unroll_bb;
  UINT32       orig_id;
  UINT8        which_unroll;
  CG_OPRAND    res_opnd[5];

public:
  CGOP (CGOPC opc, CFG_BB_IDX bb_idx,
        CG_OPRAND op1, CG_OPRAND op2,
        CG_OPRAND op3, CG_OPRAND op4) {
    opcode = opc;
    results = ISA_OPCODE_results(opc);
    operands = ISA_OPCODE_operands(opc);
    bb = bb_idx;
    index_in_bb = 0;
    res_opnd[0] = op1;
    res_opnd[1] = op2;
    res_opnd[2] = op3;
    res_opnd[3] = op4;
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

  const CG_OPRAND *getResOpnd() const {
    return res_opnd;
  }

  void setResOps(TN_IDX resOps) {
    res_ops = resOps;
  }

  void setFlags(UINT32 flags) {
    CGOP::flags = flags;
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
};


#endif //OCC_CG_BASIC_H
