cmake_minimum_required(VERSION 3.20)
project(orrery_renderer CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(orrery_renderer
    src/renderer.cpp
    src/orbit_integrator.cpp
    src/main.cpp
)
