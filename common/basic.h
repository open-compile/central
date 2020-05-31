#include <stdlib.h>
#include <stdio.h>
#include "host.h"

#define AssertDev(Condition, Params) \
    ( Condition ? (void) 1 \
      : ( Quit_with_tracing ( __FILE__, __LINE__ ),	\
	       Assertion_Failure_Print Params) )

/* Only in non-release mode */
#ifndef Release_Mode
# define AssertThat AssertDev
# define Assert_Ret(cond, msg, ...) AssertThat(cond, msg)
#else
# define AssertThat(a, b) ((void) 1)
# define Assert_Ret (cond, msg, ...)    \
  if(!(cond)) {                         \
    return __VA_ARGS__;                 \
  }
#endif

extern void Quit_with_tracing(const char *, UINT32); // Quiting 
extern void Assertion_Failure_Print ( const char *fmt, ... ); // Printf-like function
extern const char * Compilation_Phase; // Compilation Phase currently in

typedef enum {
    EXIT_NORMAL = 0,
    EXIT_UNKNOWN = 1,
    EXIT_ASSERTION = 2,
} EXIT_CODES;

#define COMP_PHASE_DRIVER "DRIVER"
#define COMP_PHASE_LEXER "LEXER"
#define COMP_PHASE_PARSER "PARSER"
#define COMP_PHASE_IR_GEN "IR_GEN"
#define COMP_PHASE_OPT_1 "OPT-1"
#define COMP_PHASE_OPT_2 "OPT-2"
#define COMP_PHASE_OPT_3 "OPT-3"
#define COMP_PHASE_OPT_LOWER "LOWER"
#define COMP_PHASE_CG "CODE_GEN"