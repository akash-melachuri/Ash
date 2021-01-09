#pragma once

#include <entt/entt.hpp>

#include "Core.h"
#include "Entity.h"

namespace Ash {

class Scene {
   public:
    Scene();
    ~Scene();

    Entity spawn();
    void destroyEntity(Entity entity);

    template <typename T>
    bool hasComponent(Entity entity) {
        return registry.has<T>(entity.getHandle());
    }

    template <typename T, typename... Args>
    T& addComponent(Entity entity, Args&&... args) {
        ASH_ASSERT(!hasComponent<T>(entity), "Entity already has component");
        T& comp = registry.emplace<T>(entity.getHandle(),
                                      std::forward<Args>(args)...);
        return comp;
    }

    template <typename T>
    void removeComponent(Entity entity) {
        if (!hasComponent<T>(entity)) return;
        registry.remove<T>(entity.getHandle());
    }

    // TODO: Systems?

   private:
    entt::registry registry;
};

}  // namespace Ash
