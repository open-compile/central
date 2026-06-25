#include "target.h"

#include <cassert>
#include <cstdio>
#include <string>

#ifndef SIMULATE_HOST_AS_PLATFORM
#error "native_ilp32_target_test requires SIMULATE_HOST_AS_PLATFORM " \
       "(see cg/CMakeLists.txt; default: x86_64-linux+ilp32)"
#endif

int main() {
  const std::string simulation = SIMULATE_HOST_AS_PLATFORM;
  std::printf("[native_ilp32_target_test] simulating host: %s\n",
              simulation.c_str());

  const HOST_DESC host = Parse_host_desc(simulation);
  std::string triple;
  std::string error;
  const bool ok = Resolve_native_target(host, &triple, &error);

  std::printf("[native_ilp32_target_test] resolved: %s\n",
              ok ? triple.c_str() : error.c_str());

  assert(!ok);
  assert(triple.empty());
  assert(error.find("ILP32") != std::string::npos);
  return 0;
}