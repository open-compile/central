
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
#define Comp_Usual_Error(msg ...) { Quit_with_tracing ( __FILE__, __LINE__ ); Comp_Failure_Print(msg); }
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
  COMPONENT_DRIVER = 100,
  // Common ones
  COMPONENT_SYMTAB = 130,
  // Front end
  COMPONENT_PREP = 210,
  COMPONENT_FE = 250,
  COMPONENT_BE = 300,
  COMPONENT_VHO = 310,
  COMPONENT_LNO = 320,
  COMPONENT_GOPT = 330,
  COMPONENT_IPA = 340,
  COMPONENT_SSA = 350,
  COMPONENT_SSA_CONV = 351,
  // Code generation
  COMPONENT_CG = 410,
  COMPONENT_CG_IR_IN = 412,
  COMPONENT_CG_CONV = 420,
  COMPONENT_CG_LRA = 430, // live range analysis
  COMPONENT_CG_REGALLOC = 440, // register alloca
  COMPONENT_CG_LAYOUT = 450,   // frame layout stuff
  COMPONENT_CG_EMIT = 460,
  COMPONENT_ASM = 500,
  COMPONENT_LD = 600,
  COMPONENT_CROSS_PHASE = 700,
  COMPONENT_MAX = 999,
};

enum TRACE_KIND {
  TRACE_DEBUG       = 0x1,
  TRACE_DATA        = 0x2,
  TRACE_INFO        = 0x4,
  TRACE_PERFORMANCE = 0x8,
  TRACE_INVOCATION  = 0x10,
  TRACE_OPTIONS     = 0x20,
  TRACE_EMIT_CORE   = 0x40,
  TRACE_EMIT_BASIC  = 0x80,
  TRACE_WARN        = 0x100,
  TRACE_ERROR       = 0x200,
  TRACE_FATAL       = 0x400,
  TRACE_OPT_DEFAULT = 0x800,
  TRACE_OPT_VERBOSE = 0x1000,
  TRACE_OPT_NOLINENO= 0x2000,
  TRACE_CUSTOM1     = 0x4000,
  TRACE_CUSTOM2     = 0x8000,
};


// Tracing option
extern BOOL Tracing(COMPONENTS_WHOLE tl, TRACE_KIND tk);
extern void Set_tracing_option(TRACE_KIND opts);
extern void Set_mod_tracing_option(COMPONENTS_WHOLE comp, TRACE_KIND desired);
extern void Init_trace_opts();

#define COMP_PHASE_DRIVER "DRIVER"
#define COMP_PHASE_LEXER "LEXER"
#define COMP_PHASE_PARSER "PARSER"
#define COMP_PHASE_IR_GEN "IR_GEN"
#define COMP_PHASE_OPT_1 "OPT-1"
#define COMP_PHASE_OPT_2 "OPT-2"
#define COMP_PHASE_OPT_3 "OPT-3"
#define COMP_PHASE_OPT_LOWER "LOWER"
#define COMP_PHASE_CG "CODE_GEN"
#define COMP_PHASE_ASM "ASSEMBLER"

#endif