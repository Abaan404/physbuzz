#pragma once

#include "../ecs/system.hpp"
#include "../app/deletion.hpp"

namespace Physbuzz {

class Transfer : public System<> {
  public:
    Transfer();

    bool build() override;
    bool destroy() override;

    bool map(const Buffer &buffer, const std::span<const std::byte> &bytes, std::uint64_t offset);
    bool map(const Image &image, const std::span<const std::byte> &bytes, vk::ImageLayout layout);
    void immediate(std::function<void(vk::CommandBuffer)> record);

  private:
    DeletionQueue m_Deletion;

    struct {
        vk::CommandPool pool;
        vk::CommandBuffer buffer;
    } m_Command;

    struct {
        vk::Fence submit;
    } m_Fences;
};

} // namespace Physbuzz
