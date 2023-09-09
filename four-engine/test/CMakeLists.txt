
project(CatchTesting
        VERSION 0.0.1
        LANGUAGES CXX
)

include_directories(${CMAKE_SOURCE_DIR}/four-engine/src)
add_subdirectory(vendor)

include(CTest)

file(GLOB SOURCES
     "${CMAKE_SOURCE_DIR}/four-engine/src/window/sdl/*.cpp"
)
add_executable(${PROJECT_NAME} test.cpp)
add_executable(window-test window-test.cpp ${CMAKE_SOURCE_DIR}/four-engine/src/window/sdl/sdlWindow.cpp ${CMAKE_SOURCE_DIR}/four-engine/src/core/log.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC vendorTest)
target_link_libraries(window-test PUBLIC vendorTest)
target_link_libraries(window-test PUBLIC vendor)
