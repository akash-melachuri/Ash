#include <Ash.h>

#include "GameLayer.h"

using namespace Ash;

int main() {
  // Load resources
  // Renderer::loadPipeline(Pipeline("assets/shaders/phong.vert.spv",
  //                                 "assets/shaders/phong.frag.spv",
  //                                 "Phong"));

  // Initialize application
  App::init();

  // Add user layers
  App::addLayer(new GameLayer());

  // Start
  App::start();

  // Cleanup
  App::cleanup();
}
