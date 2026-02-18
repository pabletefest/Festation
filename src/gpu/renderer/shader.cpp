#include "shader.hpp"
#include "hw/OpenGL/ogl_shader.hpp"

using namespace festation;

auto IShader::createUnique(const std::filesystem::path &filename) -> std::unique_ptr<IShader>
{
    return std::make_unique<OGLShader>(filename);
}

auto IShader::createUnique(const std::filesystem::path &vertShaderPath, const std::filesystem::path fragShaderPath) -> std::unique_ptr<IShader>
{
    return std::make_unique<OGLShader>(vertShaderPath, fragShaderPath);
}

auto IShader::createShared(const std::filesystem::path &filename) -> std::shared_ptr<IShader>
{
    return std::make_shared<OGLShader>(filename);
}

auto IShader::createShared(const std::filesystem::path &vertShaderPath, const std::filesystem::path fragShaderPath) -> std::shared_ptr<IShader>
{
    return std::make_shared<OGLShader>(vertShaderPath, fragShaderPath);
}