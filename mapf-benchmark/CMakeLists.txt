cmake_minimum_required(VERSION 3.28)
project(mapf-dataset)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(ORT_DIR ${CMAKE_SOURCE_DIR}/external/onnxruntime)

include_directories(${ORT_DIR}/include)
link_directories(${ORT_DIR}/lib)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(mapf-dataset src/main.cpp src/grid.cpp src/agent.cpp src/mapfaster_encode.cpp
        include/valuation.h
        include/portfolio.h
        include/mapfaster_valuation.h
        include/manifest.h
        src/manifest.cpp)

target_include_directories(mapf-dataset PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(mapf-dataset PRIVATE
        ${OpenCV_LIBS}
        onnxruntime
        nlohmann_json::nlohmann_json
)

add_subdirectory(../mapf-common ${CMAKE_CURRENT_BINARY_DIR}/mapf-common EXCLUDE_FROM_ALL)
add_subdirectory(../mapf-solvers ${CMAKE_CURRENT_BINARY_DIR}/mapf-solvers EXCLUDE_FROM_ALL)

target_link_libraries(mapf-dataset PRIVATE mapf-solvers mapf::common)