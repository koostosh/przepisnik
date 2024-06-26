cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD "20")

project(przepisnik)

if(MSVC)
    add_compile_options("/MP")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

file(GLOB_RECURSE PRZEPISNIK_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp" )

add_executable(przepisnik WIN32 ${PRZEPISNIK_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${PRZEPISNIK_SOURCES})

find_package(sdl2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

list(APPEND DependenciesList
    imgui::imgui
    SDL2::SDL2
    nlohmann_json::nlohmann_json
)

target_link_libraries(przepisnik PUBLIC ${DependenciesList}

    d3d11 # direct lib name
)

target_include_directories(przepisnik
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_custom_command(TARGET przepisnik POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:przepisnik>/data/.)

install(TARGETS przepisnik DESTINATION ".")
install(DIRECTORY "data" DESTINATION ".")
