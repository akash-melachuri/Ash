#include "App.h"

namespace Ash {
    App* App::instance = nullptr;

    App::App() {
        instance = this;
        window = Window::create(800, 600, "Ash");
    }

    App::~App() {
    }

    void App::start() {
        // Startup systems
        Log::init();
        Window::init();

        auto app = new App();
        app->run();
    }

    void App::shutdown() {
        // Shtudown systems
        instance->window->destroy();
        Window::terminate();

        delete instance;
    }

    void App::run() {
        APP_INFO("Running!");
        while (!window->shouldClose()) {
            window->swapBuffers();
            window->pollEvents();
        }
    }
}  // namespace Ash
