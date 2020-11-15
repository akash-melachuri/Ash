#include "App.h"

namespace Ash {
    App* App::instance = nullptr;

    App::App() {
        instance = this;
    }

    App::~App() {
    }

    void App::start() {
        // Startup systems
        Log::init();

        auto app = new App();
        app->run();
    }

    void App::shutdown() {
        // Shtudown systems

        delete instance;
    }

    void App::run() {
        APP_INFO("Running!");
    }
}  // namespace Ash
