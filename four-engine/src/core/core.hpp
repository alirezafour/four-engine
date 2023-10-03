#include "core/log.hpp"

// @todo : add options to enable log for core and app seperately
// add extra option that we need logging in release build
//
#ifdef FOUR_RELEASE // remove logs when release

#define LOG_CORE_TRACE(...)
#define LOG_CORE_INFO(...)
#define LOG_CORE_WARN(...)
#define LOG_CORE_ERROR(...)

#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)

#else // if not release use logging

// Core log macros
#define LOG_CORE_TRACE(...) four::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...)  four::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...)  four::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) four::Log::GetCoreLogger()->error(__VA_ARGS__)

// Client log macros
#define LOG_TRACE(...)      four::Log::GetAppLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)       four::Log::GetAppLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)       four::Log::GetAppLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)      four::Log::GetAppLogger()->error(__VA_ARGS__)
#endif
// Client log macros
