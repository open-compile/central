#include <fstream>
#include <set>
#include <vector>
#include <string>
#include "host.h"

//Validate if a file exists
extern BOOL File_exists(const std::string& filename);

//Validate if a file exists
extern BOOL File_exists(const char *filename);

// Splitting file paths by delimiters, such as ';', '/'...
std::vector<std::string> File_split_path(const std::string& str, const std::set<char> delimiters); 

//Replace file extension in 's' with 'new_extension'
void File_change_extension(std::string &s, std::string new_extension);