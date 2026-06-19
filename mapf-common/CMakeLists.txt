cmake_minimum_required(VERSION 3.28)

project(mapf-common LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(mapf-common STATIC
        src/map_reader.cpp
        src/scenario_reader.cpp
        src/solution_writer.cpp
)

add_library(mapf::common ALIAS mapf-common)

target_compile_features(mapf-common
        PUBLIC
        cxx_std_20
)

set_target_properties(mapf-common PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

target_include_directories(mapf-common
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)