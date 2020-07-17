//
// Created by xc5 on 2020/7/15.
//

#ifndef OCC_ARM_REG_H
#define OCC_ARM_REG_H


#define Int_Preg_Min_Offset             1
#define Int_Preg_Max_Offset             31
#define Float_Preg_Min_Offset           32
#define Float_Preg_Max_Offset           63
#define Fcc_Preg_Min_Offset             64
#define Fcc_Preg_Max_Offset             71
#define Last_Dedicated_Preg_Offset      Fcc_Preg_Max_Offset

/* The offsets for return registers are fixed: */ 
#define First_Int_Preg_Return_Offset	2	/* register v0 */
#define Last_Int_Preg_Return_Offset	3	/* register v1 */
#define First_Float_Preg_Return_Offset	32	/* register f0 */
#define Last_Float_Preg_Return_Offset	34	/* register f2 */
#define Stack_Pointer_Preg_Offset	29	/* register sp */
#define Frame_Pointer_Preg_Offset	30	/* register fp */
#define First_Int_Preg_Param_Offset	4	/* register a0 */
#define First_Float_Preg_Param_Offset	44	/* register f12 */
#define Static_Link_Preg_Offset		2

#define MAX_NUMBER_OF_REGISTERS_FOR_RETURN 2
#define MAX_NUMBER_OF_REGISTER_PARAMETERS 8

/* define an enumeration of the different levels of mtypes/whirl
 * that we can use for returns */
typedef enum {
  Use_Simulated,		/* use simulated types */
  Complex_Not_Simulated, 	/* lower complex but not quad */
  No_Simulated		/* all lowered to machine-level */
} Mtype_Return_Level;


class RETURN_INFO {
private:
  mINT8    count;
  BOOL     return_via_first_arg;
  TYPE_ID  mtype [MAX_NUMBER_OF_REGISTERS_FOR_RETURN];
  PREG_NUM preg  [MAX_NUMBER_OF_REGISTERS_FOR_RETURN];
public:
  friend inline mINT8 RETURN_INFO_count (const RETURN_INFO&);
  friend inline BOOL  RETURN_INFO_return_via_first_arg (const RETURN_INFO&);
  friend inline TYPE_ID  RETURN_INFO_mtype (const RETURN_INFO&, INT32);
  friend inline PREG_NUM RETURN_INFO_preg (const RETURN_INFO&, INT32);
  friend RETURN_INFO Get_Return_Info (TY_IDX rtype, Mtype_Return_Level level
#ifdef TARG_X8664
    , BOOL ff2c_abi = FALSE
#endif
  );
};

/* return whether preg is a return preg */
extern BOOL Is_Return_Preg (PREG_NUM preg);

/* return whether preg is an output preg */
extern BOOL Is_Int_Output_Preg (PREG_NUM preg);

/* return whether preg is FP output preg */
extern BOOL Is_Fp_Output_Preg (PREG_NUM preg);

/* return whether preg is an input parameter preg */
extern BOOL Is_Formal_Preg (PREG_NUM preg);

#endif //OCC_ARM_REG_H
