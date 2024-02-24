cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD "20")

project(przepisnik)

if(MSVC)
    add_compile_options("/MP")
endif()

file(GLOB_RECURSE PRZEPISNIK_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp" )

add_executable(przepisnik WIN32 ${PRZEPISNIK_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${PRZEPISNIK_SOURCES})

find_package(sdl2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(przepisnik PUBLIC
    imgui::imgui
    SDL2::SDL2 SDL2::SDL2main
    nlohmann_json::nlohmann_json
    d3d11
)
