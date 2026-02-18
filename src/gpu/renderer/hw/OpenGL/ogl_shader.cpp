#include "ogl_shader.hpp"
#include "utils/file_reader.hpp"

#include <print>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace festation;

OGLShader::OGLShader(const std::filesystem::path &filename)
{
    /** @todo */
}

OGLShader::OGLShader(const std::filesystem::path &vertShaderPath, const std::filesystem::path fragShaderPath)
{
    // std::string name = vertShaderPath.filename().string() + "_" + fragShaderPath.filename().string();
    m_shaderID = glCreateProgram();

    std::string vs = readFileAsString(vertShaderPath);
    std::string fs = readFileAsString(fragShaderPath);

    std::println("\nVERTEX SHADER: \n{}\n", vs);
    std::println("\nFRAGMENT SHADER: \n{}\n", fs);

    GLuint vsShaderID = compileShader(vs, GL_VERTEX_SHADER);
    GLuint fsShaderID = compileShader(fs, GL_FRAGMENT_SHADER);

    glLinkProgram(m_shaderID);

    glValidateProgram(m_shaderID);

    GLint result;
    glGetProgramiv(m_shaderID, GL_VALIDATE_STATUS, &result);

    if (!result) {
        GLsizei length;
        char message[1024];
        glGetProgramInfoLog(m_shaderID, sizeof(message), &length, message);
        std::println("Error validating shader program: {}!", message);
        glDeleteProgram(m_shaderID);
        exit(1);
    }

    glDetachShader(m_shaderID, vsShaderID);
    glDetachShader(m_shaderID, fsShaderID);
    glDeleteShader(vsShaderID);
    glDeleteShader(fsShaderID);
}

OGLShader::~OGLShader()
{
    glDeleteProgram(m_shaderID);
}

auto OGLShader::apply() -> void
{
    glUseProgram(m_shaderID);
}

template<>
void OGLShader::ShaderDataSetter::operator()<GLfloat>(uint32_t id, const std::string &name, 
    const GLfloat &data)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform1f(id, location, data);
}

template<>
void OGLShader::ShaderDataSetter::operator()<GLuint>(uint32_t id, const std::string &name, 
    const GLuint &data)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform1ui(id, location, data);
}

template<>
void OGLShader::ShaderDataSetter::operator()<GLint>(uint32_t id, const std::string &name, 
    const GLint &data)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform1i(id, location, data);
}

template<>
void OGLShader::ShaderDataSetter::operator()<glm::vec2>(uint32_t id, const std::string &name, 
    const glm::vec2 &data)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform2f(id, location, data.x, data.y);
}

template<>
void OGLShader::ShaderDataSetter::operator()<glm::vec3>(uint32_t id, const std::string &name, 
    const glm::vec3 &data)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform3f(id, location, data.x, data.y, data.z);
}

template<>
void OGLShader::ShaderDataSetter::operator()<glm::vec4>(uint32_t id, const std::string &name, 
    const glm::vec4 &data)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform4f(id, location, data.x, data.y, data.z, data.w);
}

template<>
void OGLShader::ShaderDataSetter::operator()<glm::mat4>(uint32_t id, const std::string &name, 
    const glm::mat4 &data)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniformMatrix4fv(id, location, 1, GL_FALSE, glm::value_ptr(data));
}

template<>
void OGLShader::ShaderDataSetter::operator()<GLfloat>(uint32_t id, const std::string &name, 
    std::vector<GLfloat> buffer)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform1fv(id, location, buffer.size(), buffer.data());
}

template<>
void OGLShader::ShaderDataSetter::operator()<glm::vec4>(uint32_t id, const std::string &name, 
    std::vector<glm::vec4> buffer)
{
    GLint location = glGetUniformLocation(id, name.c_str());
    glProgramUniform4fv(id, location, buffer.size(), glm::value_ptr(buffer.front()));
}

auto OGLShader::compileShader(const std::string &source, GLenum type) -> GLuint
{
    GLuint shaderStageID = glCreateShader(type);
    const char* shaderStageSource = source.c_str();
    glShaderSource(shaderStageID, 1, &shaderStageSource, nullptr);
    glCompileShader(shaderStageID);

    GLint result;
    glGetShaderiv(shaderStageID, GL_COMPILE_STATUS, &result);

    if (!result) {
        GLsizei length;
        char message[1024];
        glGetShaderInfoLog(shaderStageID, sizeof(message), &length, message);
        std::println("Error compiling shader: {}!", message);
        glDeleteShader(shaderStageID);
        exit(1);
    }

    glAttachShader(m_shaderID, shaderStageID);

    return shaderStageID;
}