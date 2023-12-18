#include "GameLayer.h"

#include <Components.h>
#include <Scene.h>

#include <chrono>
#include <glm/ext/scalar_constants.hpp>

GameLayer::GameLayer() {}
GameLayer::~GameLayer() {}

const std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
                                      {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
                                      {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
                                      {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}}};

const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

std::shared_ptr<Scene> scene;

struct Spin {
  double rotation{0.0f};
};

struct Bob {
  double height{0.0f};
};

Camera camera;

float yaw{180};
float pitch{};
float sensitivity = 0.1;
float speed = 0.01;

std::pair<int, int> last_mouse_pos{};

void GameLayer::init() {
  scene = std::make_shared<Scene>();

  Renderer::loadMesh("Quad", vertices, indices);
  Renderer::loadTexture("statue", "assets/textures/texture.jpg");

  std::vector<std::string> meshes = {"Quad"};
  std::vector<std::string> textures = {"statue"};
  Renderer::loadModel("Quad_statue", meshes, textures);

  Helper::importModel("viking_room",
                      "assets/models/viking_room/viking_room.obj");

  Entity e = scene->spawn();
  scene->addComponent<Renderable>(e, "viking_room", "main");
  scene->addComponent<Transform>(e, glm::vec3{0.0f, 0.0f, 0.0f});
  scene->addComponent<Spin>(e);

  Renderer::setScene(scene);

  App::getWindow()->disableCursor();
  auto dimensions = App::getWindow()->getWindowSize();
  last_mouse_pos = {dimensions.first / 2, dimensions.second / 2};
}

void GameLayer::onUpdate() {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  auto curr_mouse_pos = App::getWindow()->getCursorPos();
  std::pair<int, int> mouse_pos_delta = {
      curr_mouse_pos.first - last_mouse_pos.first,
      curr_mouse_pos.second - last_mouse_pos.second};

  yaw -= mouse_pos_delta.first * sensitivity * Ash::getFrameTime();
  pitch -= mouse_pos_delta.second * sensitivity * Ash::getFrameTime();

  pitch = fmin(89.0, pitch);
  pitch = fmax(-89.0, pitch);

  glm::vec3 dir;
  dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  dir.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  dir.z = sin(glm::radians(pitch));
  camera.center = camera.eye + dir;

  auto spinView = scene->registry.view<Spin>();
  for (auto e : spinView) {
    auto &spin = spinView.get<Spin>(e);
    spin.rotation += 1e-3 * Ash::getFrameTime();
  }

  auto bobView = scene->registry.view<Bob>();
  for (auto e : bobView) {
    auto &bob = bobView.get<Bob>(e);
    bob.height = glm::sin(time);
  }

  auto v = scene->registry.view<Spin, Transform>();
  for (auto e : v) {
    auto [spin, transform] = v.get<Spin, Transform>(e);
    transform.rotation.z = spin.rotation;
  }

  auto bobTransform = scene->registry.view<Bob, Transform>();
  for (auto e : bobTransform) {
    auto [bob, transform] = bobTransform.get<Bob, Transform>(e);
    transform.position.z = bob.height;
  }

  dir.z = 0;
  dir = glm::normalize(dir);

  glm::vec3 right = glm::cross(dir, camera.up);

  if (App::getWindow()->getKey(KEY_W) == EventStatus::PRESSED) {
    camera.eye += dir * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_S) == EventStatus::PRESSED) {
    camera.eye -= dir * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_A) == EventStatus::PRESSED) {
    camera.eye -= right * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_D) == EventStatus::PRESSED) {
    camera.eye += right * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_LSHIFT) == EventStatus::PRESSED) {
    camera.eye.z -= speed * Ash::getFrameTime();
  }

  if (App::getWindow()->getKey(KEY_SPACE) == EventStatus::PRESSED) {
    camera.eye.z += speed * Ash::getFrameTime();
  }

  Renderer::setCamera(camera);

  last_mouse_pos = curr_mouse_pos;
}
