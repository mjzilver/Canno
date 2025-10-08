#include "Utils.hpp"

#include <charconv>
#include <iomanip>
#include <ios>
#include <iostream>
#include <optional>

std::string pretty_print_double(double d) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(10) << d;
    std::string s = ss.str();

    s.erase(s.find_last_not_of('0') + 1, std::string::npos);

    if (!s.empty() && s.back() == '.') s.pop_back();

    return s;
}

bool parse_double(const std::string& str, double& out) {
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
    if (ec != std::errc()) {
        return false;
    }
    return true;
}

bool parse_int(const std::string& str, int& out) {
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
    if (ec != std::errc()) {
        return false;
    }
    return true;
}

std::optional<std::pair<int, int>> cell_ref_to_indices(const std::string& cell_ref) {
    int col = 0;
    int row = 0;
    size_t i = 0;

    while (i < cell_ref.size() && std::isalpha(cell_ref[i])) {
        char c = std::toupper(cell_ref[i]);
        col = col * 26 + (c - 'A' + 1);
        ++i;
    }

    if (!parse_int(cell_ref.substr(i), row)) {
        std::cerr << "Invalid row in cell reference: " << cell_ref << "\n";
        return std::nullopt;
    }

    if (col == 0 || row == 0) {
        std::cerr << "Invalid cell reference format: " << cell_ref << "\n";
        return std::nullopt;
    }

    return std::pair{col - 1, row - 1};
}

std::string indices_to_cell_ref(int col, int row) {
    std::string col_str;
    int c = col;
    while (c >= 0) {
        char letter = 'A' + (c % 26);
        col_str = letter + col_str;
        c = c / 26 - 1;
    }

    return col_str + std::to_string(row + 1);
}
