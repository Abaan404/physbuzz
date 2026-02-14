#pragma once

#include "../../resources/defines.hpp"
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Sampler {
  public:
    enum class Type : std::size_t {
        Linear = 0,
        None = 1,
    };

    struct Info {
        Type type;
    };

    struct Data {
        vk::Sampler sampler;
    };

    Sampler(const Info &info);

    bool build();
    bool destroy();

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info;
    Data m_Data;

    vk::Sampler createSampler() const;
};

template <>
struct IsResource<Sampler> : std::true_type {};

} // namespace Physbuzz
