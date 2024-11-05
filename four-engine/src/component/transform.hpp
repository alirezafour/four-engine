#pragma once

#include "component/component.hpp"

namespace four
{

class CTransform : public Component
{
public:
  explicit CTransform(const glm::vec2& position, const glm::vec2& velocity) : m_Position{position}, m_velocity{velocity}
  {
  }
  CTransform()  = default;
  ~CTransform() = default;

  void SetPosition(const glm::vec2& position)
  {
    m_Position = position;
  }
  void SetVelocity(const glm::vec2& velocity)
  {
    m_velocity = velocity;
  }

  glm::vec2 GetPosition() const
  {
    return m_Position;
  }
  glm::vec2 GetVelocity() const
  {
    return m_velocity;
  }

private:
  glm::vec2 m_Position{0.0f, 0.0f};
  glm::vec2 m_velocity{0.0f, 0.0f};
};

} // namespace four
