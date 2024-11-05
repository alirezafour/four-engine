#pragma once

#include "component/component.hpp"
#include <SFML/Graphics.hpp>

namespace four
{

class CShape : public Component
{
public:
  CShape()  = default;
  ~CShape() = default;

private:
  sf::CircleShape m_Shape{};
};
} // namespace four
