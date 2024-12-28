#include "camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace four
{

//==============================================================================
glm::mat4 Camera::GetViewMatrix() const
{
  const glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.0F), m_Position);
  const glm::mat4 cameraRotation    = GetRotationMatrix();

  return glm::inverse(cameraTranslation * cameraRotation);
}

//==============================================================================
glm::mat4 Camera::GetRotationMatrix() const
{
  const glm::quat pitchRotation = glm::angleAxis(m_Pitch, glm::vec3{1.0F, 0.0F, 0.0F});
  const glm::quat yawRotation   = glm::angleAxis(m_Yaw, glm::vec3{0.0F, -1.0F, 0.0F});

  return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

//==============================================================================
void Camera::OnEvent(KeyEventValue key, EventType type)
{
  switch (key)
  {
    case KeyEventValue::KeyW:
    {
      if (type == EventType::KeyPressed)
      {
        m_Velocety.z = 1.0F;
      }
      else if (type == EventType::KeyReleased)
      {
        m_Velocety.z = 0.0F;
      }
      break;
    }
    case KeyEventValue::KeyS:
    {
      if (type == EventType::KeyPressed)
      {
        m_Velocety.z = -1.0F;
      }
      else if (type == EventType::KeyReleased)
      {
        m_Velocety.z = 0.0F;
      }
      break;
    }
    case KeyEventValue::KeyA:
    {
      if (type == EventType::KeyPressed)
      {
        m_Velocety.x = -1.0F;
      }
      else if (type == EventType::KeyReleased)
      {
        m_Velocety.x = 0.0F;
      }
      break;
    }
    case KeyEventValue::KeyD:
    {
      if (type == EventType::KeyPressed)
      {
        m_Velocety.x = 1.0F;
      }
      else if (type == EventType::KeyReleased)
      {
        m_Velocety.x = 0.0F;
      }
      break;
    }
    case KeyEventValue::KeyQ:
    {
      if (type == EventType::KeyPressed)
      {
        m_Velocety.y = 1.0F;
      }
      else if (type == EventType::KeyReleased)
      {
        m_Velocety.y = 0.0F;
      }
      break;
    }
    case KeyEventValue::KeyE:
    {
      if (type == EventType::KeyPressed)
      {
        m_Velocety.y = -1.0F;
      }
      else if (type == EventType::KeyReleased)
      {
        m_Velocety.y = 0.0F;
      }
      break;
    }
    case KeyEventValue::KeyY:
    {
      if (type == EventType::KeyPressed)
      {
        m_ShouldRotate = true;
      }
      else if (type == EventType::KeyReleased)
      {
        m_ShouldRotate = false;
      }
      break;
    }
    default:
      break;
  }
}

void Camera::OnMouseMove(f32 xPos, f32 yPos)
{
  if (!m_ShouldRotate)
  {
    return;
  }
  m_Yaw += xPos / 200.0F;
  m_Pitch += yPos / 200.0F;
  LOG_CORE_INFO("camera mouse move: {} {}", m_Yaw, m_Pitch);
}
//==============================================================================
void Camera::Update([[maybe_unused]] f32 deltaTime)
{
  const glm::mat4 cameraRotation = GetRotationMatrix();
  m_Position += glm::vec3(cameraRotation * glm::vec4(m_Velocety * deltaTime, 0.0F));
  LOG_CORE_INFO("camera event: {}, {}", m_Position.z, m_Position.x);
}

//==============================================================================
} // namespace four
