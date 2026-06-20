#include "data_layout.h"
#include "target_info.h"

#include <cassert>
#include <string>

static const TARGET_INFO &Target(const char *name) {
  const TARGET_INFO *target = nullptr;
  std::string error;
  assert(Resolve_target(name, &target, &error));
  return *target;
}

int main() {
  DATA_LAYOUT arm_layout(Target("armv7").abi);
  assert(arm_layout.Pointer_size() == 4);
  assert(arm_layout.Stack_alignment() == 8);
  assert(arm_layout.Register_formal_count() == 4);
  assert(arm_layout.Prologue_save_area() == 36);

  DATA_LAYOUT x64_layout(Target("x64-linux").abi);
  assert(x64_layout.Pointer_size() == 8);
  assert(x64_layout.Stack_alignment() == 16);
  assert(x64_layout.Register_formal_count() == 6);
  assert(x64_layout.Prologue_save_area() == 16);
  return 0;
}
