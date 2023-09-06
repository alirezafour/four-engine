
project(CatchTesting
        VERSION 0.0.1
        LANGUAGES CXX
)

include_directories(${CMAKE_SOURCE_DIR}/four-engine/src)
add_subdirectory(vendor)

include(CTest)

add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC vendorTest)
