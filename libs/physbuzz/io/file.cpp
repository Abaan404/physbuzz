#include "file.hpp"

#include "logging.hpp"
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
    std::ifstream stream(m_Info.path, std::ios::in | std::ios::binary);
    if (!stream.is_open()) {
        Logger::ERROR("[File] Failed to open file: {}", m_Info.path.string());
        return false;
    }

    stream.seekg(0, std::ios::end);
    m_Data.buffer.resize(stream.tellg());
    stream.seekg(0, std::ios::beg);

    stream.read(reinterpret_cast<char *>(m_Data.buffer.data()), m_Data.buffer.size());

    if (!stream) {
        Logger::ERROR("[File] Failed to read file: {}", m_Info.path.string());
        m_Data.buffer.clear();
        return false;
    }

    return true;
}

bool File::write(const Data &data) {
    std::ofstream stream = std::ofstream(m_Info.path, std::ios::out | std::ios::binary);

    if (!stream.is_open()) {
        Logger::ERROR("[File] Failed to open file: {}", m_Info.path.string());
        return false;
    }

    stream.write(reinterpret_cast<const char *>(data.buffer.data()), data.buffer.size());

    if (stream.fail()) {
        Logger::ERROR("[File] Failed to write file: {}", m_Info.path.string());
        return false;
    }

    m_Data = data;

    return true;
}

const File::Data &File::getData() const {
    return m_Data;
}

const File::Info &File::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
