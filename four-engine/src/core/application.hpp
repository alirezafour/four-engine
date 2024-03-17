#pragma once


namespace four
{

class Engine;
class Application
{
public:
  explicit Application(const std::string& title, uint32_t width, uint32_t height);
  virtual ~Application();
  Application(const Application&)            = delete;
  Application(Application&&)                 = delete;
  Application& operator=(const Application&) = delete;
  Application& operator=(Application&&)      = delete;

  virtual void Run();
  virtual void Init()     = 0;
  virtual void OnUpdate() = 0;

  Engine* GetEngine()
  {
    return m_Engine.get();
  }


private:
  std::unique_ptr<Engine> m_Engine;
};
} // namespace four
