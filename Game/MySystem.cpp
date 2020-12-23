#include "MySystem.h"

#include <chrono>

MySystem::MySystem() {}
MySystem::~MySystem() {}

void MySystem::init() {
    Renderer::setClearColor({1.0f, 0.0f, 1.0f, 1.0f});
    Renderer::setPipeline("Phong");
}

void MySystem::onUpdate() {}
