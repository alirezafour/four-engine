#pragma once

#include "core/application.hpp"

class MyApp final : public four::Application
{
public:
  MyApp(const std::string& title, uint32_t width, uint32_t height);
  void Init() final;
  void OnUpdate(float deltaTime) final;

private:
};
