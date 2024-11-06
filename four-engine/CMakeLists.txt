cmake_minimum_required(VERSION 3.30)

#project setup
project(four-engine
        VERSION 0.0.1
        LANGUAGES CXX C
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# if on windows
if(WIN32)
file(GLOB SOURCES
     "src/*.cpp"
     "src/core/*.cpp"
     "src/core/imgui/*.cpp"
     "src/renderer/*.cpp"
     "src/renderer/vulkan/*.cpp"
     "src/window/glfw/*.cpp"
)
endif(WIN32)

# if on linux
if(UNIX)
file(GLOB SOURCES
     "src/*.cpp"
     "src/core/*.cpp"
     "src/core/imgui/*.cpp"
     "src/renderer/*.cpp"
     "src/renderer/vulkan/*.cpp"
     "src/window/glfw/*.cpp"
)
endif(UNIX)


include_directories(src)

add_subdirectory(vendor)

# imgui backend
file(GLOB IMGUI_TO_ADD "vendor/imgui/*.cpp" "vendor/imgui/backends/imgui_impl_glfw.cpp" "vendor/imgui/backends/imgui_impl_vulkan.cpp")
list(APPEND SOURCES ${IMGUI_TO_ADD})

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC vendor)

#setup PCH
target_precompile_headers(${PROJECT_NAME} PUBLIC src/four-pch.hpp src/four-pch.cpp)

add_library(four_engine INTERFACE)
target_include_directories(four_engine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(four_engine INTERFACE ${PROJECT_NAME})
target_link_libraries(four_engine INTERFACE vendor)


# Test
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
  message(STATUS "Building tests is enabled")
  add_subdirectory(test)
else()
  message(STATUS "Building tests is disabled")
endif()

