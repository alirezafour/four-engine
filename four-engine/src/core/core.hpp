#pragma once

// ======================================================================
// Shared library support
// ======================================================================
#ifdef FOUR_BUILD_SHARED
#ifdef FOUR_PLATFORM_WINDOWS
#define FOUR_ENGINE_API __declspec(dllexport)
#elif FOUR_PLATFORM_MAC
// TODO: add support for mac
#define FOUR_ENGINE_API
#elif FOUR_PLATFORM_LINUX
#define FOUR_ENGINE_API __attribute__((visibility("default")))
#else // FOUR_PLATFORM_X
#define FOUR_ENGINE_API
#endif // FOUR_PLATFORM_X
#else  // FOUR_BUILD_SHARED
#define FOUR_ENGINE_API
#endif // FOUR_BUILD

// ======================================================================
// Log macros
// ======================================================================
// TODO: add options to enable log for core and app seperately
// add extra option that we need logging in release build
//
#ifdef FOUR_RELEASE // remove logs when release

#define FOUR_ASSERT(eval, msg)

#define LOG_CORE_TRACE(...)
#define LOG_CORE_INFO(...)
#define LOG_CORE_WARN(...)
#define LOG_CORE_ERROR(...)

#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)

#else // NOT FOUR_RELEASE

#include "core/log.hpp"
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

// ======================================================================
// Breakpoints
// ======================================================================

// TODO: setup OS and debugger breakpoint
// setup breakpoint setup
#ifdef _DEBUG
#ifdef FOUR_PLATFORM_WINDOWS
#define FOUR_BREAK __debugbreak();
#elif FOUR_PLATFORM_MAC
#define FOUR_BREAK __builtin_debugtrap();
#elif FOUR_PLATFORM_LINUX
#define FOUR_BREAK __builtin_trap();
#else // FOUR_PLATFORM_X
#define FOUR_BREAK
#endif // FOUR_PLATFORM_X
#else  // _DEBUG
#define FOUR_BREAK
#endif // _DEBUG

// ======================================================================
// Assertions
// ======================================================================
#include <source_location>
#include <format>

#define FOUR_ASSERT(eval)                                                                                                     \
  if (!(eval))                                                                                                                \
  {                                                                                                                           \
    const std::source_location src = std::source_location::current();                                                         \
    LOG_CORE_ERROR("assertion failed in {}, file {}, {}:{}", src.function_name(), src.file_name(), src.line(), src.column()); \
    FOUR_BREAK;                                                                                                               \
  }

#endif // NOT FOUR_RELEASE

// ======================================================================
// includes
// ======================================================================
#include "core/type.hpp"
