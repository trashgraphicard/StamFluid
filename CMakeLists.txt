cmake_minimum_required(VERSION 4.0)
project(stam_fluid VERSION 1.0.0)

find_package(OpenGL REQUIRED)

add_executable(stam_fluid 
                src/config.hpp 
                src/main.cpp
                src/glad.c)

target_include_directories(stam_fluid PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies
)

target_link_libraries(stam_fluid PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/libglfw3dll.a
)

add_custom_command(TARGET stam_fluid POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/lib/glfw3.dll
        $<TARGET_FILE_DIR:stam_fluid>
)