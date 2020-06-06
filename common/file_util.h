#include <fstream>
#include "host.h"

BOOL fexists(const std::string& filename) {
  std::ifstream ifile(filename.c_str());
  return (BOOL)ifile;
}

BOOL fexists(const char *filename) {
  std::ifstream ifile(filename);
  return (BOOL)ifile;
}