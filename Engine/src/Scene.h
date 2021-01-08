#pragma once

#include <entt/entt.hpp>

#include "Entity.h"

namespace Ash {

class Scene {
   public:
    Scene();
    ~Scene();

    Entity createEntity();
    void destroyEntity(Entity entity);

    // TODO: addComponent, removeComponent, hasComponent

    // TODO: Systems?

   private:
    entt::registry registry;
};

}  // namespace Ash
