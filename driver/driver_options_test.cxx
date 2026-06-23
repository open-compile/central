#include "driver_options.h"

#include <assert.h>

#include <string>
#include <vector>

static bool Parse(std::vector<std::string> args, COMPILER_CONFIG *config,
                  std::vector<std::string> *filtered, std::string *error) {
  std::vector<char *> argv;
  for (std::string &arg : args) argv.push_back(&arg[0]);
  return Consume_external_driver_options(
      static_cast<int>(argv.size()), argv.data(), config, filtered, error);
}

static void Test_family_and_policy_options() {
  COMPILER_CONFIG config;
  std::vector<std::string> filtered;
  std::string error;
  assert(Parse({"compiler", "-clang", "--toolchain=required", "input.c"},
               &config, &filtered, &error));
  assert(config.toolchain_family == EXTERNAL_TOOLCHAIN_FAMILY::CLANG);
  assert(config.toolchain_mode == EXTERNAL_TOOLCHAIN_MODE::REQUIRED);
  assert((filtered == std::vector<std::string>{"compiler", "input.c"}));

  config = COMPILER_CONFIG();
  assert(Parse({"compiler", "--gcc", "--toolchain=auto", "input.c"},
               &config, &filtered, &error));
  assert(config.toolchain_family == EXTERNAL_TOOLCHAIN_FAMILY::GCC);
  assert(config.toolchain_mode == EXTERNAL_TOOLCHAIN_MODE::AUTO);

  config = COMPILER_CONFIG();
  assert(!Parse({"compiler", "-clang", "-gcc", "input.c"}, &config,
                &filtered, &error));
  assert(error.find("mutually exclusive") != std::string::npos);

  config = COMPILER_CONFIG();
  assert(!Parse({"compiler", "--toolchain=sometimes", "input.c"}, &config,
                &filtered, &error));
  assert(error.find("invalid --toolchain value") != std::string::npos);
}

static void Test_pass_through_groups_and_link_order() {
  COMPILER_CONFIG config;
  std::vector<std::string> filtered;
  std::string error;
  assert(Parse({"compiler", "input.c", "-Wa,-g,-Iinc", "-Ws,--fatal-warnings",
                "first.o", "-Wl,-Map,out.map", "-L/sdk/lib", "-L",
                "other/lib", "-lfoo", "libbar.a", "libshared.so",
                "libnative.dylib", "-Wl,--gc-sections"},
               &config, &filtered, &error));
  assert((config.assembler_arg_groups ==
          std::vector<std::vector<std::string>>{{"-g", "-Iinc"},
                                                {"--fatal-warnings"}}));
  assert(filtered == std::vector<std::string>({"compiler", "input.c"}));
  assert(config.link_items.size() == 10);
  assert(config.link_items[0].kind == DRIVER_LINK_ITEM_KIND::INPUT);
  assert(config.link_items[0].input == "first.o");
  assert(config.link_items[1].kind == DRIVER_LINK_ITEM_KIND::LINKER_ARG_GROUP);
  assert((config.link_items[1].linker_args ==
          std::vector<std::string>{"-Map", "out.map"}));
  assert(config.link_items[2].input == "-L/sdk/lib");
  assert(config.link_items[3].input == "-L");
  assert(config.link_items[4].input == "other/lib");
  assert(config.link_items[5].input == "-lfoo");
  assert(config.link_items[6].input == "libbar.a");
  assert(config.link_items[7].input == "libshared.so");
  assert(config.link_items[8].input == "libnative.dylib");
  assert((config.link_items[9].linker_args ==
          std::vector<std::string>{"--gc-sections"}));
}

static void Test_malformed_pass_through_groups() {
  for (const char *bad : {"-Wa,", "-Ws,a,,b", "-Wl,,x"}) {
    COMPILER_CONFIG config;
    std::vector<std::string> filtered;
    std::string error;
    assert(!Parse({"compiler", bad, "input.c"}, &config, &filtered, &error));
    assert(error.find("empty") != std::string::npos);
  }
}

static void Test_output_option_values_are_not_link_inputs() {
  COMPILER_CONFIG config;
  std::vector<std::string> filtered;
  std::string error;
  assert(Parse({"compiler", "-c", "source.c", "-o", "result.o"}, &config,
               &filtered, &error));
  assert((filtered == std::vector<std::string>{"compiler", "-c", "source.c",
                                               "-o", "result.o"}));
  assert(config.link_items.empty());

  config = COMPILER_CONFIG();
  assert(Parse({"compiler", "-S", "source.c", "-oresult.o"}, &config,
               &filtered, &error));
  assert((filtered == std::vector<std::string>{"compiler", "-S", "source.c",
                                               "-oresult.o"}));
  assert(config.link_items.empty());
}

static void Test_output_modes_and_paths() {
  COMPILER_CONFIG config;
  std::string error;
  assert(Configure_driver_outputs(DRIVER_OUTPUT_MODE::ASSEMBLY, "src/hello.c",
                                  "", false, 42, &config, &error));
  assert(config.assembly && !config.object_gen);
  assert(config.assembly_output_file == "hello.s");
  assert(config.final_output_file == "hello.s");
  assert(config.output_file == "hello.s");

  config = COMPILER_CONFIG();
  assert(Configure_driver_outputs(DRIVER_OUTPUT_MODE::OBJECT, "src/hello.c",
                                  "", false, 42, &config, &error));
  assert(config.object_output_file == "hello.o");
  assert(config.final_output_file == "hello.o");

  config = COMPILER_CONFIG();
  assert(Configure_driver_outputs(DRIVER_OUTPUT_MODE::OBJECT, "src/hello.c",
                                  "build/result.o", false, 42, &config,
                                  &error));
  assert(config.assembly && config.object_gen);
  assert(config.assembly_output_file == "build/.hello.central-42.s");
  assert(config.object_output_file == "build/result.o");
  assert(config.final_output_file == "build/result.o");
  assert(config.output_file == config.assembly_output_file);

  config = COMPILER_CONFIG();
  assert(Configure_driver_outputs(DRIVER_OUTPUT_MODE::LINK, "src/hello.c", "",
                                  false, 42, &config, &error));
  assert(config.assembly && !config.object_gen);
  assert(config.assembly_output_file == ".hello.central-42.s");
  assert(config.object_output_file == ".hello.central-42.o");
  assert(config.final_output_file == "a.out");

  config = COMPILER_CONFIG();
  assert(Configure_driver_outputs(DRIVER_OUTPUT_MODE::LINK, "src/hello.c",
                                  "dist/program", true, 42, &config, &error));
  assert(config.keep_intermediates);
  assert(config.assembly_output_file == "dist/hello.s");
  assert(config.object_output_file == "dist/hello.o");
  assert(config.final_output_file == "dist/program");

  config = COMPILER_CONFIG();
  config.toolchain_mode = EXTERNAL_TOOLCHAIN_MODE::OFF;
  assert(!Configure_driver_outputs(DRIVER_OUTPUT_MODE::OBJECT, "hello.c", "",
                                   false, 42, &config, &error));
  assert(error.find("--toolchain=off") != std::string::npos);
}

int main() {
  Test_family_and_policy_options();
  Test_pass_through_groups_and_link_order();
  Test_malformed_pass_through_groups();
  Test_output_option_values_are_not_link_inputs();
  Test_output_modes_and_paths();
  return 0;
}
