#include "csv.h"
#include "modify_table.h"
#include "types.h"
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <iostream>
#include <map>
#include <string>

using namespace ftxui;
using namespace std;

const map<string, Action> key_bindings = {
    {"k", Action::move_up},
    {"j", Action::move_down},
    {"h", Action::move_left},
    {"l", Action::move_right},
    {"up", Action::move_up},
    //{Event::ArrowUp.str(), Action::move_up}, // if you still want arrows

    {"a", Action::add_entry},
    {"d", Action::remove},
    {"e", Action::edit},
    {"r", Action::edit},
    {"u", Action::undo},
    {"/", Action::search},

    {"q!", Action::quit_without_saving},
    {"ca", Action::column_add},
    {"cd", Action::column_delete},
    {"gg", Action::go_to_start},
    {"ge", Action::go_to_end},
    {"wq", Action::quit},
};

Action process_key_press(State &state, Event event,
                         map<string, Action> key_bindings) {
  string pending_key = state.pending_key;

  auto command = key_bindings.find(pending_key);
  Action current_action;

  // If command is an exact match
  if (command != key_bindings.end()) {
    current_action = command->second;
    return current_action;
  }

  for (const auto &[seq, _] : key_bindings) {
    if (seq.starts_with(pending_key))
      return Action::none;
  }
  return Action::none;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << (argc > 0 ? argv[0] : "mister-lister")
              << " <file.csv>\n";
    return 1;
  }

  char delimiter = '\x1F';
  filesystem::path path = argv[1];
  vector<vector<string>> raw_data = readData(path, delimiter);
  int max_col = 0;
  for (int i = 0; i < raw_data.size(); i++) {
    if (raw_data[i].size() > max_col)
      max_col = raw_data[i].size();
  }
  vector<vector<string>> data = normalizeRows(raw_data, max_col);
  State state = {data, max_col};
  vector<string> blank_row;
  for (int i = 0; i < state.max_col; i++)
    blank_row.push_back("-");
  auto screen = ScreenInteractive::Fullscreen();

  string input_placeholder = "PLACHOLDER";
  InputOption input_options;
  input_options.placeholder = &input_placeholder;
  std::string input_buffer;
  Component input = Input(&input_buffer, input_options);

  Component table_component = Renderer([&] {
    Table table = designTable(state);
    Element table_element = table.Render() | frame;
    return table_element;
  });

  // TODO Rewrite the comp -> element -> comp redundancy when FTXUI > 7.0
  // is available
  Component edit_component = Renderer([&] {
    Element edit_element = hbox({
        text("Edit: "),
        input->Render() | flex,
        text("  Enter: save  Esc: cancel"),
    });
    return edit_element;
  });

  Component add_component = Renderer([&] {
    return hbox({
        text("Add: "),
        input->Render() | flex,
        text("  Enter: save  Esc: cancel"),
    });
  });

  Component footer_component = Renderer([&] {
    return text(to_string(state.motion_count) +
                " a: add entry, e: edit cell, c: create column, C: delete "
                "column, d: delete "
                "row, q: save and quit, Q: quit without saving, u: undo");
  });

  Component final_component = Container::Vertical(
      {edit_component | Maybe([&] { return state.mode == Mode::Edit; }),
       add_component | Maybe([&] { return state.mode == Mode::Add; }),
       footer_component | Maybe([&] {
         return state.mode != Mode::Add && state.mode != Mode::Edit;
       }),
       table_component | flex});

  // Catch keyboard input
  final_component = CatchEvent(final_component, [&](Event event) {
    // state.data[0][3] = event.character();
    // const auto binding = key_bindings.find(event);
    auto key = event.character();
    if (isdigit(key[0])) {
      state.motion_count = state.motion_count * 10 + (key[0] - '0');
      state.data[0][0] = state.motion_count;
      return true;
    }
    state.pending_key += event.character();

    if (event == Event::Escape) {
      state.mode = Mode::Normal;
      state.pending_key.clear();
      state.motion_count = 0;
      return true;
    }

    switch (state.mode) {
    case Mode::Edit:
      if (event == Event::Return) {
        state.data_history.push_back(state.data);
        state.undo_pos++;
        state.data[state.sel_row][state.sel_col] = input_buffer;
        state.mode = Mode::Normal;
        break;
      } else {
        return input->OnEvent(event);
      }

    case Mode::Add:
      if (event == Event::Return) {
        state.data_history.push_back(state.data);
        state.undo_pos++;
        state.data[state.sel_row][state.sel_col] = input_buffer;
        if (state.sel_col + 1 < state.max_col) {
          state.sel_col++;
          input_placeholder = state.data[0][state.sel_col]; // header
          input_buffer.erase();
          return true;
        } else {
          state.mode = Mode::Normal;
          return true;
        }
      }
      return input->OnEvent(event);
    }

    Action current_action = process_key_press(state, event, key_bindings);

    if (state.pending_key.size() > 1) {
      state.pending_key.clear();
    } else if (current_action != Action::none) {
      state.pending_key.clear();
    }

    switch (current_action) {

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
      // if (state.sel_col > 0)
      //   state.sel_col--;
      // else
      //   state.sel_col = state.data[state.sel_row].size() - 1;
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
      if (state.motion_count == 0) state.motion_count = 1;
      state.data.push_back(blank_row);
      state.sel_row = state.data.size() - 1;
      state.sel_col = 0;
      input_buffer.clear();
      input_placeholder = state.data[0][state.sel_col]; // header
      return true;

    case Action::column_add:
      state.data_history.push_back(state.data);
      state.undo_pos++;
      state = column_add(state);
      input_buffer.clear();
      input_placeholder = "New field";
      return true;

    case Action::column_delete:
      state.data_history.push_back(state.data);
      state.undo_pos++;
      state = column_delete(state);
      input_buffer.clear();
      input_placeholder = "New field";
      return true;

    case Action::edit:
      state.mode = Mode::Edit;
      input_buffer.clear();
      input_placeholder =
          state.data[state.sel_row]
                    [state.sel_col]; // load current value input->TakeFocus();
      return true;

    case Action::go_to_end:
      state.sel_row = state.data.size()-1;
      return true;

    case Action::go_to_start:
      state.sel_row = 0;
      return true;
      
    case Action::remove:
      state.data_history.push_back(state.data);
      state.undo_pos++;
      if (state.motion_count == 0)
        state.motion_count = 1;
      if (state.motion_count > state.data.size() - state.sel_row - 1 )
        state.motion_count = state.data.size() - state.sel_row -1;
      for (int i = 0; i < state.motion_count; i++)
        state.data.erase(state.data.begin() + state.sel_row);
      state.motion_count = 0;
      return true;

    case Action::quit:
      writeData(path, delimiter, state.data);
      screen.Exit();

    case Action::quit_without_saving:
      screen.Exit();

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
  });

  screen.Loop(final_component);
}
