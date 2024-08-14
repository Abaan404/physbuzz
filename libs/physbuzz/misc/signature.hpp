#pragma once

#include <cstdint>
#include <set>

namespace Physbuzz {

using SignatureID = std::uint32_t;

class Signature {
  public:
    template <typename T>
    inline static const SignatureID ID() {
        static SignatureID id = nextID<T>();
        return id;
    }

  private:
    template <typename T>
    inline static const SignatureID nextID() {
        if (!m_Ids.contains(m_CurId)) {
            m_Ids.insert(m_CurId);
            m_CurId++;
        }

        return m_CurId;
    }

    static SignatureID m_CurId;
    static std::set<SignatureID> m_Ids;
};

} // namespace Physbuzz
