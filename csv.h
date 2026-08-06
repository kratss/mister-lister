#pragma once

#include <filesystem>
#include <string>
#include <vector>

std::vector<std::vector<std::string>> readData(const std::filesystem::path &path,
                                const char &delimiter);

void writeData(const std::filesystem::path &path, const char &delimiter,
               const std::vector<std::vector<std::string>> &data);

std::vector<std::vector<std::string>> normalizeRows(const std::vector<std::vector<std::string>> &data,
                                     const int &max_col);

