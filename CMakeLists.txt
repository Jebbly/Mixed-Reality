cmake_minimum_required(VERSION 3.20.0)
project(mixed_reality)
set(CMAKE_CXX_STANDARD 17)

# Get required libraries.
find_package(OpenCV REQUIRED)
message(STATUS "Using OpenCV Version: ${OpenCV_VERSION}")

find_package(Eigen3 REQUIRED)
message(STATUS "Using Eigen3 Version: ${Eigen3_VERSION}")

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(extern/glfw)
include_directories(extern/glfw/include)
message(STATUS "Using GLFW Directory: extern/glfw")

include_directories(extern/glm)
message(STATUS "Using GLM Directory: extern/glm")

include_directories(extern/glad/include)
message(STATUS "Using GLAD Directory: extern/glad")

include_directories(extern/imgui)
include_directories(extern/imgui/backends)
message(STATUS "Using ImGui Directory: extern/imgui")

set(LIB_SOURCES
    extern/glad/src/glad.c
    extern/imgui/imgui.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/backends/imgui_impl_glfw.cpp
    extern/imgui/backends/imgui_impl_opengl3.cpp
)

set(ORB_SLAM3_DIR "" CACHE STRING "Path to ORB-SLAM3 Directory")
if (ORB_SLAM3_DIR STREQUAL "")
    message(FATAL_ERROR "Must provide a path to ORB-SLAM3 Directory")
endif()
message(STATUS "Using ORB-SLAM3 Directory: ${ORB_SLAM3_DIR}")

include_directories(
    ${ORB_SLAM3_DIR}
    ${ORB_SLAM3_DIR}/include
    ${ORB_SLAM3_DIR}/include/CameraModels
    ${ORB_SLAM3_DIR}/Thirdparty/Sophus
    ${EIGEN3_INCLUDE_DIR}
)
add_library(ORB_SLAM3 SHARED IMPORTED)
set_target_properties(ORB_SLAM3 PROPERTIES IMPORTED_LOCATION 
             "${ORB_SLAM3_DIR}/lib/libORB_SLAM3.so")

add_library(DBoW2 SHARED IMPORTED)
set_target_properties(DBoW2 PROPERTIES IMPORTED_LOCATION 
             "${ORB_SLAM3_DIR}/Thirdparty/DBoW2/lib/libDBoW2.so")

add_library(g2o SHARED IMPORTED)
set_target_properties(g2o PROPERTIES IMPORTED_LOCATION 
             "${ORB_SLAM3_DIR}/Thirdparty/g2o/lib/libg2o.so")

set(PROJECT_LIBS 
    ${OpenCV_LIBS} 
    ${EIGEN3_LIBS}
    ORB_SLAM3
    DBoW2
    g2o
    glfw
    ${GLFW_LIBRARIES}
)

# Setup source code
include_directories(include)
set(PROJECT_FILES
    ${LIB_SOURCES}
    src/shader_util.cpp
    src/renderer.cpp
    src/offline_camera.cpp
    src/plane.cpp
    src/main.cpp
)

# Create the actual executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})