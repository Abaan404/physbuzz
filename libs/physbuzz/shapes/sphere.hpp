#pragma once

#include "../resources/resource.hpp"

namespace Physbuzz {

class Transfer;
class Mesh;

namespace Builtin {

namespace ModelSphere {

inline Resource<Mesh> Resource = {"builtin/sphere"};

bool build(const std::shared_ptr<Transfer> transfer);

} // namespace ModelSphere

} // namespace Builtin

} // namespace Physbuzz
