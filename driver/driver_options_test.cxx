#include "driver_options.h"

#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static int failures = 0;

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                         \
      std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: "          \
                << #condition << std::endl;                                     \
      ++failures;                                                               \
    }                                                                           \
  } while (false)

static bool Parse(std::vector<std::string> args, COMPILER_CONFIG *config,
                  std::vector<std::string> *filtered, std::string *error) {
  std::vector<char *> argv;
  for (std::string &arg : args) argv.push_back(&arg[0]);
  return Consume_external_driver_options(
      static_cast<int>(argv.size()), argv.data(), config, filtered, error);
}

static std::string Make_temp_dir() {
  std::string pattern = "/tmp/central-driver-options-XXXXXX";
  std::vector<char> storage(pattern.begin(), pattern.end());
  storage.push_back('\0');
  char *result = mkdtemp(storage.data());
  CHECK(result != nullptr);
  return result == nullptr ? "" : result;
}

static void Write_file(const std::string &path, const std::string &contents) {
  std::ofstream output(path);
  CHECK(static_cast<bool>(output));
  output << contents;
  CHECK(static_cast<bool>(output));
}

static bool Is_regular_file(const std::string &path) {
  struct stat info;
  return lstat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

static std::string Reserved_path(const DRIVER_RESERVED_FILE &file) {
  return file.path;
}

static void Test_family_policy_and_malformed_groups() {
  COMPILER_CONFIG config;
  std::vector<std::string> filtered;
  std::string error;
  CHECK(Parse({"compiler", "-clang", "--toolchain=required", "input.c"},
              &config, &filtered, &error));
  CHECK(config.toolchain_family == EXTERNAL_TOOLCHAIN_FAMILY::CLANG);
  CHECK(config.toolchain_mode == EXTERNAL_TOOLCHAIN_MODE::REQUIRED);
  CHECK((filtered == std::vector<std::string>{"compiler", "input.c"}));

  config = COMPILER_CONFIG();
  CHECK(Parse({"compiler", "--gcc", "--toolchain=auto", "input.c"},
              &config, &filtered, &error));
  CHECK(config.toolchain_family == EXTERNAL_TOOLCHAIN_FAMILY::GCC);
  CHECK(config.toolchain_mode == EXTERNAL_TOOLCHAIN_MODE::AUTO);

  config = COMPILER_CONFIG();
  CHECK(!Parse({"compiler", "-clang", "-gcc", "input.c"}, &config,
               &filtered, &error));
  CHECK(error.find("mutually exclusive") != std::string::npos);

  config = COMPILER_CONFIG();
  CHECK(!Parse({"compiler", "--toolchain=sometimes", "input.c"}, &config,
               &filtered, &error));
  CHECK(error.find("invalid --toolchain value") != std::string::npos);

  for (const char *bad : {"-Wa,", "-Ws,a,,b", "-Wl,,x"}) {
    config = COMPILER_CONFIG();
    CHECK(!Parse({"compiler", bad, "input.c"}, &config, &filtered, &error));
    CHECK(error.find("empty") != std::string::npos);
  }
}

static void Test_exact_link_order_and_source_placeholder() {
  COMPILER_CONFIG config;
  std::vector<std::string> filtered;
  std::string error;
  CHECK(Parse({"compiler", "libbefore.a", "-Wl,--before", "source.c",
               "-Wl,--after", "-L", "other/lib", "-l", "split",
               "libafter.a"},
              &config, &filtered, &error));
  CHECK((filtered == std::vector<std::string>{"compiler", "source.c"}));
  CHECK(config.link_items.size() == 9);
  if (config.link_items.size() != 9) return;
  CHECK(config.link_items[0].kind == DRIVER_LINK_ITEM_KIND::INPUT);
  CHECK(config.link_items[0].input == "libbefore.a");
  CHECK(config.link_items[1].kind == DRIVER_LINK_ITEM_KIND::LINKER_ARG_GROUP);
  CHECK((config.link_items[1].linker_args ==
         std::vector<std::string>{"--before"}));
  CHECK(config.link_items[2].kind == DRIVER_LINK_ITEM_KIND::SOURCE_OBJECT);
  CHECK(config.link_items[3].kind == DRIVER_LINK_ITEM_KIND::LINKER_ARG_GROUP);
  CHECK((config.link_items[3].linker_args ==
         std::vector<std::string>{"--after"}));
  CHECK(config.link_items[4].input == "-L");
  CHECK(config.link_items[5].input == "other/lib");
  CHECK(config.link_items[6].input == "-l");
  CHECK(config.link_items[7].input == "split");
  CHECK(config.link_items[8].input == "libafter.a");
}

static void Test_end_of_options_is_positional() {
  COMPILER_CONFIG config;
  std::vector<std::string> filtered;
  std::string error;
  CHECK(Parse({"compiler", "source.c", "--", "-Wl,--late", "-lfoo",
               "literal.a"},
              &config, &filtered, &error));
  CHECK((filtered == std::vector<std::string>{
                         "compiler", "source.c", "--", "-Wl,--late",
                         "-lfoo"}));
  CHECK(config.link_items.size() == 4);
  if (config.link_items.size() != 4) return;
  CHECK(config.link_items[0].kind == DRIVER_LINK_ITEM_KIND::SOURCE_OBJECT);
  CHECK(config.link_items[1].kind == DRIVER_LINK_ITEM_KIND::SOURCE_OBJECT);
  CHECK(config.link_items[2].kind == DRIVER_LINK_ITEM_KIND::SOURCE_OBJECT);
  CHECK(config.link_items[3].input == "literal.a");
}

static void Test_output_values_are_not_link_inputs() {
  COMPILER_CONFIG config;
  std::vector<std::string> filtered;
  std::string error;
  CHECK(Parse({"compiler", "-c", "source.c", "-o", "result.o"}, &config,
              &filtered, &error));
  CHECK((filtered == std::vector<std::string>{"compiler", "-c", "source.c",
                                              "-o", "result.o"}));
  CHECK(config.link_items.size() == 1);
  if (!config.link_items.empty())
    CHECK(config.link_items[0].kind == DRIVER_LINK_ITEM_KIND::SOURCE_OBJECT);

  config = COMPILER_CONFIG();
  CHECK(Parse({"compiler", "-S", "source.c", "-oresult.o"}, &config,
              &filtered, &error));
  CHECK((filtered == std::vector<std::string>{"compiler", "-S", "source.c",
                                              "-oresult.o"}));
  CHECK(config.link_items.size() == 1);
}

static void Test_source_output_identity_is_rejected() {
  const std::string dir = Make_temp_dir();
  const std::string source = dir + "/source.c";
  const std::string hardlink = dir + "/source-hardlink.c";
  Write_file(source, "source must survive\n");
  CHECK(link(source.c_str(), hardlink.c_str()) == 0);

  for (DRIVER_OUTPUT_MODE mode : {DRIVER_OUTPUT_MODE::ASSEMBLY,
                                  DRIVER_OUTPUT_MODE::OBJECT,
                                  DRIVER_OUTPUT_MODE::LINK}) {
    for (const std::string &output :
         {source, dir + "/./source.c", hardlink}) {
      COMPILER_CONFIG config;
      std::string error;
      CHECK(!Configure_driver_outputs(mode, source, output, false, &config,
                                      &error));
      CHECK(error.find("source") != std::string::npos);
      CHECK(config.reserved_intermediate_files.empty());
    }
  }

  unlink(hardlink.c_str());
  unlink(source.c_str());
  rmdir(dir.c_str());
}

static void Test_normal_output_plans() {
  const std::string dir = Make_temp_dir();
  const std::string source = dir + "/hello.c";
  Write_file(source, "source\n");
  std::string error;

  COMPILER_CONFIG assembly;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::ASSEMBLY, source, "",
                                 false, &assembly, &error));
  CHECK(assembly.final_output_file == "hello.s");
  CHECK(assembly.assembly_output_file == "hello.s");
  CHECK(assembly.output_file == assembly.assembly_output_file);
  CHECK(assembly.assembly && !assembly.object_gen);
  CHECK(assembly.reserved_intermediate_files.empty());

  COMPILER_CONFIG object;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::OBJECT, source,
                                 dir + "/result.o", false, &object, &error));
  CHECK(object.final_output_file == dir + "/result.o");
  CHECK(object.object_output_file != object.final_output_file);
  CHECK(object.retained_assembly_output_file.empty());
  CHECK(object.retained_object_output_file.empty());
  CHECK(object.assembly && object.object_gen);
  CHECK(object.assembly_output_file != object.final_output_file);
  CHECK(object.reserved_intermediate_files.size() == 2);
  CHECK(Is_regular_file(object.assembly_output_file));
  CHECK(Is_regular_file(object.object_output_file));
  Cleanup_reserved_intermediates(&object);

  COMPILER_CONFIG link;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::LINK, source,
                                 dir + "/program", false, &link, &error));
  CHECK(link.final_output_file == dir + "/program");
  CHECK(link.retained_assembly_output_file.empty());
  CHECK(link.retained_object_output_file.empty());
  CHECK(link.reserved_intermediate_files.size() == 2);
  CHECK(Is_regular_file(link.assembly_output_file));
  CHECK(Is_regular_file(link.object_output_file));
  Cleanup_reserved_intermediates(&link);

  COMPILER_CONFIG keep;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::LINK, source,
                                 dir + "/kept-program", true, &keep, &error));
  CHECK(keep.assembly_output_file != dir + "/hello.s");
  CHECK(keep.object_output_file != dir + "/hello.o");
  CHECK(keep.retained_assembly_output_file == dir + "/hello.s");
  CHECK(keep.retained_object_output_file == dir + "/hello.o");
  CHECK(keep.reserved_intermediate_files.size() == 2);
  CHECK(Is_regular_file(keep.assembly_output_file));
  CHECK(Is_regular_file(keep.object_output_file));
  Cleanup_reserved_intermediates(&keep);

  const std::string assembly_source = dir + "/collision.s";
  Write_file(assembly_source, "source assembly\n");
  COMPILER_CONFIG keep_source_collision;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::LINK, assembly_source,
                                 dir + "/other-program", true,
                                 &keep_source_collision, &error));
  CHECK(keep_source_collision.assembly_output_file != assembly_source);
  CHECK(keep_source_collision.retained_assembly_output_file.empty());
  CHECK(keep_source_collision.retained_object_output_file ==
        dir + "/collision.o");
  CHECK(keep_source_collision.reserved_intermediate_files.size() == 2);
  Cleanup_reserved_intermediates(&keep_source_collision);

  unlink(assembly_source.c_str());
  unlink(source.c_str());
  rmdir(dir.c_str());
}

static void Test_atomic_reservation_collision_and_cleanup() {
  const std::string dir = Make_temp_dir();
  const std::string source = dir + "/hello.c";
  const std::string victim = dir + "/victim";
  const std::string visible_assembly = dir + "/hello.s";
  Write_file(source, "source\n");
  Write_file(victim, "victim\n");
  CHECK(symlink(victim.c_str(), visible_assembly.c_str()) == 0);

  COMPILER_CONFIG config;
  std::string error;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::LINK, source,
                                 dir + "/hello.o", true, &config, &error));
  CHECK(config.output_mode == DRIVER_OUTPUT_MODE::LINK);
  CHECK(config.assembly_output_file != visible_assembly);
  CHECK(config.object_output_file != config.final_output_file);
  CHECK(config.reserved_intermediate_files.size() == 2);
  for (const DRIVER_RESERVED_FILE &file : config.reserved_intermediate_files)
    CHECK(Is_regular_file(Reserved_path(file)));

  std::ofstream reopen(config.assembly_output_file,
                       std::ios::out | std::ios::trunc);
  CHECK(static_cast<bool>(reopen));
  reopen << "assembly\n";
  reopen.close();
  CHECK(Is_regular_file(config.assembly_output_file));

  std::vector<std::string> owned;
  for (const DRIVER_RESERVED_FILE &file : config.reserved_intermediate_files)
    owned.push_back(Reserved_path(file));
  Cleanup_reserved_intermediates(&config);
  CHECK(config.reserved_intermediate_files.empty());
  for (const std::string &path : owned) CHECK(access(path.c_str(), F_OK) != 0);
  CHECK(access(visible_assembly.c_str(), F_OK) == 0);
  CHECK(access(victim.c_str(), F_OK) == 0);

  unlink(visible_assembly.c_str());
  unlink(victim.c_str());
  unlink(source.c_str());
  rmdir(dir.c_str());
}

static void Test_cleanup_refuses_replaced_owned_path() {
  const std::string dir = Make_temp_dir();
  const std::string source = dir + "/hello.c";
  const std::string victim = dir + "/victim";
  Write_file(source, "source\n");
  Write_file(victim, "victim\n");

  COMPILER_CONFIG config;
  std::string error;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::LINK, source,
                                 dir + "/program", false, &config, &error));
  CHECK(config.reserved_intermediate_files.size() == 2);
  if (config.reserved_intermediate_files.size() != 2) return;
  const std::string replaced =
      Reserved_path(config.reserved_intermediate_files[0]);
  const std::string untouched =
      Reserved_path(config.reserved_intermediate_files[1]);
  CHECK(unlink(replaced.c_str()) == 0);
  CHECK(symlink(victim.c_str(), replaced.c_str()) == 0);

  Cleanup_reserved_intermediates(&config);
  CHECK(config.reserved_intermediate_files.empty());
  struct stat replacement_info;
  CHECK(lstat(replaced.c_str(), &replacement_info) == 0 &&
        S_ISLNK(replacement_info.st_mode));
  CHECK(access(victim.c_str(), F_OK) == 0);
  CHECK(access(untouched.c_str(), F_OK) != 0);

  unlink(replaced.c_str());
  unlink(victim.c_str());
  unlink(source.c_str());
  rmdir(dir.c_str());
}

static void Test_legacy_fe_only_preprocess_state_reserves_nothing() {
  COMPILER_CONFIG config;
  std::string error;
  config.toolchain_mode = EXTERNAL_TOOLCHAIN_MODE::OFF;
  CHECK(Configure_driver_outputs(DRIVER_OUTPUT_MODE::PREPROCESS, "source.c",
                                 "ignored.out", false, &config, &error));
  CHECK(config.output_mode == DRIVER_OUTPUT_MODE::PREPROCESS);
  CHECK(!config.assembly && !config.object_gen);
  CHECK(config.output_file.empty());
  CHECK(config.assembly_output_file.empty());
  CHECK(config.object_output_file.empty());
  CHECK(config.final_output_file.empty());
  CHECK(config.reserved_intermediate_files.empty());
}

int main() {
  Test_family_policy_and_malformed_groups();
  Test_exact_link_order_and_source_placeholder();
  Test_end_of_options_is_positional();
  Test_output_values_are_not_link_inputs();
  Test_source_output_identity_is_rejected();
  Test_normal_output_plans();
  Test_atomic_reservation_collision_and_cleanup();
  Test_cleanup_refuses_replaced_owned_path();
  Test_legacy_fe_only_preprocess_state_reserves_nothing();
  if (failures != 0)
    std::cerr << failures << " driver option checks failed" << std::endl;
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
