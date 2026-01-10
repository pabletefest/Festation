#include <print>
#include <thread>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "psx_system.hpp"
#include "utils/logger.hpp"

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
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    }

    glfwTerminate();
    
    return 0;
}
