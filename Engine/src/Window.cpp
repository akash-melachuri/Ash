#include "Window.h"

#include "Core.h"

namespace Ash {

static void framebufferResizeCallback(GLFWwindow* window, int width,
                                      int height) {
    auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->framebufferResized = true;
}

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

GLFWwindow* Window::get() const { return window; }

std::shared_ptr<Window> Window::create(const WindowProperties& properties) {
    std::shared_ptr<Window> ret = std::make_shared<Window>();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    ret->window = glfwCreateWindow(properties.width, properties.height,
                                   properties.title.c_str(), nullptr, nullptr);
    ASH_ASSERT(ret, "Failed to create window");
    glfwSetFramebufferSizeCallback(ret->get(), framebufferResizeCallback);
    glfwSetWindowUserPointer(ret->get(), ret.get());
    return ret;
}

void Window::destroy() {
    ASH_INFO("Destroying window...");
    glfwDestroyWindow(window);
}

}  // namespace Ash
