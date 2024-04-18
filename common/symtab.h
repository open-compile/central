#include "basic.h"
#include <map>
#include <string>
#include <string.h>
#include <vector>
#include <typeinfo>
#include <memory.h>

#ifndef OCC_SYMTAB_H
#define OCC_SYMTAB_H
#include "consts.h"

// Forward declare
class SCOPE;
class SCOPE_MANAGER;
class FILE_SYMTAB;
class FILE_MANAGER;
class TREE;

/* Kinds of types: */
enum TY_KIND {
  KIND_INVALID  = 0,    // Invalid
  KIND_SCALAR   = 1,    // integer/floating point
  KIND_ARRAY    = 2,    // array
  KIND_STRUCT   = 3,    // struct/union
  KIND_POINTER  = 4,    // pointer
  KIND_FUNCTION = 5,    // function/procedure
  KIND_VOID     = 6,    // C void type
  KIND_LAST     = 7
};


// Type flags
enum TY_FLAG {
  TY_ANONYMOUS       = 0x1,  /* Anonymous structs/classes/unions */
  TY_FLAG_CONST      = 0x2,
  TY_FLAG_INTERNAL   = 0x4,
};

class TY {
public:
  UINT64 size;      // size of the type in bytes

  TY_KIND kind     : 8;      // kind of type
  MTYPE_ID mtype   : 8;      // WHIRL data type
  TY_FLAG flags    : 16;      // misc. attributes
  INT32 align;        // alignment

  union {
    // FLD_IDX fld;
    TYLIST_IDX tylist;  // basically TY_IDX, because we use flag to see that
    ARB_IDX arb;
  } u1;        // idx to FLD_TAB, TYLIST_TAB, etc.

  STR_IDX name_idx;      // name

  union {
    TY_IDX etype;      // type of array element (array only)
    TY_IDX pointed;      // pointed-to type (pointers only)
    UINT32 pu_flags;    // attributes for KIND_FUNCTION
    ST_IDX copy_constructor;  // copy constructor X(X&) (record only)
  } u2;

  ST_IDX vtable;

  // access function for unions
  // FLD_IDX Fld () const		{ return u1.fld; }
  // void Set_fld (FLD_IDX idx)		{ u1.fld = idx; }

   TYLIST_IDX Tylist () const		{ return u1.tylist; }
   void Set_tylist (TYLIST_IDX idx)	{ u1.tylist = idx; }

  ARB_IDX Arb() const { return u1.arb; }

  void Set_arb(ARB_IDX idx) { u1.arb = idx; }

  TY_IDX Etype() const {
      AssertThat(kind == KIND_ARRAY,
                 ("non-KIND_ARRAY type has no element type"));
    return u2.etype;
  }

  void Set_etype(TY_IDX idx) { u2.etype = idx; }

  TY_IDX Pointed() const {
      AssertThat(kind == KIND_POINTER,
                 ("non-KIND_POINTER type doesn't point"));
    return u2.pointed;
  }

  void Set_pointed(TY_IDX idx) { u2.pointed = idx; }

  ST_IDX Copy_constructor() const {
      AssertThat(kind == KIND_STRUCT,
                 ("non-KIND_STRUCT type has no copy constructor"));
    return u2.copy_constructor;
  }

  void Set_copy_constructor(ST_IDX idx) { u2.copy_constructor = idx; }

  ST_IDX Vtable() const {
      AssertThat(kind == KIND_STRUCT,
                 ("non-KIND_STRUCT type has no vtable"));
    return vtable;
  }

  void Set_vtable(ST_IDX idx) { vtable = idx; }


  PU_IDX Pu_flags() const { return u2.pu_flags; }
  // void Set_pu_flag (TY_PU_FLAGS f)	{ u2.pu_flags |= f; }
  // void Clear_pu_flag (TY_PU_FLAGS f)	{ u2.pu_flags &= ~f; }

  // operations
  TY() {
      memset(this, 0,sizeof(TY));
  };

  void Verify(UINT level) const {};

  void Print(FILE *f) const;

}; // TY

struct TYLIST {
  TY_IDX ty_id;
  void Print(FILE *f);
  TY_IDX Ty_idx() { return ty_id; }
};

struct MTYPE_TAB {
  MTYPE_ID mtype;
  TY_KIND  kind;
  TY_FLAG  flag;
  UINT64   size;
  const char *name;
  TY_IDX   ty_idx;
};

extern MTYPE_TAB MTYPE_to_TY_table[]; // Mtype to TY_IDX mapping

enum PU_FLAG {
  PU_PURE = 0x1,
  PU_NO_INLINE = 0x2,
  PU_INLINE = 0x4,
};

struct PU {
  TY_IDX prototype;      // function prototype
  TY_IDX base_class;    // the class type which this PU belongs to if this PU is a member function
  UINT32 flags;          // misc. attributes about this func.
  PU_INFO_IDX pu_info_idx; // pu info idx
  // operations
  PU() {
      memset(this,0, sizeof(PU));
  } ;
  // void Verify() const;
  void Print (FILE *f) const {};

  TY_IDX getPrototype() {
    return prototype;
  }

  void setPrototype(TY_IDX prototype) {
    PU::prototype = prototype;
  }

  TY_IDX getBaseClass() {
    return base_class;
  }

  void setBaseClass(TY_IDX baseClass) {
    base_class = baseClass;
  }

  UINT32 getFlags() {
    return flags;
  }

  void setFlags(UINT32 flags) {
    PU::flags = flags;
  }

  PU_INFO_IDX getPuInfoIdx() {
    return pu_info_idx;
  }

  void setPuInfoIdx(PU_INFO_IDX puInfoIdx) {
    pu_info_idx = puInfoIdx;
  }
}; // PU


// symbol table element
class ST {
public:
  STR_IDX name_idx; // index to the name string
  UINT32  attr: 4; // SYM_ATTR
  SYM_CLASS    sym_class: 4;
  SYM_SCLASS   storage_class: 4; // storage info
  SYM_ECLASS   export_class: 4;
  ST_TLS_MODEL tls_model: 4; // Thread-Local-Storage(TLS) model
  TY_IDX       type;   // idx to high-level type
  PU_IDX       pu;   // idx to program unit
  UINT32       pad; // 4 pad bytes (initialize to zero)
  UINT32       offset; // offset from base
  ST_IDX       base_idx; // base in the allocated block.
  INITO_IDX    inito_idx; // inito idx to inito
  // ST_IDX st_idx; // my own st_idx
  // operations

  ST() {
      memset(this, 0,sizeof(ST));
  }
  // void Verify(UINT level) const;
  void Print(FILE *f, BOOL verbose);
  void Print(FILE *f) { Print(f, TRUE); };
  void Print_details(FILE *f);
  void Print_storage_class(FILE *f);
  PU_INFO_IDX Pu_info_idx();

  UINT32 getAttr() const {
    return attr;
  }

  void Set_attr(UINT32 attr) {
    ST::attr |= attr;
  }

  void Clear_attr(UINT32 attr) {
    ST::attr &= (~attr);
  }

  SYM_CLASS getSymClass() const {
    return sym_class;
  }

  void setSymClass(SYM_CLASS symClass) {
    sym_class = symClass;
  }

  SYM_SCLASS getStorageClass() const {
    return storage_class;
  }

  void setStorageClass(SYM_SCLASS storageClass) {
    storage_class = storageClass;
  }

  SYM_ECLASS getExportClass() const {
    return export_class;
  }

  void setExportClass(SYM_ECLASS exportClass) {
    export_class = exportClass;
  }

  ST_TLS_MODEL getTlsModel() const {
    return tls_model;
  }

  void setTlsModel(ST_TLS_MODEL tlsModel) {
    tls_model = tlsModel;
  }

  UINT32 getPad() const {
    return pad;
  }

  void setPad(UINT32 pad) {
    ST::pad = pad;
  }

  UINT32 getOffset() const {
    return offset;
  }

  void setOffset(UINT32 offset) {
    ST::offset = offset;
  }

  ST_IDX getBaseIdx() const {
    return base_idx;
  }

  void setBaseIdx(ST_IDX baseIdx) {
    base_idx = baseIdx;
  }

  STR_IDX getNameIdx() const {
    return name_idx;
  }

  void setNameIdx(STR_IDX nameIdx) {
    name_idx = nameIdx;
  }

  INITO_IDX getInitoIdx() {
    return inito_idx;
  }

  void setInitoIdx(INITO_IDX initoIdx) {
    inito_idx = initoIdx;
  }
}; // ST

// Give information about a dimension of an array.  The TY of the array type
// points to the ARB entry for the first dimension.  The remaining dimensions
// follow in consecutive ARB entries until a flag indicates it is the last
// dimension.
enum ARB_FLAGS {
  ARB_CONST_LBND = 0x0001,   // constant lower bound
  ARB_CONST_UBND = 0x0002,   // constant upper bound
  ARB_CONST_STRIDE = 0x0004, // constant stride
  ARB_FIRST_DIMEN = 0x0008,  // first dimension
  ARB_LAST_DIMEN = 0x0010    // last dimension
};

struct ARB {
  ARB_FLAGS flags : 16;     // misc. attributes
  UINT16 dimension; // number of dimensions
  UINT32 dummy_padding = 0;
  union {
    INT32 lbnd_val; // constant lower bound value
    struct {
      ST_IDX lbnd_var; // variable that stores the
      // non-constant lower bound
      INT32 unused; // filler, must be zero'ed
    } var;
  } u1;

  union {
    INT32 ubnd_val; // constant upper bound value
    struct {
      ST_IDX ubnd_var; // variable that stores the
      // non-constant upper bound
      INT32 unused; // filler, must be zero'ed
    } var;
  } u2;

  union {
    INT32 stride_val; // constant stride
    struct {
      ST_IDX stride_var; // variable that stores the
      // non-constant stride
      INT32 unused; // filler, must be zero'ed
    } var;
  } u3;

  // access functions
  INT64 Lbnd_val() const { return u1.lbnd_val; }

  void Set_lbnd_val(INT64 val) { u1.lbnd_val = val; }

  ST_IDX Lbnd_var() const { return u1.var.lbnd_var; }

  void Set_lbnd_var(ST_IDX st) {
    u1.var.lbnd_var = st;
    u1.var.unused = 0;
  }

  INT64 Ubnd_val() const { return u2.ubnd_val; }

  void Set_ubnd_val(INT64 val) { u2.ubnd_val = val; }

  ST_IDX Ubnd_var() const { return u2.var.ubnd_var; }

  void Set_ubnd_var(ST_IDX st) {
    u2.var.ubnd_var = st;
    u2.var.unused = 0;
  }

  INT64 Stride_val() const { return u3.stride_val; }

  void Set_stride_val(INT64 val) { u3.stride_val = val; }

  ST_IDX Stride_var() const { return u3.var.stride_var; }

  void Set_stride_var(ST_IDX st) {
    u3.var.stride_var = st;
    u3.var.unused = 0;
  }

  // operations
  ARB() {
      memset(this, 0,sizeof(ARB));
  }
  void Init_const (UINT64 ubnd_val, UINT64 stride_val, UINT32 dimen, UINT32 flag) {
    flags = (ARB_FLAGS) flag;
    dimension = dimen;
    u1.lbnd_val = 0;
    u2.ubnd_val = ubnd_val;
    u3.stride_val = stride_val;
  }
  void Init_var (ST_IDX ubnd_var, UINT64 stride_val, UINT32 dimen, UINT32 flag) {
    flags = (ARB_FLAGS) flag;
    dimension = dimen;
    u1.lbnd_val = 0;
    u2.var.ubnd_var = ubnd_var;
    u3.stride_val = stride_val;
  }

  // void Verify(UINT16 dim) const;
  void Print(FILE *f) const {};

}; // ARB

using ARB_LIST = std::vector<ARB *>;

enum LABEL_KIND {
  LKIND_DEFAULT = 0,
  LKIND_NEVER = 1,
  LKIND_ENTRY = 2,
  LKIND_EXIT = 3,
  LKIND_BEGIN_HANDLER = 4,
  LKIND_END_HANDLER = 5,
  LKIND_TAG = 6, // symbolic address, never branched to
  LKIND_RELOC = 7, // addr label, used for loading addressing for
                   // variables in .data section.
};

enum LABEL_FLAGS {
  LABEL_TARGET_OF_GOTO_OUTER_BLOCK = 1,
  LABEL_ADDR_SAVED = 2,
  LABEL_ADDR_PASSED = 4
};

struct LABEL {
  STR_IDX    name_idx;
  UINT32     flags: 24;
  LABEL_KIND kind: 8;
  ST_IDX     temp_sym;

  // operations
  LABEL() {
    // AssertThat(FALSE, ("LABEL default constructor must not be called."));
    name_idx = 0;
    kind = LKIND_DEFAULT;
    flags = 0;
    temp_sym = 0;
  }

  STR_IDX Get_name_idx() const {
    return name_idx;
  }

  void Set_name_idx(STR_IDX nameIdx) {
    name_idx = nameIdx;
  }

  UINT32 Get_flags() const {
    return flags;
  }

  void Set_flags(UINT32 flags) {
    LABEL::flags = flags;
  }

  LABEL_KIND Get_kind() const {
    return kind;
  }

  void Set_kind(LABEL_KIND kind) {
    LABEL::kind = kind;
  }

  ST_IDX Get_temp_sym() const {
    return temp_sym;
  }

  void Set_temp_sym(ST_IDX tempSym) {
    temp_sym = tempSym;
  }

  LABEL(STR_IDX idx, LABEL_KIND k) : name_idx(idx), kind(k) {}
  void Verify(UINT level) const {};
  void Print(FILE *f) const;
}; // LABEL

struct PREG {
  STR_IDX name_idx;
  UINT32  desire_reg_num;
  // operations
  PREG(void) {
    memset(this, 0,sizeof(PREG));
  }
  void Print(FILE *file) {
    fprintf(file, "[PREG] name_idx: %d, desired_reg = %d\n", name_idx, desire_reg_num);
  }

  STR_IDX getNameIdx() const {
    return name_idx;
  }

  void setNameIdx(STR_IDX nameIdx) {
    name_idx = nameIdx;
  }

  UINT32 getDesireRegNum() const {
    return desire_reg_num;
  }

  void setDesireRegNum(UINT32 desireRegNum) {
    desire_reg_num = desireRegNum;
  }

}; // PREG

struct TCON {
  TCON_IDX my_idx;
  UINT32 kind;
};

template<typename U, typename T>
class GROWING_TABLE {
  UINT32 count = 0; // Count of items in the table
  std::vector<void *> table;
public:
  T *operator[](U idx) {
    return Get(idx);
  }
  U Add() {
    T *empty_obj = new T();
    U current_idx = count;
    table.push_back((void *) empty_obj);
    count++;
    return current_idx;
  }
  void Set(U idx, T *obj) {
    table[idx] = (void *) obj;
  }
  T *Get(U idx) {
    AssertThat(idx < count,
               ("Trying to locate a item by id larger than total count, total = %0#x, query = %0#x",
                count, idx));
    return (T*) table[idx];
  }
  UINT32 Length() {
    return table.size();
  }
  std::vector<void *>::iterator Begin() {
    return table.begin();
  }
  std::vector<void *>::iterator End() {
    return table.end();
  }
  // TODO: add dump here to dump the table to a region.
};


// initial value
enum INITVKIND {
  INITVKIND_UNK = 0,
  INITVKIND_SYMOFF = 1,
  INITVKIND_ZERO = 2,
  INITVKIND_ONE = 3,
  INITVKIND_VAL = 4,
  INITVKIND_BLOCK = 5,
  INITVKIND_PAD = 6,
  INITVKIND_LABEL = 9
};

struct INITV {
  // INITV_IDX next;      // next value for non-scalar member
  INITVKIND kind: 16;    // kind of value
  UINT16 repeat1;      // repeat factor (repeat2 used for
  // INITVKIND_VAL
  union {
    struct {      // this field for SYMOFF and SYMIPLT
      ST_IDX st;
      INT32 ofst;
    } sto;        // address + offset
    struct {
      LABEL_IDX lab;    // for INITVKIND_LABEL
      INT16 flags;    // flags, see INITVLABELFLAGS
      mTYPE_ID mtype;             // type for label values
    } lab;
    struct {
      LABEL_IDX lab1;
      ST_IDX st2;
    } stdiff;      // lab1 - st2

    struct {
      union {
        TCON_IDX tc;    // value
        mTYPE_ID mtype;    // machine type for INITVKIND_ZERO
        // and INITVKIND_ONE
      } u;
      UINT32 repeat2;    // 32-bits for repeat factor
    } tcval;

    struct {
      INITV_IDX blk;    // useful for aggregate values
      INT32 flags;    // flags
      INT32 unused;    // filler, must be zero
    } blk;

    struct {
      INT32 pad;      // amount of padding in bytes
      INT32 unused;    // filler, must be zero
    } pad;
    struct {
      INT64 c_val;
    };
  } u;

  ST_IDX St() const { return u.sto.st; }

  INT32 Ofst() const { return u.sto.ofst; }

  LABEL_IDX Lab() const { return u.lab.lab; }

  INT16 Lab_flags() const { return u.lab.flags; }

  mTYPE_ID Lab_mtype() const { return u.lab.mtype; }

  LABEL_IDX Lab1() const { return u.stdiff.lab1; }

  ST_IDX St2() const { return u.stdiff.st2; }

  TCON_IDX Tc() const { return u.tcval.u.tc; }

  mTYPE_ID Mtype() const { return u.tcval.u.mtype; }

  UINT32 Repeat2() const { return u.tcval.repeat2; }

  INITV_IDX Blk() const { return u.blk.blk; }

  INT32 Pad() const { return u.pad.pad; }

  void Verify(UINT level) const { };
  void Print(FILE *file) const;
  INT64 Val() const { return u.c_val; }
  void Set_val(INT64 cval) { u.c_val = cval; }
  INITVKIND Kind() const { return kind; }
  void Set_kind(INITVKIND cval) { kind = cval; }
  void Set_pad(INT32 cval) { u.pad.pad = cval; }
}; // INITV

// initialized objects
class INITO {
  ST_IDX st_idx;      // item being initialized
  std::vector<INITV> val;      // initial value
  // void Verify (UINT level) const;
public:
  void Print  (FILE* f)  const;
  INITO() {
      memset(this, 0,sizeof(INITO));
  }
  UINT32 Size() {
    return val.size();
  }
  INITV *Value(UINT32 i) {
    return &(val[i]);
  }
  INITV_IDX Add_value() {
    val.push_back(INITV());
    return val.size() - 1;
  }
  INITV_IDX Add_value(ST_IDX st_idx, INITV initv) {
    this->st_idx = st_idx;
    val.push_back(initv);
    return val.size() - 1;
  }
  void Set_sym(ST_IDX i) {
    st_idx = i;
  }
  ST_IDX Get_sym() { return st_idx; }
};


class SCOPE {
public:
  // MEM_POOL *pool;		  // mem pool for local tables
  ST_IDX   st_idx;        // ST * for the current pu, in global sym table
  GROWING_TABLE<ST_IDX, ST> *st_tab;
  GROWING_TABLE<LABEL_IDX, LABEL> *label_tab;
  GROWING_TABLE<PREG_IDX, PREG> *preg_tab;
  GROWING_TABLE<INITO_IDX, INITO> *inito_tab;
  /**
   * Initialize the scope by using the current-existing tables
   * @param sym
   * @param st_tab
   * @param label_tab
   * @param preg_tab
   * @param inito_tab
   */
  void Init(ST_IDX sym, GROWING_TABLE<ST_IDX, ST> *st_tab,
            GROWING_TABLE<LABEL_IDX, LABEL> *label_tab,
            GROWING_TABLE<PREG_IDX, PREG> *preg_tab,
            GROWING_TABLE<INITO_IDX, INITO> *inito_tab) {
    this->st_idx = sym;
    this->st_tab = st_tab;
    this->label_tab = label_tab;
    this->preg_tab = preg_tab;
    this->inito_tab = inito_tab;
  }

  /**
   * Initialize the scope by creating the according tables
   * Used for create_function, etc.
   * @param sym
   */
  void Init(ST_IDX sym) {
    this->st_idx = sym;
    this->st_tab = new GROWING_TABLE<ST_IDX, ST>;
    this->st_tab->Add();
    this->label_tab = new GROWING_TABLE<LABEL_IDX, LABEL>;
    this->label_tab->Add();
    this->preg_tab = new GROWING_TABLE<PREG_IDX, PREG>;
    this->preg_tab->Add();
    this->inito_tab = new GROWING_TABLE<INITO_IDX, INITO>;
    this->inito_tab->Add();
  }

  ST_IDX getSt() const {
    return st_idx;
  }
  GROWING_TABLE<ST_IDX, ST> *getStTab() const {
    return st_tab;
  }
  GROWING_TABLE<LABEL_IDX, LABEL> *getLabelTab() const {
    return label_tab;
  }
  GROWING_TABLE<PREG_IDX, PREG> *getPregTab() const {
    return preg_tab;
  }
  GROWING_TABLE<INITO_IDX, INITO> *getInitoTab() const {
    return inito_tab;
  }
  SCOPE(ST_IDX st_idx) {
      memset(this, 0,sizeof(SCOPE));
    this->st_idx = st_idx;
    this->Init(st_idx);
  }
  void Print(FILE *f);
}; // SCOPE

struct FILE_INFO {
  STR_IDX file_name;
  FILE_INFO();
};

/**
 * The data structure that describes a program unit
 */
struct PU_INFO {
  PU_INFO_IDX pu_info_idx;
  PU_IDX pu_idx;
  ST_IDX proc_sym;
  TREE *entry;
  SCOPE scope;
  // SSA_TREE *ssa;
  // ̄ALIAS_INFO_TREE *alias;
  PU_INFO() : scope(0) {
      memset(this, 0, sizeof(PU_INFO));
  };
  void Set_proc_sym(ST_IDX proc_sym) {
    this->proc_sym = proc_sym;
    this->scope.st_idx = proc_sym;
  };
  void Print(FILE *) {};
  void Verify(FILE *) {};
  void Print_function_verbose(FILE *f);
};

// Growing table iterator
using GT_ITERATOR = std::vector<void *>::iterator;

/**
 * Global accessor for symtab table <T>
 * @tparam IDX
 * @tparam T
 */
template<typename IDX, typename T>
class GLOBAL_SYMTAB_ACCESS {
  GROWING_TABLE<IDX, T> tab;
public:
  IDX Add();
  T *Get(IDX);
  T *operator[](IDX idx) {
    return Get(idx);
  };
  void Set(IDX idx, T *obj) { return tab.Set(idx, obj); } // overriding an old value
  void Print(FILE *pFile);
  std::vector<void *>::iterator Begin() {
    return tab.Begin();
  }
  std::vector<void *>::iterator End() {
    return tab.End();
  }

  UINT32 Length();
};

enum TABLE_KIND{
  TABLE_KIND_ST = 1,
  TABLE_KIND_PREG = 2,
  TABLE_KIND_LABEL = 3,
  TABLE_KIND_INITO = 4,
  TABLE_KIND_TY = 5,
};

/**
 * Two-level symtable structure
 * When level <= 1, then use global table
 * When level == 2, then use local table
 * @tparam IDX
 * @tparam T
 */
template<typename IDX, class T, TABLE_KIND KIND>
class RELATED_SYMTAB_ACCESS {
private:
  FILE_SYMTAB *_symtab;
  GROWING_TABLE<IDX, T> tab;
public:
  explicit RELATED_SYMTAB_ACCESS(FILE_SYMTAB *symtab) : _symtab(symtab) {
    //Is_Trace(Tracing(COMPONENT_FE, TRACE_INVOCATION),
    //         (TFile, "Creating a table %s\n", typeid(this).name()));
  };
  T *operator[] (IDX idx) {
    return Get(idx);
  };
  IDX Add(UINT8 level); // Adding an object to the table, returning an IDX
  T *Get(IDX); // Retrieve an object by using the IDX
  GROWING_TABLE<IDX, T> *Scoped_table();
  GROWING_TABLE<IDX, T> *Scoped_table(SCOPE *scope);
  void Set(IDX idx, T *obj) { return tab.Set(idx, obj); } // overriding an old value
//  typename std::vector<T>::iterator &Iterate();
  void Print(FILE *f); // Print global only here
  void Print(FILE *f, SCOPE *scope);  // Print function-level table only here
  std::vector<void *>::iterator Begin() { return tab.Begin(); }
  std::vector<void *>::iterator End() { return tab.End(); }
  UINT32 Length() { return tab.Length(); };
  std::vector<void *>::iterator Begin(SCOPE *scope) { return Scoped_table(scope)->Begin(); }
  std::vector<void *>::iterator End(SCOPE *scope) { return Scoped_table(scope)->End(); }
  UINT32 Length(SCOPE *scope) { return Scoped_table(scope)->Length(); };
};

typedef GLOBAL_SYMTAB_ACCESS<TY_IDX, TY> TY_TABLE;
typedef GLOBAL_SYMTAB_ACCESS<ARB_IDX, ARB> ARB_TABLE;
typedef GLOBAL_SYMTAB_ACCESS<TYLIST_IDX, TYLIST> TYLIST_TABLE;
typedef GLOBAL_SYMTAB_ACCESS<PU_IDX, PU> PU_TABLE;
typedef GLOBAL_SYMTAB_ACCESS<PU_INFO_IDX , PU_INFO> PU_INFO_TABLE;
typedef RELATED_SYMTAB_ACCESS<ST_IDX, ST, TABLE_KIND_ST> ST_TABLE;
typedef RELATED_SYMTAB_ACCESS<PREG_IDX, PREG, TABLE_KIND_PREG> PREG_TABLE;
typedef RELATED_SYMTAB_ACCESS<LABEL_IDX, LABEL, TABLE_KIND_LABEL> LABEL_TABLE;
typedef RELATED_SYMTAB_ACCESS<INITO_IDX, INITO, TABLE_KIND_INITO> INITO_TABLE;

// How do I dump everything to a file, then load a file afterwards?
// Perhaps some speed-up on Unix-based systems using mmap?
// To do that, I need a IDX group + Data-chunk based
// INITO should be just fine
// How to make sure that a "Table" is persistable....
// Index +

/**
 * File-level symbol tables
 */
class FILE_SYMTAB {
private:
  SCOPE_MANAGER * _scope_manager;
  TY_TABLE      * _ty_tab;
  ST_TABLE      * _st_tab;
  PU_TABLE      * _pu_tab;
  PU_INFO_TABLE * _pu_info_tab;
  TYLIST_TABLE  * _tylist_tab;
  ARB_TABLE     * _arb_tab;
  LABEL_TABLE   * _label_tab;
  PREG_TABLE    * _preg_tab;
  INITO_TABLE   * _inito_tab;

  // STRING TABLE SPECIFIC
  char * internal_str_tab_buffer;
  UINT64 allocated_size_of_buffer;
  UINT64 used_size_of_buffer;
public:
  explicit FILE_SYMTAB(SCOPE_MANAGER *scope) : _scope_manager(scope) {
    _st_tab          = new ST_TABLE(this);
    _st_tab->Add(0);
    _ty_tab          = new TY_TABLE();
    _ty_tab->Add();
    _pu_info_tab     = new PU_INFO_TABLE();
    _pu_info_tab->Add();
    _arb_tab         = new ARB_TABLE();
    _arb_tab->Add();
    _pu_info_tab     = new PU_INFO_TABLE();
    _pu_info_tab->Add();
    _pu_tab          = new PU_TABLE();
    _pu_tab->Add();
    _tylist_tab      = new TYLIST_TABLE();
    _tylist_tab->Add();
    _label_tab       = new LABEL_TABLE(this);
    _label_tab->Add(0);
    _preg_tab       = new PREG_TABLE(this);
    _preg_tab->Add(0);
    _inito_tab      = new INITO_TABLE(this);
    _inito_tab->Add(0);
    internal_str_tab_buffer = NULL;
    allocated_size_of_buffer = 0;
    used_size_of_buffer = 1;
  };

  // Tables
  TY_TABLE *Ty()           { return _ty_tab;  };
  ARB_TABLE *Arb()         { return _arb_tab; };
  ST_TABLE *Sym()          { return _st_tab;  };
  PU_TABLE *Pu()           { return _pu_tab;  };
  TYLIST_TABLE *Tylist()   { return _tylist_tab;  };
  PU_INFO_TABLE *Pu_info() { return _pu_info_tab; };
  LABEL_TABLE *Label()     { return _label_tab;   };
  PREG_TABLE *Preg()       { return _preg_tab;    };
  INITO_TABLE *Inito()     { return _inito_tab;   };

  /**
   * Utilities
   */
  SCOPE_MANAGER *Scope() { return _scope_manager; };
  PU_INFO_IDX Get_pu_info_by_st_idx(ST_IDX func);
  STR_IDX Save_string(const char *string);
  void Print(FILE *file);

  void Initialize();

//  template <typename T, typename V>
//  V *Get_table(T *);
  ST_TABLE *Get_table(ST *base) {
    return _st_tab;
  }
  TY_TABLE *Get_table(TY *base) {
    return _ty_tab;
  }
  TYLIST_TABLE  *Get_table(TYLIST *base) {
    return _tylist_tab;
  }
  PU_INFO_TABLE *Get_table(PU_INFO *base) {
    return _pu_info_tab;
  }
  ARB_TABLE *Get_table(ARB *base) {
    return _arb_tab;
  }
  PU_TABLE *Get_table(PU *base) {
    return _pu_tab;
  }
  LABEL_TABLE *Get_table(LABEL *base) {
    return _label_tab;
  }
  PREG_TABLE *Get_table(PREG *base) {
    return _preg_tab;
  }
  INITO_TABLE *Get_table(INITO *base) {
    return _inito_tab;
  }
  const char *Get_string(STR_IDX idx);

  void Print_functions(FILE *f);
};

/**
 *  Managing the context changes between iterating over functions
 * */
class SCOPE_MANAGER {
private:
  FILE_MANAGER *_file_manager;
  std::map<ST_IDX, SCOPE *> _in_memory_function_info; // used for storing scope for each function
  SCOPE *_current_function;
public:
  SCOPE_MANAGER(FILE_MANAGER *file_man) {
    this->_file_manager = file_man;
    this->_current_function = NULL;
    _in_memory_function_info.clear();
  }
  SCOPE *Current();
  BOOL Goto_function(ST_IDX);
  void Finish_function(ST_IDX func, PU_INFO_IDX func_info);
  void Print(FILE *file);
};

/**
 * File manager representing an IR FILE
 * You could open a file, close a file, and access the Tables(), or Scopes()
 * Tables(), symbol table data...
 * Scopes(), a traversal utility for storing scope related info
 */
class FILE_MANAGER {
private:
  FILE_SYMTAB *_file_symtab;
  SCOPE_MANAGER *_scope_manager;
public:
  std::vector<FILE_INFO *> _file_info;

  /**
   * File open / read, dumping, TODO: These three have not been implemented
   * IR file structure....
   * Header
   * Type Info ...
   * Symbol info ...
   * Function Unit Code .....
   * INITO ....
   * INITV ....
   * @param file_name
   */
  void Open_ir_file(const char *file_name);
  void Create_ir_file(const char *file_name);
  void Write_data_to_file(const char *file_name);


  /**
  *  Probably need some code-gen work done to generate llvm-ir
  *  or generate WHIRL instead. function unit in VH-whirl to VL-whirl
  */


  /**
   * Accessing Data
   */
  FILE_SYMTAB *Tables() {
    AssertThat(_file_symtab != NULL, ("incomplete data, _file_symtab is null"));
    return _file_symtab;
  }

  SCOPE_MANAGER *Scopes() {
    AssertThat(_scope_manager != NULL, ("incomplete data, _scope_manager is null"));
    return _scope_manager;
  };

  /**
   * Utilities
   */
  // Creating a function in the table
  PU_INFO_IDX Create_function(ST_IDX func, TY_IDX prototype);
  // Invoked after creating the function
  void Finish_creating_function(ST_IDX func);

  FILE_MANAGER() {
    _scope_manager = new SCOPE_MANAGER(this);
    _file_symtab = new FILE_SYMTAB(_scope_manager);
  };

  void Initialize();

  TY_IDX Create_array_ty(STR_IDX string, TY_FLAG ty_flag,
                         TY_IDX element_type, ARB_IDX arb);
  TY_IDX Create_func_ty(STR_IDX string, UINT64 size, MTYPE_ID mtype,
                        TY_FLAG ty_flag, std::vector<TY_IDX> &ret_and_params);
  LABEL_IDX Create_label(STR_IDX name, UINT32 flags, LABEL_KIND lbk);
  ST_IDX Create_var(STR_IDX string, TY_IDX idx, UINT8 level,
                    SYM_SCLASS sclass, SYM_ECLASS eclass, SYM_CLASS symclass);
  INITO_IDX Create_inito(ST_IDX st_idx, std::vector<INITV> &initv, UINT8 level);

  STR_IDX Save_string(const char *string); // Save a null-term-string to string tab
  void Print(FILE *f);
  ARB_IDX Create_array_bound_const(UINT64 ubnd_val, UINT64 stride_val,
                                   UINT32 dimen, UINT32 flag);
  ARB_IDX Create_array_bound_var(ST_IDX ubnd_var, UINT64 stride_val,
                                 UINT32 dimen, UINT32 flag);

  ST_IDX Find_symbol_by_name(const char *name);
  LABEL_IDX Create_preg(STR_IDX preg_name, UINT32 desire_num);

  LABEL_IDX Get_func_exit_label();

  ST_IDX Get_preg_sym(MTYPE_ID mt, PREG_IDX regid);
};
const char *STR_str(STR_IDX idx);

// Returning the current pu idx
extern FILE_MANAGER *File();
extern const char * MTYPE_name(MTYPE_ID mtype);
extern UINT64 MTYPE_size(MTYPE_ID mtype);
extern TY_IDX MTYPE_to_ty(MTYPE_ID id);

#define OCC_SYMTAB_ACCESS_DECL_MODE
#include "symtab_access.h"
#undef OCC_SYMTAB_ACCESS_DECL_MODE

#endif
