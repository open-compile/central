#include <cstdlib>
#include <fstream>
#include <string>

namespace {

std::string Basename(const std::string &path) {
  const size_t slash = path.find_last_of("/\\");
  return path.substr(slash == std::string::npos ? 0 : slash + 1);
}

}  // namespace

int main(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--version") return 0;
  }

  const char *log_path = std::getenv("CENTRAL_FAKE_TOOL_LOG");
  if (log_path == nullptr || *log_path == '\0') return 2;

  std::ofstream log(log_path, std::ios::app);
  if (!log) return 2;
  log << Basename(argv[0]);
  for (int i = 1; i < argc; ++i) log << '\t' << argv[i];
  log << '\n';
  if (!log) return 2;

  for (int i = 1; i + 1 < argc; ++i) {
    if (std::string(argv[i]) != "-o") continue;
    std::ofstream output(argv[i + 1]);
    return output ? 0 : 2;
  }
  return 0;
}
