#include "../include/SheetSerializer.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

std::string csv_escape(std::string_view field) {
    bool needs_quotes = false;
    for (char c : field) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needs_quotes = true;
            break;
        }
    }

    if (!needs_quotes) return std::string(field);

    std::string escaped;
    escaped.reserve(field.size() + 2);
    escaped.push_back('"');

    for (char c : field) {
        if (c == '"')
            escaped += "\"\"";  // escape quote
        else
            escaped.push_back(c);
    }

    escaped.push_back('"');
    return escaped;
}

bool SheetSerializer::save_csv(const Sheet& sheet, const std::filesystem::path& path) {
    std::ofstream out(path);
    if (!out) return false;

    for (const auto& row : sheet.get_rows()) {
        for (size_t col = 0; col < row.size(); col++) {
            if (col > 0) out << ',';
            out << csv_escape(row[col] ? row[col]->get_formula_or_value() : "");
        }
        out << '\n';
    }

    return true;
}

bool SheetSerializer::load_csv(Sheet& sheet, const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) return false;

    sheet.clear();

    int row_count = 0;
    std::string line;
    while (std::getline(in, line)) {
        int col_count = 0;
        std::string field;
        bool in_quotes = false;

        for (size_t i = 0; i <= line.size(); i++) {
            char c = line[i];
            if (in_quotes) {
                if (c == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        field += '"';
                        i++;
                    } else {
                        in_quotes = false;
                    }
                } else {
                    field += c;
                }
            } else {
                if (c == '"') {
                    in_quotes = true;
                } else if (c == ',' || i == line.size()) {
                    sheet.set_cell(row_count, col_count, field);
                    field.clear();
                    col_count++;
                } else {
                    field += c;
                }
            }
        }
        row_count++;
    }

    return true;
}
