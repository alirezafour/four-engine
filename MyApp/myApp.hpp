#pragma once

#include "core/application.hpp"

class MyApp final : public four::Application
{
public:
  MyApp(std::string_view title, std::uint32_t width, std::uint32_t height);
  void Init() final;
  void OnUpdate(float deltaTime) final;

private:
};
