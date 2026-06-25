#include "target.h"

#include <cassert>
#include <cstdio>
#include <string>

namespace {

const char *Os_name(HOST_OS os) {
  switch (os) {
    case HOST_OS::LINUX:       return "linux";
    case HOST_OS::DARWIN:      return "darwin";
    case HOST_OS::UNSUPPORTED: return "unsupported";
  }
  return "?";
}

const char *Arch_name(HOST_ARCH arch) {
  switch (arch) {
    case HOST_ARCH::ARMV7:      return "armv7";
    case HOST_ARCH::AARCH64:    return "aarch64";
    case HOST_ARCH::I386:       return "i386";
    case HOST_ARCH::X86_64:     return "x86_64";
    case HOST_ARCH::UNSUPPORTED: return "unsupported";
  }
  return "?";
}

void Expect(const char *input, HOST_OS os, HOST_ARCH arch,
            const char *variant) {
  const HOST_DESC h = Parse_host_desc(input);
  std::fprintf(stderr,
              "[host_desc_test] %-30s -> {os=%-11s arch=%-11s variant=\"%s\"} "
              "expected {os=%d arch=%d variant=\"%s\"}\n",
              input, Os_name(h.os), Arch_name(h.arch), h.variant.c_str(),
              static_cast<int>(os), static_cast<int>(arch), variant);
  assert(h.os == os);
  assert(h.arch == arch);
  assert(h.variant == variant);
}

}  // namespace

int main() {
  std::printf("[host_desc_test] direct unit tests for Parse_host_desc\n");

  // 合法组合
  Expect("x86_64-linux", HOST_OS::LINUX, HOST_ARCH::X86_64, "");
  Expect("x86_64-linux+ilp32", HOST_OS::LINUX, HOST_ARCH::X86_64, "ilp32");
  Expect("aarch64-linux+ilp32", HOST_OS::LINUX, HOST_ARCH::AARCH64, "ilp32");
  Expect("arm64-darwin", HOST_OS::DARWIN, HOST_ARCH::AARCH64, "");
  Expect("arm64-apple", HOST_OS::DARWIN, HOST_ARCH::AARCH64, "");
  Expect("armv7-linux", HOST_OS::LINUX, HOST_ARCH::ARMV7, "");
  Expect("i386-linux", HOST_OS::LINUX, HOST_ARCH::I386, "");
  Expect("x86_64-darwin", HOST_OS::DARWIN, HOST_ARCH::X86_64, "");

  // 畸形 / 未知输入
  Expect("", HOST_OS::UNSUPPORTED, HOST_ARCH::UNSUPPORTED, "");
  Expect("mips-linux", HOST_OS::UNSUPPORTED, HOST_ARCH::UNSUPPORTED, "");
  Expect("x86_64-windows", HOST_OS::UNSUPPORTED, HOST_ARCH::UNSUPPORTED, "");
  Expect("x86_64", HOST_OS::UNSUPPORTED, HOST_ARCH::UNSUPPORTED, "");
  Expect("-linux", HOST_OS::UNSUPPORTED, HOST_ARCH::UNSUPPORTED, "");
  Expect("x86_64-", HOST_OS::UNSUPPORTED, HOST_ARCH::UNSUPPORTED, "");

  std::printf("[host_desc_test] all assertions passed\n");
  return 0;
}