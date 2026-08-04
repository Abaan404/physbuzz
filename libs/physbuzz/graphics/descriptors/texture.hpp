#pragma once

#include "../../events/handler.hpp"
#include "../../io/image.hpp"
#include "../../resources/defines.hpp"
#include "../defines.hpp"
#include "../memory.hpp"
#include "sampler.hpp"

namespace Physbuzz {

class Renderer;
class TransferBatch;

class Texture : public EventSubject {
  public:
    using Format = vk::Format;

    enum class Type {
        Cube,
        Dim2D,
    };

    enum class Usage {
        Sampled,
        Storage,
    };

    struct Info {
        Type type;
        Usage usage = Usage::Sampled;
        std::uint32_t mipLevels = 1;
        Format format = Format::eR16G16B16A16Sfloat;

        Sampler sampler = {{Sampler::Type::None}};
        std::vector<Image::ViewInfo> additionalViews = {};
    };

    struct Data {
        Image image = {{}};
    };

    Texture(const Info &info);

    bool build(const glm::uvec3 &resolution);
    bool destroy();

    bool rebuild(const RenderContext &context, const glm::uvec3 &size);

    bool write(const ImageFile::Info &imageFile, TransferBatch &batch);
    bool write(std::vector<std::byte> &&bytes, TransferBatch &batch);

    const Info &getInfo() const;
    const Data &getData() const;

    glm::uvec3 getSize() const;

  private:
    Info m_Info;
    Data m_Data;
};

template <>
struct IsResource<Texture> : std::true_type {};

} // namespace Physbuzz
