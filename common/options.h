
#include "host.h"
#include <string>
#include <vector>
#include "args.h" // MIT License

using STRVEC = std::vector<std::string>;
using std::string;

enum OPT_KIND {
  OPT_KIND_ARITH       = 1,
  OPT_KIND_LDID_CONST  = 2,
};

class COMPILER_CONFIG {
public:
  INT32  opt_level   = 2;
  BOOL   assembly    = FALSE;
  BOOL   object_gen  = FALSE;
  BOOL   fe_only     = FALSE;
  BOOL   run_prep    = FALSE;
  BOOL   opt_bin     = TRUE;
  STRVEC files;
  STRVEC assemble_files;
  STRVEC object_files;
  string output_file;
  BOOL Opt_enabled(OPT_KIND k) {
    return true;
  }
};

// Parsing arguments
int Parse_args(int argc, char **argv, char **envp, COMPILER_CONFIG &conf);
