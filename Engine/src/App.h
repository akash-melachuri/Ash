#pragma once

#include <memory>
#include <vector>

#include "Scene.h"
#include "System.h"
#include "Window.h"

namespace Ash {

class App {
   public:
    App();
    ~App();

    static void init();
    static void start();
    static void cleanup();
    static void addLayer(Layer* layer);
    static void setScene(std::shared_ptr<Scene> scene);

    inline static App* get() { return instance; }

    inline static std::shared_ptr<Window> getWindow() {
        return instance->window;
    }

   private:
    void run();

    std::shared_ptr<Window> window;
    std::vector<Layer*> systems;

    static App* instance;
};

}  // namespace Ash

