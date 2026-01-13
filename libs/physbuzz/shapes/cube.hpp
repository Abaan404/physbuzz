#pragma once

#include "../resources/resource.hpp"

namespace Physbuzz {

class Transfer;
class Mesh;

namespace Builtin {

namespace ModelCube {

inline Resource<Mesh> Resource = {"builtin/cube"};

bool build(const std::shared_ptr<Transfer> transfer);

} // namespace ModelCube

} // namespace Builtin

} // namespace Physbuzz
