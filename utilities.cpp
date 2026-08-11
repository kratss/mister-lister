#include <filesystem>
#include <fstream>
#include <string_view>
#include <tuple>

std::tuple<std::filesystem::path, char, std::string> parse_args(int argc,
                                                                char *argv[]) {
  std::filesystem::path path;
  char delimiter = '\x1f';
  std::string error_message;

  for (int i = 0; i < argc; i++) {
    std::string_view arg = argv[i];

    if (arg == "-d" || arg == "--delimiter") {
      if (i + 1 < argc) {
        delimiter = argv[i + 1][0];
        i++;
      } else {
        error_message = "Delimeter requires value\n \
                         Usage: --delimiter CHAR";
      }
    } else if (!arg.starts_with('-')) {
      path = arg;
    }
  }

  std::ifstream file(path);
  if (!file.is_open()) error_message = "Unable to open file";    
  return {path, delimiter, error_message};
}
