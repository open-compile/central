#include "basic.h"
#include <stdarg.h>

const char * Compilation_Phase = COMP_PHASE_DRIVER;

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
    fprintf(stderr, "\n\n\n### In file %s:%d\n", file, line);
}

/**
 *  The tracing option function used to determine whether to enable tracing for this kind of trace 
 **/
BOOL Tracing(COMPONENTS_WHOLE tl, TRACE_KIND tk) {
  return TRUE;
}