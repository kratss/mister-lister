#include "csv.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<std::vector<std::string>>
readData(const std::filesystem::path &path, const char &delimiter) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Unable to open file");
  }
  std::vector<std::vector<std::string>> data;
  for (std::string line; getline(file, line);) {
    std::vector<std::string> row;
    std::stringstream line_stream(line);
    for (std::string field; getline(line_stream, field, delimiter);) {
      row.push_back(field);
    }
    data.push_back(row);
  }
  return data;
}

void writeData(const std::filesystem::path &path, const char &delimiter,
               const std::vector<std::vector<std::string>> &data) {
  std::ofstream file;
  file.open(path);
  for (const auto& row : data) {
    for (std::size_t i = 0; i < row.size(); ++i) {
      if (i != 0) file << delimiter;
      file << row[i];
    }
    file << '\n';
  }
}

std::vector<std::vector<std::string>>
normalizeRows(const std::vector<std::vector<std::string>> &data,
              const int &max_col) {
  std::vector<std::vector<std::string>> normalized_data = data;
  for (int i = 0; i < data.size(); i++) {
    for (int j = data[i].size(); j < max_col; j++) {
      normalized_data[i].push_back("-");
    }
  }
  return normalized_data;
}
