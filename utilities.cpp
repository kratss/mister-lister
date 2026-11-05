#include "csv.h"
#include "modify_table.h"
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
    std::ofstream file(path);
    if (!file.is_open()) {
      error_message = "Unable to open file";
    } else {
      file << "-" << delimiter;
    }
  }

  return {path, delimiter, error_message};
}

bool handle_edit(State &state, ftxui::Event &event,
                 std::string &input_placeholder) {
  if (event == Event::Return) {
    state.data_history.push_back(state.data);
    state.undo_pos++;
    state.data[state.sel_row][state.sel_col] = state.input_buffer;
    state.mode = Mode::Normal;
    state.input_buffer.clear();
    state.pending_key.clear();
    return true;
  } else {
    return state.input->OnEvent(event); // Let input component handle event and
                                        // indicate T/F whether it handled it
  }
}

bool handle_add(State &state, ftxui::Event &event,
                std::string &input_placeholder) {
  if (event == Event::Return) {
    state.data_history.push_back(state.data);
    state.undo_pos++;
    state.data[state.sel_row][state.sel_col] = state.input_buffer;
    if (state.sel_col + 1 < state.max_col) {
      state.sel_col++;
      state.input_placeholder = state.data[0][state.sel_col]; // header
      state.input_buffer.erase();
      return true;
    } else {
      state.pending_key.clear();
      state.mode = Mode::Normal;
      return true;
    }
  }
  return state.input->OnEvent(event);
}

bool handle_normal(State &state, ftxui::Event &event,
                   std::string &input_placeholder) {

  switch (state.current_action) {

  case Action::move_up:
    if (state.sel_row > 0 && state.motion_count > 0 &&
        state.motion_count <= state.sel_row) {
      state.sel_row = state.sel_row - state.motion_count;
    } else if (state.sel_row > 0 && state.motion_count > 0) {
      state.sel_row = 0;
    } else if (state.sel_row == 0) {
      state.sel_row = state.data.size() - 1;
    } else {
      state.sel_row--;
    }
    state.motion_count = 0;
    return true;

  case Action::move_down:
    if (state.sel_row < state.data.size() && state.motion_count > 0 &&
        state.motion_count <= state.data.size() - 1 - state.sel_row) {
      state.sel_row = state.sel_row + state.motion_count;
    } else if (state.sel_row > 0 && state.motion_count > 0) {
      state.sel_row = state.data.size() - 1;
    } else if (state.sel_row == state.data.size() - 1) {
      state.sel_row = 0;
    } else {
      state.sel_row++;
    }
    state.motion_count = 0;
    return true;

  case Action::move_left:
    if (state.sel_col > 0 && state.motion_count > 0 &&
        state.motion_count <= state.sel_col) {
      state.sel_col = state.sel_col - state.motion_count;
    } else if (state.sel_col > 0 && state.motion_count > 0) {
      state.sel_col = 0;
    } else if (state.sel_col == 0) {
      state.sel_col = state.data[0].size() - 1;
    } else {
      state.sel_col--;
    }
    state.motion_count = 0;
    return true;

  case Action::move_right:
    if (state.sel_col < state.data[0].size() && state.motion_count > 0 &&
        state.motion_count <= state.data[0].size() - 1 - state.sel_col) {
      state.sel_col = state.sel_col + state.motion_count;
    } else if (state.sel_col > 0 && state.motion_count > 0) {
      state.sel_col = state.data[0].size() - 1;
    } else if (state.sel_col == state.data[0].size() - 1) {
      state.sel_col = 0;
    } else {
      state.sel_col++;
    }
    state.motion_count = 0;
    return true;

  case Action::add_entry:
    state.mode = Mode::Add;
    if (state.motion_count == 0)
      state.motion_count = 1;
    state.data.push_back(state.blank_row);
    state.sel_row = state.data.size() - 1;
    state.sel_col = 0;
    state.input_buffer.clear();
    input_placeholder = state.data[0][state.sel_col]; // header
    return true;

  case Action::column_add:
    state.data_history.push_back(state.data);
    state.undo_pos++;
    state = column_add(state);
    state.input_buffer.clear();
    input_placeholder = "New field";
    return true;

  case Action::column_delete:
    state.data_history.push_back(state.data);
    state.undo_pos++;
    state = column_delete(state);
    state.input_buffer.clear();
    input_placeholder = "New field";
    return true;

  case Action::edit:
    state.mode = Mode::Edit;
    state.input_buffer.clear();
    input_placeholder =
        state.data[state.sel_row]
                  [state.sel_col]; // load current value input->TakeFocus();
    return true;

  case Action::go_to_end:
    state.sel_row = state.data.size() - 1;
    return true;

  case Action::go_to_start:
    state.sel_row = 0;
    return true;

  case Action::remove:
    state.data_history.push_back(state.data);
    state.undo_pos++;
    if (state.motion_count == 0)
      state.motion_count = 1;
    if (state.motion_count > state.data.size() - state.sel_row - 1)
      state.motion_count = state.data.size() - state.sel_row - 1;
    for (int i = 0; i < state.motion_count; i++)
      state.data.erase(state.data.begin() + state.sel_row);
    state.motion_count = 0;
    return true;

  case Action::show_key_map:
    if (state.show_popup)
      state.show_popup = false;
    else
      state.show_popup = true;
    return true;

  case Action::quit:
    writeData(state.path, state.delimiter, state.data);
    state.exit = true;
    return true;

  case Action::quit_without_saving:
    state.exit = true;
    return true;

  case Action::undo:
    if (state.undo_pos > 0) {
      state.undo_pos--;
      state.data = state.data_history[state.undo_pos];
      state.data_history.erase(state.data_history.begin() + state.undo_pos,
                               state.data_history.end());
      return true;
    }
  case Action::none:
    return true;
  }
  return true;
}

