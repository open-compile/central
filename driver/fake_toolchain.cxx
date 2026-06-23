#include <cstdlib>
#include <fstream>
#include <string>

namespace {

std::string Basename(const std::string &path) {
  const size_t slash = path.find_last_of("/\\");
  return path.substr(slash == std::string::npos ? 0 : slash + 1);
}

int Environment_exit(const char *name) {
  const char *value = std::getenv(name);
  if (value == nullptr || *value == '\0') return 0;
  char *end = nullptr;
  const long parsed = std::strtol(value, &end, 10);
  if (*end != '\0' || parsed < 0 || parsed > 255) return 2;
  return static_cast<int>(parsed);
}

bool Environment_enabled(const char *name) {
  const char *value = std::getenv(name);
  return value != nullptr && *value != '\0' && std::string(value) != "0";
}

}  // namespace

int main(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--version") {
      return Environment_exit("CENTRAL_FAKE_TOOL_PROBE_EXIT");
    }
  }

  const char *log_path = std::getenv("CENTRAL_FAKE_TOOL_LOG");
  if (log_path == nullptr || *log_path == '\0') return 2;

  std::ofstream log(log_path, std::ios::app);
  if (!log) return 2;
  log << Basename(argv[0]);
  for (int i = 1; i < argc; ++i) log << '\t' << argv[i];
  log << '\n';
  if (!log) return 2;

  bool object_phase = false;
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-c") object_phase = true;
  }
  const char *output_path = nullptr;
  for (int i = 1; i + 1 < argc; ++i) {
    if (std::string(argv[i]) == "-o") output_path = argv[i + 1];
  }
  const int requested_exit = Environment_exit(
      object_phase ? "CENTRAL_FAKE_TOOL_OBJECT_EXIT"
                   : "CENTRAL_FAKE_TOOL_LINK_EXIT");
  if (requested_exit != 0) return requested_exit;

  if (output_path == nullptr &&
      !Environment_enabled("CENTRAL_FAKE_TOOL_ALLOW_MISSING_OUTPUT")) {
    return 2;
  }
  if (Environment_enabled("CENTRAL_FAKE_TOOL_SKIP_OUTPUT") ||
      Environment_enabled(object_phase ? "CENTRAL_FAKE_TOOL_OBJECT_SKIP_OUTPUT"
                                       : "CENTRAL_FAKE_TOOL_LINK_SKIP_OUTPUT")) {
    return 0;
  }

  if (output_path != nullptr) {
    std::ofstream output(output_path);
    return output ? 0 : 2;
  }
  return 0;
}
