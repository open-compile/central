
#include "host.h"
#include <string>
#include <vector>
#include "args.h" // MIT License

using STRVEC = std::vector<std::string>;
using std::string;

class COMPILER_CONFIG {
public:
  INT32  opt_level   = 2;
  BOOL   assembly    = FALSE;
  BOOL   object_gen  = FALSE;
  BOOL   fe_only     = FALSE;
  STRVEC files;
  STRVEC assemble_files;
  STRVEC object_files;
  string output_file;
};

// Parsing arguments
int Parse_args(int argc, char **argv, char **envp, COMPILER_CONFIG &conf);
