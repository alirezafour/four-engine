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
void Camera::Update([[maybe_unused]] f32 detaTime)
{
  const glm::mat4 cameraRotation = GetRotationMatrix();
  m_Position += glm::vec3(cameraRotation * glm::vec4(m_Velocety * 0.5F, 0.0F));
}

//==============================================================================
} // namespace four
