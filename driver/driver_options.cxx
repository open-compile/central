#include "driver_options.h"

#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>

namespace {

bool Has_suffix(const std::string &value, const std::string &suffix) {
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
             0;
}

bool Is_link_file(const std::string &arg) {
  return Has_suffix(arg, ".o") || Has_suffix(arg, ".a") ||
         Has_suffix(arg, ".so") || Has_suffix(arg, ".dylib");
}

struct FILTERED_OPTION_METADATA {
  const char *name;
  bool takes_separate_value;
};

const FILTERED_OPTION_METADATA kFiltered_options[] = {
    {"-o", true},             {"--output", true},
    {"-x", true},             {"--language", true},
    {"--log", true},          {"--loglevel", true},
    {"--felevel", true},       {"--feloglevel", true},
    {"--belevel", true},       {"--beloglevel", true},
    {"--cglevel", true},       {"--cgloglevel", true},
    {"--graloglevel", true},   {"--linkerlevel", true},
    {"--linkerloglevel", true}, {"--symtablevel", true},
    {"--symtabloglevel", true}, {"-O", true},
    {"-m", true},             {"--width", true},
    {"--arch", true},         {"--march", true},
    {"--target", true},       {"-I", true},
    {"--include", true},      {"-i", true},
    {"--include-sys", true},  {"--dump-ir-after", true},
    {"--load-ir", true},
};

bool Option_takes_separate_value(const std::string &arg) {
  for (const auto &metadata : kFiltered_options) {
    if (arg == metadata.name) return metadata.takes_separate_value;
  }
  return false;
}

bool Split_group(const std::string &arg, size_t prefix_size,
                 std::vector<std::string> *group, std::string *error) {
  const std::string suffix = arg.substr(prefix_size);
  if (suffix.empty()) {
    *error = "empty pass-through argument group in '" + arg + "'";
    return false;
  }

  size_t start = 0;
  while (start <= suffix.size()) {
    const size_t comma = suffix.find(',', start);
    const std::string item = suffix.substr(
        start, comma == std::string::npos ? std::string::npos : comma - start);
    if (item.empty()) {
      *error = "empty pass-through argument in '" + arg + "'";
      return false;
    }
    group->push_back(item);
    if (comma == std::string::npos) break;
    start = comma + 1;
  }
  return true;
}

void Add_link_input(const std::string &input, COMPILER_CONFIG *config) {
  DRIVER_LINK_ITEM item;
  item.kind = DRIVER_LINK_ITEM_KIND::INPUT;
  item.input = input;
  config->link_items.push_back(item);
}

void Add_linker_group(const std::vector<std::string> &args,
                      COMPILER_CONFIG *config) {
  DRIVER_LINK_ITEM item;
  item.kind = DRIVER_LINK_ITEM_KIND::LINKER_ARG_GROUP;
  item.linker_args = args;
  config->link_items.push_back(item);
}

void Add_source_object(COMPILER_CONFIG *config) {
  DRIVER_LINK_ITEM item;
  item.kind = DRIVER_LINK_ITEM_KIND::SOURCE_OBJECT;
  config->link_items.push_back(item);
}

bool Set_family(EXTERNAL_TOOLCHAIN_FAMILY family, COMPILER_CONFIG *config,
                std::string *error) {
  if (config->toolchain_family != EXTERNAL_TOOLCHAIN_FAMILY::AUTO &&
      config->toolchain_family != family) {
    *error = "-clang and -gcc are mutually exclusive";
    return false;
  }
  config->toolchain_family = family;
  return true;
}

std::string Basename_without_extension(const std::string &path) {
  const size_t slash = path.find_last_of("/\\");
  std::string name = path.substr(slash == std::string::npos ? 0 : slash + 1);
  const size_t dot = name.find_last_of('.');
  if (dot != std::string::npos && dot != 0) name.erase(dot);
  return name;
}

std::string Directory(const std::string &path) {
  const size_t slash = path.find_last_of("/\\");
  return slash == std::string::npos ? "" : path.substr(0, slash + 1);
}

std::string Absolute_lexical_path(const std::string &path) {
  std::string absolute = path;
  if (absolute.empty() || absolute[0] != '/') {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) return path;
    absolute = std::string(cwd) + "/" + absolute;
  }

  std::vector<std::string> components;
  size_t start = 0;
  while (start <= absolute.size()) {
    const size_t slash = absolute.find('/', start);
    const std::string component = absolute.substr(
        start, slash == std::string::npos ? std::string::npos : slash - start);
    if (component.empty() || component == ".") {
    } else if (component == "..") {
      if (!components.empty()) components.pop_back();
    } else {
      components.push_back(component);
    }
    if (slash == std::string::npos) break;
    start = slash + 1;
  }

  std::string result = "/";
  for (size_t i = 0; i < components.size(); ++i) {
    if (i != 0) result += '/';
    result += components[i];
  }
  return result;
}

bool Paths_equivalent(const std::string &left, const std::string &right) {
  if (Absolute_lexical_path(left) == Absolute_lexical_path(right)) return true;

  struct stat left_info;
  struct stat right_info;
  if (stat(left.c_str(), &left_info) == 0 &&
      stat(right.c_str(), &right_info) == 0) {
    return left_info.st_dev == right_info.st_dev &&
           left_info.st_ino == right_info.st_ino;
  }
  return false;
}

bool Path_conflicts(const std::string &path,
                    const std::vector<std::string> &occupied) {
  for (const std::string &other : occupied) {
    if (Paths_equivalent(path, other)) return true;
  }
  return false;
}

bool Reserve_intermediate(
    const std::string &directory, const std::string &source_base,
    const std::string &extension, bool keep,
    const std::vector<std::string> &occupied, COMPILER_CONFIG *config,
    std::string *reserved, std::string *error) {
  const std::string visible = directory + source_base + extension;
  if (keep && !Path_conflicts(visible, occupied)) {
    struct stat visible_info;
    if (lstat(visible.c_str(), &visible_info) != 0 ||
        S_ISREG(visible_info.st_mode)) {
      *reserved = visible;
      return true;
    }
  }

  std::string pattern = directory + "." + source_base + ".central-XXXXXX" +
                        extension;
  std::vector<char> storage(pattern.begin(), pattern.end());
  storage.push_back('\0');
  const int fd = mkstemps(storage.data(), static_cast<int>(extension.size()));
  if (fd < 0) {
    *error = "cannot reserve intermediate file '" + pattern + "': " +
             std::strerror(errno);
    return false;
  }
  if (close(fd) != 0) {
    const int saved_errno = errno;
    unlink(storage.data());
    *error = "cannot close reserved intermediate file '" +
             std::string(storage.data()) + "': " + std::strerror(saved_errno);
    return false;
  }
  *reserved = storage.data();
  config->reserved_intermediate_files.push_back(*reserved);
  return true;
}

}  // namespace

bool Consume_external_driver_options(int argc, char **argv,
                                     COMPILER_CONFIG *config,
                                     std::vector<std::string> *filtered_args,
                                     std::string *error) {
  filtered_args->clear();
  error->clear();
  if (argc > 0) filtered_args->push_back(argv[0]);

  bool end_of_options = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if (end_of_options) {
      if (Is_link_file(arg)) {
        Add_link_input(arg, config);
      } else {
        Add_source_object(config);
        filtered_args->push_back(arg);
      }
      continue;
    }
    if (arg == "--") {
      end_of_options = true;
      filtered_args->push_back(arg);
      continue;
    }
    if (arg == "-clang" || arg == "--clang") {
      if (!Set_family(EXTERNAL_TOOLCHAIN_FAMILY::CLANG, config, error))
        return false;
      continue;
    }
    if (arg == "-gcc" || arg == "--gcc") {
      if (!Set_family(EXTERNAL_TOOLCHAIN_FAMILY::GCC, config, error))
        return false;
      continue;
    }
    if (arg.compare(0, 12, "--toolchain=") == 0) {
      const std::string value = arg.substr(12);
      if (value == "auto") {
        config->toolchain_mode = EXTERNAL_TOOLCHAIN_MODE::AUTO;
      } else if (value == "required") {
        config->toolchain_mode = EXTERNAL_TOOLCHAIN_MODE::REQUIRED;
      } else if (value == "off") {
        config->toolchain_mode = EXTERNAL_TOOLCHAIN_MODE::OFF;
      } else {
        *error = "invalid --toolchain value '" + value +
                 "'; use auto, required, or off";
        return false;
      }
      continue;
    }
    if (arg.compare(0, 4, "-Wa,") == 0 ||
        arg.compare(0, 4, "-Ws,") == 0) {
      std::vector<std::string> group;
      if (!Split_group(arg, 4, &group, error)) return false;
      config->assembler_arg_groups.push_back(group);
      continue;
    }
    if (arg.compare(0, 4, "-Wl,") == 0) {
      std::vector<std::string> group;
      if (!Split_group(arg, 4, &group, error)) return false;
      Add_linker_group(group, config);
      continue;
    }
    if (Option_takes_separate_value(arg)) {
      filtered_args->push_back(arg);
      if (i + 1 < argc) filtered_args->push_back(argv[++i]);
      continue;
    }
    if (arg == "-L" || arg == "-l") {
      if (i + 1 >= argc || std::string(argv[i + 1]).empty()) {
        *error = arg + " expects a non-empty value";
        return false;
      }
      Add_link_input(arg, config);
      Add_link_input(argv[++i], config);
      continue;
    }
    if ((arg.compare(0, 2, "-L") == 0 ||
         arg.compare(0, 2, "-l") == 0) &&
        arg.size() > 2) {
      Add_link_input(arg, config);
      continue;
    }
    if (!arg.empty() && arg[0] != '-' && Is_link_file(arg)) {
      Add_link_input(arg, config);
      continue;
    }
    if (!arg.empty() && arg[0] != '-') Add_source_object(config);
    filtered_args->push_back(arg);
  }
  return true;
}

bool Configure_driver_outputs(DRIVER_OUTPUT_MODE mode,
                              const std::string &source_file,
                              const std::string &requested_output,
                              bool keep_intermediates,
                              COMPILER_CONFIG *config, std::string *error) {
  error->clear();
  Cleanup_reserved_intermediates(config);
  if (config->toolchain_mode == EXTERNAL_TOOLCHAIN_MODE::OFF &&
      mode != DRIVER_OUTPUT_MODE::PREPROCESS &&
      mode != DRIVER_OUTPUT_MODE::ASSEMBLY) {
    *error = "--toolchain=off is only valid with -E or -S";
    return false;
  }

  config->output_mode = mode;
  config->keep_intermediates = keep_intermediates ? TRUE : FALSE;
  config->assembly = FALSE;
  config->object_gen = FALSE;
  config->output_file.clear();
  config->assembly_output_file.clear();
  config->object_output_file.clear();
  config->final_output_file.clear();
  if (mode == DRIVER_OUTPUT_MODE::PREPROCESS) return true;

  const std::string source_base = Basename_without_extension(source_file);
  if (source_base.empty()) {
    *error = "cannot derive output path from source file '" + source_file +
             "'";
    return false;
  }

  config->assembly = TRUE;
  config->object_gen = mode == DRIVER_OUTPUT_MODE::OBJECT ? TRUE : FALSE;

  const std::string final_output = requested_output.empty()
                                       ? (mode == DRIVER_OUTPUT_MODE::ASSEMBLY
                                              ? source_base + ".s"
                                              : mode == DRIVER_OUTPUT_MODE::OBJECT
                                                    ? source_base + ".o"
                                                    : "a.out")
                                       : requested_output;
  if (Paths_equivalent(source_file, final_output)) {
    *error = "output file '" + final_output +
             "' is the same as source file '" + source_file + "'";
    return false;
  }

  if (mode == DRIVER_OUTPUT_MODE::ASSEMBLY) {
    config->assembly_output_file = final_output;
    config->object_output_file.clear();
    config->final_output_file = config->assembly_output_file;
  } else {
    config->final_output_file = final_output;
    const std::string directory = Directory(config->final_output_file);
    if (!Reserve_intermediate(directory, source_base, ".s", keep_intermediates,
                              {source_file, config->final_output_file}, config,
                              &config->assembly_output_file, error)) {
      Cleanup_reserved_intermediates(config);
      return false;
    }
    if (mode == DRIVER_OUTPUT_MODE::OBJECT) {
      config->object_output_file = config->final_output_file;
    } else {
      if (!Reserve_intermediate(
              directory, source_base, ".o", keep_intermediates,
              {source_file, config->final_output_file,
               config->assembly_output_file},
              config, &config->object_output_file, error)) {
        Cleanup_reserved_intermediates(config);
        return false;
      }
    }
  }
  config->output_file = config->assembly_output_file;
  return true;
}

void Cleanup_reserved_intermediates(COMPILER_CONFIG *config) {
  for (const std::string &path : config->reserved_intermediate_files) {
    unlink(path.c_str());
  }
  config->reserved_intermediate_files.clear();
}
