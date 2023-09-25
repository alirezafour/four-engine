#pragma once
#include "spdlog/logger.h"
#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace four
{

/*
 * Logger class to create log files or log on console
 * @brief Logger Class
 */
class Log
{
public:
  Log()                = delete;
  ~Log()               = default;
  Log(Log&)            = delete;
  Log(Log&&)           = delete;
  Log operator=(Log&)  = delete;
  Log operator=(Log&&) = delete;

  /**
   * @brief Initialize Loggers if not Initialized yet
   *
   * @return true if successfully Initialize or already Initialized
   */
  static bool Init()
  {
    // check if already Initialized
    if (sm_CoreLogger && sm_CoreLogger)
    {
      return true;
    }

    // create 2 different logger for core an app
    Log::sm_CoreLogger = spdlog::stdout_color_mt("Core");
    Log::sm_AppLogger  = spdlog::stdout_color_mt("App");


    // check if successfully created
    if (sm_CoreLogger == nullptr || sm_AppLogger == nullptr)
    {
      return false;
    }
    // set output pattern for logs
    spdlog::set_pattern("%^[%T] %n: %v%$");
    return true;
  }

  /**
   * @brief Shutdown Core and App Logger
   */
  static void Shutdown()
  {
    sm_CoreLogger.reset();
    sm_AppLogger.reset();
  }

  /**
    * @brief return engine logger
    * @return get engine core logger 
    */
  inline static auto GetCoreLogger() noexcept
  {
    // check if not Initialized
    if (!sm_CoreLogger)
    {
      Log::Init();
    }
    return sm_CoreLogger.get();
  }
  /**
    * @brief return app logger (client logger)
    * @return get application logger
    */
  inline static auto GetAppLogger() noexcept
  {
    // check if not Initialized
    if (!sm_AppLogger)
    {
      Log::Init();
    }
    return sm_AppLogger.get();
  }

private:
  /** logger for engine logging */
  static std::shared_ptr<spdlog::logger> sm_CoreLogger;

  /** logger for application logging */
  static std::shared_ptr<spdlog::logger> sm_AppLogger;
};
//
} // namespace four

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
