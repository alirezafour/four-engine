cmake_minimum_required(VERSION 3.30)

#project setup
project(my-game
        VERSION 0.0.1
        LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

add_subdirectory(four-engine)
add_subdirectory(MyApp)
