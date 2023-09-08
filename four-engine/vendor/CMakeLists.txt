set(BUILD_SHARED_LIBS=OFF)
add_subdirectory(SDL)
add_subdirectory(spdlog)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE SDL3::SDL3)
target_link_libraries(vendor INTERFACE spdlog::spdlog)
