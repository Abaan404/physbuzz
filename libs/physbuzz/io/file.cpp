#include "file.hpp"

#include "../debug/logging.hpp"
#include <fstream>

namespace Physbuzz {

File::File(const Info &file)
    : m_Info(file) {}

File::~File() {}

bool File::build() {
    return true;
}

bool File::destroy() {
    return true;
}

bool File::read() {
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
    if (*buffer.end() != '\0') {
        buffer.push_back('\0');
    }

    if (stream.fail()) {
        Logger::ERROR("[FileResource] Failed to read file: {}", m_Info.path.string());
        return false;
    }

    return true;
}

bool File::write() {
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

const std::streampos &File::getSize() const {
    return m_Size;
}

const std::filesystem::path &File::getPath() const {
    return m_Info.path;
}

} // namespace Physbuzz
