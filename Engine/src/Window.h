#pragma once

#include <GLFW/glfw3.h>

namespace Ash {

class Window {
   public:
    Window();
    ~Window();

    static void init();
    static void terminate();

    bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;

    static std::unique_ptr<Window> create(uint32_t width, uint32_t height, std::string title);
    void destroy();

   private:
    GLFWwindow* window;
};

}  // namespace Ash
