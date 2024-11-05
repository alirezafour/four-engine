#pragma once

#include "core/core.hpp"

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
class FOUR_ENGINE_API Log
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
    // check if Initialized
    if (sm_CoreLogger)
      return sm_CoreLogger.get();
    Log::Init();
    return sm_CoreLogger.get();
  }
  /**
    * @brief return app logger (client logger)
    * @return get application logger
    */
  inline static auto GetAppLogger() noexcept
  {
    // check if Initialized
    if (sm_AppLogger)
      return sm_AppLogger.get();
    Log::Init();
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
