project(CatchTesting
        VERSION 0.0.1
        LANGUAGES CXX
)

add_subdirectory(vendor)


add_library(test_dep INTERFACE)
target_link_libraries(test_dep INTERFACE vendorTest)
target_link_libraries(test_dep INTERFACE four_engine)

add_executable(window-test window-test.cpp)
target_link_libraries(window-test PRIVATE test_dep)

add_executable(application-test application-test.cpp)
target_link_libraries(application-test PRIVATE test_dep)
