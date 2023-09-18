#pragma once

namespace four
{

template <typename Derived>
struct Observer
{
public:
  virtual void FiledChanged(Derived& source, std::string_view filedName) const = 0;
};

template <typename T>
struct Observable
{
public:
  void Notify(T& source, std::string_view filedName)
  {
    for (auto observer : m_Observers)
    {
      observer->FiledChanged(source, filedName);
    }
  }

  void Subscribe(Observer<T>& source)
  {
    m_Observers.push_back(&source);
  }

  void Unsubscribe(Observer<T>& source)
  {
    m_Observers.erase(std::remove(m_Observers.begin(), m_Observers.end(), &source), m_Observers.end());
  }

private:
  std::vector<Observer<T>*> m_Observers;
};
} // namespace four
