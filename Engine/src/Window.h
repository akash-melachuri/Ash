#pragma once

#include <GLFW/glfw3.h>

namespace Ash {

struct WindowProperties {
    std::string title;
    uint32_t width, height;

    WindowProperties(const std::string& title, uint32_t width, uint32_t height)
        : title(title), width(width), height(height) {}
};

class Window {
   public:
    Window();
    ~Window();

    static void init();
    static void cleanup();

    bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;
    GLFWwindow* getInstance() const;

    static std::unique_ptr<Window> create(const WindowProperties& properties);
    void destroy();

   private:
    GLFWwindow* window;
};

}  // namespace Ash
