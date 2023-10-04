project(MyApp)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC four_engine)

# add_executable(glfwVulkan testglfw.cpp)
# target_link_libraries(glfwVulkan PUBLIC vendor)


