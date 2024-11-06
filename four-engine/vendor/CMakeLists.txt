
# create interface for user 
add_library(vendor INTERFACE)

# GLFW
add_subdirectory(glfw)
target_compile_definitions(vendor INTERFACE GLFW_INCLUDE_VULKAN)
target_link_libraries(vendor INTERFACE glfw)

# log using spdlog
add_subdirectory(spdlog)
target_link_libraries(vendor INTERFACE spdlog::spdlog)

# glm (math library)
add_subdirectory(glm)
target_link_libraries(vendor INTERFACE glm::glm)

# imgui
target_include_directories(vendor INTERFACE imgui)
target_include_directories(vendor INTERFACE imgui/backends)

# add vulkan with package

# find Vulkan SDK
find_package( Vulkan REQUIRED )

# Require Vulkan version ≥ 1.3.256 (earliest version when the Vulkan module was available)
if( ${Vulkan_VERSION} VERSION_LESS "1.3.256" )
  message( FATAL_ERROR "Minimum required Vulkan version for C++ modules is 1.3.256. "
           "Found ${Vulkan_VERSION}."
  )
endif()

# set up Vulkan C++ module as a library
add_library( VulkanHppModule )
target_sources( VulkanHppModule PRIVATE
  FILE_SET CXX_MODULES
  BASE_DIRS ${Vulkan_INCLUDE_DIR}
  FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
target_link_libraries( VulkanHppModule PUBLIC Vulkan::Vulkan )
target_link_libraries( vendor INTERFACE VulkanHppModule )
