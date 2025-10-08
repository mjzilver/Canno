#pragma once

#include <optional>
#include <string>

std::string pretty_print_double(double d);
bool parse_double(const std::string& str, double& out);
bool parse_int(const std::string& str, int& out);
std::optional<std::pair<int, int>> cell_ref_to_indices(const std::string& cell_ref);
std::string indices_to_cell_ref(int x, int y);