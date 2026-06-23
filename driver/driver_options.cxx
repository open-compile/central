#include "driver_options.h"

#include <sstream>

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

bool Option_takes_separate_value(const std::string &arg) {
  return arg == "-o" || arg == "--output" || arg == "-x" ||
         arg == "--language" || arg == "--log" || arg == "--loglevel" ||
         arg == "--felevel" || arg == "--feloglevel" ||
         arg == "--belevel" || arg == "--beloglevel" ||
         arg == "--cglevel" || arg == "--cgloglevel" ||
         arg == "--graloglevel" || arg == "--linkerlevel" ||
         arg == "--linkerloglevel" || arg == "--symtablevel" ||
         arg == "--symtabloglevel" || arg == "-O" || arg == "-m" ||
         arg == "--width" || arg == "--arch" || arg == "--march" ||
         arg == "--target" || arg == "-I" || arg == "--include" ||
         arg == "-i" || arg == "--include-sys" ||
         arg == "--dump-ir-after" || arg == "--load-ir";
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

std::string Intermediate_path(const std::string &directory,
                              const std::string &source_base,
                              const std::string &extension, bool keep,
                              long process_id) {
  if (keep) return directory + source_base + extension;
  std::ostringstream path;
  path << directory << "." << source_base << ".central-" << process_id
       << extension;
  return path.str();
}

}  // namespace

bool Consume_external_driver_options(int argc, char **argv,
                                     COMPILER_CONFIG *config,
                                     std::vector<std::string> *filtered_args,
                                     std::string *error) {
  filtered_args->clear();
  error->clear();
  if (argc > 0) filtered_args->push_back(argv[0]);

  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
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
    if (arg == "-L") {
      if (i + 1 >= argc || std::string(argv[i + 1]).empty()) {
        *error = "-L expects a non-empty path";
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
    filtered_args->push_back(arg);
  }
  return true;
}

bool Configure_driver_outputs(DRIVER_OUTPUT_MODE mode,
                              const std::string &source_file,
                              const std::string &requested_output,
                              bool keep_intermediates, long process_id,
                              COMPILER_CONFIG *config, std::string *error) {
  error->clear();
  if (config->toolchain_mode == EXTERNAL_TOOLCHAIN_MODE::OFF &&
      mode != DRIVER_OUTPUT_MODE::ASSEMBLY) {
    *error = "--toolchain=off is only valid with -S";
    return false;
  }

  const std::string source_base = Basename_without_extension(source_file);
  if (source_base.empty()) {
    *error = "cannot derive output path from source file '" + source_file +
             "'";
    return false;
  }

  config->output_mode = mode;
  config->keep_intermediates = keep_intermediates ? TRUE : FALSE;
  config->assembly = TRUE;
  config->object_gen = mode == DRIVER_OUTPUT_MODE::OBJECT ? TRUE : FALSE;

  if (mode == DRIVER_OUTPUT_MODE::ASSEMBLY) {
    config->assembly_output_file =
        requested_output.empty() ? source_base + ".s" : requested_output;
    config->object_output_file.clear();
    config->final_output_file = config->assembly_output_file;
  } else {
    config->final_output_file = requested_output.empty()
                                    ? (mode == DRIVER_OUTPUT_MODE::OBJECT
                                           ? source_base + ".o"
                                           : "a.out")
                                    : requested_output;
    const std::string directory = Directory(config->final_output_file);
    config->assembly_output_file = Intermediate_path(
        directory, source_base, ".s", keep_intermediates, process_id);
    if (mode == DRIVER_OUTPUT_MODE::OBJECT) {
      config->object_output_file = config->final_output_file;
    } else {
      config->object_output_file = Intermediate_path(
          directory, source_base, ".o", keep_intermediates, process_id);
    }
  }
  config->output_file = config->assembly_output_file;
  return true;
}
