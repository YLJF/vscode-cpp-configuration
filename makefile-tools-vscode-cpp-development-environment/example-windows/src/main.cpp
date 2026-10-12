#include "demo/math.hpp"
#include <iostream>

int main() {
    const int a = 2;
    const int b = 3;
    std::cout << "example_app: " << a << " + " << b << " = "
              << demo::add(a, b) << '\n';
    std::cout << "Debug build: " << DEMO_BUILD_DEBUG << '\n';
    return 0;
}
