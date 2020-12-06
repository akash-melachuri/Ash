#pragma once

#include <memory>

#include "Core.h"
#include "VulkanAPI.h"
#include "Window.h"

namespace Ash {

class App {
   public:
    App();
    ~App();

    static void start();
    static void cleanup();

    inline static App* get() { return instance; }

    inline std::shared_ptr<Window> getWindow() const { return window; }

   private:
    void run();

    std::shared_ptr<Window> window;
    std::unique_ptr<VulkanAPI> api;  // TODO: Move to Renderer

    static App* instance;
};

}  // namespace Ash

