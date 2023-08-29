cmake_minimum_required(VERSION 3.16..3.26)

# Silence warning about PROJECT_VERSION
cmake_policy(SET CMP0048 NEW)

#project setup
project(four-engine
        VERSION 0.0.1
        LANGUAGES CXX
)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES
     "src/*.h"
     "src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})
