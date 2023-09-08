#pragma once

#include "spdlog/spdlog.h"

namespace four::Engine2D
{
// @todo : add custom log class to have customized behaviour for spd log or other logger
// class Log
// {
// public:
// };
//
} // namespace four::Engine2D

// @todo : add options to enable log for core and app seperately
// add extra option that we need logging in release build
//
#ifdef _F_RELEASE // remove logs when release

#define LOG_CORE_TRACE(...)
#define LOG_CORE_INFO(...)
#define LOG_CORE_WARN(...)
#define LOG_CORE_ERROR(...)
#define LOG_CORE_FATAL(...)

#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...)

#else // if not release use logging

// Core log macros
#define LOG_CORE_TRACE(...) spdlog::trace(__VA_ARGS__)
#define LOG_CORE_INFO(...)  spdlog::info(__VA_ARGS__)
#define LOG_CORE_WARN(...)  spdlog::warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) spdlog::error(__VA_ARGS__)
#define LOG_CORE_FATAL(...) spdlog::fatal(__VA_ARGS__)

// Client log macros
#define LOG_TRACE(...)      spdlog::trace(__VA_ARGS__)
#define LOG_INFO(...)       spdlog::info(__VA_ARGS__)
#define LOG_WARN(...)       spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...)      spdlog::error(__VA_ARGS__)
#define LOG_FATAL(...)      spdlog::fatal(__VA_ARGS__)
#endif
// Client log macros
