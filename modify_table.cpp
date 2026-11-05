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

  for (int index : state.visible_rows){
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

// void search(const std::vector<std::vector<std::string>> &data,
//             std::string search_terms) {
//   search_results result;
//   std::vector<std::string> cleaned_data;
//   for (std::vector<std::string> row : data) {
//     std::string line;
//     for (std::string item : row) {
//       line.append(item);
//     }

//     std::vector<std::string> tokens;
//     std::istringstream iss(search_terms);
//     std::string token;

//     while (iss >> token) {
//       tokens.push_back(token);
//     }

//     // Transform string to lower-case
//     std::ranges::transform(line, line.begin(),
//                            [](unsigned char c) { return std::tolower(c); });
//     cleaned_data.push_back(line);

//     for (std::vector<std::string> row : data) {
//       for (std::string item : row) {
//         for (std::string token : tokens) {
//           for (char letter : token) {
//             if (item.contains(letter)){
//                           }
//           }
//         }
//       }
//     }
//   }
// }
