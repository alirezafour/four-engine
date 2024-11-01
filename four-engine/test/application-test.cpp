#include "catch2/catch_test_macros.hpp"

#include "core/application.hpp"
// #include "core/imgui/imguiLayer.hpp"
// #include "core/layerStack.hpp"

class MyApp final : public four::Application
{
public:
  MyApp(const std::string& title, uint32_t width, uint32_t height) : four::Application(title, width, height)
  {
  }
  void Init() final
  {
  }
  void OnUpdate(float deltaTime) final
  {
  }

private:
};

#include <memory>

TEST_CASE("Application test")
{
  MyApp app("MyApp", 400, 600);
  // SECTION("testing LayerStack Push")
  // {
  //   four::LayerStack<four::ImGuiLayer> layerStack;
  //
  //   auto  layer    = std::make_unique<four::ImGuiLayer>();
  //   auto* ref      = layer.get();
  //   auto* refStack = layerStack.PushLayer(std::move(layer));
  //   REQUIRE(ref == refStack);
  //   REQUIRE(layerStack.Count() == 1);
  // }
  //
  // SECTION("testing LayerStack Remove")
  // {
  //   four::LayerStack<four::ImGuiLayer> layerStack;
  //
  //   auto* refStack = layerStack.PushLayer(std::make_unique<four::ImGuiLayer>());
  //
  //   REQUIRE(layerStack.Count() == 1);
  //   layerStack.RemoveLayer(refStack);
  //   REQUIRE(layerStack.Count() == 0);
  // }
}
