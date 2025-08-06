#pragma once

#include "../io/image.hpp"
#include "../resources/defines.hpp"

namespace Physbuzz {

enum class TextureType;

class Texture2D {
  public:
    struct Info {
        ImageFile::Info image;
        TextureType type;
    };

    Texture2D(const Info &texture2D);
    ~Texture2D();

    bool build();
    bool destroy();

    std::int32_t activate(std::int32_t unit = -1) const;

    const Info &getInfo() const;
    std::uint32_t getImGuiTextureHandle() const;

  private:
    Info m_Info;
    std::uint32_t m_Texture = 0;
};

template <>
struct IsResource<Texture2D> : std::true_type {};

} // namespace Physbuzz
