#include "mesh.hpp"

#include "../app/application.hpp"

namespace Physbuzz {

VertexDescription::VertexDescription(const Info &info)
    : m_Info(info) {
    m_Binding = {
        .binding = m_Info.binding,
        .stride = m_Info.size,
        .inputRate = m_Info.inputRate,
    };

    m_Attributes.clear();
    for (std::size_t i = 0; i < m_Info.attributes.size(); i++) {
        m_Attributes.push_back({
            .location = static_cast<std::uint32_t>(i),
            .binding = m_Info.binding,
            .format = m_Info.attributes[i].format,
            .offset = m_Info.attributes[i].offset,
        });
    }

    m_VertexInputStateCreateInfo = {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &m_Binding,
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(m_Attributes.size()),
        .pVertexAttributeDescriptions = m_Attributes.data(),
    };
}

bool Mesh::build() {
    if (m_Vertex.buffer != nullptr) {
        Logger::WARNING("[Mesh] Trying to build a constructed mesh.");
        return true;
    }

    m_Vertex.buffer = PBZ_VK_CHECK(App::Device.createBuffer({
        .size = m_Vertices.size(),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive,
    }));

    m_Index.buffer = PBZ_VK_CHECK(App::Device.createBuffer({
        .size = m_Indices.size(),
        .usage = vk::BufferUsageFlagBits::eIndexBuffer,
        .sharingMode = vk::SharingMode::eExclusive,
    }));

    vk::MemoryRequirements memRequirements = App::Device.getBufferMemoryRequirements(m_Vertex.buffer);

    m_Vertex.memory = PBZ_VK_CHECK(App::Device.allocateMemory({
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
    }));

    m_Index.memory = PBZ_VK_CHECK(App::Device.allocateMemory({
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
    }));

    void *vertex = PBZ_VK_CHECK(App::Device.mapMemory(m_Vertex.memory, 0, m_Vertices.size()));
    memcpy(vertex, m_Vertices.data(), m_Vertices.size());
    App::Device.unmapMemory(m_Vertex.memory);

    vk::Result result = App::Device.bindBufferMemory(m_Vertex.buffer, m_Vertex.memory, 0);

    if (result != vk::Result::eSuccess) {
        Logger::CRITICAL("[Mesh] Failed to bind vertex memory");
    }

    void *index = PBZ_VK_CHECK(App::Device.mapMemory(m_Index.memory, 0, m_Indices.size()));
    memcpy(index, m_Indices.data(), m_Indices.size());
    App::Device.unmapMemory(m_Index.memory);

    return true;
}

bool Mesh::destroy() {
    if (m_Vertex.buffer == nullptr) {
        Logger::WARNING("[Mesh] Trying to destroy a destructed mesh.");
        return true;
    }

    App::Device.freeMemory(m_Index.memory);
    App::Device.destroyBuffer(m_Index.buffer);
    m_Index = {
        .buffer = nullptr,
        .memory = nullptr,
    };

    App::Device.freeMemory(m_Vertex.memory);
    App::Device.destroyBuffer(m_Vertex.buffer);
    m_Vertex = {
        .buffer = nullptr,
        .memory = nullptr,
    };

    return true;
}

void Mesh::draw(const vk::CommandBuffer &commandBuffer) const {
    commandBuffer.bindVertexBuffers(0, {m_Vertex.buffer}, {0});
    commandBuffer.draw(m_VertexCount, 1, 0, 0);
}

std::uint32_t Mesh::findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = App::PhysicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Logger::CRITICAL("[Mesh] failed to find suitable memory type!");
}

const VertexDescription *Mesh::getDescription() const {
    return m_Description;
}

} // namespace Physbuzz
