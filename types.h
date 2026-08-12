#pragma once
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <string>
#include <vector>

enum class Mode { Normal, Add, Edit, Search, Column_Add, Key_Map };

enum class Action {
  move_up,
  move_down,
  move_left,
  move_right,
  add_entry,
  edit,
  column_add,
  column_delete,
  go_to_start,
  go_to_end,
  none,
  quit,
  quit_without_saving,
  remove,
  search,
  show_key_map,
  undo,
};

struct Binding {
  std::string keys;
  Action action;
};

struct State {
  std::vector<std::vector<std::string>> data;
  int max_col = 0;
  Mode mode = Mode::Normal;
  int sel_row = 0;
  int sel_col = 0;
  std::vector<std::vector<std::vector<std::string>>>
      data_history; // undo history
  int undo_pos = 0;
  std::string pending_key;
  int motion_count = 0; // for vim style `5k`: move up 5 lines
  int key_map_int = 0;
  Action current_action;
  bool show_popup = false;
  char delimiter;
  std::filesystem::path path;

  std::vector<std::string> create_blank_row(int max_col) {
    std::vector<std::string> blank_row;
    for (int i = 0; i < max_col; i++)
      blank_row.push_back("-");
    return blank_row;
  }

  std::vector<std::string> blank_row = create_blank_row(max_col);

  std::string input_placeholder = "PLACHOLDER";
  std::string input_buffer;
  ftxui::InputOption input_options = {.placeholder = &input_placeholder};
  ftxui::Component input;

  State(std::vector<std::vector<std::string>> initial_data, int initial_max_col)
      : data(initial_data), max_col(initial_max_col) {
    data_history.push_back(data);
  }
};
