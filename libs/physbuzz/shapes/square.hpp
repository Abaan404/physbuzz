#pragma once

#include "../resources/resource.hpp"

namespace Physbuzz {

class TransferBatch;
class Mesh;

namespace Builtin {

namespace ModelSquare {

inline Resource<Mesh> Resource = {"builtin/square"};

bool build(TransferBatch &batch);

} // namespace ModelSquare

} // namespace Builtin

} // namespace Physbuzz
