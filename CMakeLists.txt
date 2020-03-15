cmake_minimum_required(VERSION 3.16)
include(FetchContent)

project(minesweeperizer)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE
        src/helpers/texture.cpp)

add_executable(minesweeperizer
        ${SOURCE}

        src/main.cpp)

find_package(OpenCV REQUIRED)
find_package(SDL2 REQUIRED)

target_link_libraries(minesweeperizer
        ${OpenCV_LIBS}
        SDL2_image
        SDL2::SDL2main
        SDL2::SDL2)
