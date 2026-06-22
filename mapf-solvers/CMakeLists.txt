cmake_minimum_required(VERSION 3.28)
project(mapf-solvers VERSION 1.0)

# =======================================================
# GLOBAL SETTINGS
# =======================================================
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set C version
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED 1)

# BUMPED: Changed to 20 because mapf-common strictly requires C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

# Set release build default
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# =======================================================
# ADD EXTERNAL DEPENDENCIES
# =======================================================

# bcp
add_subdirectory(solvers/bcp-mapf)

# cbsh2-rtc
add_subdirectory(solvers/CBSH2-RTC)

# =======================================================
# LIBRARY AND EXECUTABLES
# =======================================================

add_library(mapf-solvers STATIC
        src/solvers.cpp
)

target_link_libraries(mapf-solvers PRIVATE
        bcp-mapf
        CBSH2_Library
        mapf::common   # ADDED: Link against the alias defined in mapf-common
)

target_include_directories(mapf-solvers
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

option(MAPF_BUILD_APP "standalone app" OFF)

if (MAPF_BUILD_APP)
    add_executable(MyApp src/main.cpp)
    target_link_libraries(MyApp PRIVATE
            bcp-mapf
            CBSH2_Library
            mapf::common  # ADDED: Also linked to the app
    )
endif()
