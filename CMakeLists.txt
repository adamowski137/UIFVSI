cmake_minimum_required(VERSION 3.30)

project(uifvsi)
add_executable(${PROJECT_NAME} 
  main.cpp
  core/Window.cpp
  core/Application.cpp
  core/Camera.cpp
  core/Shader.cpp
  core/models/Torus.cpp
  core/models/Object.cpp
  core/models/Point.cpp
  core/models/Line.cpp
  core/models/Cursor.cpp
)

set(CMAKE_BUILD_TYPE Debug)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/math137)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glew-cmake)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)

find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)


target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_include_directories(math137 PUBLIC external/math137/include)
target_link_libraries(imgui PRIVATE glfw OpenGL::GL)
target_link_libraries(${PROJECT_NAME} imgui math137 libglew_static OpenGL::GL glfw)
