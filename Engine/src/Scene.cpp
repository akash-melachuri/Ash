#include "Scene.h"

namespace Ash {

Scene::Scene() {}
Scene::~Scene() {}

Entity Scene::createEntity() { return Entity(registry.create()); }

void Scene::destroyEntity(Entity entity) {
    registry.destroy(entity.getHandle());
}

}  // namespace Ash
