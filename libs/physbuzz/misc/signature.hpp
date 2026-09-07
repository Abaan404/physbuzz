#pragma once

#include <cstdint>

namespace Physbuzz {

using SignatureID = std::uint32_t;

class Signature {
  public:
    template <typename T>
    inline static SignatureID ID() {
        static const SignatureID id = m_CurId++;
        return id;
    }

  private:
    inline static SignatureID m_CurId = 0;
};

} // namespace Physbuzz
