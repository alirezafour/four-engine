#pragma once

#include "core/application.hpp"
#include "renderer/simpleRenderSystem.hpp"

class MyApp final : public four::Application
{
public:
  MyApp(const std::string& title, uint32_t width, uint32_t height);
  void Init() final;
  void OnUpdate(float deltaTime) final;

private:
  void LoadGameObjects();

  four::SimpleRenderSystem       m_SimpleRenderSystem;
  std::vector<four::TempGameObj> m_GameObjects;
};
