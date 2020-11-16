#include "Window.h"

#include "Core.h"

namespace Ash {

    Window::Window() {}

    Window::~Window() {}

    void Window::init() {
        ASH_ASSERT(glfwInit(), "Failed to initialize GLFW");
    }

    void Window::terminate() {
        glfwTerminate();
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(window);
    }

    void Window::swapBuffers() const {
        glfwSwapBuffers(window);
    }

    void Window::pollEvents() const {
        glfwPollEvents();
    }

    std::unique_ptr<Window> Window::create(uint32_t width, uint32_t height, std::string title) {
        std::unique_ptr<Window> ret = std::make_unique<Window>();
        ret->window = glfwCreateWindow(800, 600, title.c_str(), nullptr, nullptr);
        ASH_ASSERT(ret, "Failed to create window");
        return ret;
    }

    void Window::destroy() {
        glfwDestroyWindow(window);
    }

}
