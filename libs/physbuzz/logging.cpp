#include <format>
#include <iostream>

namespace Physbuzz {

namespace Logger {

#ifndef NDEBUG

void INFO(std::string_view message) {
    std::cout << std::format("INFO: {}\n", message);
}

void WARNING(std::string_view message) {
    std::cerr << std::format("WARNING: {}\n", message);
}

void DEBUG(std::string_view message) {
    std::cout << std::format("DEBUG: {}\n", message);
}

void ASSERT(bool condition, std::string_view reason) {
    if (!condition) {
        std::cerr << std::format("ASSERTION FAILED: {}\n", reason);
        std::abort();
    }
}

#else

void INFO(std::string_view message) {}
void WARNING(std::string_view message) {}
void DEBUG(std::string_view message) {}
void ASSERT(bool condition, std::string_view reason) {}

#endif

} // namespace Logger

} // namespace Physbuzz
