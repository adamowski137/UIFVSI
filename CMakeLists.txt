cmake_minimum_required(VERSION 3.30)
set(set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON))

project(uifvsi)
add_executable(${PROJECT_NAME} 
  main.cpp
  core/Window.cpp
  core/Application.cpp
  core/Camera.cpp
  core/InputHandler.cpp
  core/Scene.cpp
  core/SceneManager.cpp
  core/Transformations.cpp
  core/render/Shader.cpp
  core/render/Renderer.cpp
  core/models/primitives/Torus.cpp
  core/models/primitives/Point.cpp
  core/models/primitives/Cursor.cpp
  core/models/primitives/Ground.cpp
  core/models/curves/BSpline.cpp
  core/models/curves/BezierCurve.cpp
  core/models/curves/IBSpline.cpp
  core/models/curves/Curve.cpp
  core/models/ObjectBuilder.cpp
  core/models/Object.cpp
  core/models/surfaces/BezierC0.cpp
  core/models/surfaces/BezierC2.cpp
  core/models/surfaces/Surface.cpp
  core/models/surfaces/SurfaceBuilder.cpp
  core/serialize/Serializer.cpp
  core/models/surfaces/Gregory.cpp
)

set(CMAKE_BUILD_TYPE Debug)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/math137)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glew-cmake)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuiFileDialog)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/json)

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

target_include_directories(imgui PUBLIC 
  external/imgui 
  external/imgui/backends
)
target_include_directories(math137 PUBLIC external/math137/include)
target_link_libraries(imgui PRIVATE glfw OpenGL::GL)
target_link_libraries(ImGuiFileDialog PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC 
  imgui 
  math137 
  libglew_static 
  OpenGL::GL 
  glfw 
  ImGuiFileDialog 
  nlohmann_json::nlohmann_json
)
