#include "App.h"

#include "Renderer.h"

namespace Ash {

App* App::instance = nullptr;

App::App() { instance = this; }

App::~App() {}

void App::start() {
    auto app = new App();

    // Startup systems
    Log::init();
    Window::init();

    // Initialize window
    instance->window = Window::create({
        "Ash",
        800,
        600,
    });

    Renderer::init();

    app->run();
}

void App::cleanup() {
    ASH_INFO("Cleaning up resources...");

    // Shtudown systems
    Renderer::cleanup();
    instance->window->destroy();
    Window::cleanup();

    delete instance;
}

void App::run() {
    APP_INFO("Running!");

    while (!window->shouldClose()) {
        Renderer::render();

        window->swapBuffers();
        window->pollEvents();
    }
}

}  // namespace Ash
