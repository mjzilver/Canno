#include <iostream>
#include <sstream>
#include <string>

#include "Sheet.hpp"

int main() {
    auto sheet = std::make_shared<Sheet>();
    sheet->init_cells();

    std::cout << "Welcome to Canno.\n";
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
                      << "get REF       -- Gets the value for the cell at REF (e.g., B2)\n"
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
        } else {
            std::cout << "Unknown command: " << command << "\n";
        }
    }

    std::cout << "Exiting Canno\n";
}
