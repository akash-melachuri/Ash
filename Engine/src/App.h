#pragma once

#include "Core.h"
#include "Window.h"

namespace Ash {

class App {
   public:
    App();
    ~App();

    static void start();
    static void shutdown();

    inline static App* get() { return instance; }
   private:
    void run();

    std::unique_ptr<Window> window;

    static App* instance;
};

}  // namespace Ash

