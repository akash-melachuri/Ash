#include <Ash.h>

using namespace Ash;

int main() {
    Renderer::loadPipeline(Pipeline("assets/shaders/phong.vert.spv",
                                    "assets/shaders/phong.frag.spv", "Phong"));
    Renderer::setClearColor({0.0f, 1.0f, 0.0f, 1.0f});
    Renderer::setPipeline(1);
    App::start();
    // Game code goes here
    App::cleanup();
}
