#include "GameLayer.h"

#include <Components.h>
#include <Scene.h>

#include <chrono>

GameLayer::GameLayer() {}
GameLayer::~GameLayer() {}

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};

const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

std::shared_ptr<Scene> scene;

struct Spin {
    double rotation{0.0f};
};

struct Bob {
    double height{0.0f};
};

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

    Entity e2 = scene->spawn();
    scene->addComponent<Renderable>(e2, "Quad_statue", "main");
    scene->addComponent<Transform>(e2, glm::vec3{0.0f, 0.0f, 0.0f});
    scene->addComponent<Bob>(e2);

    Renderer::setScene(scene);
}

void GameLayer::onUpdate() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    auto spinView = scene->registry.view<Spin>();
    for (auto e : spinView) {
        auto& spin = spinView.get<Spin>(e);
        spin.rotation = glm::radians(time * 90.0f);
    }

    auto bobView = scene->registry.view<Bob>();
    for (auto e : bobView) {
        auto& bob = bobView.get<Bob>(e);
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
}
