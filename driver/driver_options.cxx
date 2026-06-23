#include "driver_options.h"

#include <sys/stat.h>
#include <fcntl.h>
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

bool Reserve_work_directory(const std::string &directory,
                            const std::string &source_base,
                            COMPILER_CONFIG *config, std::string *error) {
  std::string pattern = directory + "." + source_base + ".central-XXXXXX";
  std::vector<char> storage(pattern.begin(), pattern.end());
  storage.push_back('\0');
  if (mkdtemp(storage.data()) == nullptr) {
    *error = "cannot reserve intermediate directory '" + pattern + "': " +
             std::strerror(errno);
    return false;
  }
  if (chmod(storage.data(), 0700) != 0) {
    const int saved_errno = errno;
    rmdir(storage.data());
    *error = "cannot protect intermediate directory '" +
             std::string(storage.data()) + "': " + std::strerror(saved_errno);
    return false;
  }
  struct stat reserved_info;
  if (lstat(storage.data(), &reserved_info) != 0 ||
      !S_ISDIR(reserved_info.st_mode)) {
    const int saved_errno = errno == 0 ? EINVAL : errno;
    rmdir(storage.data());
    *error = "cannot identify intermediate directory '" +
             std::string(storage.data()) + "': " + std::strerror(saved_errno);
    return false;
  }
  config->working_directory.path = storage.data();
  config->working_directory.device =
      static_cast<std::uint64_t>(reserved_info.st_dev);
  config->working_directory.inode =
      static_cast<std::uint64_t>(reserved_info.st_ino);
  return true;
}

std::string Retained_path(const std::string &directory,
                          const std::string &source_base,
                          const std::string &extension,
                          const std::vector<std::string> &occupied) {
  const std::string path = directory + source_base + extension;
  return Path_conflicts(path, occupied) ? "" : path;
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
  config->executable_output_file.clear();
  config->final_output_file.clear();
  config->retained_assembly_output_file.clear();
  config->retained_object_output_file.clear();
  config->working_directory = DRIVER_WORK_DIRECTORY();
  // PREPROCESS is the legacy FE-only/no-output path. Run_preprocess is still a
  // stub; implementing a real preprocessor is outside external-toolchain work.
  if (mode == DRIVER_OUTPUT_MODE::PREPROCESS) return true;

  const std::string source_base = Basename_without_extension(source_file);
  if (source_base.empty()) {
    *error = "cannot derive output path from source file '" + source_file +
             "'";
    return false;
  }

  config->assembly = TRUE;
  config->object_gen =
      (mode == DRIVER_OUTPUT_MODE::OBJECT || mode == DRIVER_OUTPUT_MODE::LINK)
          ? TRUE
          : FALSE;

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
    if (!Reserve_work_directory(directory, source_base, config, error)) {
      Cleanup_reserved_intermediates(config);
      return false;
    }
    config->assembly_output_file = config->working_directory.path + "/assembly.s";
    config->object_output_file = config->working_directory.path + "/object.o";
    if (mode == DRIVER_OUTPUT_MODE::LINK)
      config->executable_output_file =
          config->working_directory.path + "/executable";
    if (keep_intermediates) {
      config->retained_assembly_output_file = Retained_path(
          directory, source_base, ".s",
          {source_file, config->final_output_file});
      if (mode == DRIVER_OUTPUT_MODE::LINK) {
        config->retained_object_output_file = Retained_path(
            directory, source_base, ".o",
            {source_file, config->final_output_file,
             config->retained_assembly_output_file});
      }
    }
  }
  config->output_file = config->assembly_output_file;
  return true;
}

void Cleanup_reserved_intermediates(COMPILER_CONFIG *config) {
  const DRIVER_WORK_DIRECTORY directory = config->working_directory;
  const int directory_fd = directory.path.empty()
                               ? -1
                               : open(directory.path.c_str(),
                                      O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
  struct stat directory_info;
  const bool directory_owned =
      directory_fd >= 0 && fstat(directory_fd, &directory_info) == 0 &&
      S_ISDIR(directory_info.st_mode) &&
      (directory_info.st_mode & 0777) == 0700 &&
      static_cast<std::uint64_t>(directory_info.st_dev) == directory.device &&
      static_cast<std::uint64_t>(directory_info.st_ino) == directory.inode;
  for (const DRIVER_RESERVED_FILE &owned :
       config->reserved_intermediate_files) {
    if (!directory_owned || Directory(owned.path) != directory.path + "/")
      continue;
    struct stat current;
    const std::string name = owned.path.substr(directory.path.size() + 1);
    if (name.empty() || name.find('/') != std::string::npos ||
        fstatat(directory_fd, name.c_str(), &current, AT_SYMLINK_NOFOLLOW) != 0 ||
        !S_ISREG(current.st_mode) ||
        static_cast<std::uint64_t>(current.st_dev) != owned.device ||
        static_cast<std::uint64_t>(current.st_ino) != owned.inode) {
      continue;
    }
    unlinkat(directory_fd, name.c_str(), 0);
  }
  if (directory_fd >= 0) close(directory_fd);
  struct stat current_directory;
  if (directory_owned && lstat(directory.path.c_str(), &current_directory) == 0 &&
      S_ISDIR(current_directory.st_mode) &&
      static_cast<std::uint64_t>(current_directory.st_dev) == directory.device &&
      static_cast<std::uint64_t>(current_directory.st_ino) == directory.inode)
    rmdir(directory.path.c_str());
  config->reserved_intermediate_files.clear();
  config->working_directory = DRIVER_WORK_DIRECTORY();
}
