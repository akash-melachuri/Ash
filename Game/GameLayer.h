#pragma once

#include <Ash.h>

using namespace Ash;

class GameLayer : public Layer {
   public:
    GameLayer();
    virtual ~GameLayer();
    virtual void init();
    virtual void onUpdate();
};
