#pragma once

#include <filesystem>
#include <memory>
#include <vector>

namespace festation {
    class IShader {
        public:
            virtual ~IShader() = default;

            virtual auto apply() -> void = 0;

            template<typename T>
            auto setData(const std::string& name, const T& value) -> void;
            
            template<typename T>
            auto setDataBuffer(const std::string& name, std::vector<T> values) -> void;

            inline auto getShaderID() const -> uint32_t { return m_shaderID; }

            static auto createUnique(const std::filesystem::path& filename) -> std::unique_ptr<IShader>;
            static auto createUnique(const std::filesystem::path& vertShaderPath, 
                const std::filesystem::path fragShaderPath) -> std::unique_ptr<IShader>;
            static auto createShared(const std::filesystem::path& filename) -> std::shared_ptr<IShader>;
            static auto createShared(const std::filesystem::path& vertShaderPath, 
                const std::filesystem::path fragShaderPath) -> std::shared_ptr<IShader>;

        protected:
            struct ShaderDataSetter {
                template<typename T>
                void operator()(uint32_t id, const std::string& name, const T& data);
                
                template<typename T>
                void operator()(uint32_t id, const std::string& name, std::vector<T> buffer);
            };

            uint32_t m_shaderID;
    };

    template <typename T>
    inline auto IShader::setData(const std::string &name, const T &value) -> void
    {
        std::invoke(ShaderDataSetter(), m_shaderID, name, value);
    }
    
    template <typename T>
    inline auto IShader::setDataBuffer(const std::string &name, std::vector<T> values) -> void
    {
        std::invoke(ShaderDataSetter(), m_shaderID, name, values);
    }
};