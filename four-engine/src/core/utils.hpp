#pragma once
#include "core/core.hpp"

namespace four
{
template <typename Derived>
class FOUR_ENGINE_API Singleton
{
  friend Derived;

public:
  Singleton(const Singleton&)                = delete;
  Singleton(Singleton&&) noexcept            = delete;
  Singleton& operator=(const Singleton&)     = delete;
  Singleton& operator=(Singleton&&) noexcept = delete;
  bool       operator==(const Singleton&)    = delete;
  operator bool() const
  {
    return sm_Instance != nullptr;
  }
  static Derived* Get()
  {
    if (sm_Instance)
    {
      return sm_Instance.get();
    }

    sm_Instance = std::unique_ptr<Derived>();
    return sm_Instance.get();
  }

  static void Destroy()
  {
    sm_Instance.reset();
  }

private:
  Singleton()  = default;
  ~Singleton() = default;

private:
  static std::unique_ptr<Derived> sm_Instance{}; // Derived instance
};
} // namespace four
