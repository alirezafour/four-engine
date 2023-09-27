#include "catch2/catch_test_macros.hpp"

// #include "core/imgui/imguiLayer.hpp"
// #include "core/layerStack.hpp"

#include <memory>

TEST_CASE("Application test")
{
  // log used in every systems so it required to initialized first
  four::Log::Init();

  // SECTION("testing LayerStack Push")
  // {
  //   four::LayerStack<four::ImGuiLayer> layerStack;
  //
  //   auto  layer    = std::make_unique<four::ImGuiLayer>();
  //   auto* ref      = layer.get();
  //   auto* refStack = layerStack.PushLayer(std::move(layer));
  //   layerStack.PushLayer(std::make_unique<four::ImGuiLayer>());
  //   REQUIRE(ref == refStack);
  //   REQUIRE(layerStack.Count() == 2);
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
