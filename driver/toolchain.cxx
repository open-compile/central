#include "toolchain.h"

#include "driver_options.h"
#include "options.h"
#include "basic.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <utility>

namespace {

TOOLCHAIN_DRIVER Driver(TOOLCHAIN_FAMILY family, const std::string &program,
                        std::vector<std::string> target_args) {
  return TOOLCHAIN_DRIVER{family, program, std::move(target_args)};
}

void Add_if_requested(const TOOLCHAIN_DRIVER &driver,
                      const TOOLCHAIN_REQUEST &request,
                      std::vector<TOOLCHAIN_DRIVER> *drivers) {
  if (request.family == TOOLCHAIN_FAMILY::AUTO ||
      request.family == driver.family) {
    drivers->push_back(driver);
  }
}

bool Append_group(const std::vector<std::string> &group, const char *prefix,
                  TOOLCHAIN_COMMAND_RESULT *result) {
  if (group.empty()) {
    result->error = "toolchain argument groups must not be empty";
    return false;
  }

  std::string argument(prefix);
  for (const auto &element : group) {
    if (element.empty()) {
      result->error = "toolchain argument group elements must not be empty";
      return false;
    }
    argument += ',';
    argument += element;
  }
  result->argv.push_back(std::move(argument));
  return true;
}

void Initialize_command(const TOOLCHAIN_DRIVER &driver,
                        TOOLCHAIN_COMMAND_RESULT *result) {
  result->argv.push_back(driver.program);
  result->argv.insert(result->argv.end(), driver.target_args.begin(),
                      driver.target_args.end());
}

TOOLCHAIN_COMMAND_RESULT Invalid_result(TOOLCHAIN_COMMAND_RESULT result) {
  result.argv.clear();
  return result;
}

}  // namespace

TOOLCHAIN_LINK_ITEM Link_input(const std::string &input) {
  return TOOLCHAIN_LINK_ITEM{TOOLCHAIN_LINK_ITEM_KIND::INPUT, input, {}};
}

TOOLCHAIN_LINK_ITEM Source_object() {
  return TOOLCHAIN_LINK_ITEM{TOOLCHAIN_LINK_ITEM_KIND::SOURCE_OBJECT, "", {}};
}

TOOLCHAIN_LINK_ITEM Linker_arg_group(std::vector<std::string> linker_args) {
  return TOOLCHAIN_LINK_ITEM{TOOLCHAIN_LINK_ITEM_KIND::LINKER_ARG_GROUP, "",
                             std::move(linker_args)};
}

std::vector<TOOLCHAIN_DRIVER> Toolchain_candidates(
    const std::string &triple, const TOOLCHAIN_REQUEST &request) {
  std::vector<TOOLCHAIN_DRIVER> drivers;

  if (triple == "arm64-apple-darwin") {
    // There is intentionally no GNU GCC candidate for Apple targets. Apple's
    // /usr/bin/gcc is Clang, and no compatible GNU executable can be assumed.
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "arm64-apple-macos"}),
                     request, &drivers);
  } else if (triple == "x86_64-apple-darwin") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "x86_64-apple-macos"}),
                     request, &drivers);
  } else if (triple == "x86_64-linux-gnu") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC, "gcc", {"-m64"}), request,
                     &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "x86_64-linux-gnu"}),
                     request, &drivers);
  } else if (triple == "i386-linux-gnu") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC, "gcc", {"-m32"}), request,
                     &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "i386-linux-gnu"}),
                     request, &drivers);
  } else if (triple == "armv7-linux-gnueabihf") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC,
                            "arm-linux-gnueabihf-gcc",
                            {"-marm", "-march=armv7-a",
                             "-mfloat-abi=hard"}),
                     request, &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "armv7-linux-gnueabihf", "-marm",
                             "-march=armv7-a", "-mfloat-abi=hard"}),
                     request, &drivers);
  } else if (triple == "aarch64-linux-gnu") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC, "aarch64-linux-gnu-gcc",
                            {}),
                     request, &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "aarch64-linux-gnu"}),
                     request, &drivers);
  }

  return drivers;
}

namespace {

std::string Format_argv(const std::vector<std::string> &argv) {
  std::ostringstream out;
  for (size_t i = 0; i < argv.size(); ++i) {
    if (i != 0) out << ' ';
    out << argv[i];
  }
  return out.str();
}

std::string Failure(const char *phase, const std::string &triple,
                    const std::vector<std::string> &argv,
                    const std::string &detail) {
  return "external tool failure: phase=" + std::string(phase) +
         " target=" + triple + " argv=[" + Format_argv(argv) + "] " +
         detail;
}

}  // namespace

TOOLCHAIN_RUN_RESULT Run_tool_command(const std::vector<std::string> &argv,
                                      const char *phase,
                                      const std::string &triple,
                                      bool show_command) {
  TOOLCHAIN_RUN_RESULT result;
  if (argv.empty() || argv[0].empty()) {
    result.error = Failure(phase, triple, argv, "launch=empty-command");
    return result;
  }
  if (show_command) {
    std::cerr << "external tool: phase=" << phase << " target=" << triple
              << " argv=[" << Format_argv(argv) << "]\n";
  }

  int launch_pipe[2];
  if (pipe(launch_pipe) != 0) {
    result.error = Failure(phase, triple, argv,
                           "launch=" + std::string(std::strerror(errno)));
    return result;
  }
  fcntl(launch_pipe[1], F_SETFD, FD_CLOEXEC);

  const pid_t pid = fork();
  if (pid < 0) {
    const int saved = errno;
    close(launch_pipe[0]);
    close(launch_pipe[1]);
    result.error = Failure(phase, triple, argv,
                           "launch=" + std::string(std::strerror(saved)));
    return result;
  }
  if (pid == 0) {
    close(launch_pipe[0]);
    std::vector<char *> child_argv;
    child_argv.reserve(argv.size() + 1);
    for (const std::string &arg : argv)
      child_argv.push_back(const_cast<char *>(arg.c_str()));
    child_argv.push_back(nullptr);
    execvp(child_argv[0], child_argv.data());
    const int saved = errno;
    (void)!write(launch_pipe[1], &saved, sizeof(saved));
    _exit(127);
  }

  close(launch_pipe[1]);
  int child_errno = 0;
  ssize_t launch_bytes;
  do {
    launch_bytes = read(launch_pipe[0], &child_errno, sizeof(child_errno));
  } while (launch_bytes < 0 && errno == EINTR);
  close(launch_pipe[0]);
  int wait_status = 0;
  while (waitpid(pid, &wait_status, 0) < 0) {
    if (errno == EINTR) continue;
    result.error = Failure(phase, triple, argv,
                           "wait=" + std::string(std::strerror(errno)));
    return result;
  }
  if (launch_bytes > 0) {
    result.error = Failure(phase, triple, argv,
                           "launch=" + std::string(std::strerror(child_errno)));
    return result;
  }

  result.launched = true;
  if (WIFEXITED(wait_status)) {
    result.status = WEXITSTATUS(wait_status);
  } else if (WIFSIGNALED(wait_status)) {
    result.status = 128 + WTERMSIG(wait_status);
  } else {
    result.status = 255;
  }
  if (result.status != 0) {
    result.error = Failure(phase, triple, argv,
                           "status=" + std::to_string(result.status));
  }
  return result;
}

bool Discover_toolchain(const std::string &triple,
                        const TOOLCHAIN_REQUEST &request,
                        TOOLCHAIN_DRIVER *selected, std::string *error) {
  error->clear();
  const auto candidates = Toolchain_candidates(triple, request);
  if (candidates.empty()) {
    if (request.family == TOOLCHAIN_FAMILY::GCC &&
        triple.find("apple-darwin") != std::string::npos) {
      *error = "GNU GCC is not supported for Apple target " + triple;
    } else {
      *error = "no external toolchain candidates for target " + triple;
    }
    return false;
  }
  std::string last_error;
  for (const auto &candidate : candidates) {
    std::vector<std::string> probe{candidate.program};
    probe.insert(probe.end(), candidate.target_args.begin(),
                 candidate.target_args.end());
    probe.push_back("--version");
    const auto result = Run_tool_command(probe, "probe", triple, false);
    if (result.ok()) {
      *selected = candidate;
      return true;
    }
    last_error = result.error;
  }
  *error = "no usable external toolchain for target " + triple;
  if (!last_error.empty()) *error += ": " + last_error;
  return false;
}

TOOLCHAIN_COMMAND_RESULT Build_assemble_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &assembly,
    const std::string &object,
    const TOOLCHAIN_ARG_GROUPS &assembler_arg_groups) {
  TOOLCHAIN_COMMAND_RESULT result;
  Initialize_command(driver, &result);
  result.argv.push_back("-c");
  for (const auto &group : assembler_arg_groups) {
    if (!Append_group(group, "-Wa", &result)) {
      return Invalid_result(std::move(result));
    }
  }
  result.argv.push_back(assembly);
  result.argv.push_back("-o");
  result.argv.push_back(object);
  return result;
}

TOOLCHAIN_COMMAND_RESULT Build_link_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &object,
    const std::vector<TOOLCHAIN_LINK_ITEM> &items, const std::string &output) {
  TOOLCHAIN_COMMAND_RESULT result;
  Initialize_command(driver, &result);
  size_t source_objects = 0;
  for (const auto &item : items) {
    if (item.kind == TOOLCHAIN_LINK_ITEM_KIND::INPUT) {
      if (item.input.empty()) {
        result.error = "toolchain link inputs must not be empty";
        return Invalid_result(std::move(result));
      }
      result.argv.push_back(item.input);
    } else if (item.kind == TOOLCHAIN_LINK_ITEM_KIND::SOURCE_OBJECT) {
      ++source_objects;
      result.argv.push_back(object);
    } else if (!Append_group(item.linker_args, "-Wl", &result)) {
      return Invalid_result(std::move(result));
    }
  }
  if (source_objects != 1) {
    result.error = "toolchain link command requires exactly one source object";
    return Invalid_result(std::move(result));
  }
  result.argv.push_back("-o");
  result.argv.push_back(output);
  return result;
}

namespace {

TOOLCHAIN_FAMILY Requested_family(EXTERNAL_TOOLCHAIN_FAMILY family) {
  switch (family) {
    case EXTERNAL_TOOLCHAIN_FAMILY::CLANG: return TOOLCHAIN_FAMILY::CLANG;
    case EXTERNAL_TOOLCHAIN_FAMILY::GCC: return TOOLCHAIN_FAMILY::GCC;
    case EXTERNAL_TOOLCHAIN_FAMILY::AUTO: return TOOLCHAIN_FAMILY::AUTO;
  }
  return TOOLCHAIN_FAMILY::AUTO;
}

const DRIVER_RESERVED_FILE *Owned_file(const COMPILER_CONFIG &config,
                                       const std::string &path) {
  for (const auto &owned : config.reserved_intermediate_files) {
    if (owned.path == path) return &owned;
  }
  return nullptr;
}

bool Verify_owned(const COMPILER_CONFIG &config, const std::string &path,
                  bool require_content, std::string *error) {
  const DRIVER_RESERVED_FILE *owned = Owned_file(config, path);
  struct stat info;
  if (owned == nullptr || lstat(path.c_str(), &info) != 0 ||
      !S_ISREG(info.st_mode) ||
      static_cast<std::uint64_t>(info.st_dev) != owned->device ||
      static_cast<std::uint64_t>(info.st_ino) != owned->inode) {
    *error = "owned output identity changed: " + path;
    return false;
  }
  if (require_content && info.st_size == 0) {
    *error = "missing output (reserved file was not written): " + path;
    return false;
  }
  return true;
}

std::string Directory(const std::string &path) {
  const size_t slash = path.find_last_of('/');
  return slash == std::string::npos ? "" : path.substr(0, slash + 1);
}

std::string Name(const std::string &path) {
  const size_t slash = path.find_last_of('/');
  return path.substr(slash == std::string::npos ? 0 : slash + 1);
}

bool Copy_owned_atomically(const COMPILER_CONFIG &config,
                           const std::string &source,
                           const std::string &destination,
                           std::string *error) {
  if (destination.empty()) return true;
  if (!Verify_owned(config, source, true, error)) return false;
  std::string pattern = Directory(destination) + "." + Name(destination) +
                        ".central-publish-XXXXXX";
  std::vector<char> temporary(pattern.begin(), pattern.end());
  temporary.push_back('\0');
  const int output = mkstemp(temporary.data());
  if (output < 0) {
    *error = "cannot reserve publication for " + destination + ": " +
             std::strerror(errno);
    return false;
  }
  const int input = open(source.c_str(), O_RDONLY | O_NOFOLLOW);
  bool ok = input >= 0;
  const DRIVER_RESERVED_FILE *owned = Owned_file(config, source);
  struct stat input_info;
  if (ok &&
      (owned == nullptr || fstat(input, &input_info) != 0 ||
       !S_ISREG(input_info.st_mode) ||
       static_cast<std::uint64_t>(input_info.st_dev) != owned->device ||
       static_cast<std::uint64_t>(input_info.st_ino) != owned->inode)) {
    *error = "owned input identity changed while publishing: " + source;
    ok = false;
  }
  char buffer[16384];
  while (ok) {
    const ssize_t count = read(input, buffer, sizeof(buffer));
    if (count == 0) break;
    if (count < 0) {
      if (errno == EINTR) continue;
      ok = false;
      break;
    }
    ssize_t written = 0;
    while (written < count) {
      const ssize_t amount =
          write(output, buffer + written, static_cast<size_t>(count - written));
      if (amount < 0 && errno == EINTR) continue;
      if (amount <= 0) {
        ok = false;
        break;
      }
      written += amount;
    }
  }
  if (input >= 0 && close(input) != 0) ok = false;
  if (fsync(output) != 0 || close(output) != 0) ok = false;
  if (ok && !Verify_owned(config, source, true, error)) ok = false;
  if (ok && rename(temporary.data(), destination.c_str()) != 0) ok = false;
  if (!ok) {
    const int saved = errno;
    unlink(temporary.data());
    if (error->empty()) {
      *error = "cannot publish " + destination + ": " +
               std::strerror(saved);
    }
    return false;
  }
  return true;
}

bool Publish_owned(COMPILER_CONFIG *config, const std::string &source,
                   const std::string &destination, std::string *error) {
  if (!Verify_owned(*config, source, true, error)) return false;
  if (rename(source.c_str(), destination.c_str()) != 0) {
    *error = "cannot atomically publish " + destination + ": " +
             std::strerror(errno);
    return false;
  }
  return true;
}

std::vector<TOOLCHAIN_LINK_ITEM> Convert_link_items(
    const std::vector<DRIVER_LINK_ITEM> &items) {
  std::vector<TOOLCHAIN_LINK_ITEM> converted;
  converted.reserve(items.size());
  for (const auto &item : items) {
    if (item.kind == DRIVER_LINK_ITEM_KIND::INPUT) {
      converted.push_back(Link_input(item.input));
    } else if (item.kind == DRIVER_LINK_ITEM_KIND::SOURCE_OBJECT) {
      converted.push_back(Source_object());
    } else {
      converted.push_back(Linker_arg_group(item.linker_args));
    }
  }
  return converted;
}

std::string Phase_failure(const char *phase, const COMPILER_CONFIG &config,
                          const std::vector<std::string> &argv,
                          const std::string &detail) {
  return Failure(phase, config.target.triple, argv, detail);
}

}  // namespace

int Run_external_toolchain(COMPILER_CONFIG *config, std::string *error) {
  error->clear();
  TOOLCHAIN_REQUEST request;
  request.family = Requested_family(config->toolchain_family);
  TOOLCHAIN_DRIVER driver;
  if (!Discover_toolchain(config->target.triple, request, &driver, error))
    return 1;

  const auto assemble = Build_assemble_command(
      driver, config->assembly_output_file, config->object_output_file,
      config->assembler_arg_groups);
  std::vector<std::string> final_argv = assemble.argv;
  if (!assemble.ok()) {
    *error = Phase_failure("object", *config, assemble.argv,
                           "status=configuration-error " + assemble.error);
    return 1;
  }
  Is_Trace(Tracing(COMPONENT_ASM, TRACE_INVOCATION),
           (TFile, "external object phase for %s\n",
            config->target.triple.c_str()));
  auto run = Run_tool_command(assemble.argv, "object", config->target.triple,
                              config->show_external_commands);
  if (!run.ok()) {
    *error = run.error;
    return run.launched ? run.status : 1;
  }
  std::string output_error;
  if (!Verify_owned(*config, config->object_output_file, true, &output_error)) {
    *error = Phase_failure("object", *config, assemble.argv,
                           "status=0 " + output_error);
    return 1;
  }

  if (config->output_mode == DRIVER_OUTPUT_MODE::LINK) {
    const auto link = Build_link_command(
        driver, config->object_output_file,
        Convert_link_items(config->link_items), config->executable_output_file);
    final_argv = link.argv;
    if (!link.ok()) {
      *error = Phase_failure("link", *config, link.argv,
                             "status=configuration-error " + link.error);
      return 1;
    }
    Is_Trace(Tracing(COMPONENT_LD, TRACE_INVOCATION),
             (TFile, "external link phase for %s\n",
              config->target.triple.c_str()));
    run = Run_tool_command(link.argv, "link", config->target.triple,
                           config->show_external_commands);
    if (!run.ok()) {
      *error = run.error;
      return run.launched ? run.status : 1;
    }
    if (!Verify_owned(*config, config->executable_output_file, true,
                      &output_error)) {
      *error = Phase_failure("link", *config, link.argv,
                             "status=0 " + output_error);
      return 1;
    }
  }

  if (config->keep_intermediates &&
      !Copy_owned_atomically(*config, config->assembly_output_file,
                             config->retained_assembly_output_file, error)) {
    *error = Phase_failure(config->output_mode == DRIVER_OUTPUT_MODE::LINK
                               ? "link"
                               : "object",
                           *config, final_argv,
                           "status=publication-error " + *error);
    return 1;
  }
  if (config->output_mode == DRIVER_OUTPUT_MODE::LINK &&
      config->keep_intermediates &&
      !Copy_owned_atomically(*config, config->object_output_file,
                             config->retained_object_output_file, error)) {
    *error = Phase_failure("link", *config, final_argv,
                           "status=publication-error " + *error);
    return 1;
  }

  const std::string published_source =
      config->output_mode == DRIVER_OUTPUT_MODE::OBJECT
          ? config->object_output_file
          : config->executable_output_file;
  if (!Publish_owned(config, published_source, config->final_output_file,
                     error)) {
    *error = Phase_failure(config->output_mode == DRIVER_OUTPUT_MODE::LINK
                               ? "link"
                               : "object",
                           *config, final_argv,
                           "status=publication-error " + *error);
    return 1;
  }
  Cleanup_reserved_intermediates(config);
  return 0;
}
