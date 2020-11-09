#pragma once 

#include <iostream> 
namespace Ash {
    class Log {
        public: 
            Log();
            ~Log();

            static void init();
    };
}

#define ASH_DEBUG(...) std::cout << "[Ash]: " << __VA_ARGS__ << std::endl;
#define APP_DEBUG(...) std::cout << "[App]: " << __VA_ARGS__ << std::endl;
#define ASH_ERROR(...) std::cout << "[Ash ERROR]: " << __VA_ARGS__ << std::endl;
#define APP_ERROR(...) std::cout << "[App ERROR]: " << __VA_ARGS__ << std::endl;
