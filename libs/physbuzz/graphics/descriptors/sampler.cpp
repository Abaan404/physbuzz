#include "sampler.hpp"

#include "../../app/application.hpp"
#include "../../app/deletion.hpp"
#include "../../debug/macros.hpp"

namespace Physbuzz {

Sampler::Sampler(const Info &info)
    : m_Info(info) {}

bool Sampler::build() {
    m_Data = {
        .sampler = createSampler(),
    };

    return true;
}

bool Sampler::destroy() {
    // Note: samplers are destroyed on engine shutdown, could refcount it but unnecessary for this project's scope
    m_Data = {
        .sampler = nullptr,
    };

    return true;
}

const Sampler::Info &Sampler::getInfo() const {
    return m_Info;
}

const Sampler::Data &Sampler::getData() const {
    return m_Data;
}

vk::Sampler Sampler::createSampler() const {
    switch (m_Info.type) {
    case Type::Linear:
        static vk::Sampler linear = PBZ_VK_CHECK(App::Device.createSampler({
            .flags = {},
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .mipLodBias = 0.0f,
            .anisotropyEnable = vk::True,
            .maxAnisotropy = App::PhysicalDeviceProperties.limits.maxSamplerAnisotropy,
            .compareEnable = vk::False,
            .compareOp = vk::CompareOp::eAlways,
            .minLod = 0.0f,
            .maxLod = 1.0f,
            .borderColor = vk::BorderColor::eIntOpaqueBlack,
            .unnormalizedCoordinates = vk::False,
        }));

        static std::once_flag flag;
        std::call_once(flag, []() {
            App::Deletion.enqueue(linear);
        });

        return linear;

    case Type::None:
        return nullptr;
    }

    return nullptr;
}

} // namespace Physbuzz
