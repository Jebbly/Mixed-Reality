cmake_minimum_required(VERSION 3.20.0)
project(mixed_reality)
set(CMAKE_CXX_STANDARD 17)

# Get required libraries.
find_package(OpenCV REQUIRED)
message(STATUS "Using OpenCV Version: ${OpenCV_VERSION}")

find_package(Eigen3 REQUIRED)
message(STATUS "Using Eigen3 Version: ${Eigen3_VERSION}")

add_subdirectory(extern/glfw)
include_directories(extern/glfw/include)
message(STATUS "Using GLFW Directory: extern/glfw")

set(GLAD_SOURCES
    extern/glad/src/glad.c)
include_directories(extern/glad/include)
message(STATUS "Using GLAD Directory: extern/glad")

set(ORB_SLAM3_DIR "" CACHE STRING "Path to ORB-SLAM3 Directory")
if (ORB_SLAM3_DIR STREQUAL "")
    message(FATAL_ERROR "Must provide a path to ORB-SLAM3 Directory")
endif()
message(STATUS "Using ORB-SLAM3 Directory: ${ORB_SLAM3_DIR}")

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
    ${GLAD_SOURCES}
    src/main.cpp
)

# Create the actual executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${EIGEN3_LIBS} ORB_SLAM3 DBoW2 g2o glfw)