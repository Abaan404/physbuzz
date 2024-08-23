#include "shaders.hpp"

#include "../debug/logging.hpp"
#include <bitset>
#include <vector>

namespace Physbuzz {

Shader::Shader(const ShaderInfo &info, const ShaderType &type)
    : m_Info(info), m_Type(type) {}

Shader::~Shader() {}

bool Shader::build() {
    if (m_Shader != 0) {
        Logger::WARNING("[Shader] Trying to build a constructed shader '{}'.", m_Info.file.path.string());
    }

    m_Shader = glCreateShader(static_cast<GLuint>(m_Type));
    return true;
}

bool Shader::destroy() {
    if (m_Shader == 0) {
        Logger::WARNING("[Shader] Trying to destroy a constructed shader '{}'.", m_Info.file.path.string());
        return false;
    }

    glDeleteShader(m_Shader);
    m_Shader = 0;
    return true;
}

bool Shader::compile() {
    if (m_Info.file.path.empty()) {
        return false;
    }

    if (m_Shader == 0) {
        Logger::ERROR("[Shader] Trying to compile a destructed shader '{}'.", m_Info.file.path.string());
        return false;
    }

    FileResource file = FileResource(m_Info.file);
    if (!file.build()) {
        Logger::ERROR("[Shader] Could not build file '{}'", m_Info.file.path.string());
        return false;
    }

    if (!file.read()) {
        Logger::ERROR("[Shader] Could not read file '{}'", m_Info.file.path.string());
        file.destroy();
        return false;
    }

    Logger::INFO("[Shader] Compiling shader {}", m_Info.file.path.string());
    preprocess(file);
    const char *source = file.buffer.data();
    glShaderSource(m_Shader, 1, &source, NULL);
    glCompileShader(m_Shader);
    file.destroy();

    GLint result;
    glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetShaderInfoLog(m_Shader, logLength, NULL, errorMessage.data());

        Logger::ERROR(std::format("[Shader] Shader compilation failed!\n{}", errorMessage.data()));
        return false;
    }

    return true;
}

void Shader::preprocess(FileResource &file) {
    if (file.buffer.empty()) {
        return;
    }

    std::unordered_map<std::string, std::function<bool(FileResource &, std::size_t)>> directives = {
        {"pbz_include ", std::bind(&Shader::preprocessInclude, this, std::placeholders::_1, std::placeholders::_2)},
    };

    for (std::size_t position = 0; position < file.buffer.size(); position = file.buffer.find('#', position + 1)) {
        if (position >= file.buffer.size()) {
            break;
        }

        for (const auto &[directive, func] : directives) {
            if (file.buffer.compare(position + 1, directive.size(), directive) == 0) {
                if (position != 0 && file.buffer[position - 1] != '\n') {
                    break;
                }

                if (!func(file, position)) {
                    std::string line = file.buffer.substr(position, std::min(file.buffer.find('\n', position), file.buffer.size()) - position);
                    Logger::ERROR("[Shader] Could not parse directive '{}'", line);
                }
            }
        }
    }
}

bool Shader::preprocessInclude(FileResource &file, std::size_t position) {
    std::size_t pathBegin = file.buffer.find('\"', position) + 1;
    if (pathBegin >= file.buffer.size()) {
        return false;
    }

    std::size_t pathEnd = file.buffer.find('\"', pathBegin + 1);
    if (pathEnd >= file.buffer.size()) {
        return false;
    }

    std::filesystem::path cwdIncludePath = file.buffer.substr(pathBegin, pathEnd - pathBegin);
    std::filesystem::path relativeIncludePath = file.getPath().parent_path() / cwdIncludePath;

    // check relative path first, the cwd path
    std::filesystem::path includePath;
    if (std::filesystem::is_regular_file(relativeIncludePath)) {
        includePath = relativeIncludePath;
    } else if (std::filesystem::is_regular_file(cwdIncludePath)) {
        includePath = cwdIncludePath;
    }

    // could not locate file
    if (includePath.empty()) {
        return false;
    }

    std::string top = file.buffer.substr(0, position);
    std::string bottom = file.buffer.substr(
        std::min(file.buffer.find('\n', position), file.buffer.size()),
        file.buffer.size());

    // guard headers by default
    if (m_IncludedPaths.contains(includePath)) {
        file.buffer = top + bottom;
        return true;
    }

    FileResource includeFile = FileResource({
        .path = includePath,
    });

    if (!includeFile.build() || !includeFile.read()) {
        Logger::ERROR("[Shader] Could not process file '{}'", includeFile.getPath().string());
        return false;
    }
    m_IncludedPaths.insert(includePath);

    // recursively apply directives to included path
    preprocess(includeFile);
    includeFile.buffer[includeFile.buffer.size()] = ' '; // pop null terminator
    file.buffer = top + includeFile.buffer + bottom;

    includeFile.destroy();

    return true;
}

bool Shader::attach(GLuint program) const {
    if (m_Shader == 0) {
        return false;
    }

    glAttachShader(program, m_Shader);
    return true;
}

bool Shader::detach(GLuint program) const {
    if (m_Shader == 0) {
        return false;
    }

    glDetachShader(program, m_Shader);
    return true;
}

const GLuint &Shader::getShader() const {
    return m_Shader;
}

const ShaderType &Shader::getType() const {
    return m_Type;
}

const std::set<std::filesystem::path> Shader::getIncludedPaths() const {
    return m_IncludedPaths;
}

template <std::size_t N>
inline std::bitset<N> buildShaders(std::array<Shader, N> &shaders, const GLuint &program) {
    std::bitset<N> compiled = 0;

    for (int i = 0; i < shaders.size(); i++) {
        shaders[i].build();

        if (!shaders[i].compile()) {
            shaders[i].destroy();
            continue;
        }

        compiled[i] = true;
        shaders[i].attach(program);
    }

    return compiled;
}

template <std::size_t N>
inline void destroyShaders(std::array<Shader, N> &shaders, const GLuint &program, const std::bitset<N> &compiled) {
    for (int i = 0; i < shaders.size(); i++) {
        if (!compiled[i]) {
            continue;
        }

        shaders[i].detach(program);
        shaders[i].destroy();
    }
}

ShaderPipelineResource::ShaderPipelineResource(const ShaderPipelineInfo &info)
    : m_Info(info) {}

ShaderPipelineResource::~ShaderPipelineResource() {}

bool ShaderPipelineResource::build() {
    if (m_Program != 0) {
        Logger::WARNING("[ShaderPipelineResource] Trying to build a constructed pipeline.");
    }

    std::array shaders = {
        Shader(m_Info.vertex, ShaderType::Vertex),
        Shader(m_Info.tessControl, ShaderType::TessControl),
        Shader(m_Info.tessEvaluation, ShaderType::TessEvaluation),
        Shader(m_Info.geometry, ShaderType::Geometry),
        Shader(m_Info.fragment, ShaderType::Fragment),
        Shader(m_Info.compute, ShaderType::Compute),
    };

    m_Program = glCreateProgram();
    std::bitset compiled = buildShaders(shaders, m_Program);

    if (compiled[5]) {
        Logger::ERROR("[ShaderPipelineResource] Compute shaders are not supported by the engine.");
        destroyShaders(shaders, m_Program, compiled);
        destroy();
        return false;
    }

    if (!compiled[0]) {
        Logger::ERROR("[ShaderPipelineResource] Could not compile vertex shader");
        destroyShaders(shaders, m_Program, compiled);
        destroy();
        return false;
    }

    if (!compiled[4]) {
        Logger::ERROR("[ShaderPipelineResource] Could not compile fragment shader.");
        destroyShaders(shaders, m_Program, compiled);
        destroy();
        return false;
    }

    glLinkProgram(m_Program);
    glValidateProgram(m_Program);
    destroyShaders(shaders, m_Program, compiled);

    GLint result;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetProgramInfoLog(m_Program, logLength, NULL, errorMessage.data());

        Logger::ERROR(std::format("[ShaderPipelineResource] Shader Linking failed!\n{}", errorMessage.data()));
        destroy();
        return false;
    }

    // add to watched paths
    m_Paths.insert(m_Info.vertex.file.path);
    m_Paths.insert(m_Info.tessControl.file.path);
    m_Paths.insert(m_Info.tessEvaluation.file.path);
    m_Paths.insert(m_Info.geometry.file.path);
    m_Paths.insert(m_Info.fragment.file.path);
    m_Paths.insert(m_Info.compute.file.path);

    for (int i = 0; i < shaders.size(); i++) {
        if (!compiled[i]) {
            continue;
        }

        std::set<std::filesystem::path> includedPaths = shaders[i].getIncludedPaths();
        m_Paths.merge(includedPaths);
    }

    return true;
}

bool ShaderPipelineResource::destroy() {
    if (m_Program == 0) {
        Logger::WARNING("[ShaderPipelineResource] Trying to destroy a destructed pipeline.");
    }

    glDeleteProgram(m_Program);
    m_Program = 0;
    return true;
}

bool ShaderPipelineResource::reload() {
    if (!m_RequestedReload) {
        // no reload was necessary, expected behaviour
        return true;
    }

    m_RequestedReload = false;

    if (!destroy()) {
        Logger::ERROR("[ShaderPipelineResource] Reload stage destroy() failed.");
        return false;
    }

    if (!build()) {
        Logger::ERROR("[ShaderPipelineResource] Reload stage build() failed.");
        return false;
    }

    return true;
}

void ShaderPipelineResource::requestReload() {
    m_RequestedReload = true;
}

bool ShaderPipelineResource::bind() const {
    if (m_Program == 0) {
        return false;
    }

    glUseProgram(m_Program);
    return true;
}

bool ShaderPipelineResource::unbind() const {
    if (m_Program == 0) {
        return false;
    }

    glUseProgram(0); // Undefined Behaviour, upto the user to use the right shader
    return true;
}

const GLuint &ShaderPipelineResource::getProgram() const {
    return m_Program;
}

void ShaderPipelineResource::setUniformInternal(const GLint location, const float &data) const { glUniform1fv(location, 1, &data); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::vec2 &data) const { glUniform2fv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::vec3 &data) const { glUniform3fv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::vec4 &data) const { glUniform4fv(location, 1, &data[0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const int &data) const { glUniform1iv(location, 1, &data); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::ivec2 &data) const { glUniform2iv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::ivec3 &data) const { glUniform3iv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::ivec4 &data) const { glUniform4iv(location, 1, &data[0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const unsigned int &data) const { glUniform1uiv(location, 1, &data); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::uvec2 &data) const { glUniform2uiv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::uvec3 &data) const { glUniform3uiv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::uvec4 &data) const { glUniform4uiv(location, 1, &data[0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat2 &data) const { glUniformMatrix2fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat3 &data) const { glUniformMatrix3fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat4 &data) const { glUniformMatrix4fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat2x3 &data) const { glUniformMatrix2x3fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat3x2 &data) const { glUniformMatrix3x2fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat2x4 &data) const { glUniformMatrix2x4fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat4x2 &data) const { glUniformMatrix4x2fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat3x4 &data) const { glUniformMatrix3x4fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat4x3 &data) const { glUniformMatrix4x3fv(location, 1, GL_FALSE, &data[0][0]); }

template <>
bool ResourceContainer<ShaderPipelineResource>::insert(const std::string &identifier, ShaderPipelineResource &&resource) {
    resource.m_WatchId = FileWatcher::insert([identifier](const FileWatcherInfo &info) {
        ShaderPipelineResource *resource = ResourceRegistry::get<ShaderPipelineResource>(identifier);

        if (info.action != WatchAction::Modified) {
            return;
        }

        if (std::any_of(resource->m_Paths.begin(), resource->m_Paths.end(), [&](const std::filesystem::path &path) { return path.filename() == info.path.filename(); })) {
            // OpenGL's context must exist in the main thread (not necessarily but adds too much complexity)
            // hence why the creation is not done in this watcher thread
            resource->requestReload();
        }
    });

    return base_insert(identifier, std::move(resource));
}

template <>
bool ResourceContainer<ShaderPipelineResource>::erase(const std::string &identifier) {
    ShaderPipelineResource *resource = get(identifier);
    FileWatcher::erase(resource->m_WatchId);

    return base_erase(identifier);
}

} // namespace Physbuzz
