cmake_minimum_required(VERSION 3.20)
project(client)

set(CMAKE_CXX_STANDARD 17)

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/libs/SDL2")
set(SDL2_IMAGE_PATH "${CMAKE_SOURCE_DIR}/libs/SDL2_image")
set(SDL2_NET_PATH "${CMAKE_SOURCE_DIR}/libs/SDL2_net")
set(SDL2_TTF_PATH "${CMAKE_SOURCE_DIR}/libs/SDL2_ttf")

include_directories(
        ${SDL2_PATH}/include/SDL2
        ${SDL2_IMAGE_PATH}/include/SDL2
        ${SDL2_NET_PATH}/include/SDL2
        ${SDL2_TTF_PATH}/include/SDL2
        src
)

link_directories(
        ${SDL2_PATH}/lib
        ${SDL2_IMAGE_PATH}/lib
        ${SDL2_NET_PATH}/lib
        ${SDL2_TTF_PATH}/lib
)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(client ${SRC_FILES})

target_link_libraries(client
        mingw32
        SDL2main
        SDL2
        SDL2_net
        SDL2_ttf
        SDL2_image
)
