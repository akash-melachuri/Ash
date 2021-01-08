#pragma once

#include <entt/entt.hpp>

namespace Ash {

class Entity {
   public:
    Entity() = default;
    Entity(entt::entity handle) : handle(handle) {}
    ~Entity();

    entt::entity& getHandle() { return handle; }

   private:
    entt::entity handle;
};

}  // namespace Ash
