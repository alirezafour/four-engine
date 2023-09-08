cmake_minimum_required(VERSION 3.24)

#project setup
project(four-engine
        VERSION 0.0.1
        LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES
     "src/*.hpp"
     "src/*.cpp"
)

add_subdirectory(vendor)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC vendor)


# Test
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
  message(STATUS "Building tests is enabled")
  add_subdirectory(test)
else()
  message(STATUS "Building tests is disabled")
endif()

