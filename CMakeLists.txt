cmake_minimum_required(VERSION 3.10)
project(Engine2D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

# ---------------------------------------------------------------------------
# Biblioteca estática Engine2D
# ---------------------------------------------------------------------------
add_library(Engine2D STATIC
    src/Engine2D.cpp
)

target_include_directories(Engine2D PUBLIC
    include
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(Engine2D PUBLIC
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
)

target_compile_options(Engine2D PRIVATE -Wall -Wextra)

# ---------------------------------------------------------------------------
# Exemplos (opcional, ligado por padrão)
# ---------------------------------------------------------------------------
option(ENGINE2D_BUILD_EXAMPLES "Compilar os exemplos de demonstração" ON)

if(ENGINE2D_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp)
    foreach(EXAMPLE_SRC ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
        target_link_libraries(${EXAMPLE_NAME} PRIVATE Engine2D)
    endforeach()
endif()
