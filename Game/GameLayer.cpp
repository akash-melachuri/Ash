#include "GameLayer.h"

#include <Scene.h>

GameLayer::GameLayer() {}
GameLayer::~GameLayer() {}

const std::vector<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

struct Position {
    float x, y, z;
};

std::shared_ptr<Scene> scene;

void GameLayer::init() {
    scene = std::make_shared<Scene>();
    Renderer::loadMesh("Mesh", vertices, indices);
    Renderer::queueMesh("Mesh");

    // Entity e = scene->spawn();
    // scene->addComponent<Position>(e, 0.0f, 0.0f, 0.0f);
    Renderer::setScene(scene);
}

void GameLayer::onUpdate() {
    // Example user system

    // auto v = scene->registry.view<Position>();
    // for (auto e : v) {
    //  auto& transform = v.get<Position>(e);
    // }
}
