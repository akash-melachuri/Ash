#include "MySystem.h"

MySystem::MySystem() {}
MySystem::~MySystem() {}

const std::vector<Vertex> tri1 = {{{0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}},
                                  {{-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}}};

const std::vector<Vertex> tri2 = {{{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                  {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

void MySystem::init() {
    Renderer::drawVertices(tri1);
    Renderer::drawVertices(tri2);
}

void MySystem::onUpdate() {}
