#include <Ash.h>

#include "MySystem.h"

using namespace Ash;

int main() {
    // Load resources
    // Renderer::loadPipeline(Pipeline("assets/shaders/phong.vert.spv",
    //                                 "assets/shaders/phong.frag.spv",
    //                                 "Phong"));

    // Initialize application
    App::init();

    // Add user layers
    App::addSystem(new MySystem());

    // Start
    App::start();

    // Cleanup
    App::cleanup();
}
