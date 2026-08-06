#include <filesystem>
#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
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
  remove,
  search,
};

struct State {
  vector<vector<string>> data;
  int max_col = 0;
  Mode mode = Mode::Normal;
  int sel_row = 0;
  int sel_col = 0;
};

vector<vector<string>> readData(const filesystem::path &path,
                                const char &delimiter) {
  ifstream file(path);
  if (!file) {
    throw std::runtime_error("Unable to open file");
  }
  vector<vector<string>> data;
  for (string line; getline(file, line);) {
    vector<string> row;
    stringstream line_stream(line);
    for (string field; getline(line_stream, field, delimiter);) {
      row.push_back(field);
    }
    data.push_back(row);
  }
  return data;
}

void writeData(const filesystem::path &path, const char &delimiter,
               const vector<vector<string>> &data) {
  ofstream file;
  file.open(path);
  string file_string;
  for (vector<string> row : data) {
    file_string.append(row[0]);
    for (int j = 1; j < row.size(); j++) {
      file_string.push_back(delimiter);
      file_string.append(row[j]);
    }
    file_string.push_back('\n');
  }
  file << file_string;
}

vector<vector<string>> normalizeRows(const vector<vector<string>> &data,
                                     const int &max_col) {
  vector<vector<string>> normalized_data = data;
  for (int i = 0; i < data.size(); i++) {
    for (int j = data[i].size(); j < max_col; j++) {
      normalized_data[i].push_back("-");
    }
  }
  return normalized_data;
}

State columnAdd(const State &state) {
  State new_state = state;
  new_state.max_col = state.max_col+1;
  new_state.data = normalizeRows(state.data, new_state.max_col);
  new_state.sel_col = new_state.max_col-1; // Fixed off-by-one
  new_state.sel_row = 0;
  new_state.mode = Mode::Edit;
  return new_state;
}

int main() {
  char delimiter = ',';
  filesystem::path path = "/home/m/knowledge/books.csv";
  vector<vector<string>> raw_data = readData(path, delimiter);
  int max_col = 0;
  for (int i = 0; i < raw_data.size(); i++) {
    if (raw_data[i].size() > max_col)
      max_col = raw_data[i].size();
  }
  vector<vector<string>> data = normalizeRows(raw_data, max_col);
  vector<string> blank_row;
  for (int i = 0; i < max_col; i++)
    blank_row.push_back("-");
  int sel_row = 0;
  int sel_col = 0;
  Mode mode = Mode::Normal;
  auto screen = ScreenInteractive::Fullscreen();
  string input_placeholder = "PLACHOLDER";
  InputOption input_options;
  input_options.placeholder = &input_placeholder;

  State state = {data,max_col};

  std::string input_buffer;
  Component input = Input(&input_buffer, input_options);

  Component table_component = Renderer([&] {
    Table table = Table(state.data);
    table.SelectCell(state.sel_col, sel_row).Border(LIGHT);
    table.SelectCell(state.sel_col, sel_row).DecorateCells(inverted);
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

    Element table_element = table.Render();
    if (state.mode == Mode::Normal) {
      return table_element;
    } else if (state.mode == Mode::Edit) {
      return vbox({
          hbox({
              text("Edit: "),
              input->Render() | flex,
              text("  Enter: save  Esc: cancel"),
          }),
          separator(),
          table_element,

      });
    } else if (state.mode == Mode::Add) {
      return vbox({
          hbox({
              text("Add: "),
              input->Render() | flex,
              text("  Enter: save  Esc: cancel"),
          }),
          separator(),
          table_element,

      });
    }
  });

  map<Event, Action> key_bindings = {{Event::ArrowUp, Action::move_up},
                                     {Event::ArrowDown, Action::move_down},
                                     {Event::ArrowLeft, Action::move_left},
                                     {Event::ArrowRight, Action::move_right},
                                     {Event::k, Action::move_up},
                                     {Event::j, Action::move_down},
                                     {Event::h, Action::move_left},
                                     {Event::l, Action::move_right},
                                     {Event::a, Action::add_entry},
                                     {Event::e, Action::edit},
                                     {Event::d, Action::remove},
                                     {Event::q, Action::quit},
                                     {Event::c, Action::column_add},
                                     {Event::Character('/'), Action::search}};

  struct Binding {
    string keys;
    Action action;
  };
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
  table_component = CatchEvent(table_component, [&](Event event) {
    const auto binding = key_bindings.find(event);

    if (event == Event::Escape) {
      state.mode = Mode::Normal;
      return true;
    }

    switch (state.mode) {
    case Mode::Edit:
      if (event == Event::Return) {
        state.data[sel_row][state.sel_col] = input_buffer;
        state.mode = Mode::Normal;
        break;
        return true;
      } else {
        return input->OnEvent(event);
      }

    case Mode::Add:
      if (event == Event::Return) {
        state.data[sel_row][state.sel_col] = input_buffer;
        if (state.sel_col + 1 < max_col) {
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
      if (sel_row > 0)
        sel_row--;
      else
        sel_row = state.data.size() - 1;
      return true;

    case Action::move_down:
      if (sel_row < state.data.size() - 1)
        sel_row++;
      else
        sel_row = 0;
      return true;

    case Action::move_left:
      if (state.sel_col > 0)
        state.sel_col--;
      else
        state.sel_col = state.data[sel_row].size() - 1;
      return true;

    case Action::move_right:
      if (state.sel_col < state.data[sel_row].size() - 1)
        state.sel_col++;
      else
        state.sel_col = 0;
      return true;

    case Action::add_entry:
      state.mode = Mode::Add;
      state.data.push_back(blank_row);
      sel_row = state.data.size() - 1;
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
          state.data[sel_row][state.sel_col]; // load current value input->TakeFocus();
      return true;

    case Action::quit:
      writeData(path, delimiter, state.data);
      screen.Exit();

    case Action::remove:
      state.data.erase(state.data.begin() + sel_row);
    }
    return false;
  });

  screen.Loop(table_component);
}
