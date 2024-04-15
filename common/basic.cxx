#include "basic.h"
#include <stdarg.h>

const char * Compilation_Phase = COMP_PHASE_DRIVER;
static TRACE_KIND Current_Cmd_Opt = TRACE_ERROR;
static TRACE_KIND Trace_opts[sizeof(COMPONENTS_WHOLE) * 8];

void Assertion_Failure_Print ( const char *fmt, ... )
{
  va_list vp;
  
  char msg[512];
  INT32 pos;

  /* Prepare header line: */
  va_start ( vp, fmt );
  pos = sprintf ( &msg[0], "### Assertion Failure in Phase %s: \n", Compilation_Phase);
  pos += vsprintf ( &msg[pos], fmt, vp );
  sprintf ( &msg[pos], "\n" );
  va_end ( vp );
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_ASSERTION);
}


void Comp_Failure_Print ( const char *fmt, ... )
{
  va_list vp;
  
  char msg[512];
  INT32 pos;

  /* Prepare header line: */
  va_start ( vp, fmt );
  pos = sprintf ( &msg[0], "Compilation failure in phase %s: \n", Compilation_Phase);
  pos += vsprintf ( &msg[pos], fmt, vp );
  sprintf ( &msg[pos], "\n" );
  va_end ( vp );
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_COMP_ERR);
}

/***
 *  Quit the processing with trace
 **/ 
void Quit_with_tracing(const char * file, UINT32 line) {
  // In case we don't want to expose the source code,
  // we could remove it compile-time by specify OCC_NO_TRACE_LINENO
  // we could remove it run-time by specify TRACE_OPT_NOLINENO
#ifndef OCC_NO_TRACE_LINENO
  if (Current_Cmd_Opt != TRACE_KIND::TRACE_OPT_NOLINENO) {
    fprintf(stderr, "### In file %s:%d\n", file, line);
  }
#endif
  fprintf(stderr, "%s", "occ: fatal error: ");
}

INT32 Right_most_bit(UINT32 n) {
  if (n == 0)
    return -1; // No bits are set

  // Calculate the position of the rightmost set bit
  int position = 1; // Position starts at 1, not 0
  unsigned int mask = n & (-n);

  // Shift the mask until we find the set bit
  while ((mask & 1) == 0) {
    mask >>= 1;
    position++;
  }

  return position;
}

/**
 * Get tracing level for specific component, this should not be exposed elsewhere than this file.
 * @param whole
 */
static TRACE_KIND Get_tracing_level(COMPONENTS_WHOLE whole) {
  INT32 outcome = Right_most_bit(whole);
  if (outcome == -1) {
    AssertThat(false, ("Failed to process component id = %.8x, to trace ", whole));
  }
  AssertThat(outcome < sizeof(COMPONENTS_WHOLE) * 8, ("Failed to process component id = %.8x, to trace ", whole));
  AssertThat(outcome >= 0, ("Failed to process component id = %.8x, to trace ", whole));
  return Trace_opts[outcome];
}

void Init_trace_opts() {
  Current_Cmd_Opt = TRACE_ERROR;
  UINT32 max = sizeof(COMPONENTS_WHOLE) * 8;
  for (UINT32 i = 0; i < max; i++) {
    Trace_opts[i] = TRACE_ERROR;
  }
}

/**
 *  The tracing option function used to determine whether to enable tracing for this kind of trace 
 **/
BOOL Tracing(COMPONENTS_WHOLE tl, TRACE_KIND tk) {
  // Make sure I have a table to set each component's log level here.
  TRACE_KIND lvl = Get_tracing_level(tl);
  return !!(lvl & tk);
}

/**
 * Set up tracing option given command line argument
 * @param opts can be either default / verbose or a specific trace level.
 */
void Set_tracing_option(TRACE_KIND opts) {
  if (opts == TRACE_OPT_DEFAULT || opts == TRACE_OPT_NOLINENO) {
    Current_Cmd_Opt = opts;
    UINT32 max = sizeof(COMPONENTS_WHOLE) * 8;
    for (UINT32 i = 0; i < max; i++) {
      Trace_opts[i] = TRACE_ERROR;
    }
    return;
  } else if (opts == TRACE_OPT_VERBOSE) {
    Current_Cmd_Opt = TRACE_OPT_VERBOSE;
    UINT32 max = sizeof(COMPONENTS_WHOLE) * 8;
    for (UINT32 i = 0; i < max; i++) {
      Trace_opts[i] = static_cast<TRACE_KIND>(0x7fffffff);
    }
    return;
  } else {
    Current_Cmd_Opt = opts;
    UINT32 max = sizeof(COMPONENTS_WHOLE) * 8;
    for (UINT32 i = 0; i < max; i++) {
      Trace_opts[i] = opts;
    }
  }
}