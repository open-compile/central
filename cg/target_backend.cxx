#include "target_backend.h"

#include "arm64_target.h"
#include "armv7_linux_target.h"
#include "x86_target.h"

std::unique_ptr<TARGET_BACKEND> Create_target_backend(
    const TARGET_INFO &info, std::string *error) {
  if (info.arch == TARGET_ARCH::ARMV7 && info.os == TARGET_OS::LINUX) {
    if (error != nullptr) error->clear();
    return std::unique_ptr<TARGET_BACKEND>(new ARMV7_LINUX_TARGET(info));
  }
  if (info.arch == TARGET_ARCH::AARCH64) {
    if (error != nullptr) error->clear();
    return std::unique_ptr<TARGET_BACKEND>(new ARM64_TARGET(info));
  }
  if (info.arch == TARGET_ARCH::I386 || info.arch == TARGET_ARCH::X86_64) {
    if (error != nullptr) error->clear();
    return std::unique_ptr<TARGET_BACKEND>(new X86_TARGET(info));
  }
  if (error != nullptr) {
    *error = "target backend is not implemented yet: " + info.triple;
  }
  return nullptr;
}
