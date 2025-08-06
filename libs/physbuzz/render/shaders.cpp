#include "shaders.hpp"

#include "../debug/logging.hpp"
#include <bitset>
#include <glm/gtc/type_ptr.hpp>

namespace Physbuzz {

Shader::Shader(const Info &info, const Type &type)
    : m_Info(info), m_Type(type) {}

Shader::~Shader() {}

bool Shader::build() {
    // if (m_Shader != 0) {
    //     Logger::WARNING("[Shader] Trying to build a constructed shader '{}'.", m_Info.file.path.string());
    //     return true;
    // }
    //
    // m_Shader = glCreateShader(static_cast<GLenum>(m_Type));
    return true;
}

bool Shader::destroy() {
    // if (m_Shader == 0) {
    //     Logger::WARNING("[Shader] Trying to destroy a constructed shader '{}'.", m_Info.file.path.string());
    //     return true;
    // }
    //
    // glDeleteShader(m_Shader);
    // m_Shader = 0;
    return true;
}

bool Shader::compile() {
    // if (m_Info.file.path.empty()) {
    //     return false;
    // }
    //
    // if (m_Shader == 0) {
    //     Logger::ERROR("[Shader] Trying to compile a destructed shader '{}'.", m_Info.file.path.string());
    //     return false;
    // }
    //
    // File file = File(m_Info.file);
    // if (!file.build()) {
    //     Logger::ERROR("[Shader] Could not build file '{}'", m_Info.file.path.string());
    //     return false;
    // }
    //
    // if (!file.read()) {
    //     Logger::ERROR("[Shader] Could not read file '{}'", m_Info.file.path.string());
    //     file.destroy();
    //     return false;
    // }
    //
    // Logger::INFO("[Shader] Compiling shader {}", m_Info.file.path.string());
    //
    // std::string output = preprocess(file);
    // const char *source = output.data();
    // glShaderSource(m_Shader, 1, &source, NULL);
    // glCompileShader(m_Shader);
    // file.destroy();
    //
    // std::int32_t result;
    // glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &result);
    // if (result == GL_FALSE) {
    //     std::int32_t logLength;
    //     glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &logLength);
    //
    //     std::vector<char> errorMessage(logLength + 1);
    //     glGetShaderInfoLog(m_Shader, logLength, NULL, errorMessage.data());
    //
    //     Logger::ERROR("[Shader] Shader compilation failed!\n{}", errorMessage.data());
    //     return false;
    // }
    //
    // m_Paths.insert(std::filesystem::canonical(m_Info.file.path));
    return true;
}

const std::string Shader::preprocess(const File &file) {
    const File::Data &data = file.getData();

    if (data.buffer.empty()) {
        return "";
    }

    std::string output = data.buffer;

    std::unordered_map<std::string, std::function<bool(const File &, std::string &buffer, std::size_t)>> directives = {
        {"pbz_include ", std::bind(&Shader::preprocessInclude, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
    };

    for (std::size_t position = 0; position < output.size(); position = output.find('#', position + 1)) {
        if (position >= output.size()) {
            break;
        }

        for (const auto &[directive, func] : directives) {
            if (output.compare(position + 1, directive.size(), directive) == 0) {
                if (position != 0 && output[position - 1] != '\n') {
                    break;
                }

                if (!func(file, output, position)) {
                    std::string line = output.substr(position, std::min(output.find('\n', position), output.size()) - position);
                    Logger::ERROR("[Shader] Could not parse directive '{}'", line);
                }
            }
        }
    }

    return output;
}

bool Shader::preprocessInclude(const File &file, std::string &output, std::size_t position) {
    std::size_t pathBegin = output.find('\"', position) + 1;
    if (pathBegin >= output.size()) {
        return false;
    }

    std::size_t pathEnd = output.find('\"', pathBegin + 1);
    if (pathEnd >= output.size()) {
        return false;
    }

    std::filesystem::path cwdIncludePath = output.substr(pathBegin, pathEnd - pathBegin);
    std::filesystem::path relativeIncludePath = file.getInfo().path.parent_path() / cwdIncludePath;

    // check relative path first, the cwd path
    std::filesystem::path path;
    if (std::filesystem::is_regular_file(relativeIncludePath)) {
        path = relativeIncludePath;
    } else if (std::filesystem::is_regular_file(cwdIncludePath)) {
        path = cwdIncludePath;
    }

    // could not locate file
    if (path.empty()) {
        return false;
    }

    path = std::filesystem::canonical(path);

    std::string top = output.substr(0, position);
    std::string bottom = output.substr(
        std::min(output.find('\n', position), output.size()),
        output.size());

    // guard headers by default
    if (m_Paths.contains(path)) {
        output = top + bottom;
        return true;
    }

    File include = File({
        .path = path,
    });

    if (!include.build() || !include.read()) {
        Logger::ERROR("[Shader] Could not process file '{}'", path.string());
        return false;
    }
    m_Paths.insert(path);

    output = top + preprocess(include) + bottom;

    include.destroy();

    return true;
}

void Shader::attach(std::uint32_t program) const {
    // PBZ_ASSERT(m_Shader != 0, "[Shader] trying to attach an incomplete shader to a pipeline.");
    // glAttachShader(program, m_Shader);
}

void Shader::detach(std::uint32_t program) const {
    // PBZ_ASSERT(m_Shader != 0, "[Shader] trying to detach an incomplete shader to a pipeline.");
    // glDetachShader(program, m_Shader);
}

template <std::size_t N>
inline void destroyShaders(std::array<Shader, N> &shaders, const std::uint32_t &program, const std::bitset<N> &compiled) {
    for (int i = 0; i < shaders.size(); i++) {
        if (!compiled[i]) {
            continue;
        }

        shaders[i].detach(program);
        shaders[i].destroy();
    }
}

ShaderPipeline::ShaderPipeline(const Info &info)
    : m_Info(info) {}

ShaderPipeline::~ShaderPipeline() {}

bool ShaderPipeline::build() {
    // if (m_Program != 0) {
    //     Logger::WARNING("[ShaderPipeline] Trying to build a constructed pipeline.");
    //     return true;
    // }
    //
    // std::array shaders = {
    //     Shader(m_Info.vertex, Shader::Type::Vertex),
    //     Shader(m_Info.tessControl, Shader::Type::TessControl),
    //     Shader(m_Info.tessEvaluation, Shader::Type::TessEvaluation),
    //     Shader(m_Info.geometry, Shader::Type::Geometry),
    //     Shader(m_Info.fragment, Shader::Type::Fragment),
    //     Shader(m_Info.compute, Shader::Type::Compute),
    // };
    //
    // m_Program = glCreateProgram();
    //
    // std::bitset<6> compiled;
    // for (int i = 0; i < shaders.size(); i++) {
    //     shaders[i].build();
    //
    //     if (!shaders[i].compile()) {
    //         shaders[i].destroy();
    //         continue;
    //     }
    //
    //     compiled[i] = true;
    //     shaders[i].attach(m_Program);
    // }
    //
    // if (compiled[5]) {
    //     Logger::ERROR("[ShaderPipeline] Compute shaders are not supported by the engine.");
    //     destroyShaders(shaders, m_Program, compiled);
    //     destroy();
    //     return false;
    // }
    //
    // if (!compiled[0]) {
    //     Logger::ERROR("[ShaderPipeline] Could not compile vertex shader");
    //     destroyShaders(shaders, m_Program, compiled);
    //     destroy();
    //     return false;
    // }
    //
    // if (!compiled[4]) {
    //     Logger::ERROR("[ShaderPipeline] Could not compile fragment shader.");
    //     destroyShaders(shaders, m_Program, compiled);
    //     destroy();
    //     return false;
    // }
    //
    // glLinkProgram(m_Program);
    // glValidateProgram(m_Program);
    // destroyShaders(shaders, m_Program, compiled);
    //
    // std::int32_t result;
    // glGetProgramiv(m_Program, GL_LINK_STATUS, &result);
    // if (result == GL_FALSE) {
    //     std::int32_t logLength;
    //     glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &logLength);
    //
    //     std::vector<char> errorMessage(logLength + 1);
    //     glGetProgramInfoLog(m_Program, logLength, NULL, errorMessage.data());
    //
    //     Logger::ERROR("[ShaderPipeline] Shader Linking failed!\n{}", errorMessage.data());
    //     destroy();
    //     return false;
    // }
    //
    // std::set<std::filesystem::path> paths;
    // for (int i = 0; i < shaders.size(); i++) {
    //     if (!compiled[i]) {
    //         continue;
    //     }
    //
    //     std::set<std::filesystem::path> shaderPaths = shaders[i].m_Paths;
    //     paths.merge(shaderPaths);
    // }
    //
    // m_ReloadCallback = [paths](const ResourceWatcherData &event) {
    //     if (!paths.contains(event.path)) {
    //         return;
    //     }
    //
    //     // OpenGL's context must exist in the main thread (not necessarily but adds too much complexity)
    //     // hence why reload cannot be done in the watcher thread
    //     Resource<ShaderPipeline>(event.identifier)->m_RequestedReload = true;
    // };
    //
    return true;
}

bool ShaderPipeline::destroy() {
    // if (m_Program == 0) {
    //     Logger::WARNING("[ShaderPipeline] Trying to destroy a destructed pipeline.");
    //     return true;
    // }
    //
    // glDeleteProgram(m_Program);
    // m_Program = 0;
    return true;
}

bool ShaderPipeline::reload() {
    if (!m_RequestedReload) {
        // no reload was necessary, expected behaviour
        return true;
    }

    m_RequestedReload = false;

    if (!m_FailedReload && !destroy()) {
        Logger::ERROR("[ShaderPipeline] Reload failed.");
        return false;
    }

    if (!build()) {
        m_FailedReload = true;
        return false;
    }

    m_FailedReload = false;
    return true;
}

void ShaderPipeline::draw(Scene &scene, ObjectID object) const {
    // dont draw this shader on a failed reload
    if (m_FailedReload) {
        return;
    }

    PBZ_ASSERT(m_Program != 0, "[ShaderPipeline] trying to draw an incomplete pipeline.");
    m_Info.draw(this, scene, object);
}

void ShaderPipeline::bind() const {
    // // dont use this shader on a failed reload
    // if (m_FailedReload) {
    //     return;
    // }
    //
    // PBZ_ASSERT(m_Program != 0, "[ShaderPipeline] trying to bind an incomplete pipeline.");
    // glUseProgram(m_Program);
}

void ShaderPipeline::unbind() const {
    // glUseProgram(0);
}

const ShaderPipeline::Info &ShaderPipeline::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
