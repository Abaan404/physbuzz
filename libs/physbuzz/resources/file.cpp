#include "file.hpp"

#include "../debug/logging.hpp"
#include <fstream>

namespace Physbuzz {

FileResource::FileResource(const FileInfo &file)
    : m_Info(file) {}

FileResource::~FileResource() {}

bool FileResource::build() {
    return true;
}

bool FileResource::destroy() {
    return true;
}

bool FileResource::read() {
    std::ifstream stream = std::ifstream(m_Info.path, std::ios::in | std::ios::binary);

    if (!stream.is_open()) {
        Logger::ERROR("[FileResource] Failed to open file: {}", m_Info.path.string());
        return false;
    }

    stream.seekg(0, std::ios::end);
    m_Size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    buffer.resize(m_Size);
    stream.read(buffer.data(), buffer.size());
    buffer.push_back('\0');

    if (stream.fail()) {
        Logger::ERROR("[FileResource] Failed to read file: {}", m_Info.path.string());
        return false;
    }

    return true;
}

bool FileResource::write() {
    std::ofstream stream = std::ofstream(m_Info.path, std::ios::out | std::ios::binary);

    if (!stream.is_open()) {
        Logger::ERROR("[FileResource] Failed to open file: {}", m_Info.path.string());
        return false;
    }

    stream.write(buffer.data(), buffer.size());

    if (stream.fail()) {
        Logger::ERROR("[FileResource] Failed to write file: {}", m_Info.path.string());
        return false;
    }

    return true;
}

const std::streampos &FileResource::getSize() const {
    return m_Size;
}

} // namespace Physbuzz
