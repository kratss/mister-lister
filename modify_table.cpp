#include "types.h"
#include <algorithm> // for std::ranges::transform
#include <cctype>    // for std::tolowerusing namespace ftxui;
#include <ftxui/dom/table.hpp>
#include <ranges>
#include <string>

using namespace ftxui;

State column_add(const State &state) {
  State new_state = state;
  new_state.max_col = state.max_col + 1;

  for (auto &row : new_state.data) {
    row.insert(row.begin() + state.sel_col + 1, "-");
  }

  new_state.sel_col = state.sel_col + 1;
  new_state.sel_row = 0;
  new_state.mode = Mode::Edit;
  return new_state;
}

State column_delete(const State &state) {
  State new_state = state;
  new_state.max_col = state.max_col - 1;
  for (int i = 0; i < state.data.size(); i++) {
    new_state.data[i].erase(new_state.data[i].begin() + state.sel_col);
  }
  return new_state;
}

std::vector<std::vector<std::string>> column_sort(const State &state) {
  auto sorted = state.data;
  const int column = state.sel_col;

  if (sorted.size() > 1) {
    std::stable_sort(sorted.begin() + 1, sorted.end(),
                     [column](const auto &a, const auto &b) {
                       return a[column] < b[column];
                     });
  }
  return sorted;
}

Table designTable(const State &state) {
  std::vector<std::vector<std::string>> visible_data;
  visible_data.reserve(state.visible_rows.size());

  for (int index : state.visible_rows) {
    visible_data.push_back(state.data.at(index));
  }

  Table table = Table(visible_data);
  auto focused_cell = table.SelectCell(state.sel_col, state.sel_row);
  auto focused_row = table.SelectRow(state.sel_row);
  auto all_cells = table.SelectAll();
  auto header_cells = table.SelectRow(0);

  all_cells.DecorateCells([](Element element) {
    return hbox({
        text(" "),
        element,
        text(" "),
    });
  });

  focused_cell.Decorate(bgcolor(Color::Magenta));
  focused_cell.Decorate(focus);
  focused_row.Decorate(bgcolor(Color::Yellow));
  focused_row.Decorate(color(Color::Black));
  header_cells.Decorate(bold);
  header_cells.Decorate(bgcolor(Color::Black));
  all_cells.DecorateAlternateRow(
      bgcolor(Color::GrayDark) | bgcolor(Color::White), 2, 0);
  all_cells.Border(LIGHT);
  return table;
}

struct search_results {
  std::vector<int> index;
  std::vector<int> score;
};

State search(State const &state) {
  State new_state = state;
  std::vector<std::vector<std::string>> data = state.data;
  std::string query = state.input_buffer;

  std::vector<std::string> monoRow(data.size(), " ");
  for (int row; row < data.size(); row++) {
    for (int cell; cell < data[row].size(); cell++) {
      monoRow[row].append(data[row][cell]);
    }
  }

  if (new_state.sel_row > data.size()) {
    new_state.sel_row = 0;
  }
  if (new_state.sel_col > data[0].size()) {
    new_state.sel_col = 0;
  }

  std::vector<std::string> queries;
  std::stringstream query_stream(state.input_buffer);
  for (std::string word; std::getline(query_stream, word);) {
    queries.push_back(word);
  }

  std::vector<int> matches;
  for (std::size_t row = 1; row < data.size(); ++row) {
    for (const auto &cell : data[row]) {
      for (std::string q : queries) {
        if (cell.find(query) != std::string::npos) {
          matches.push_back(static_cast<int>(row));
          break; // Include this row only once.
        }
      }
    }
  }
  matches.insert(matches.begin(), 0);
  new_state.visible_rows = matches;
  return new_state;
}
