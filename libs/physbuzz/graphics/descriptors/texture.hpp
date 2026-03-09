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

    struct Info {
        Type type;
        Sampler sampler = {{Sampler::Type::None}};
        Format format = Format::eR8G8B8A8Unorm;
    };

    struct Data {
        Image image = {{}};

        vk::ImageView view = nullptr;
        vk::ImageSubresourceRange subresourceRange = {};
    };

    Texture(const Info &info);

    bool build(const glm::uvec3 &resolution);
    bool destroy();

    bool write(const ImageFile::Info &imageFile, TransferBatch &batch) const;
    bool write(std::vector<std::byte> &&bytes, TransferBatch &batch) const;

    bool rebuild(const RenderContext &context, const glm::uvec3 &size);

    const Info &getInfo() const;
    const Data &getData() const;

    glm::uvec3 getSize() const;

  private:
    std::tuple<vk::ImageView, vk::ImageSubresourceRange> createImageView(const Image &image) const;

    Info m_Info;
    Data m_Data;
};

template <>
struct IsResource<Texture> : std::true_type {};

} // namespace Physbuzz
