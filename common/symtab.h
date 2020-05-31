#include "basic.h"

// Symbol Table Frame Work
typedef enum {
    ST_ = 0
} SYM_ATTR;

typedef enum {
    SYMC_UNKNOWN = 0,
    SYMC_AUTO = 1,
    SYMC_FUNC_STATIC = 2,
    SYMC_FILE_STATIC = 3,
    SYMC_EXTERN = 4,
    SYMC_GLOBAL_UNDEF = 5,
    SYMC_GLOBAL_DEF = 6,
    SYMC_TEXT = 7,
} SYM_SCLASS;

typedef enum {
    SYME_UNKNOWN = 0,
    SYME_INTERNAL = 1,
    SYME_EXTERNAL = 2,
} SYM_ECLASS;

typedef struct {
    
} FUNCTION_ENTRY;

typedef struct  {

} INIT_OBJECT_ENTRY; // Static-Init object table entry

typedef struct  {

} INIT_VALUE_ENTRY; // Static-Init object value entry

typedef struct {

} STRING_ENTRY;

typedef struct {

} TARGET_CONSTANT_ENTRY;

typedef struct {

} MTYPE; // Machine type

typedef struct {

} TYE; // Type table entry

typedef struct {
  const char *name;
} STE; // Symbol table entry


/**
 *  Utility Functions, 
 *  for accessing the above data structures
*/
const char * SYMC_name(STE *symbol) {
    const char *res = 0;
    AssertThat(NULL != symbol, ("Symbol should not be null"));
    AssertThat(NULL != res, ("Res shoould not be null"));
    return res;
}

/**
 *  Utility Functions, 
 *  for accessing the above data structures
*/
const char * SYMC_idx(STE *symbol) {
    AssertThat(NULL != symbol, ("Symbol should not be null"));
    AssertThat(NULL != symbol->name, ("Symbol name should not be null"));
    return symbol->name;
}