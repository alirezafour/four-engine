#include "MyApp.hpp"
#include "core/engine.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

MyApp::MyApp(const std::string& title, uint32_t width, uint32_t height) :
four::Application{title, width, height},
m_SimpleRenderSystem{*GetEngine()->GetVulkDevice(), GetEngine()->GetRenderer()->GetSwapChainRenderPass()}
{
  GetEngine()->AddApplicaiton(this);
}

//==================================================================================================
void MyApp::Init()
{
  LoadGameObjects();
}

//==================================================================================================
void MyApp::LoadGameObjects()
{
  std::vector<four::VulkModel::Vertex> vertices = {{{0.0F, -0.5F}, {1.0F, 0.0F, 0.0F}},
                                                   {{0.5F, 0.5F}, {0.0F, 1.0F, 0.0F}},
                                                   {{-0.5F, 0.5F}, {0.0F, 0.0F, 1.0F}}};

  auto vulkModel = std::make_shared<four::VulkModel>(*GetEngine()->GetVulkDevice(), vertices);

  auto triangle = four::TempGameObj::Create();
  triangle.SetModel(vulkModel);
  triangle.SetColor(glm::vec3(0.1F, 0.8F, 0.1F));
  triangle.SetTransform2D({{0.2F, 0.0F}, {2.0F, 0.5F}, 0.25F * glm::two_pi<float>()});


  m_GameObjects.push_back(std::move(triangle));
}

//==================================================================================================
void MyApp::OnUpdate(float deltaTime)
{
  if (auto* commandBuffer = GetEngine()->GetRenderer()->BeginFrame())
  {
    GetEngine()->GetRenderer()->BeginSwapChainRenderPass(commandBuffer);
    m_SimpleRenderSystem.RenderGameObjects(commandBuffer, m_GameObjects, deltaTime);
    GetEngine()->GetRenderer()->EndSwapChainRenderPass(commandBuffer);
    GetEngine()->GetRenderer()->EndFrame();
  }
}
