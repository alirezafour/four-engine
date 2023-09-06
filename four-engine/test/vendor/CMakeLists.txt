
add_subdirectory(Catch2)

add_library(vendorTest INTERFACE)
target_link_libraries(vendorTest INTERFACE Catch2::Catch2WithMain)
