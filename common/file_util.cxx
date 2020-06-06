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
BOOL File_exists(const std::string& filename) {
  std::ifstream ifile(filename.c_str());
  return (BOOL)ifile;
}

/**
 * Validate if a file exists
 * */
BOOL File_exists(const char *filename) {
  std::ifstream ifile(filename);
  return (BOOL)ifile;
}

/**
 *  Splitting the path to separate components by delimiters 
 **/
vector<string> File_split_path(const string& str, const set<char> delimiters)
{
  vector<string> result;

  char const* pch = str.c_str();
  char const* start = pch;
  for(; *pch; ++pch)
  {
    if (delimiters.find(*pch) != delimiters.end())
    {
      if (start != pch)
      {
        string str(start, pch);
        result.push_back(str);
      }
      else
      {
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
   string::size_type i = s.rfind('.', s.length());
   if (i != string::npos) {
      s.replace(i+1, new_extension.length(), new_extension);
   }
}