#pragma once

#include "component/component.hpp"

namespace four
{

class CLifeSpan : public Component
{
public:
  explicit CLifeSpan(int totalLifeSpan) : m_TotalLifeSpan{totalLifeSpan}, m_RemainingLifeSpan{totalLifeSpan}
  {
  }
  CShape()  = default;
  ~CShape() = default;


private:
  int m_TotalLifeSpan{0};
  int m_RemainingLifeSpan{0};
};
} // namespace four
