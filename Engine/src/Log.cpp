#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Ash {

    std::shared_ptr<spdlog::logger> Log::coreConsole;
    std::shared_ptr<spdlog::logger> Log::appLogger;

    void Log::init() {
        coreConsole = spdlog::stdout_color_mt("Ash");
        appLogger = spdlog::stdout_color_mt("App");
        spdlog::set_pattern("%^[%T] %n: %v%$");
		spdlog::set_level(spdlog::level::trace);
    }

}
