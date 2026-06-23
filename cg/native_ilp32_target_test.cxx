#include "target.h"

#include <cassert>
#include <string>

int main() {
#if defined(__aarch64__) || defined(__x86_64__)
  std::string triple;
  std::string error;
  assert(!Detect_native_target(&triple, &error));
  assert(triple.empty());
  assert(error.find("ILP32") != std::string::npos);
#endif
  return 0;
}
