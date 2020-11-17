#include "App.h"

namespace Ash {

App* App::instance = nullptr;

App::App() {
    instance = this;
    window = Window::create({
        "Ash",
        800,
        600,
    });

    api = std::make_unique<VulkanAPI>();
    api->init();
}

App::~App() {}

void App::start() {
    // Startup systems
    Log::init();
    Window::init();

    auto app = new App();
    app->run();
}

void App::cleanup() {
    // Shtudown systems
    ASH_INFO("Cleaning up resources...");
    instance->api->cleanup();
    instance->window->destroy();
    Window::cleanup();

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
