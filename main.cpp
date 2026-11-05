#include "config.h"
#include "csv.h"
#include "modify_table.h"
#include "types.h"
#include "utilities.h"
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
  auto [path, delimiter, error_message] = parse_args(argc, argv);
  if (!error_message.empty()) {
    cerr << error_message;
    return 1;
  }

  vector<vector<string>> raw_data = readData(path, delimiter);
  int max_col = 0;
  for (int i = 0; i < raw_data.size(); i++) {
    if (raw_data[i].size() > max_col)
      max_col = raw_data[i].size();
  }

  vector<vector<string>> data = normalizeRows(raw_data, max_col);
  State state = {data, max_col};
  state.path = path;
  state.delimiter = delimiter;
  vector<string> blank_row;
  for (int i = 0; i < state.max_col; i++)
    blank_row.push_back("-");
  auto screen = ScreenInteractive::Fullscreen();
  bool show_popup = false;

  string input_placeholder = "PLACHOLDER";
  InputOption input_options;
  input_options.placeholder = &input_placeholder;
  // std::string state.input_buffer;
  state.input = Input(&state.input_buffer, input_options);

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
        state.input->Render() | flex,
        text("  Enter: save  Esc: cancel"),
    });
    return edit_element;
  });

  Component add_component = Renderer([&] {
    return hbox({
        text("Add: "),
        state.input->Render() | flex,
        text("  Enter: save  Esc: cancel"),
    });
  });

  Component footer_component = Renderer(
      [&] { return text(to_string(state.motion_count) + state.pending_key); });

  Component window = Window({
      .inner = Renderer([&] { return key_bindings_text[state.key_map_int]; }),
      .title = "Key Command",
      .left = 0,
      .top = 0,
      .width = 35,
      .height = 20,
  });

  Component main_component = Container::Vertical(
      {edit_component | Maybe([&] { return state.mode == Mode::Edit; }),
       add_component | Maybe([&] { return state.mode == Mode::Add; }),
       table_component | flex, footer_component | Maybe([&] {
                                 return state.mode != Mode::Add &&
                                        state.mode != Mode::Edit;
                               })});

  main_component |= Modal(window, &state.show_popup);

  using Handler = std::function<bool(State &, ftxui::Event, string)>;

  std::map<Mode, Handler> handlers = {
      {Mode::Edit,
       [](State &s, ftxui::Event e, string i) { return handle_edit(s, e, i); }},
      {Mode::Add,
       [](State &s, ftxui::Event e, string i) { return handle_add(s, e, i); }},
      {Mode::Normal, [](State &s, ftxui::Event e,
                        string i) { return handle_normal(s, e, i); }},
  };

  main_component = CatchEvent(main_component, [&](Event event) {
    auto key = event.character();
    if (isdigit(key[0]) && state.mode == Mode::Normal) {
      state.motion_count = state.motion_count * 10 + (key[0] - '0');
      return true;
    }
    state.pending_key += event.character();

    if (event == Event::Escape) {
      state.mode = Mode::Normal;
      state.pending_key.clear();
      state.motion_count = 0;
      return true;
    }

    state.current_action = process_key_press(state, event, key_bindings);

    if (state.pending_key.size() > 1) {
      state.pending_key.clear();
    } else if (state.current_action != Action::none) {
      state.pending_key.clear();
    }

    if (auto it = handlers.find(state.mode); it != handlers.end()) {
      bool handled = it->second(state, event, input_placeholder);
      if (state.exit) {
        screen.Exit();
        return true;
      }
      return handled;
    }
    return false;
  });
  screen.Loop(main_component);
}
