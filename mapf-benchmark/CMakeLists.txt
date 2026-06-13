cmake_minimum_required(VERSION 3.28)
project(mapf-dataset)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(ORT_DIR ${CMAKE_SOURCE_DIR}/external/onnxruntime)

include_directories(${ORT_DIR}/include)
link_directories(${ORT_DIR}/lib)

find_package(OpenCV REQUIRED)

add_executable(mapf-dataset src/main.cpp src/grid.cpp src/agent.cpp src/mapfaster_encode.cpp)

target_include_directories(mapf-dataset PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(mapf-dataset PRIVATE
        ${OpenCV_LIBS}
        onnxruntime
)