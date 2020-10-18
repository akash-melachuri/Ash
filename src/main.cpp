#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include <iostream>

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}


int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW failed to initialize" << std::endl;
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window) {
        std::cerr << "Window creation failed" << std::endl;
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
