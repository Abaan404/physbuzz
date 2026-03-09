#pragma once

#include "../resources/resource.hpp"

namespace Physbuzz {

class TransferBatch;
class Mesh;

namespace Builtin {

namespace ModelCircle {

inline Resource<Mesh> Resource = {"builtin/circle"};

bool build(TransferBatch &batch);

} // namespace ModelCircle

} // namespace Builtin

} // namespace Physbuzz
