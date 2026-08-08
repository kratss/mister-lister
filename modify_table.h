#pragma once

#include "csv.h"
#include "types.h"
#include <filesystem>
#include <ftxui/dom/table.hpp>
#include <string>
#include <vector>

State column_add(const State &state);

State column_delete(const State &state);

ftxui::Table designTable(const State &state);
