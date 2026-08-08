#include "types.h"
#include <ftxui/dom/table.hpp>

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

Table designTable(const State &state) {
  Table table = Table(state.data);
  auto focused_cell = table.SelectCell(state.sel_col,state.sel_row);
  auto focused_row = table.SelectRow(state.sel_row);
  auto all_cells = table.SelectAll();
  auto header_cells = table.SelectRow(0);

  table.SelectAll().DecorateCells([](Element element) {
    return hbox({
        text(" "),
        element,
        text(" "),
    });
  });

  focused_cell.Decorate(bgcolor(Color::Magenta));
  focused_row.Decorate(bgcolor(Color::Yellow));
  focused_row.Decorate(color(Color::Black));
  header_cells.Decorate(bold);
  header_cells.Decorate(bgcolor(Color::Black));
  all_cells.DecorateAlternateRow(
      bgcolor(Color::GrayDark) | bgcolor(Color::White), 2, 0);
   
  return table;
}
