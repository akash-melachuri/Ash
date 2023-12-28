#include "GameLayer.h"

#include <Components.h>
#include <Scene.h>

#include <assimp/postprocess.h>
#include <chrono>
#include <glm/ext/scalar_constants.hpp>

GameLayer::GameLayer() {}
GameLayer::~GameLayer() {}

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
float sensitivity = 0.01;
float speed = 0.01;

std::pair<int, int> last_mouse_pos{};

void GameLayer::init() {
  scene = std::make_shared<Scene>();

  Helper::importModel("bp", "assets/models/backpack/backpack.obj");

  Entity e = scene->spawn();
  scene->addComponent<Renderable>(e, "bp", "main");
  Transform transform{{0, 0, 0}};
  scene->addComponent<Transform>(e, transform);

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

  yaw += mouse_pos_delta.first * sensitivity * Ash::getFrameTime();
  pitch -= mouse_pos_delta.second * sensitivity * Ash::getFrameTime();

  pitch = fmin(89.0, pitch);
  pitch = fmax(-89.0, pitch);

  glm::vec3 dir;
  dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  dir.y = sin(glm::radians(pitch));
  dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

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

  glm::vec3 forward = dir;
  forward.y = 0;
  forward = glm::normalize(forward);

  glm::vec3 right = glm::cross(dir, camera.up);

  if (App::getWindow()->getKey(KEY_W) == EventStatus::PRESSED) {
    camera.eye += forward * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_S) == EventStatus::PRESSED) {
    camera.eye -= forward * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_A) == EventStatus::PRESSED) {
    camera.eye -= right * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_D) == EventStatus::PRESSED) {
    camera.eye += right * speed * static_cast<float>(Ash::getFrameTime());
  }

  if (App::getWindow()->getKey(KEY_LSHIFT) == EventStatus::PRESSED) {
    camera.eye.y -= speed * Ash::getFrameTime();
  }

  if (App::getWindow()->getKey(KEY_SPACE) == EventStatus::PRESSED) {
    camera.eye.y += speed * Ash::getFrameTime();
  }

  camera.center = camera.eye + dir;

  Renderer::setCamera(camera);

  last_mouse_pos = curr_mouse_pos;
}
