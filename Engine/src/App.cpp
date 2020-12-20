#include "App.h"

#include <chrono>

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

#ifdef ASH_LINUX
    ASH_INFO("Detected Linux Operating System");
#endif
#ifdef ASH_WINDOWS
    ASH_INFO("Detected Windows Operating System");
#endif

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

    // Temporary
    auto t1 = std::chrono::high_resolution_clock::now();
    bool pipeline = true;

    while (!window->shouldClose()) {
        Renderer::render();

        // Temporary
        auto t2 = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);

        // Temporary
        if (time.count() > 0.5) {
            if (pipeline) {
                Renderer::setPipeline("main");
            } else {
                Renderer::setPipeline("Phong");
            }
            t1 = std::chrono::high_resolution_clock::now();
            pipeline = !pipeline;
        }

        window->swapBuffers();
        window->pollEvents();
    }
}

}  // namespace Ash
