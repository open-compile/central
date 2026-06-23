#ifndef OCC_DRIVER_OPTIONS_H
#define OCC_DRIVER_OPTIONS_H

#include "options.h"

#include <string>
#include <vector>

bool Consume_external_driver_options(int argc, char **argv,
                                     COMPILER_CONFIG *config,
                                     std::vector<std::string> *filtered_args,
                                     std::string *error);

bool Configure_driver_outputs(DRIVER_OUTPUT_MODE mode,
                              const std::string &source_file,
                              const std::string &requested_output,
                              bool keep_intermediates,
                              COMPILER_CONFIG *config, std::string *error);

void Cleanup_reserved_intermediates(COMPILER_CONFIG *config);

#endif  // OCC_DRIVER_OPTIONS_H
