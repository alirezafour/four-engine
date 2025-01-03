#pragma once

namespace four
{

class Engine;
class Application
{
public:
  explicit Application(std::string_view title, std::uint32_t width, std::uint32_t height);
  virtual ~Application();
  Application(const Application&)            = delete;
  Application(Application&&)                 = delete;
  Application& operator=(const Application&) = delete;
  Application& operator=(Application&&)      = delete;

  virtual void Run();
  virtual void Init()                    = 0;
  virtual void OnUpdate(float deltaTime) = 0;

  Engine* GetEngine()
  {
    return m_Engine;
  }


private:
  Engine* m_Engine;
};
} // namespace four
