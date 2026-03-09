#pragma once

#include "../resources/resource.hpp"

namespace Physbuzz {

class TransferBatch;
class Mesh;

namespace Builtin {

namespace ModelSphere {

inline Resource<Mesh> Resource = {"builtin/sphere"};

bool build(TransferBatch &batch);

} // namespace ModelSphere

} // namespace Builtin

} // namespace Physbuzz
