#include "basic.h"
#include <map>
#include <string>
#include <vector>

typedef UINT32 STR_IDX; // string table index
typedef UINT32 ST_IDX; // symbol info idx
typedef UINT32 TY_IDX; //type info idx
typedef UINT32 PU_IDX; //program unit idx
typedef UINT32 TCON_IDX;
typedef UINT32 mTYPE_ID;
typedef UINT32 ARB_IDX;
typedef UINT32 INITV_IDX;
typedef UINT32 INITO_IDX;
typedef UINT32 LABEL_IDX;

// Symbol Table Frame Work
enum SYM_ATTR
{
    ST_CONST = 0,
} ;

enum SYM_SCLASS
{
    SYMC_UNKNOWN = 0,
    SYMC_AUTO = 1,
    SYMC_FUNC_STATIC = 2,
    SYMC_FILE_STATIC = 3,
    SYMC_EXTERN = 4,
    SYMC_GLOBAL_UNDEF = 5,
    SYMC_GLOBAL_DEF = 6,
    SYMC_TEXT = 7,
};

typedef enum
{
    SYME_UNKNOWN = 0,
    SYME_INTERNAL = 1,
    SYME_EXTERNAL = 2,
    SYME_PREEMPTIBLE = 3,
} SYM_ECLASS;

enum SYM_CLASS
{
    CLASS_UNK	= 0,
    CLASS_VAR	= 1,			// data variable
    CLASS_FUNC	= 2,			// addrress of a function.
    CLASS_CONST	= 3,			// constant value
    CLASS_PREG	= 4,			// pseudo register
    CLASS_BLOCK	= 5,			// base to a block of data
    CLASS_NAME  = 6,			// just hold an ST name
    CLASS_COUNT = 7			// total number of classes
}; // SYM_CLASS

typedef enum {
    TLS_NONE = 0
} ST_TLS_MODEL;


/* Kinds of types: */
enum TY_KIND
{
    KIND_INVALID	= 0,		// Invalid
    KIND_SCALAR		= 1,		// integer/floating point
    KIND_ARRAY		= 2,		// array
    KIND_STRUCT		= 3,		// struct/union
    KIND_POINTER	= 4,		// pointer
    KIND_FUNCTION	= 5,		// function/procedure
    KIND_VOID		= 6,		// C void type
    KIND_LAST		= 8
};

// Type flags
enum TY_FLAG
{
  TY_ANONYMOUS	= 0x1,	/* Anonymous structs/classes/unions */
};

class TY
{
public:
    UINT64 size;			// size of the type in bytes

    TY_KIND kind   : 8;			// kind of type
    mTYPE_ID mtype : 8;			// WHIRL data type
    UINT16 flags;			// misc. attributes

    union {
      // FLD_IDX fld;
      // TYLIST_IDX tylist;
      ARB_IDX arb;
    } u1;				// idx to FLD_TAB, TYLIST_TAB, etc.

    STR_IDX name_idx;			// name 

    union {
      TY_IDX etype;			// type of array element (array only)
      TY_IDX pointed;			// pointed-to type (pointers only)
      UINT32 pu_flags;		// attributes for KIND_FUNCTION
      ST_IDX copy_constructor;	// copy constructor X(X&) (record only)
    } u2;
  
    ST_IDX vtable;

    // access function for unions
    // FLD_IDX Fld () const		{ return u1.fld; }
    // void Set_fld (FLD_IDX idx)		{ u1.fld = idx; }
    
    // TYLIST_IDX Tylist () const		{ return u1.tylist; }
    // void Set_tylist (TYLIST_IDX idx)	{ u1.tylist = idx; }
    
    ARB_IDX Arb () const		{ return u1.arb; }
    void Set_arb (ARB_IDX idx)		{ u1.arb = idx; }

    TY_IDX Etype () const
    {
      AssertThat(kind == KIND_ARRAY,
                 ("non-KIND_ARRAY type has no element type"));
      return u2.etype;
    }
    void Set_etype (TY_IDX idx)		{ u2.etype = idx; }
    
    TY_IDX Pointed () const
    {
        AssertThat(kind == KIND_POINTER,
                 ("non-KIND_POINTER type doesn't point"));
        return u2.pointed;
    }
    void Set_pointed (TY_IDX idx)	{ u2.pointed = idx; }

    ST_IDX Copy_constructor () const
    {
      AssertThat(kind == KIND_STRUCT,
                 ("non-KIND_STRUCT type has no copy constructor"));
      return u2.copy_constructor;
    }
    void Set_copy_constructor (ST_IDX idx)	{ u2.copy_constructor = idx; }

    ST_IDX Vtable () const
    {
      AssertThat(kind == KIND_STRUCT,
                  ("non-KIND_STRUCT type has no vtable"));
      return vtable;
    }
    void Set_vtable (ST_IDX idx)	{ vtable = idx; }


    PU_IDX Pu_flags () const		{ return u2.pu_flags; }
    // void Set_pu_flag (TY_PU_FLAGS f)	{ u2.pu_flags |= f; }
    // void Clear_pu_flag (TY_PU_FLAGS f)	{ u2.pu_flags &= ~f; }

    // operations
    TY ();
    void Verify(UINT level) const;
    void Print (FILE *f) const;

}; // TY

enum PU_FLAG {
  PU_PURE = 0x1,
  PU_NO_INLINE = 0x2,
  PU_INLINE = 0x4,
};

struct PU
{
    TY_IDX prototype;			// function prototype
    TY_IDX base_class;    // the class type which this PU belongs to if this PU is a member function
    UINT32 flags;			    // misc. attributes about this func.
    // operations
    PU ();
    // void Verify() const;
    // void Print (FILE *f) const;
}; // PU


// symbol table element
class ST
{
public:
    // after add new member, Make sure to update function eq_const_st::operator()
    // in file ipc_symtab_merge.cxx
    STR_IDX name_idx; // index to the name string
    SYM_ATTR attr : 4;
    SYM_CLASS sym_class : 4;
    SYM_SCLASS storage_class : 4; // storage info
    SYM_ECLASS export_class : 4;  
    ST_TLS_MODEL tls_model : 4; // Thread-Local-Storage(TLS) model
    union {
        TY_IDX type;   // idx to high-level type
        PU_IDX pu;   // idx to program unit
    } u2;
    UINT32 pad; // 4 pad bytes (initialize to zero)
    UINT32 offset; // offset from base
    ST_IDX base_idx; // base of the allocated block
    ST_IDX st_idx; // my own st_idx
    // operations

    ST() { 
      // AssertThat(FALSE, ("ST default constructor must not be called.")); 
    }
    // void Verify(UINT level) const;
    // void Print(FILE *f, BOOL verbose = TRUE) const;
    // BOOL operator==(ST &st) const;
    // friend std::ostream &operator<<(std::ostream &os, const ST &st);

}; // ST

// Give information about a dimension of an array.  The TY of the array type
// points to the ARB entry for the first dimension.  The remaining dimensions
// follow in consecutive ARB entries until a flag indicates it is the last
// dimension.
enum ARB_FLAGS
{
    ARB_CONST_LBND = 0x0001,   // constant lower bound
    ARB_CONST_UBND = 0x0002,   // constant upper bound
    ARB_CONST_STRIDE = 0x0004, // constant stride
    ARB_FIRST_DIMEN = 0x0008,  // first dimension
    ARB_LAST_DIMEN = 0x0010    // last dimension
};

struct ARB
{
    UINT16 flags;     // misc. attributes
    UINT16 dimension; // number of dimensions
    UINT32 dummy_padding = 0;
    union {
        INT32 lbnd_val; // constant lower bound value
        struct
        {
            ST_IDX lbnd_var; // variable that stores the
                // non-constant lower bound
            INT32 unused; // filler, must be zero'ed
        } var;
    } u1;

    union {
        INT32 ubnd_val; // constant upper bound value
        struct
        {
            ST_IDX ubnd_var; // variable that stores the
                // non-constant upper bound
            INT32 unused; // filler, must be zero'ed
        } var;
    } u2;

    union {
        INT32 stride_val; // constant stride
        struct
        {
            ST_IDX stride_var; // variable that stores the
                // non-constant stride
            INT32 unused; // filler, must be zero'ed
        } var;
    } u3;

    // access functions
    INT64 Lbnd_val() const { return u1.lbnd_val; }
    void Set_lbnd_val(INT64 val) { u1.lbnd_val = val; }

    ST_IDX Lbnd_var() const { return u1.var.lbnd_var; }
    void Set_lbnd_var(ST_IDX st)
    {
        u1.var.lbnd_var = st;
        u1.var.unused = 0;
    }

    INT64 Ubnd_val() const { return u2.ubnd_val; }
    void Set_ubnd_val(INT64 val) { u2.ubnd_val = val; }

    ST_IDX Ubnd_var() const { return u2.var.ubnd_var; }
    void Set_ubnd_var(ST_IDX st)
    {
        u2.var.ubnd_var = st;
        u2.var.unused = 0;
    }

    INT64 Stride_val() const { return u3.stride_val; }
    void Set_stride_val(INT64 val) { u3.stride_val = val; }

    ST_IDX Stride_var() const { return u3.var.stride_var; }
    void Set_stride_var(ST_IDX st)
    {
        u3.var.stride_var = st;
        u3.var.unused = 0;
    }

    // operations
    ARB()
    {
        bzero(this, sizeof(ARB));
    }
    // void Verify(UINT16 dim) const;
    // void Print(FILE *f) const;

}; // ARB

enum LABEL_KIND
{
    LKIND_DEFAULT = 0,
    LKIND_ASSIGNED = 1, // in ASSIGNED statement
    LKIND_BEGIN_EH_RANGE = 2,
    LKIND_END_EH_RANGE = 3,
    LKIND_BEGIN_HANDLER = 4,
    LKIND_END_HANDLER = 5,
    LKIND_TAG = 6 // symbolic address, never branched to
};

enum LABEL_FLAGS
{
    LABEL_TARGET_OF_GOTO_OUTER_BLOCK = 1,
    LABEL_ADDR_SAVED = 2,
    LABEL_ADDR_PASSED = 4
};

struct LABEL
{
    STR_IDX name_idx;
    UINT32 flags : 24;
    LABEL_KIND kind : 8;

    // operations
    LABEL() { AssertThat(FALSE, ("LABEL default constructor must not be called.")); }
    LABEL(STR_IDX idx, LABEL_KIND k) : name_idx(idx), kind(k) {}
    void Verify(UINT level) const;
    void Print(FILE *f) const;
}; // LABEL

struct PREG
{
    STR_IDX name_idx;

    // operations
    PREG(void)
    {
        AssertThat(FALSE, ("PREG default is invoked."));
    }
    PREG(STR_IDX idx) : name_idx(idx) {}

}; // PREG

struct TCON {
  TCON_IDX my_idx;
  UINT32 kind;
};

template <class T>
class GROWING_TABLE {
  std::map<UINT32, T*> single_level_tab;
  public:
  T* operator[] (UINT32 idx) {
      return single_level_tab[idx];
  }
};

template <class T>
class SECOND_LEVEL_GROWING_TABLE {
  GROWING_TABLE<T> global_tab;
  std::map<PU_IDX, std::map<UINT32, T*> > two_level_tab;
  public:
  T* operator[] (UINT32 idx);
  void add (UINT8 scope_level, T *obj);
};

// initialized objects
struct INITO {
    ST_IDX st_idx;			// item being initialized
    INITV_IDX val;			// initial value

    // void Verify (UINT level) const;
    // void Print  (FILE* f)    const;
};


// initial value
enum INITVKIND {
    INITVKIND_UNK	= 0,
    INITVKIND_SYMOFF	= 1,
    INITVKIND_ZERO	= 2,
    INITVKIND_ONE	= 3,
    INITVKIND_VAL	= 4,
    INITVKIND_BLOCK	= 5,
    INITVKIND_PAD	= 6,
    INITVKIND_LABEL     = 9
};

struct INITV
{
  INITV_IDX next;			// next value for non-scalar member
  INITVKIND kind : 16;		// kind of value
  UINT16 repeat1;			// repeat factor (repeat2 used for
                      // INITVKIND_VAL
  union {
    struct {			// this field for SYMOFF and SYMIPLT
        ST_IDX st;			
        INT32 ofst;
    } sto;				// address + offset 
    struct {
        LABEL_IDX lab;		// for INITVKIND_LABEL
        INT16 flags;		// flags, see INITVLABELFLAGS
        mTYPE_ID mtype;             // type for label values
    } lab;
    struct {
        LABEL_IDX lab1;
        ST_IDX st2;
    } stdiff;			// lab1 - st2
    
    struct {
        union {
          TCON_IDX tc;		// value
          mTYPE_ID mtype;		// machine type for INITVKIND_ZERO
            // and INITVKIND_ONE
        } u;
        UINT32 repeat2;		// 32-bits for repeat factor 
    } tcval;

    struct {
        INITV_IDX blk;		// useful for aggregate values
        INT32 flags;		// flags
        INT32 unused;		// filler, must be zero
    } blk;
    
    struct {
        INT32 pad;			// amount of padding in bytes
        INT32 unused;		// filler, must be zero
    } pad;
  } u;

    ST_IDX St () const			{ return u.sto.st; }
    INT32 Ofst () const			{ return u.sto.ofst; }

    LABEL_IDX Lab () const		{ return u.lab.lab; }
    INT16 Lab_flags () const            { return u.lab.flags; }
    mTYPE_ID Lab_mtype () const         { return u.lab.mtype; }

    LABEL_IDX Lab1 () const		{ return u.stdiff.lab1; }
    ST_IDX St2 () const			{ return u.stdiff.st2; }

    TCON_IDX Tc () const		{ return u.tcval.u.tc; }
    mTYPE_ID Mtype () const		{ return u.tcval.u.mtype; }
    UINT32 Repeat2 () const		{ return u.tcval.repeat2; }

    INITV_IDX Blk () const		{ return u.blk.blk; }
    INT32 Pad () const			{ return u.pad.pad; }

    void Verify (UINT level) const;
}; // INITV


/**
 * 
 *  Symbol Table Declarations
 * 
 * */
class OIR {
public:
  GROWING_TABLE<std::string> String_Tab;
  SECOND_LEVEL_GROWING_TABLE<ST> Global_Sym_Tab;
  GROWING_TABLE<TY> Global_Type_Tab;
  GROWING_TABLE<TCON> Global_Const_Tab;
  GROWING_TABLE<PU> Global_Pu_Tab;
  GROWING_TABLE<INITO> Global_Inito_Tab;
  GROWING_TABLE<INITV> Global_Initv_Tab;
  ST *Get_symbol(ST_IDX st_idx);
  TY *Get_type(TY_IDX ty_idx);
};

PU_IDX Get_current_pu_idx();