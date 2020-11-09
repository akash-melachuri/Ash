#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

#include "Log.h"

void error_callback(int error, const char* description) {
    ASH_ERROR("Error: " << description);
}

int main() {
    Ash::Log::init();

    if (!glfwInit()) {
        ASH_ERROR("GLFW failed to initialize");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Lion", NULL, NULL);
    if (!window) {
        ASH_ERROR("Window creation failed");
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        // Keep running

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return EXIT_SUCCESS;
}
