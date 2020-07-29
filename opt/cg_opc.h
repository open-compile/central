//
// Created by xc5 on 2020/7/27.
//
#ifndef CGOPDEF
#error "Cannot find CGOPDEF definition"
#endif

// Opcode                       write?  n_res,   n_oper, OPR1, OPR2, OPR3,
CGOPDEF( CGOPC_MOV   , 1,   1,  true,   CGOPR_R,  CGOPR_IMM,CGOPR_N,   "mov"  , CGOPK_NONE  ) // Memory
CGOPDEF( CGOPC_MOVT  , 1,   1,  true,   CGOPR_R,  CGOPR_IMM,CGOPR_N,   "movt" , CGOPK_NONE  ) // Memory
CGOPDEF( CGOPC_STR   , 1,   2,  false,  CGOPR_R,  CGOPR_R,  CGOPR_IMM, "str"  , CGOPK_LDST  )
CGOPDEF( CGOPC_LDRLBL, 1,   1,  true,   CGOPR_R,  CGOPR_R,  CGOPR_N,   "ldr"  , CGOPK_NONE  ) // load of label
CGOPDEF( CGOPC_LDR   , 1,   2,  true,   CGOPR_R,  CGOPR_R,  CGOPR_IMM, "ldr"  , CGOPK_LDST  )
CGOPDEF( CGOPC_LEAVE , 1,   0,  false,  CGOPR_R,  CGOPR_N,  CGOPR_N,   "leave", CGOPK_NONE  ) // Control
CGOPDEF( CGOPC_CALL  , 1,   1,  false,  CGOPR_R,  CGOPR_N,  CGOPR_N,   "call" , CGOPK_NONE  )
CGOPDEF( CGOPC_BR    , 0,   1,  false,  CGOPR_IMM,CGOPR_N,  CGOPR_N,   "br"   , CGOPK_NONE  )
CGOPDEF( CGOPC_B     , 0,   1,  false,  CGOPR_N,  CGOPR_IMM,CGOPR_N,   "b"    , CGOPK_NONE  ) // branch as well
CGOPDEF( CGOPC_BL    , 0,   1,  false,  CGOPR_N,  CGOPR_IMM,CGOPR_N,   "bl"   , CGOPK_NONE  ) // branch indirect with label
CGOPDEF( CGOPC_BX    , 0,   1,  false,  CGOPR_N,  CGOPR_R,  CGOPR_N,   "bx"   , CGOPK_NONE  ) // branch-register, not setting LR
CGOPDEF( CGOPC_BLX   , 0,   1,  false,  CGOPR_N,  CGOPR_R,  CGOPR_N,   "blx"  , CGOPK_NONE  ) // branch-register, will set LR
CGOPDEF( CGOPC_BEQ   , 1,   1,  false,  CGOPR_R,  CGOPR_R,  CGOPR_R,   "beq"  , CGOPK_NONE  )
CGOPDEF( CGOPC_BNE   , 1,   1,  false,  CGOPR_R,  CGOPR_R,  CGOPR_R,   "bne"  , CGOPK_NONE  )
CGOPDEF( CGOPC_BGE   , 1,   1,  false,  CGOPR_R,  CGOPR_R,  CGOPR_R,   "bge"  , CGOPK_NONE  )
CGOPDEF( CGOPC_BLT   , 1,   1,  false,  CGOPR_R,  CGOPR_R,  CGOPR_R,   "blt"  , CGOPK_NONE  )
CGOPDEF( CGOPC_BGT   , 1,   1,  false,  CGOPR_R,  CGOPR_R,  CGOPR_R,   "bgt"  , CGOPK_NONE  )
CGOPDEF( CGOPC_BLE   , 1,   1,  false,  CGOPR_R,  CGOPR_R,  CGOPR_R,   "ble"  , CGOPK_NONE  )
CGOPDEF( CGOPC_ADD   , 1,   2,  true,   CGOPR_R,  CGOPR_R,  CGOPR_R,   "add"  , CGOPK_NONE  ) // Arithmetic ... TODO: to be addeed
CGOPDEF( CGOPC_MUL   , 1,   2,  true,   CGOPR_R,  CGOPR_R,  CGOPR_R,   "mul"  , CGOPK_NONE  )
CGOPDEF( CGOPC_SUBS  , 1,   2,  true,   CGOPR_R,  CGOPR_R,  CGOPR_R,   "subs" , CGOPK_NONE  )