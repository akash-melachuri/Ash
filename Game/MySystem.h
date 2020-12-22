#pragma once

#include <Ash.h>

using namespace Ash;

class MySystem : public System {
   public:
    MySystem();
    virtual ~MySystem();
    virtual void init();
    virtual void onUpdate();
};
