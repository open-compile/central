#include "basic.h"
#include "host.h"
#include "options.h"
#include "be_export.h"
#include "file_util.h"
#include <fstream>

using std::ofstream;

INT32 BE_MAIN_NAME(INT32 argc, char **argv) {
    AssertThat(argc > 1, ("not enough arguments"));
    return 0;
}

INT32 BE_EXTERNAL_MAIN_NAME(COMPILER_CONFIG &conf) {
    AssertThat(conf.opt_level >= 0, ("not enough compile level"));
    Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS), 
             (TFile, "Writing assembly to %s\n", conf.output_file.c_str()));
    AssertThat(conf.output_file.size() > 2, ("Incorrect output file name"));
    if (File_exists(conf.output_file)) {
        // delete the file if it exists
        Is_Trace(Tracing(COMPONENT_BE, TRACE_OPTIONS), 
                 (TFile, "Removing old output file under %s\n", conf.output_file.c_str()));
        if(remove(conf.output_file.c_str()) != 0) {
            Comp_Failure("Cannot delete file : %s", conf.output_file.c_str());
        }
        return 0;
    }
    
    ofstream myfile;
    myfile.open (conf.output_file.c_str());
    myfile << ".section text\n";
    myfile << ".byte 0x0\n";
    myfile.close();
    return 0;
}