#include "four-pch.h"
#include "core/log.hpp"

#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace four
{

const std::shared_ptr<spdlog::logger> Log::sm_CoreLogger = spdlog::stdout_color_mt("Core");
const std::shared_ptr<spdlog::logger> Log::sm_AppLogger  = spdlog::stdout_color_mt("App");

void Log::Init()
{
  // set output pattern for logs
  spdlog::set_pattern("%^[%T] %n: %v%$");
}

} // namespace four
