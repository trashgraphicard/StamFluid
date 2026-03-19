cmake_minimum_required(VERSION 4.0)
project(opengl_template VERSION 1.0.0)

find_package(OpenGL REQUIRED)

add_executable(opengl_template 
                src/config.h 
                src/main.cpp
                src/glad.c
)

target_include_directories(opengl_template PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies
)

target_link_libraries(opengl_template PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/libglfw3dll.a
)

add_custom_command(TARGET opengl_template POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/lib/glfw3.dll
        $<TARGET_FILE_DIR:opengl_template>
)