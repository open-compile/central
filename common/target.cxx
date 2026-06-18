#include "target.h"

#include <algorithm>
#include <cctype>

static std::string Normalize_target_name(std::string s) {
  size_t start = 0;
  while (start < s.size() &&
         isspace(static_cast<unsigned char>(s[start]))) {
    ++start;
  }
  size_t end = s.size();
  while (end > start &&
         isspace(static_cast<unsigned char>(s[end - 1]))) {
    --end;
  }
  s = s.substr(start, end - start);

  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return static_cast<char>(tolower(c)); });
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '_' || s[i] == '.') s[i] = '-';
  }
  return s;
}

static const TARGET_INFO Target_infos[] = {
  { TARGET_ARCH_ARMV8_A32, "armv8-a32", "arm-linux-gnueabihf",
    4, 4, 8, 4, "arm-linux-gnueabihf-as", TRUE },
  { TARGET_ARCH_ARMV8_A64, "armv8-a64", "aarch64-linux-gnu",
    8, 8, 16, 8, "aarch64-linux-gnu-as", FALSE },
  { TARGET_ARCH_X86_64, "x86-64", "x86_64-linux-gnu",
    8, 8, 16, 6, "as", FALSE },
};

const TARGET_INFO &Target_info(TARGET_ARCH arch) {
  for (UINT32 i = 0; i < sizeof(Target_infos) / sizeof(Target_infos[0]); ++i) {
    if (Target_infos[i].arch == arch) return Target_infos[i];
  }
  return Target_infos[0];
}

const char *Target_arch_name(TARGET_ARCH arch) {
  return Target_info(arch).name;
}

BOOL Parse_target_arch(const std::string &text, TARGET_ARCH *arch) {
  std::string name = Normalize_target_name(text);
  if (name == "armv8-a32" || name == "armv8-32" || name == "arm32" ||
      name == "aarch32" || name == "arm") {
    *arch = TARGET_ARCH_ARMV8_A32;
    return TRUE;
  }
  if (name == "armv8-a64" || name == "armv9-a64" || name == "armv8-64" ||
      name == "armv9-64" || name == "arm64" || name == "aarch64") {
    *arch = TARGET_ARCH_ARMV8_A64;
    return TRUE;
  }
  if (name == "x86-64" || name == "x64" || name == "amd64" ||
      name == "x86_64") {
    *arch = TARGET_ARCH_X86_64;
    return TRUE;
  }
  return FALSE;
}

std::string Supported_target_arches() {
  return "armv8-a32/aarch32, armv8-a64/armv9-a64/aarch64, x86-64";
}
