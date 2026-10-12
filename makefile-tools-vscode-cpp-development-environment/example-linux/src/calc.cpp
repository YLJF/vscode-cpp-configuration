#include "demo/math.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const int a = argc > 1 ? std::stoi(argv[1]) : 12;
    const int b = argc > 2 ? std::stoi(argv[2]) : 30;
    std::cout << "demo_calc: " << a << " + " << b << " = "
              << demo::add(a, b) << '\n';
    return 0;
}
