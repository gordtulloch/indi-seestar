cmake_minimum_required(VERSION 3.16)

project(indi_seestar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(INDI REQUIRED IMPORTED_TARGET libindi)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(indi_seestar_telescope
    src/indi_seestar_telescope.cpp
    src/seestar_client.cpp
)

target_include_directories(indi_seestar_telescope PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(indi_seestar_telescope PRIVATE
    PkgConfig::INDI
    indidriver
    nlohmann_json::nlohmann_json
)

# Many INDI distros expect position-independent code.
set_target_properties(indi_seestar_telescope PROPERTIES POSITION_INDEPENDENT_CODE ON)
