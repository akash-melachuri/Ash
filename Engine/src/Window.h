#pragma once

#include <GLFW/glfw3.h>
#include <bits/stdint-uintn.h>

#include <iostream>

namespace Ash {

    class Window {
       public:
        Window(uint32_t width, uint32_t height) : width(width), height(height) {
            if (!initialized) {
                if (!glfwInit()) {
                    std::cerr << "GLFW failed to initialize" << std::endl;
                }
            }
        } 
        ~Window();
    
        void init();
        void destroy();
        bool shouldClose();
        void swapBuffers();
        void pollEvents();
    
       private:
        GLFWwindow* window;
        uint32_t width;
        uint32_t height;
        bool initialized = false;
    };

}
