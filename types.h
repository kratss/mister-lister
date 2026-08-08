#pragma once
#include <string>
#include <vector>


enum class Mode { Normal, Add, Edit, Search, Column_Add };

enum class Action {
  move_up,
  move_down,
  move_left,
  move_right,
  add_entry,
  edit,
  column_add,
  column_delete,
  none,
  quit,
  quit_without_saving,
  remove,
  search,
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
  std::vector<std::vector<std::vector<std::string>>> data_history; // undo history
  int undo_pos = 0;
  std::string pending_key;
  int motion_count = 0; // for vim style `5k`: move up 5 lines

  State(std::vector<std::vector<std::string>> initial_data, int initial_max_col)
      : data(initial_data), max_col(initial_max_col) {
    data_history.push_back(data);
  }
};

