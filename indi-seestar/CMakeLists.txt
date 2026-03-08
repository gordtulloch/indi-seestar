cmake_minimum_required(VERSION 3.16)
project(indi-seestar VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(INDI REQUIRED libindi)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../indi-alpaca
    ${INDI_INCLUDE_DIRS}
)

# Seestar CCD Driver
add_executable(indi_seestar_ccd
    indi_seestar_ccd.cpp
    ../indi-alpaca/indi_alpaca_ccd.cpp
)

target_link_libraries(indi_seestar_ccd
    ${INDI_LIBRARIES}
    indidriver
)

# Install
install(TARGETS indi_seestar_ccd
    RUNTIME DESTINATION bin
)

# Install driver XML
install(FILES indi_seestar.xml
    DESTINATION ${CMAKE_INSTALL_PREFIX}/../share/indi
)
