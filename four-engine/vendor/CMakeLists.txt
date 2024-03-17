
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
find_package(Vulkan REQUIRED)
target_include_directories(vendor INTERFACE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(vendor INTERFACE ${Vulkan_LIBRARIES})




