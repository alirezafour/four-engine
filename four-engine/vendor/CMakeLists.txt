# check of both sdl and sfml is not on
if(FOUR_USE_SDL AND FOUR_USE_SFML)
  message(FATAL_ERROR "You have to chose SDL or SFML you can't use both at the same time.")
endif(FOUR_USE_SDL AND FOUR_USE_SFML)

# create interface for user 
add_library(vendor INTERFACE)

# if sdl is used
if(FOUR_USE_SDL)
  # setup static build for sdl
  set(SDL_STATIC ON)
  set(SDL_SHARED OFF)
  add_subdirectory(SDL)
  target_compile_definitions(vendor INTERFACE FOUR_USE_SDL)

# if sfml is used
elseif(FOUR_USE_SFML)
  set(SFML_USE_STATIC_STD_LIBS ON)
  add_subdirectory(SFML)
  target_compile_definitions(vendor INTERFACE FOUR_USE_SFML)
endif(FOUR_USE_SDL)

# log using spdlog
add_subdirectory(spdlog)

#glfw
add_subdirectory(glfw)
add_subdirectory(glm)

if(FOUR_USE_SDL) # link if sdl is used
  target_link_libraries(vendor INTERFACE SDL3::SDL3)
elseif(FOUR_USE_SFML) # link if sfml is used
  target_link_libraries(vendor INTERFACE sfml-graphics)
endif(FOUR_USE_SDL)

# link spdlog for logging
target_link_libraries(vendor INTERFACE spdlog::spdlog)

target_link_libraries(vendor INTERFACE glfw)
target_link_libraries(vendor INTERFACE glm::glm)


find_package(Vulkan REQUIRED)

target_include_directories(vendor INTERFACE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(vendor INTERFACE ${Vulkan_LIBRARIES})




