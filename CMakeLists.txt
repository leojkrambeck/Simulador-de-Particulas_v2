cmake_minimum_required(VERSION 3.14)
project(SimuladorParticulas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.2
)

set(SFML_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(SFML_BUILD_DOC OFF CACHE INTERNAL "")
set(SFML_BUILD_AUDIO OFF CACHE INTERNAL "")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

add_executable(app
    src/main.cpp
    src/Principal.cpp
    src/Particula.cpp
    src/Eletron.cpp
    src/Proton.cpp
    src/GerenciadorFisico.cpp
    src/Menu.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app PRIVATE sfml-graphics sfml-window sfml-system)
