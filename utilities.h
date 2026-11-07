#pragma once

#include "types.h"
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

std::tuple<std::filesystem::path, char, std::string> parse_args(int argc,
                                                                char *argv[]);

bool handle_edit(State &state, ftxui::Event &event, std::string &input_placeholder);
bool handle_add(State &state, ftxui::Event &event, std::string &input_placeholder);
bool handle_search(State &state, ftxui::Event &event, std::string &input_placeholder);
bool handle_normal(State &state, ftxui::Event &event, std::string &input_placeholder);
