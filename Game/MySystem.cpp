#include "MySystem.h"

#include <chrono>

MySystem::MySystem() {}
MySystem::~MySystem() {}

void MySystem::init() {
    Renderer::setClearColor({0.0f, 1.0f, 0.0f, 1.0f});
    Renderer::setPipeline("Phong");
}

void MySystem::onUpdate() {
    static auto t1 = std::chrono::high_resolution_clock::now();
    static bool pipeline = true;

    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);

    if (time.count() > 0.5) {
        if (pipeline) {
            Renderer::setPipeline("main");
        } else {
            Renderer::setPipeline("Phong");
        }
        t1 = std::chrono::high_resolution_clock::now();
        pipeline = !pipeline;
    }
}
