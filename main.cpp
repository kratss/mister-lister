#include "csv.h"
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

using namespace ftxui;
using namespace std;

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
  quit,
  quit_without_saving,
  remove,
  search,
  undo,
};

struct Binding {
  string keys;
  Action action;
};

struct State {
  vector<vector<string>> data;
  int max_col = 0;
  Mode mode = Mode::Normal;
  int sel_row = 0;
  int sel_col = 0;
  vector<vector<vector<string>>> data_history; // undo history
  int undo_pos = 0;

  State(vector<vector<string>> initial_data, int initial_max_col)
      : data(initial_data), max_col(initial_max_col) {
    data_history.push_back(data);
  }
};

State columnAdd(const State &state) {
  State new_state = state;
  new_state.max_col = state.max_col + 1;
  new_state.data = normalizeRows(state.data, new_state.max_col);
  new_state.sel_col = new_state.max_col - 1; // Fixed off-by-one
  new_state.sel_row = 0;
  new_state.mode = Mode::Edit;
  return new_state;
}

Table designTable(const State &state) {
  Table table = Table(state.data);
  table.SelectCell(state.sel_col, state.sel_row).Border(LIGHT);
  table.SelectCell(state.sel_col, state.sel_row).DecorateCells(inverted);
  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).Border(DOUBLE);
  table.SelectAll().DecorateCellsAlternateRow(
      color(Color::GrayDark) | color(Color::White), 2, 0);
  table.SelectAll().DecorateCells([](Element element) {
    return hbox({
        text(" "),
        element,
        text(" "),
    });
  });
  return table;
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
    Element table_element = table.Render();
    return table_element;
  });

  // TODO Rewrite the comp -> element -> comp redundancy when FTXUI > 7.0
  // available
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

  Component footer_component = Renderer([] {
    return text("a: Add entry, e: Edit cell, c: Create new column, d: Delete "
                "row, q: quit");
  });

  Component final_component = Container::Vertical(
      {edit_component | Maybe([&] { return state.mode == Mode::Edit; }),
       add_component | Maybe([&] { return state.mode == Mode::Add; }),
       table_component | flex, footer_component});

  map<Event, Action> key_bindings = {{Event::ArrowUp, Action::move_up},
                                     {Event::ArrowDown, Action::move_down},
                                     {Event::ArrowLeft, Action::move_left},
                                     {Event::ArrowRight, Action::move_right},
                                     {Event::k, Action::move_up},
                                     {Event::j, Action::move_down},
                                     {Event::h, Action::move_left},
                                     {Event::l, Action::move_right},
                                     {Event::a, Action::add_entry},
                                     {Event::c, Action::column_add},
                                     {Event::e, Action::edit},
                                     {Event::d, Action::remove},
                                     {Event::q, Action::quit},
                                     {Event::Q, Action::quit_without_saving},
                                     {Event::u, Action::undo},
                                     {Event::Character('/'), Action::search}};

  // TODO support two character commands
  const vector<Binding> key_bindings_two = {
      {"k", Action::move_up},
      {"j", Action::move_down},
      {"h", Action::move_left},
      {"l", Action::move_right},
      {"up", Action::move_up},
      //{Event::ArrowUp.str(), Action::move_up}, // if you still want arrows

      {"a", Action::add_entry},
      {"e", Action::edit},
      {"q", Action::quit},
      {"/", Action::search},

      {"ha", Action::column_add},
      {"hd", Action::column_delete},
  };

  // Catch keyboard input
  final_component = CatchEvent(final_component, [&](Event event) {
    const auto binding = key_bindings.find(event);

    if (event == Event::Escape) {
      state.mode = Mode::Normal;
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

    if (binding == key_bindings.end())
      return false; // Handle undefined events
    switch (binding->second) {

    case Action::move_up:
      if (state.sel_row > 0)
        state.sel_row--;
      else
        state.sel_row = state.data.size() - 1;
      return true;

    case Action::move_down:
      if (state.sel_row < state.data.size() - 1)
        state.sel_row++;
      else
        state.sel_row = 0;
      return true;

    case Action::move_left:
      if (state.sel_col > 0)
        state.sel_col--;
      else
        state.sel_col = state.data[state.sel_row].size() - 1;
      return true;

    case Action::move_right:
      if (state.sel_col < state.data[state.sel_row].size() - 1)
        state.sel_col++;
      else
        state.sel_col = 0;
      return true;

    case Action::add_entry:
      state.mode = Mode::Add;
      state.data.push_back(blank_row);
      state.sel_row = state.data.size() - 1;
      state.sel_col = 0;
      input_buffer.clear();
      input_placeholder = state.data[0][state.sel_col]; // header
      return true;

    case Action::column_add:
      state = columnAdd(state);
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

    case Action::remove:
      state.data_history.push_back(state.data);
      state.undo_pos++;
      state.data.erase(state.data.begin() + state.sel_row);
      return true;

    case Action::quit:
      writeData(path, delimiter, state.data);
      screen.Exit();

    case Action::quit_without_saving:
      screen.Exit();

    case Action::undo:
      if (state.undo_pos > 0){
        state.undo_pos--;
        state.data = state.data_history[state.undo_pos];
        state.data_history.erase(state.data_history.begin() + state.undo_pos,
                                 state.data_history.end());
         return true;
         }
    }
    return false;
  });

  screen.Loop(final_component);
}
