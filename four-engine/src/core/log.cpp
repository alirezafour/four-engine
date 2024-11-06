#include "core/log.hpp"


namespace four
{
std::shared_ptr<spdlog::logger> Log::sm_CoreLogger = nullptr;
std::shared_ptr<spdlog::logger> Log::sm_AppLogger  = nullptr;
} // namespace four
