cmake_minimum_required(VERSION 3.16)

project(CBSH2_RTC LANGUAGES CXX)

find_package(Boost REQUIRED)

file(GLOB CBSH2_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

list(REMOVE_ITEM CBSH2_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/driver.cpp"
)

add_library(CBSH2_Library STATIC
        ${CBSH2_SOURCES}
        adapter/cbsh_adapter.cpp
)

target_include_directories(CBSH2_Library
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(CBSH2_Library
        PUBLIC
        Boost::boost
)