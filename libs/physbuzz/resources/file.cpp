#include "file.hpp"

#include "../debug/logging.hpp"
#include <format>
#include <fstream>

namespace Physbuzz {

FileResource::FileResource(const FileInfo &file)
    : m_Info(file) {}

FileResource::~FileResource() {}

void FileResource::build() {}

void FileResource::destroy() {}

void FileResource::read() {
    std::ifstream stream = std::ifstream(m_Info.path, std::ios::in | std::ios::binary);

    if (!stream.is_open()) {
        Logger::WARNING(std::format("[File] Failed to open file: {}", m_Info.path));
        return;
    }

    stream.seekg(0, std::ios::end);
    m_Size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    buffer.resize(m_Size);
    stream.read(buffer.data(), buffer.size());
    buffer.push_back('\0');

    if (stream.fail()) {
        Logger::WARNING(std::format("[File] Failed to read file: {}", m_Info.path));
        return;
    }

    stream.close();
}

void FileResource::write() {
    std::ofstream stream = std::ofstream(m_Info.path, std::ios::out | std::ios::binary);

    if (!stream.is_open()) {
        Logger::WARNING(std::format("[File] Failed to open file: {}", m_Info.path));
        return;
    }

    stream.write(buffer.data(), buffer.size());

    if (stream.fail()) {
        Logger::WARNING(std::format("[File] Failed to write file: {}", m_Info.path));
        return;
    }

    stream.close();
    if (stream.bad()) {
        Logger::WARNING(std::format("[File] Failed to close file: {}", m_Info.path));
        return;
    }
}

const std::streampos &FileResource::getSize() const {
    return m_Size;
}

} // namespace Physbuzz
