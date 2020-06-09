#include "file_util.h"
#include <vector>
#include <set>
#include <string>

using std::vector;
using std::string;
using std::set;

/**
 * Validate if a file exists
 * */
BOOL File_exists(const std::string &filename) {
  std::ifstream ifile(filename.c_str());
  return (BOOL) ifile;
}

/**
 * Validate if a file exists
 * */
BOOL File_exists(const char *filename) {
  std::ifstream ifile(filename);
  return (BOOL) ifile;
}

/**
 *  Splitting the path to separate components by delimiters 
 **/
vector<string> File_split_path(const string &str, const set<char> delimiters) {
  vector<string> result;

  char const *pch = str.c_str();
  char const *start = pch;
  for (; *pch; ++pch) {
    if (delimiters.find(*pch) != delimiters.end()) {
      if (start != pch) {
        string str(start, pch);
        result.push_back(str);
      } else {
        result.push_back("");
      }
      start = pch + 1;
    }
  }
  result.push_back(start);

  return result;
}

/**
 *  Replacing the file extension for 's', with 'new_extension'
 * */
void File_change_extension(std::string &s, std::string new_extension) {

  std::set<char> delims({'\\', '/'});
  std::vector<std::string> last_component = File_split_path(s, delims);
  if (last_component.size() > 0) {
    string::size_type i = s.rfind('.', s.length());
    std::string last_one = last_component.at(last_component.size() - 1);
    unsigned long last_comp_i = last_one.rfind(".", s.length());
    if (last_comp_i != string::npos && i != string::npos && last_comp_i != 0) {
      //ok
      s = s.substr(0, i) + "." + new_extension;
      return;
    }
  }
  s = s + "." + new_extension;
}