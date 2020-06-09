
#ifndef _OCC_BASIC_H_
#define _OCC_BASIC_H_

#include <stdlib.h>
#include <stdio.h>
#include "host.h"

#define AssertDev(Condition, Params) \
(Condition ? (void) 1 \
:(Quit_with_tracing ( __FILE__, __LINE__ ),	\
Assertion_Failure_Print Params) );

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
extern void Comp_Failure_Print ( const char *fmt, ... ); // Compilation Failure message printing

#define Comp_Failure(msg ...) { Quit_with_tracing ( __FILE__, __LINE__ ); Comp_Failure_Print(msg); }
#define TFile stderr
#define Is_Trace(cond, printval) { if ((cond)) { fprintf printval; } }

typedef enum {
    EXIT_NORMAL = 0,
    EXIT_UNKNOWN = 1,
    EXIT_ASSERTION = 2,
    EXIT_OPTION_ERR = 3,
    EXIT_COMP_ERR = 4,
} EXIT_CODES;

enum COMPONENTS_WHOLE {
  COMPONENT_FE = 1,
  COMPONENT_BE = 2,
  COMPONENT_ASM = 3,
  COMPONENT_LD = 4,
  COMPONENT_DRIVER = 10,
};

enum COMPONENTS_IN_BE {
  COMPONENT_IR_VERIFY = 1,
  COMPONENT_VHO = 2,
  COMPONENT_LNO = 2,
  COMPONENT_GOPT = 3,
  COMPONENT_IPA = 4,
  COMPONENT_CG = 5,
};

enum TRACE_KIND {
  TRACE_WARN = 0x1,
  TRACE_INFO = 0x2,
  TRACE_INVOCATION = 0x4,
  TRACE_PERFORMANCE = 0x8,
  TRACE_DATA = 0x10,
  TRACE_OPTIONS = 0x20,
};


// Tracing option
extern BOOL Tracing(COMPONENTS_WHOLE tl, TRACE_KIND tk);

#define COMP_PHASE_DRIVER "DRIVER"
#define COMP_PHASE_LEXER "LEXER"
#define COMP_PHASE_PARSER "PARSER"
#define COMP_PHASE_IR_GEN "IR_GEN"
#define COMP_PHASE_OPT_1 "OPT-1"
#define COMP_PHASE_OPT_2 "OPT-2"
#define COMP_PHASE_OPT_3 "OPT-3"
#define COMP_PHASE_OPT_LOWER "LOWER"
#define COMP_PHASE_CG "CODE_GEN"

#endif