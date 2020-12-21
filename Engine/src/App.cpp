#include "App.h"

#include <chrono>

#include "Renderer.h"

namespace Ash {

App* App::instance = nullptr;

App::App() { instance = this; }

App::~App() {}

void App::init() {
    new App();

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

#ifdef ASH_LINUX
    ASH_INFO("Detected Linux Operating System");
#endif
#ifdef ASH_WINDOWS
    ASH_INFO("Detected Windows Operating System");
#endif
}

void App::start() { instance->run(); }

void App::cleanup() {
    ASH_INFO("Cleaning up resources...");

    // Shtudown systems
    Renderer::cleanup();
    instance->window->destroy();
    Window::cleanup();

    for (auto layer : instance->layers) delete layer;

    delete instance;
}

void App::addLayer(Layer* layer) {
    layer->init();
    instance->layers.push_back(layer);
}

void App::run() {
    APP_INFO("Running!");

    while (!window->shouldClose()) {
        for (auto layer : layers) layer->onUpdate();

        Renderer::render();

        window->swapBuffers();
        window->pollEvents();
    }
}

}  // namespace Ash
