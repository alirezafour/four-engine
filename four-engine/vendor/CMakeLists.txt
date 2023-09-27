
# create interface for user 
add_library(vendor INTERFACE)

set(BUILD_SHARED_LIBS ON)
add_subdirectory(glfw)
add_subdirectory(glm)

#imgui setup
file(GLOB IMGUI_SOURCE
  "imgui/*.cpp"
)
add_library(imgui STATIC ${IMGUI_SOURCE})
target_include_directories(imgui PUBLIC imgui)

# log using spdlog
add_subdirectory(spdlog)

target_link_libraries(vendor INTERFACE glfw)
target_link_libraries(vendor INTERFACE imgui)
target_include_directories(vendor INTERFACE glad/include)
target_link_libraries(vendor INTERFACE glm::glm)

# link spdlog for logging
target_link_libraries(vendor INTERFACE spdlog::spdlog)
