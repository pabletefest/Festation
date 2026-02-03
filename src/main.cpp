#include <print>
#include <thread>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "psx_system.hpp"
#include "utils/logger.hpp"

#include "gpu/renderer/shader.hpp"
#include <glm/vec4.hpp>

/* Sets constants */
#define WIDTH 800
#define HEIGHT 600
#define DELAY 3000

namespace festation
{
    PSXSystem psxSystem;

    static constexpr const char* EMU_TITLE = "Festation (PSX Emulator)";
};

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(int, char**)
{
    LOG_INFO("Hello, from Festation!");

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH, HEIGHT, festation::EMU_TITLE, NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        LOG_ERROR("Failed to initialize GLAD");
        return -1;
    }

    static const GLfloat g_trianglePositions[] = {
        -1.0f, -1.0f,
        0.0f,  1.0f,
        1.0f, -1.0f
    };

    static const GLuint g_triangleIndexes[] = {
        0, 2, 1
    };

    std::filesystem::path vsPath = "./../../../res/shaders/flat_color.glsl.vert";
    std::filesystem::path fsPath = "./../../../res/shaders/flat_color.glsl.frag";

    std::unique_ptr<festation::IShader> m_shader = festation::IShader::createUnique(vsPath, fsPath);

    GLuint m_VAO, m_VBO, m_IBO;

    glCreateVertexArrays(1, &m_VAO);

    glCreateBuffers(1, &m_VBO);
    glNamedBufferStorage(m_VBO, sizeof(g_trianglePositions), g_trianglePositions, GL_DYNAMIC_STORAGE_BIT);
    
    glCreateBuffers(1, &m_IBO);
    glNamedBufferStorage(m_IBO, sizeof(g_triangleIndexes), g_triangleIndexes, GL_DYNAMIC_STORAGE_BIT);
    
    glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, 2 * sizeof(float));
    glVertexArrayElementBuffer(m_VAO, m_IBO);

    glEnableVertexArrayAttrib(m_VAO, 0);

    glVertexArrayAttribFormat(m_VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayAttribBinding(m_VAO, 0, 0);

    glBindVertexArray(0);

    festation::psxSystem.sideloadExeFile(std::filesystem::current_path().concat("/../../../res/tests/psxtest_cpu.exe"));

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            continue;
        }

        festation::psxSystem.runWholeFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClear(GL_COLOR_BUFFER_BIT);

        m_shader->apply();
        m_shader->setData<glm::vec4>("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, sizeof(g_triangleIndexes)/sizeof(g_triangleIndexes[0]), GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    }

    glBindVertexArray(0);

    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_IBO);
    glDeleteVertexArrays(1, &m_VAO);

    glfwTerminate();
    
    return 0;
}
