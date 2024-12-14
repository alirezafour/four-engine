#pragma once

#include "core/core.hpp"
#include "glm/glm.hpp"


namespace four
{
class Camera
{
public:
  glm::mat4 GetViewMatrix();
  glm::mat4 GetRotationMatrix();

  void processEvent(SDL_Event& e);

  void update();

private:
  glm::vec3 velocity;
  glm::vec3 position;
  // vertical rotation
  float pitch{};
  // horizontal rotation
  float yaw{};
};
} // namespace four
