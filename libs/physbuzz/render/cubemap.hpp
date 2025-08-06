#pragma once

#include "../io/image.hpp"
#include "../resources/defines.hpp"

namespace Physbuzz {

class Cubemap {
  public:
    struct Info {
        ImageFile::Info right;
        ImageFile::Info left;
        ImageFile::Info top;
        ImageFile::Info bottom;
        ImageFile::Info back;
        ImageFile::Info front;
    };

    Cubemap(const Info &info);
    ~Cubemap();

    bool build();
    bool destroy();

    std::int32_t activate(std::int32_t unit = -1) const;

    const Info &getInfo() const;

  private:
    Info m_Info;
    std::int32_t m_Texture = 0;
};

template <>
struct IsResource<Cubemap> : std::true_type {};

} // namespace Physbuzz
