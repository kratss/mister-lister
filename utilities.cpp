#include "types.h"
#include <filesystem>
#include <fstream>
#include <ftxui/component/event.hpp>
#include <optional>
#include <string_view>
#include <tuple>

using namespace ftxui;

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

  if (!std::filesystem::exists(path)) {
  }

  std::ofstream file(path);
  if (!file.is_open()) {
    error_message = "Unable to open file";
  } else if (std::filesystem::file_size(path) == 0) {
    file << "-" << delimiter;
  }

  return {path, delimiter, error_message};
}

std::optional<bool> do_mode_edit(State &state,
                                 std::shared_ptr<ComponentBase> &input,
                                 Event &event, std::string &input_buffer) {

  if (event == Event::Return) {
    state.data_history.push_back(state.data);
    state.undo_pos++;
    state.data[state.sel_row][state.sel_col] = input_buffer;
    state.mode = Mode::Normal;
    return std::nullopt;
  } else {
    return true;
  }
}
