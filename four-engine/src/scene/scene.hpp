#pragma once

namespace four
{
class Scene
{
public:
  Scene();
  ~Scene();

private:
  std::vector<Entity*> m_Entities;
  std::vector<System*> m_Systems;
};
