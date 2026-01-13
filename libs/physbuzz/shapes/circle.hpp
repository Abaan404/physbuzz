#pragma once

#include "../resources/resource.hpp"

namespace Physbuzz {

class Transfer;
class Mesh;

namespace Builtin {

namespace ModelCircle {

inline Resource<Mesh> Resource = {"builtin/circle"};

bool build(const std::shared_ptr<Transfer> transfer);

} // namespace ModelCircle

} // namespace Builtin

} // namespace Physbuzz
