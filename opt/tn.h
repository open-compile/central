//
// Created by xc5 on 2020/7/24.
//

#ifndef OCC_TN_H
#define OCC_TN_H

#include "basic.h"
#include "consts.h"
#include "register.h"

typedef enum {
  TN_RELOC_NONE	   = 0x00,
  TN_RELOC_GPREL16   = 0x01,	/* gp-relative reference to symbol */
  TN_RELOC_LOW16	   = 0x02,	/* Bits 0..15 of symbolic val */
  TN_RELOC_HIGH16	   = 0x03,	/* Bits 16..31 of symbol */
  TN_RELOC_HIGHER	   = 0x04,	/* Bits 32..47 of symbol */
  TN_RELOC_HIGHEST   = 0x05,	/* Bits 48..63 of symbol */
  TN_RELOC_GOT_DISP  = 0x06,
  TN_RELOC_GOT_PAGE  = 0x07,
  TN_RELOC_GOT_OFST  = 0x08,
  TN_RELOC_CALL16    = 0x09,
  TN_RELOC_GOT_HI16  = 0x0a,
  TN_RELOC_GOT_LO16  = 0x0b,
  TN_RELOC_CALL_HI16 = 0x0c,
  TN_RELOC_CALL_LO16 = 0x0d,
  TN_RELOC_NEG	     = 0x0e,
  TN_RELOC_GPSUB     = 0x0f,	/* gp - sym (pic2 prolog) */
  TN_RELOC_LO_GPSUB  = 0x10,
  TN_RELOC_HI_GPSUB  = 0x11,
  TN_RELOC_GPIDENT   = 0x12,	/* gp value (pic1 prolog) */
  TN_RELOC_LO_GPIDENT= 0x13,
  TN_RELOC_HI_GPIDENT= 0x14,
  TN_RELOC_IA_IMM14	= 0x20,      /* IA symbols starts at 0x20 */
  TN_RELOC_IA_IMM22	= 0x21,
  TN_RELOC_IA_PCREL	= 0x22,
  TN_RELOC_IA_GPREL22	= 0x23,
  TN_RELOC_IA_LTOFF22	= 0x24,
  TN_RELOC_IA_LTOFF_FPTR = 0x25,
  TN_RELOC_ARM_PC24      = 0x30, // S – P + A  ARM B/BL
  TN_RELOC_ARM_ABS32     = 0x31, // S + A   32-bit word
  TN_RELOC_ARM_REL32     = 0x32, // S – P + A   32-bit word
  TN_RELOC_ARM_PC13      = 0x33, // S – P + A,  ARM LDR r, [pc,…]
  TN_RELOC_ARM_ABS16     = 0x34, // S + A  16-bit half-word
  TN_RELOC_ARM_ABS12     = 0x35, // S + A, ARM LDR/STR
  TN_RELOC_ARM_THM_ABS5  = 0x36, // S + A, Thumb LDR/STR
  TN_RELOC_ARM_ABS8      = 0x37, // S + A  8-bit byte
  TN_RELOC_ARM_SBREL32   = 0x38, // S - B + A, 32-bit word
  TN_RELOC_ARM_THM_PC22  = 0x39, // S - P + A, Thumb BL pair
  TN_RELOC_ARM_THM_PC8   = 0x3a, // S - P + A, Thumb BL pair
  TN_RELOC_ARM_TPOFF32_seg_reg = 0x35, /* like above, with segment register */
  TN_RELOC_ARM_GOTTPOFF   = 0x36,	 /* TLS with GOT entry */
  TN_RELOC_ARM_GOTNTPOFF  = 0x37,      /* TLS with GOT entry under PIC */
  TN_RELOC_ARM_DTPOFF     = 0x38,      /* Local Dynamic TLS */
  TN_RELOC_ARM_TLSGD      = 0x39,      /* Global Dynamic TLS */
  TN_RELOC_ARM_TLSLD      = 0x3a,      /* Local Dynamic TLS  */

  /* IA-32 relocations start at 0x40 */
  TN_RELOC_IA32_ALL   = 0x40,	     /* All 32 bits of a symbol value. */
  TN_RELOC_IA32_GOT   = 0x41,
  TN_RELOC_IA32_GLOBAL_OFFSET_TABLE = 0x42,
  TN_RELOC_IA32_GOTOFF              = 0x43   /* 32 bit offset to GOT */
} TN_RELOCS;


class TN {
public:
  UINT32   tn_idx;
  /* offset 0 */
  union {
    INT64	value;		/* TN_has_value: Integer constant */
    INT64	offset;		/* Offset from symbol (constant) */
    struct {
      UINT8  number;		/* The ID of the register TN */
      CLASS_REG_PAIR save_creg;	/* if save_tn, the corresponding save_reg */
      CLASS_REG_PAIR class_reg; /* Dedicated/allocated register ID
				   and register class (see register.h) */
    } reg_tn;
  } u1;
  /* offset 8 */
  UINT32	flags;		/* Attribute flags */
  UINT8	  relocs;		/* Relocation flags (for symbol TNs) */
  UINT8  	size;		/* Size of the TN in bytes (must be <= 16) */
  /* offset 12 */
  union {
    LABEL_IDX	    label;		/* Label constant */
    // ISA_ENUM_CLASS_VALUE ecv;	/* Enum constant */
    ST_IDX        var;		  /* Symbolic constant (user variable) */
    union {			/* Spill location */
      ST_IDX      spill;		/* ...for register TN */
      IRNODE_IDX  home;		  /* IR home if rematerializable */
    } u3;
  } u2;
  TN (UINT32 tn_number) {
    memset(this, 0, sizeof(TN));
    tn_idx = tn_number;
  }
  TN_IDX Get_tn_idx() { return tn_idx; }
  void Print(FILE * file = stderr);
  void Dup_from(TN *pTn);
  void Set_type(MTYPE_ID id);
  void Set_value(UINT64 v) { u1.value = v; };
  void Set_var(ST_IDX v)   { u2.var = v; };
  void Set_label(LABEL_IDX l)   { u2.label = l; };
  UINT32 Get_flags() const      { return flags;  }
  void Set_flags(UINT32 flags)  { TN::flags = flags;  }
  UINT8 Get_relocs() const      { return relocs;  }
  void Set_relocs(UINT8 relocs) { TN::relocs = relocs;  }
  UINT8 Get_size() const        { return size;  }
  void Set_size(UINT8 size)     { TN::size = size;  }
};

enum {
  TN_CONSTANT         = 0x0001,  /* Constant value, numeric or label */
  TN_HAS_VALUE        = 0x0002,  /* Constant numeric value */
  TN_LABEL            = 0x0004,  /* Constant label value */
  TN_TAG              = 0x0008,  /* Constant tag value */
  TN_SYMBOL           = 0x0010,  /* References symbol table element */
  TN_FLOAT            = 0x0020,  /* => tn for a Floating-Point value */
  TN_DEDICATED        = 0x0040,  /* Dedicated, register if reg!=NULL */
  TN_FPU_INT          = 0x0080,  /* Int value in FPU (also TN_FLOAT) */
  TN_GLOBAL_REG       = 0x0100,  /* TN is a GTN (global register) */
  TN_IF_CONV_COND     = 0x0200,  /* TN is an if conversion conditinal */
  TN_REMATERIALIZABLE = 0x0400,  /* TN is rematerializable from whirl */
  TN_GRA_HOMEABLE     = 0x0800,  /* TN can be homed by gra */
  TN_ENUM             = 0x1000,  /* Constant enum value */
  TN_GRA_CANNOT_SPLIT = 0x2000,  /* its live range not to be split by GRA */
  TN_PREALLOCATED	    = 0x4000 ,  /* TN is pre-allocated in LRA */
  TN_THREAD_SEG_PTR   = 0x8000 ,  /* TN is pointer to thread-local storage */
  TN_NO_RENAME        = 0x10000,  /* TN does not get auto renamed */
};

#define CAN_USE_TN(t) (t)
/* Define the access functions: */
#define     TN_flags(t)		(CAN_USE_TN(t)->flags)
#define Set_TN_flags(t,x)	(CAN_USE_TN(t)->flags = (x))
/* define TN_is_{constant,register} ahead of time */
#define       TN_is_constant(r)	(TN_flags(r) &   TN_CONSTANT)
#define   Set_TN_is_constant(r)	(TN_flags(r) |=  TN_CONSTANT)
#define       TN_is_register(r)	(!TN_is_constant(r))

#define     TN_relocs(t)	(CAN_USE_TN(t)->relocs)
#define Set_TN_relocs(t,x)	(CAN_USE_TN(t)->relocs = (x))
#define     TN_size(t)		(CAN_USE_TN(t)->size+0)
#define Set_TN_size(t,x)	(CAN_USE_TN(t)->size = (x))
#define     TN_number(t)	(CAN_USE_REG_TN(t)->u1.reg_tn.number+0)

static inline TN_IDX TN_tn_idx(TN *tn) {
  return tn->tn_idx;
}

#define	    TN_class_reg(t)	(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg)
#define	Set_TN_class_reg(t,x)	(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg = (x))
#define     TN_register(t)	\
    (CLASS_REG_PAIR_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg)+0)
#define Set_TN_register(t,x)	\
    (Set_CLASS_REG_PAIR_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg,(x)))
#define     TN_register_class(t) \
    (CLASS_REG_PAIR_rclass(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg))
#define Set_TN_register_class(t,x) \
    (Set_CLASS_REG_PAIR_rclass(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg,(x)))
#define     TN_register_and_class(t) \
    (CLASS_REG_PAIR_class_n_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg)+0)
#define Set_TN_register_and_class(t,x) \
    (Set_CLASS_REG_PAIR_class_n_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg,(x)))
#define     TN_save_creg(t)	(CAN_USE_REG_TN(t)->u1.reg_tn.save_creg)
#define     TN_save_reg(t)	(CLASS_REG_PAIR_reg(TN_save_creg(t))+0)
#define     TN_save_rclass(t)	(CLASS_REG_PAIR_rclass(TN_save_creg(t)))
#define Set_TN_save_creg(t,x)	(CAN_USE_REG_TN(t)->u1.reg_tn.save_creg = (x))
#define     TN_is_save_reg(t)	(!CLASS_REG_PAIR_EqualP(TN_save_creg(t),CLASS_REG_PAIR_undef))
#define     TN_spill(t)		(CAN_USE_TN(t)->u2.u3.spill)
#define Set_TN_spill(t,x)	(CAN_USE_TN(t)->u2.u3.spill = (x))
#define     TN_spill_is_valid(t)(TN_is_register(t) && !(TN_is_rematerializable(t) || TN_is_gra_homeable(t)))
#define     TN_has_spill(t)	(TN_spill_is_valid(t) && (TN_spill(t) != NULL))
#define     TN_value(t)		(CAN_USE_TN(t)->u1.value)
#define Set_TN_value(t,x)	(CAN_USE_TN(t)->u1.value = (x))
#define     TN_offset(t)	(CAN_USE_TN(t)->u1.offset)
#define Set_TN_offset(t,x)	(CAN_USE_TN(t)->u1.offset = (x))
#define     TN_label(t)		(CAN_USE_TN(t)->u2.label)
#define Set_TN_label(t,x)	(CAN_USE_TN(t)->u2.label = (x))
#define     TN_enum(t)		(CAN_USE_TN(t)->u2.ecv)
#define Set_TN_enum(t,x)	(CAN_USE_TN(t)->u2.ecv = (x))
#define     TN_var(t)		(CAN_USE_TN(t)->u2.var)
#define Set_TN_var(t,x)		(CAN_USE_TN(t)->u2.var = (x))
#define     TN_home(t)		(CAN_USE_TN(t)->u2.u3.home)
#define Set_TN_home(t,x)	(CAN_USE_TN(t)->u2.u3.home = (x))

/* Define the TN_flags access functions: */
#define       TN_has_value(r)	(TN_flags(r) &   TN_HAS_VALUE)
#define   Set_TN_has_value(r)	(TN_flags(r) |=  TN_HAS_VALUE)
#define       TN_is_label(r)	(TN_flags(r) &   TN_LABEL)
#define   Set_TN_is_label(r)	(TN_flags(r) |=  TN_LABEL)
#define       TN_is_tag(r)	(TN_flags(r) &   TN_TAG)
#define   Set_TN_is_tag(r)	(TN_flags(r) |=  TN_TAG)
#define       TN_is_symbol(r)	(TN_flags(r) &   TN_SYMBOL)
#define   Set_TN_is_symbol(r)	(TN_flags(r) |=  TN_SYMBOL)
#define       TN_is_enum(r)	(TN_flags(r) &   TN_ENUM)
#define   Set_TN_is_enum(r)	(TN_flags(r) |=  TN_ENUM)

#define       TN_is_float(x)    (TN_flags(x) &   TN_FLOAT)
#define   Set_TN_is_float(x)    (TN_flags(x) |=  TN_FLOAT)
#define       TN_is_fpu_int(x)  (TN_flags(x) &   TN_FPU_INT)
#define   Set_TN_is_fpu_int(x)  (TN_flags(x) |=  TN_FPU_INT)
#define Reset_TN_is_fpu_int(x)  (TN_flags(x) &=  ~TN_FPU_INT)
#define       TN_is_global_reg(x) (TN_flags(x) &   TN_GLOBAL_REG)
#define   Set_TN_is_global_reg(x) (TN_flags(x) |=  TN_GLOBAL_REG)
#define Reset_TN_is_global_reg(x) (TN_flags(x) &= ~TN_GLOBAL_REG)
#define      TN_is_dedicated(r)	(TN_flags(r) &   TN_DEDICATED)
#define  Set_TN_is_dedicated(r)	(TN_flags(r) |=  TN_DEDICATED)
#define Reset_TN_is_dedicated(r) (TN_flags(r)&= ~TN_DEDICATED)

#define      TN_is_if_conv_cond(r)  (TN_flags(r) &   TN_IF_CONV_COND)
#define  Set_TN_is_if_conv_cond(r)  (TN_flags(r) |=  TN_IF_CONV_COND)
#define Reset_TN_is_if_conv_cond(r) (TN_flags(r) &= ~TN_IF_CONV_COND)

#define      TN_is_rematerializable(r)  (TN_flags(r) &   TN_REMATERIALIZABLE)
#define  Set_TN_is_rematerializable(r)  (TN_flags(r) |=  TN_REMATERIALIZABLE)
#define Reset_TN_is_rematerializable(r) (TN_flags(r) &= ~TN_REMATERIALIZABLE)

#define      TN_is_gra_homeable(r)  (TN_flags(r) &   TN_GRA_HOMEABLE)
#define  Set_TN_is_gra_homeable(r)  (TN_flags(r) |=  TN_GRA_HOMEABLE)
#define Reset_TN_is_gra_homeable(r) (TN_flags(r) &= ~TN_GRA_HOMEABLE)

#define      TN_is_gra_cannot_split(r)  (TN_flags(r) &   TN_GRA_CANNOT_SPLIT)
#define  Set_TN_is_gra_cannot_split(r)  (TN_flags(r) |=  TN_GRA_CANNOT_SPLIT)

#define       TN_is_preallocated(r)  (TN_flags(r) &   TN_PREALLOCATED)
#define   Set_TN_is_preallocated(r)  (TN_flags(r) |=  TN_PREALLOCATED)
#define Reset_TN_is_preallocated(r)  (TN_flags(r) &= ~TN_PREALLOCATED)
#define       TN_is_norename(r)	(TN_flags(r) &   TN_NO_RENAME)
#define   Set_TN_is_norename(r)	(TN_flags(r) |=  TN_NO_RENAME)
#define Reset_TN_is_norename(r)	(TN_flags(r) &= ~TN_NO_RENAME)

/* Macros to check if a TN is a particular dedicated register. */
#define TN_is_sp_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_sp)
#define TN_is_gp_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_gp)
#define TN_is_ep_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_ep)
#define TN_is_fp_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_fp)
#define TN_is_ra_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_ra)
#define TN_is_zero_reg(r)  (TN_register_and_class(r) == CLASS_AND_REG_zero)
#define TN_is_static_link_reg(r) (TN_register_and_class(r) == CLASS_AND_REG_static_link)
#define TN_is_pfs_reg(r)   (TN_register_and_class(r) == CLASS_AND_REG_pfs)
#define TN_is_lc_reg(r)   (TN_register_and_class(r) == CLASS_AND_REG_lc)
#define TN_is_ec_reg(r)   (TN_register_and_class(r) == CLASS_AND_REG_ec)
#define TN_is_true_pred(r) (TN_register_and_class(r) == CLASS_AND_REG_true)
#define TN_is_fzero_reg(r) (TN_register_and_class(r) == CLASS_AND_REG_fzero)
#define TN_is_fone_reg(r)  (TN_register_and_class(r) == CLASS_AND_REG_fone)



namespace TN_CONTEXT {
  extern TN_IDX Last_Dedicated_TN;/* The last dedicated TN number */
  extern TN_IDX Last_Distinct_Dedicated_TN;
  extern TN_IDX First_Regular_TN;  /* after all the preallocated TNs */
  extern TN_IDX First_REGION_TN;  /* The first non-dedicated TN in the current REGION */
  extern	TN *Pfs_TN;		// Previous Function State TN
  extern	TN *LC_TN;		// Loop Counter TN
  extern	TN *EC_TN;		// Epilog Counter TN
  extern	TN *True_TN;		// TN for true condition (predicate)
  extern  TN *FZero_TN;		// Floating zero (0.0) register TN
  extern  TN *FOne_TN;		// Floating one (1.0) register TN
  extern  BOOL is_str_expand;
  inline TN * CAN_USE_REG_TN (const TN *t)
  {
    if(!is_str_expand)
      AssertThat(TN_is_register(t), ("not a register tn"));
    return (TN*)t;
  }

}

using namespace TN_CONTEXT;

inline void  Set_TN_number(TN *t, int x)
{
  (CAN_USE_REG_TN(t)->u1.reg_tn.number = (x));
}

/* Register assignment: */

inline BOOL TN_Is_Allocatable( const TN *tn )
{
  return ! ( TN_is_constant(tn) || TN_is_dedicated(tn));
}

inline void TN_Allocate_Register( TN *tn, REGISTER reg )
{
  AssertThat(TN_Is_Allocatable(tn), ("Invalid TN for register allocation"));
  Set_TN_register(tn, reg);
}

inline BOOL TN_is_dedicated_class_and_reg( TN *tn, UINT16 class_n_reg )
{
  return    TN_is_dedicated(tn)
            && TN_register_and_class(tn) == class_n_reg;
}


/* Intialize the dedicated TNs at the start of the compilation. */
extern  void Init_Dedicated_TNs (void);

/* Initialize the TN data structure at the start of each PU. */
extern	void Init_TNs_For_PU (void);


/* The following set of routines can be used only for register TNs */
extern  TN* Gen_Register_TN (ISA_REGISTER_CLASS rclass, INT size);
extern  TN *Build_Dedicated_TN ( ISA_REGISTER_CLASS rclass, REGISTER reg, INT size);
extern	TN *Dup_TN ( TN *tn );	/* Duplicate an existing TN */
extern  TN *TN_tn(TN_IDX tn_idx); // Getting tn from tn_idx.
extern  TN *Gen_TN();
extern  TN_IDX Gen_TN(MTYPE_ID mtype);


/* Only the following routines should be used to build constant TNs. */
extern	TN *Gen_Literal_TN ( INT64 val, INT size );
// normally literals are hashed and reused; this creates unique TN
extern  TN *Gen_Unique_Literal_TN (INT64 ivalue, INT size);
extern  TN *Gen_Symbol_TN ( ST_IDX s, INT64 offset, INT32 relocs);
extern  TN *Gen_Label_TN ( LABEL_IDX lab, INT64 offset );
extern	TN *Gen_Adjusted_TN( TN *tn, INT64 adjust );

#endif //OCC_TN_H
