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
                              bool keep_intermediates, long process_id,
                              COMPILER_CONFIG *config, std::string *error);

#endif  // OCC_DRIVER_OPTIONS_H
