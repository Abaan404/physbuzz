#include "file.hpp"

#include "../debug/logging.hpp"
#include <fstream>

namespace Physbuzz {

File::File(const Info &file)
    : m_Info(file) {}

bool File::build() {
    return true;
}

bool File::destroy() {
    return true;
}

bool File::read() {
    std::ifstream stream = std::ifstream(m_Info.path, std::ios::in | std::ios::binary);

    if (!stream.is_open()) {
        Logger::ERROR("[File] Failed to open file: {}", m_Info.path.string());
        return false;
    }

    stream.seekg(0, std::ios::end);
    m_Data.size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    m_Data.buffer.resize(m_Data.size);
    stream.read(m_Data.buffer.data(), m_Data.buffer.size());
    if (*m_Data.buffer.end() != '\0') {
        m_Data.buffer.push_back('\0');
    }

    if (stream.fail()) {
        Logger::ERROR("[File] Failed to read file: {}", m_Info.path.string());
        return false;
    }

    return true;
}

bool File::write() {
    std::ofstream stream = std::ofstream(m_Info.path, std::ios::out | std::ios::binary);

    if (!stream.is_open()) {
        Logger::ERROR("[File] Failed to open file: {}", m_Info.path.string());
        return false;
    }

    stream.write(m_Data.buffer.data(), m_Data.buffer.size());

    if (stream.fail()) {
        Logger::ERROR("[File] Failed to write file: {}", m_Info.path.string());
        return false;
    }

    return true;
}

const File::Data &File::getData() const {
    return m_Data;
}

const File::Info &File::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
