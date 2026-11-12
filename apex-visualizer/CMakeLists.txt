cmake_minimum_required(VERSION 3.24)
project(ApexVisualizer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
add_executable(projection_tests tests/projection_tests.cpp)
target_include_directories(projection_tests PRIVATE src)
add_test(NAME projection COMMAND projection_tests)
if(WIN32)
  include(FetchContent)
  FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.8 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(imgui)
  add_executable(apex_visualizer WIN32 src/main.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp)
  target_include_directories(apex_visualizer PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends src)
  target_compile_definitions(apex_visualizer PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
  target_link_libraries(apex_visualizer PRIVATE d3d11 dxgi d3dcompiler dwmapi)
endif()
