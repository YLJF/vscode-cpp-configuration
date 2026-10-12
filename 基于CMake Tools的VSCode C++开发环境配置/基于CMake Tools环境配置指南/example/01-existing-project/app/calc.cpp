#include "acme/math.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 1 && argc != 3) {
        std::cerr << "Usage: acme_calc [left right]\n";
        return 1;
    }

    try {
        const int left = argc == 3 ? std::stoi(argv[1]) : 2;
        const int right = argc == 3 ? std::stoi(argv[2]) : 3;
        const int result = acme::add(left, right);
        std::cout << acme::app_label() << ": " << left << " + " << right << " = " << result << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Invalid operands: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
