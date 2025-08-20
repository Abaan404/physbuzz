#include "buffer.hpp"

namespace Physbuzz {

Buffer::Buffer(const Info &info)
    : m_Info(info) {}

bool Buffer::build() {
    m_Data.buffer = PBZ_VK_CHECK(App::Device.createBuffer({
        .size = m_Info.size,
        .usage = m_Info.usage,
        .sharingMode = m_Info.sharingMode,
    }));

    vk::MemoryRequirements memRequirements = App::Device.getBufferMemoryRequirements(m_Data.buffer);

    m_Data.memory = PBZ_VK_CHECK(App::Device.allocateMemory({
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, m_Info.properties),
    }));

    vk::Result result = App::Device.bindBufferMemory(m_Data.buffer, m_Data.memory, 0);

    if (result != vk::Result::eSuccess) {
        Logger::ERROR("[Mesh] Failed to bind vertex memory");
        destroy();
        return false;
    }

    return true;
}

bool Buffer::destroy() {
    App::Device.freeMemory(m_Data.memory);
    App::Device.destroyBuffer(m_Data.buffer);

    m_Data = {
        .buffer = nullptr,
        .memory = nullptr,
    };

    return true;
}

const Buffer::Info &Buffer::getInfo() const {
    return m_Info;
}

const Buffer::Data &Buffer::getData() const {
    return m_Data;
}

std::uint32_t Buffer::findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = App::PhysicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Logger::CRITICAL("[Mesh] failed to find suitable memory type!");
}

bool Transfer::build() {
    m_Command.pool = PBZ_VK_CHECK(App::Device.createCommandPool({
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = App::Indices.transfer,
    }));

    std::vector<vk::CommandBuffer> buffers = PBZ_VK_CHECK(App::Device.allocateCommandBuffers({
        .commandPool = m_Command.pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    }));

    m_Command.buffer = buffers.front();

    m_Fences.submit = PBZ_VK_CHECK(App::Device.createFence({
        .flags = vk::FenceCreateFlagBits::eSignaled,
    }));
    return true;
}

bool Transfer::destroy() {
    App::Device.destroyFence(m_Fences.submit);

    App::Device.freeCommandBuffers(m_Command.pool, 1, &m_Command.buffer);
    App::Device.destroyCommandPool(m_Command.pool);

    return true;
}

void Transfer::tick() {
    if (m_PendingCopies.empty()) {
        return; // nothing to do
    }

    {
        vk::Result result = App::Device.waitForFences(m_Fences.submit, vk::True, std::numeric_limits<std::uint64_t>::max());

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Transfer] Transfer fence failed ({})", vk::to_string(result));
            return;
        }
    }

    App::Device.resetFences(m_Fences.submit);
    m_Command.buffer.reset();

    {
        vk::Result result = m_Command.buffer.begin({
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        });

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Transfer] Transfer begin failed ({})", vk::to_string(result));
            return;
        }
    }

    std::vector<Buffer> pendingErase = {};
    for (const CopyOp &op : m_PendingCopies) {
        m_Command.buffer.copyBuffer(op.src.m_Data.buffer, op.dst.m_Data.buffer, vk::BufferCopy(0, 0, op.size));
        if (op.eraseSrc) {
            pendingErase.push_back(op.src);
        }
    }

    {
        vk::Result result = m_Command.buffer.end();

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Transfer] Transfer end failed ({})", vk::to_string(result));
            return;
        }
    }

    const vk::SubmitInfo submitInfo = {
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_Command.buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    {
        vk::Result result = App::Queues.transfer.submit(submitInfo, m_Fences.submit);

        if (result != vk::Result::eSuccess) {
            Logger::ERROR("[Renderer] Queue submission failed ({})", vk::to_string(result));
            return;
        }
    }

    for (const auto &buffer : pendingErase) {
        if (!eraseBuffer(buffer)) {
            Logger::WARNING("[Transfer] Failed to erase a buffer.");
        }
    }

    m_PendingCopies.clear();
}

std::optional<Buffer> Transfer::createBuffer(const Buffer::Info &info) {
    Buffer buffer = {info};
    if (!buffer.build()) {
        return std::nullopt;
    }

    return buffer;
}

bool Transfer::eraseBuffer(Buffer buffer) {
    return buffer.destroy();
}

bool Transfer::copy(const Buffer &src, const Buffer &dst, std::size_t size, bool eraseSrc) {
    if (!src.getData().buffer || !dst.getData().buffer || size == 0) {
        Logger::ERROR("[Transfer] Invalid copy arguments.");
        return false;
    }

    m_PendingCopies.push_back({
        .src = src,
        .dst = dst,
        .size = static_cast<vk::DeviceSize>(size),
        .eraseSrc = eraseSrc,
    });

    return true;
}

} // namespace Physbuzz
