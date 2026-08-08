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
  table.SelectCell(state.sel_col, state.sel_row).Border(LIGHT);
  table.SelectCell(state.sel_col, state.sel_row).DecorateCells(inverted);
  table.SelectCell(state.sel_col, state.sel_row).DecorateCells(focus);
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
