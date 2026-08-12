#pragma once

#include "types.h"
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

std::tuple<std::filesystem::path, char, std::string> parse_args(int argc,
                                                                char *argv[]);


std::optional<bool> do_mode_edit(State &state,
                                 std::shared_ptr<ftxui::ComponentBase> &input,
                                 ftxui::Event &event, std::string &input_buffer);
