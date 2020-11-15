#pragma once

#include "Core.h"

namespace Ash {

class App {
   public:
    App();
    ~App();

    static void start();
    static void shutdown();

    inline static App* get() { return instance; }
   private:
    void run();

    static App* instance;
};

}  // namespace Ash

