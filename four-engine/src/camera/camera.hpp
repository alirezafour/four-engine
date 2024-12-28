#pragma once

#include "core/core.hpp"
#include "event/event.hpp"
#include "glm/glm.hpp"

namespace four
{

class FOUR_ENGINE_API Camera
{
public:
  Camera(const Camera&)            = default;
  Camera(Camera&&)                 = default;
  Camera& operator=(const Camera&) = default;
  Camera& operator=(Camera&&)      = default;
  explicit Camera(const glm::vec3& pos, f32 yaw, f32 pitch, const glm::vec3& velocity = {0.0F, 0.0F, 0.0F}) :
  m_Position(pos),
  m_Yaw(yaw),
  m_Pitch(pitch),
  m_Velocety(velocity) {};
  ~Camera() = default;

  [[nodiscard]] glm::mat4 GetViewMatrix() const;
  [[nodiscard]] glm::mat4 GetRotationMatrix() const;

  void OnEvent(KeyEventValue key, EventType type);
  void OnMouseMove(f32 xPos, f32 yPos);
  void Update(f32 deltaTime);

  [[nodiscard]] glm::vec3 GetPosition() const
  {
    return m_Position;
  }
  void SetPosition(const glm::vec3& pos)
  {
    m_Position = pos;
  }

  [[nodiscard]] glm::vec3 GetVelocity() const
  {
    return m_Velocety;
  }
  void SetVelocity(const glm::vec3& vel)
  {
    m_Velocety = vel;
  }

  [[nodiscard]] f32 GetYaw() const
  {
    return m_Yaw;
  }
  [[nodiscard]] f32 GetPitch() const
  {
    return m_Pitch;
  }
  void SetYaw(f32 yaw)
  {
    m_Yaw = yaw;
  }
  void SetPitch(f32 pitch)
  {
    m_Pitch = pitch;
  }

private:
  glm::vec3 m_Velocety;
  glm::vec3 m_Position;
  f32       m_Yaw{0.0F};
  f32       m_Pitch{0.0F};
  bool      m_ShouldRotate{false};
};

} // namespace four
