#pragma once

namespace Ash {

class System {
   public:
    System() {}
    virtual ~System() = default;

    virtual void init() = 0;
    virtual void onUpdate() = 0;
};

}  // namespace Ash
