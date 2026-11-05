#pragma once

#include "csv.h"
#include "types.h"
#include <filesystem>
#include <ftxui/dom/table.hpp>
#include <string>
#include <vector>

State column_add(const State &state);

State column_delete(const State &state);

std::vector<std::vector<std::string>> column_sort(const State &state);

ftxui::Table designTable(const State &state);
