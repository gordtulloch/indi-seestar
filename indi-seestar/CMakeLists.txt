cmake_minimum_required(VERSION 3.16)
project(indi-seestar VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build test programs" OFF)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(INDI REQUIRED libindi)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INDI_INCLUDE_DIRS}
)

# Source files for telescope driver
set(INDI_SEESTAR_TELESCOPE_SOURCES
    indi_seestar.cpp
)

# Source files for CCD driver
set(INDI_SEESTAR_CCD_SOURCES
    indi_seestar_ccd.cpp
)

# Headers
set(INDI_SEESTAR_HEADERS
    indi_seestar.h
    indi_seestar_ccd.h
)

# Build the telescope driver
add_executable(indi_seestar_telescope ${INDI_SEESTAR_TELESCOPE_SOURCES})

target_link_libraries(indi_seestar_telescope
    indidriver
    nova
    Threads::Threads
)

# Build the CCD driver
add_executable(indi_seestar_ccd ${INDI_SEESTAR_CCD_SOURCES})

target_link_libraries(indi_seestar_ccd
    indidriver
    nova
    cfitsio
    Threads::Threads
)

# Installation
install(TARGETS indi_seestar_telescope indi_seestar_ccd RUNTIME DESTINATION bin)
install(FILES indi_seestar.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/../share/indi)

# Tests subdirectory
if(BUILD_TESTS)
    add_subdirectory(../tests tests)
endif()
