#pragma once

#include "ashpch.h"

#include "Log.h"

#include <GLFW/glfw3.h>

namespace Ash {

    class Window {
       public:
        Window(uint32_t width, uint32_t height) : width(width), height(height) {
            if (!initialized) {
                if (!glfwInit()) {
                    ASH_ERROR("GLFW failed to initialize");
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
