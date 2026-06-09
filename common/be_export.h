#ifndef _OCC_BE_EXPORT_H_
#define _OCC_BE_EXPORT_H_

#include "options.h"

#define BE_MAIN_NAME bemain
#define BE_EXTERNAL_MAIN_NAME Opt_run_optimizer

INT32 BE_MAIN_NAME(INT32 argc, char **argv);
INT32 BE_EXTERNAL_MAIN_NAME(COMPILER_CONFIG &conf);
INT32 CG_full_process(COMPILER_CONFIG &conf);

#endif