
#include "host.h"
#include <string>
#include <vector>
#include "args.h" // MIT License

class COMPILER_CONFIG {
public:
  INT32 opt_level = 2;
  BOOL  assembly = FALSE;
  BOOL object_gen = FALSE;
  std::vector<std::string> files;
  std::vector<std::string> assemble_files;
  std::vector<std::string> object_files;
  std::string output_file;
};

// Parsing arguments
int Parse_args(int argc, char **argv, char **envp, COMPILER_CONFIG &conf);
