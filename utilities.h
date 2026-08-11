#pragma once

#include <filesystem>

std::tuple<std::filesystem::path, char, std::string> parse_args(int argc, char* argv[]);

