#include <iostream>
#include <sstream>
#include <string>

#include <SFML/Graphics.hpp>

#include "Sheet.hpp"

int main() {
    auto sheet = std::make_shared<Sheet>();
    sheet->init_cells();

    std::cout << "Welcome to Canno -- Console env.\n";
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "quit") {
            break;
        } else if (command == "help") {
            std::cout << "Commands available for Canno:\n"
                      << "help          -- Shows this helpful message\n"
                      << "set REF VALUE -- Sets the value for the cell at REF (e.g., A1)\n"
                      << "get REF       -- Gets the value for the cell at REF\n"
                      << "getf REF      -- Gets the formula for the cell at REF\n"
                      << "print         -- Prints the whole grid\n"
                      << "quit          -- Quits the Canno program\n";
        } else if (command == "set") {
            std::string cell_ref;
            iss >> cell_ref;

            std::string value;
            std::getline(iss, value);
            if (!value.empty() && value[0] == ' ') value.erase(0, 1);

            if (sheet->set_cell(cell_ref, value)) {
                std::cout << "Set " << cell_ref << " = \"" << value << "\"\n";
            } else {
                std::cout << "Failed to set " << cell_ref << "\n";
            }
        } else if (command == "get") {
            std::string cell_ref;
            iss >> cell_ref;

            auto value = sheet->get_cell_val(cell_ref);
            if (value) {
                std::cout << "get " << cell_ref << " = \"" << *value << "\"\n";
            } else {
                std::cout << "Failed to get " << cell_ref << "\n";
            }
        } else if (command == "getf") {
            std::string cell_ref;
            iss >> cell_ref;

            auto value = sheet->get_cell_formula(cell_ref);
            if (value) {
                std::cout << "get formula " << cell_ref << " = \"" << *value << "\"\n";
            } else {
                std::cout << "Failed to get formula " << cell_ref << "\n";
            }
        } else if (command == "print") {
            // TODO remove this when a frontend is attached
            std::cout << "  ";
            for (size_t i = 0; i < sheet->SHEET_COLS; ++i) {
                auto col_to_string = [](size_t col) -> std::string {
                    std::string result;
                    while (col >= 0) {
                        result = char('A' + (col % 26)) + result;
                        if (col < 26) break;
                        col = col / 26 - 1;
                    }
                    return result;
                };

                std::cout << col_to_string(i);
                if(i + (int)'a' < (int)'z') {
                    std::cout << " ";
                }
            }
            std::cout << "\n";

            for (size_t y = 0; y < sheet->SHEET_ROWS; ++y) {
                std::cout << y + 1;
                if (y < 9) std::cout << " ";

                for (size_t x = 0; x < sheet->SHEET_COLS; ++x) {
                    auto cell_val = sheet->get_cell_val(x, y);
                    if (cell_val.has_value() && cell_val != "") {
                        std::cout << cell_val.value() << "|";
                    }
                    std::cout << " |";
                }
                std::cout << std::endl;
            }
        } else {
            std::cout << "Unknown command: " << command << "\n";
        }
    }

    std::cout << "Exiting Canno\n";
}
