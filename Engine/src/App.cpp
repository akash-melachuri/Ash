#include "App.h"

#include <chrono>

#include "Renderer.h"

namespace Ash {

App *App::instance = nullptr;

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
      1600,
      1200,
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

  for (auto system : instance->systems)
    delete system;

  delete instance;
}

void App::addLayer(Layer *layer) {
  layer->init();
  instance->systems.push_back(layer);
}

void App::setScene(std::shared_ptr<Scene> scene) {
  Renderer::setScene(scene);
  // Record command buffers with scene data

  // Set App reference to scene
  // Allow for user to make systems that update on certain components
  // Maybe hand raw entt iterable to user for ultimate control?
}

void App::run() {
  APP_INFO("Running!");

  auto now = std::chrono::high_resolution_clock::now();
  auto dNow = std::chrono::high_resolution_clock::now();

  uint32_t frames = 0;

  while (!window->shouldClose()) {
    for (auto system : systems)
      system->onUpdate();

    Renderer::render();

    window->swapBuffers();
    window->pollEvents();

    frames++;

    auto end = std::chrono::high_resolution_clock::now();
    auto frametime =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - now)
            .count();

    delta = std::chrono::duration_cast<std::chrono::nanoseconds>(end - dNow)
                .count() /
            1e6;

    dNow = std::chrono::high_resolution_clock::now();

    if (frametime >= 1000.0f) {
      ASH_INFO("Average frame time: {} ms", (float)frametime / (float)frames);

      now = std::chrono::high_resolution_clock::now();
      frames = 0;
    }
  }
}

} // namespace Ash
