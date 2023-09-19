#include "catch2/catch_test_macros.hpp"

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"

#include <memory>

TEST_CASE("Application test")
{
  SECTION("testing layers")
  {
    four::LayerStack<four::ImGuiLayer> layerStack;

    auto  layer    = std::make_unique<four::ImGuiLayer>();
    auto* ref      = layer.get();
    auto* refStack = layerStack.PushLayer(std::move(layer));
    REQUIRE(ref == refStack);
    layerStack.RemoveLayer(ref);
  }
}
