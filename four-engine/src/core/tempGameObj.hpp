#pragma once

#include "glm/glm.hpp"
#include <cmath>
namespace four
{

struct Transform2DComponent
{
  glm::vec2 translation{};
  glm::vec2 scale{1.0F, 1.0F};
  float     rotation{};

  [[nodiscard]] glm::mat2 mat2() const
  {
    const float sin_ = std::sin(rotation);
    const float cos_ = std::cos(rotation);
    glm::mat2   rotMat{{cos_, sin_}, {-sin_, cos_}};

    glm::mat2 scaleMat{{scale.x, 0.0F}, {0.0F, scale.y}};
    return rotMat * scaleMat;
  }
};

class TempGameObj
{
public:
  using id_t = uint32_t;

  [[nodiscard]] static TempGameObj Create()
  {
    static id_t nextId = 0;
    return TempGameObj{nextId++};
  }

  TempGameObj(const TempGameObj&)            = delete;
  TempGameObj& operator=(const TempGameObj&) = delete;
  TempGameObj(TempGameObj&&)                 = default;
  TempGameObj& operator=(TempGameObj&&)      = default;
  virtual ~TempGameObj()                     = default;


  [[nodiscard]] id_t GetId() const
  {
    return m_Id;
  }

  // more testing
  std::shared_ptr<VulkModel> GetModel()
  {
    return m_Model;
  }

  void SetModel(std::shared_ptr<VulkModel> model)
  {
    m_Model = std::move(model);
  }

  void SetColor(glm::vec3 color)
  {
    m_Color = color;
  }

  [[nodiscard]] glm::vec3 GetColor() const
  {
    return m_Color;
  }

  void SetTransform2D(Transform2DComponent translation)
  {
    m_Transform2D = translation;
  }

  [[nodiscard]] Transform2DComponent& GetTransform2D()
  {
    return m_Transform2D;
  }


private:
  explicit TempGameObj(id_t ObjId) : m_Id{ObjId}
  {
  }

private:
  id_t                       m_Id{0};
  std::shared_ptr<VulkModel> m_Model{};
  glm::vec3                  m_Color = {0.0F, 0.0F, 0.0F};
  Transform2DComponent       m_Transform2D{};
};
} // namespace four
