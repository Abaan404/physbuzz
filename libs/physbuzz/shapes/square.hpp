#pragma once

#include "../resources/resources.hpp"

namespace Physbuzz {

class Transfer;
class Mesh;

namespace Builtin {

namespace ModelSquare {

inline Resource<Mesh> Resource = {"builtin/square"};

bool build(const std::shared_ptr<Transfer> transfer);

} // namespace ModelSquare

} // namespace Builtin

} // namespace Physbuzz
