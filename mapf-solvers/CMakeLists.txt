cmake_minimum_required(VERSION 3.21)
project(mapf-solvers VERSION 1.0)

# =======================================================
# GLOBAL SETTINGS
# =======================================================
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set C version
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED 1)

# Set C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

# Set release build default
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# bcp
add_subdirectory(solvers/bcp-mapf)

# cbsh2-rtc
add_subdirectory(solvers/CBSH2-RTC)

#
## =======================================================
## LIBRARY 3: reLOC (MDDSAT)
## =======================================================
#set(MDDSAT "solvers/MDDSAT")
#file(GLOB MDDSAT_SOURCES "${MDDSAT}/src/*.cpp")
#message(STATUS "MDDSAT Sources found: ${MDDSAT_SOURCES}")
#add_library(MDDSAT_Library STATIC ${MDDSAT_SOURCES})

# wrapper
add_library(mapf-solvers STATIC
        src/solvers.cpp
)

target_link_libraries(mapf-solvers PRIVATE
        bcp-mapf
        CBSH2_Library
#        MDDSAT_Library
)

target_include_directories(mapf-solvers
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
#        PRIVATE
#        "${CMAKE_CURRENT_SOURCE_DIR}/solvers/MDDSAT/src"
)

option(MAPF_BUILD_APP "standalone app" OFF)

if (MAPF_BUILD_APP)
    add_executable(MyApp src/main.cpp)
    target_link_libraries(MyApp PRIVATE
            bcp-mapf
            CBSH2_Library
#            MDDSAT_Library
    )
#    target_include_directories(MyApp PRIVATE
#            "${CMAKE_CURRENT_SOURCE_DIR}/solvers/MDDSAT/src"
#    )
endif()

