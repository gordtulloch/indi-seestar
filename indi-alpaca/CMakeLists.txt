cmake_minimum_required(VERSION 3.16)
project(indi-alpaca VERSION 1.0.0 LANGUAGES CXX)

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
set(INDI_alpaca_TELESCOPE_SOURCES
    indi_alpaca_telescope.cpp
)

# Source files for CCD driver
set(INDI_alpaca_CCD_SOURCES
    indi_alpaca_ccd.cpp
)

# Source files for FilterWheel driver
set(INDI_alpaca_FILTERWHEEL_SOURCES
    indi_alpaca_filterwheel.cpp
)

# Source files for Focuser driver
set(INDI_alpaca_FOCUSER_SOURCES
    indi_alpaca_focuser.cpp
)

# Headers
set(INDI_alpaca_HEADERS
    indi_alpaca.h
    indi_alpaca_ccd.h
    indi_alpaca_filterwheel.h
    indi_alpaca_focuser.h
)

# Build the telescope driver
add_executable(indi_alpaca_telescope ${INDI_alpaca_TELESCOPE_SOURCES})

target_link_libraries(indi_alpaca_telescope
    indidriver
    nova
    Threads::Threads
)

# Build the CCD driver
add_executable(indi_alpaca_ccd ${INDI_alpaca_CCD_SOURCES})

target_link_libraries(indi_alpaca_ccd
    indidriver
    nova
    cfitsio
    Threads::Threads
)

# Build the FilterWheel driver
add_executable(indi_alpaca_filterwheel ${INDI_alpaca_FILTERWHEEL_SOURCES})

target_link_libraries(indi_alpaca_filterwheel
    indidriver
    nova
    Threads::Threads
)

# Build the Focuser driver
add_executable(indi_alpaca_focuser ${INDI_alpaca_FOCUSER_SOURCES})

target_link_libraries(indi_alpaca_focuser
    indidriver
    nova
    Threads::Threads
)

# Installation
install(TARGETS indi_alpaca_telescope indi_alpaca_ccd indi_alpaca_filterwheel indi_alpaca_focuser RUNTIME DESTINATION bin)
install(FILES indi_alpaca.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/../share/indi)

# Alpaca tests subdirectory
if(BUILD_TESTS)
    add_subdirectory(../alpaca-tests alpaca-tests)
endif()
