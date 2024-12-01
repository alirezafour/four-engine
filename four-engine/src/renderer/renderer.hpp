#pragma once
#include "core/core.hpp"

namespace four
{

template <typename T>
concept RendererConcept = requires(T t) {
  { t.DrawFrame() } -> std::same_as<void>;
  { t.StopRenderImpl() } -> std::same_as<void>;
};

template <typename Derived>
// requires RendererConcept<Derived>
class FOUR_ENGINE_API Renderer
{
  friend Derived;

public:
  virtual ~Renderer() = default;

  Renderer(const Renderer&)                = delete;
  Renderer& operator=(const Renderer&)     = delete;
  Renderer(Renderer&&) noexcept            = delete;
  Renderer& operator=(Renderer&&) noexcept = delete;


  void Render()
  {
    GetDerived()->DrawFrame();
  }

  void StopRender()
  {
    GetDerived()->StopRenderImpl();
  }

private:
  explicit Renderer() = default;
  /**
   * @brief cast and return derived object pointer
   * @return non-const pointer to derived class
   */
  Derived* GetDerived()
  {
    return static_cast<Derived*>(this);
  }

  /**
   * @brief cast and return derived object pointer as const
   * @return const pointer to derived class
   */
  const Derived* GetDerived() const
  {
    return static_cast<const Derived*>(this);
  }
};
} // namespace four
