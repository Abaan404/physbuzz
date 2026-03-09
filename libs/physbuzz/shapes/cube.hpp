#pragma once

#include "../resources/resource.hpp"

namespace Physbuzz {

class TransferBatch;
class Mesh;

namespace Builtin {

namespace ModelCube {

inline Resource<Mesh> Resource = {"builtin/cube"};

bool build(TransferBatch &batch);

} // namespace ModelCube

} // namespace Builtin

} // namespace Physbuzz
