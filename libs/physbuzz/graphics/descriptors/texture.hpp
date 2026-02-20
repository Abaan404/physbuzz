#pragma once

#include "../../events/handler.hpp"
#include "../../io/image.hpp"
#include "../../resources/defines.hpp"
#include "../defines.hpp"
#include "../memory.hpp"
#include "sampler.hpp"
#include <memory>

namespace Physbuzz {

class Renderer;

class Texture : public EventSubject {
  public:
    using Format = vk::Format;

    enum class Type {
        Cube,
        Dim2D,
    };

    struct Info {
        Type type;
        Sampler::Info sampler;
        Format format = Format::eR8G8B8A8Unorm;
    };

    struct Data {
        Sampler sampler = {{Sampler::Type::None}};
        Image image = {{}};

        vk::ImageView view = nullptr;
        vk::ImageSubresourceRange subresourceRange = {};
    };

    Texture(const Info &info);

    bool build(std::vector<ImageFile::Info> imageInfos, const std::shared_ptr<Transfer> transfer);
    bool build(const glm::uvec3 &resolution);
    bool destroy();

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
