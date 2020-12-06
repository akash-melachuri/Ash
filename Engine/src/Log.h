#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace Ash {

class Log {
   public:
    Log();
    ~Log();

    static void init();

    inline static std::shared_ptr<spdlog::logger>& getConsoleLogger() {
        return coreConsole;
    }
    inline static std::shared_ptr<spdlog::logger>& getAppLogger() {
        return appLogger;
    }

   private:
    static std::shared_ptr<spdlog::logger> coreConsole;
    static std::shared_ptr<spdlog::logger> appLogger;
};

}  // namespace Ash

#define ASH_INFO(...) Ash::Log::getConsoleLogger()->info(__VA_ARGS__);
#define ASH_WARN(...) Ash::Log::getConsoleLogger()->warn(__VA_ARGS__);
#define ASH_TRACE(...) Ash::Log::getConsoleLogger()->trace(__VA_ARGS__);
#define ASH_ERROR(...) Ash::Log::getConsoleLogger()->error(__VA_ARGS__);
#define ASH_CRITICAL(...) Ash::Log::getConsoleLogger()->critical(__VA_ARGS__);

#define APP_INFO(...) Ash::Log::getAppLogger()->info(__VA_ARGS__);
#define APP_WARN(...) Ash::Log::getAppLogger()->warn(__VA_ARGS__);
#define APP_TRACE(...) Ash::Log::getAppLogger()->trace(__VA_ARGS__);
#define APP_ERROR(...) Ash::Log::getAppLogger()->error(__VA_ARGS__);
#define APP_CRITICAL(...) Ash::Log::getAppLogger()->critical(__VA_ARGS__);
