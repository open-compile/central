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

void Quit_with_tracing(const char * file, UINT32 line) {
    fprintf(stderr, "### In file %s:%d\n", file, line);
}