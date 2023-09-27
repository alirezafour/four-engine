cmake_minimum_required(VERSION 3.24)

# Silence warning about PROJECT_VERSION
cmake_policy(SET CMP0048 NEW)

#project setup
project(my-game
        VERSION 0.0.1
        LANGUAGES CXX C
)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_subdirectory(four-engine/vendor)
add_subdirectory(MyApp)
