#include "acme/math.hpp"

#include <iostream>

int main() {
    const int check = acme::add(6, 7);
    std::cout << "Application: " << acme::app_label() << '\n';
    std::cout << "Library check: 6 + 7 = " << check << '\n';
    return 0;
}
