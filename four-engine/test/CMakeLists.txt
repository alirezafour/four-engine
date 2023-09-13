
project(CatchTesting
        VERSION 0.0.1
        LANGUAGES CXX
)

add_subdirectory(vendor)

include(CTest)

add_executable(window-test window-test.cpp)
target_link_libraries(window-test PRIVATE vendorTest)
target_link_libraries(window-test PRIVATE four_engine)

add_executable(application-test application-test.cpp)
target_link_libraries(application-test PRIVATE vendorTest)
target_link_libraries(application-test PRIVATE four_engine)
