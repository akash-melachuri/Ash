#include "MySystem.h"

#include <chrono>

MySystem::MySystem() {}
MySystem::~MySystem() {}

const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

void MySystem::init() { Renderer::drawVertices(vertices); }

void MySystem::onUpdate() {}
