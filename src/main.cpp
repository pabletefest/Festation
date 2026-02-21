#include <print>
#include <thread>
#include <sstream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "psx_system.hpp"
#include "utils/logger.hpp"

#include <glm/vec4.hpp>

namespace festation
{
    static constexpr const char* EMU_TITLE = "Festation (PSX Emulator)";
    static constexpr const int EMU_WIDTH = 1024;
    static constexpr const int EMU_HEIGHT = 512;
};

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam)
{
    LOG_ERROR("{}", message);
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

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(festation::EMU_WIDTH, festation::EMU_HEIGHT, festation::EMU_TITLE, NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        LOG_ERROR("Failed to initialize GLAD");
        return -1;
    }

    int flags; 
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, nullptr, GL_FALSE);
    }

    festation::PSXSystem psxSystem;

    // psxSystem.sideloadExeFile(std::filesystem::current_path().concat("/../../../res/tests/psxtest_cpu.exe"));
    psxSystem.sideloadExeFile(std::filesystem::current_path().concat("/../../../res/tests/PeterLemon-PSX/HelloWorld/16BPP/HelloWorld16BPP.exe"));
    
    float time = (float)glfwGetTime();
    float delta = 0.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            continue;
        }

        float current = (float)glfwGetTime();
        delta = current - time;
        time = current;

        std::stringstream strStream;
        strStream << festation::EMU_TITLE;
        strStream << std::string(" | ");
        strStream << std::to_string(1.0f / delta);
        strStream << std::string(" FPS");
        glfwSetWindowTitle(window, strStream.str().c_str());

        psxSystem.runWholeFrame();

        // int display_w, display_h;
        // glfwGetFramebufferSize(window, &display_w, &display_h);
        // glViewport(0, 0, display_w, display_h);
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    
    return 0;
}
