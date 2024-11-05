pragma once

#include "component/component.hpp"

  namespace four
{
  class Entity
  {
  public:
    Entity();
    ~Entity();

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args);
    template <typename T>
    void                         RemoveComponent(); // get<T>() = T();
    tempate<typename T> T&       GetComponent();
    tempate<typename T> const T& GetComponent() const;
    template <typename T>
    bool               HasComponent() const;
    bool               IsAlive() const;
    void               Destroy();
    const std::string& GetName() const;
    size_t             GetID() const;


  private:
    std::vector<Component*> m_Components{};
    std::string             m_Name{"Default"};
    size_t                  m_ID{0};
    bool                    m_Alive{true};
  };
} // namespace four
