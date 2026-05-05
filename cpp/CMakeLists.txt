cmake_minimum_required(VERSION 3.20)
project(orrery_renderer CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)

add_executable(orrery_renderer
    src/renderer.cpp
    src/orbit_integrator.cpp
    src/main.cpp
)
target_include_directories(orrery_renderer PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(orrery_renderer ${SDL2_LIBRARIES})
