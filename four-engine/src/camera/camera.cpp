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
void Camera::SetupEvents(WindowType& window)
{
  KeyPressedEvent key;
  key.SetupCallBack([&](u32 key) { OnEvent(key); });
  window.RegisterEvent(std::move(key));

  MouseMovement mouseMove;
  mouseMove.SetupCallBack([&](f32 x, f32 y) { OnMouseMove(x, y); });
  window.RegisterEvent(std::move(mouseMove));
}

//==============================================================================
void Camera::OnEvent(u32 key)
{
  switch (key)
  {
    case 1:
      m_Position.z += 0.2F;
      break;
    case 2:
      m_Position.z -= 0.2F;
      break;
    case 3:
      m_Position.x += 0.2F;
      break;
    case 4:
      m_Position.x -= 0.2F;
      break;
    case 5:
      m_ShouldRotate = true;
      break;
    case 6:
      m_ShouldRotate = false;
      break;
    default:
      break;
  }
  LOG_CORE_INFO("camera event: {}, {}", m_Position.z, m_Position.x);
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
void Camera::Update([[maybe_unused]] f32 detaTime)
{
  const glm::mat4 cameraRotation = GetRotationMatrix();
  m_Position += glm::vec3(cameraRotation * glm::vec4(m_Velocety * 0.5F, 0.0F));
}

//==============================================================================
} // namespace four
