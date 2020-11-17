#include "Window.h"

#include "Core.h"

namespace Ash {

Window::Window() {}

Window::~Window() {}

void Window::init() { ASH_ASSERT(glfwInit(), "Failed to initialize GLFW"); }

void Window::cleanup() {
    ASH_INFO("Terminating GLFW");
    glfwTerminate();
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window); }

void Window::swapBuffers() const { glfwSwapBuffers(window); }

void Window::pollEvents() const { glfwPollEvents(); }

std::unique_ptr<Window> Window::create(const WindowProperties& properties) {
    std::unique_ptr<Window> ret = std::make_unique<Window>();
    ret->window = glfwCreateWindow(properties.width, properties.height,
                                   properties.title.c_str(), nullptr, nullptr);
    ASH_ASSERT(ret, "Failed to create window");
    return ret;
}

void Window::destroy() {
    ASH_INFO("Destroying window...");
    glfwDestroyWindow(window);
}

}  // namespace Ash
