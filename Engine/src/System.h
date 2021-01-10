#pragma once

namespace Ash {

class Layer {
   public:
    Layer() {}
    virtual ~Layer() = default;

    virtual void init() = 0;
    virtual void onUpdate() = 0;
};

}  // namespace Ash
