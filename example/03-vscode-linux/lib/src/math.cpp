#include "acme/math.hpp"

namespace acme {
int add(int left, int right) {
    return left + right;
}

const char* app_label() {
    return ACME_APP_LABEL;
}
} // namespace acme
