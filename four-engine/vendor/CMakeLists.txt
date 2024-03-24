
# create interface for user 
add_library(vendor INTERFACE)

# check of both sdl and sfml is not on
if(FOUR_USE_SDL AND FOUR_USE_GLFW)
  message(FATAL_ERROR "You have to chose SDL or SFML you can't use both at the same time.")

# if sdl is used
elseif(FOUR_USE_SDL)
  # setup static build for sdl
  set(SDL_STATIC ON)
  set(SDL_SHARED OFF)
  add_subdirectory(SDL)
  target_compile_definitions(vendor INTERFACE FOUR_USE_SDL)
  target_link_libraries(vendor INTERFACE SDL3::SDL3)

# if sfml is used
elseif(FOUR_USE_GLFW)
  add_subdirectory(glfw)
  target_compile_definitions(vendor INTERFACE FOUR_USE_GLFW)
  target_compile_definitions(vendor INTERFACE GLFW_INCLUDE_VULKAN)
  target_link_libraries(vendor INTERFACE glfw)
endif()

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
target_compile_features( VulkanHppModule PUBLIC cxx_std_20 )
target_link_libraries( VulkanHppModule PUBLIC Vulkan::Vulkan )
target_link_libraries( vendor INTERFACE VulkanHppModule )
