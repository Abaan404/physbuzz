#pragma once

#include <string_view>

// a very basic logging lib until i need a better solution

namespace Physbuzz {

namespace Logger {

void INFO(std::string_view message);
void WARNING(std::string_view message);

void DEBUG(std::string_view message);
void ASSERT(bool condition, std::string_view reason);

}; // namespace Logger

} // namespace Physbuzz
