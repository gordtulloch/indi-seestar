cmake_minimum_required(VERSION 3.16)
project(indi-switchbot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(INDI REQUIRED libindi)
find_package(Threads REQUIRED)

# Find Bluetooth library
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(BLUEZ bluez)
endif()

if(NOT BLUEZ_FOUND)
    find_library(BLUETOOTH_LIBRARY bluetooth)
    if(NOT BLUETOOTH_LIBRARY)
        message(FATAL_ERROR "Bluetooth library not found. Please install libbluetooth-dev")
    endif()
endif()

# Find cpp-switchbot library
set(CPP_SWITCHBOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpp-switchbot")

if(NOT EXISTS "${CPP_SWITCHBOT_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "cpp-switchbot not found in ${CPP_SWITCHBOT_DIR}. Please ensure the submodule is initialized.")
endif()

# Add cpp-switchbot subdirectory
add_subdirectory(${CPP_SWITCHBOT_DIR})

# INDI SwitchBot driver
set(INDI_SWITCHBOT_SOURCES
    indi_switchbot.cpp
)

set(INDI_SWITCHBOT_HEADERS
    indi_switchbot.h
)

# Build driver executable
add_executable(indi_switchbot ${INDI_SWITCHBOT_SOURCES} ${INDI_SWITCHBOT_HEADERS})

target_include_directories(indi_switchbot
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${INDI_INCLUDE_DIRS}
        ${CPP_SWITCHBOT_DIR}/include
)

target_link_libraries(indi_switchbot
    PRIVATE
        indidriver
        Threads::Threads
        switchbot
)

# Add bluetooth library
if(BLUEZ_FOUND)
    target_link_libraries(indi_switchbot PRIVATE ${BLUEZ_LIBRARIES})
else()
    target_link_libraries(indi_switchbot PRIVATE ${BLUETOOTH_LIBRARY})
endif()

# Install driver
install(TARGETS indi_switchbot RUNTIME DESTINATION bin)

# Install driver XML
install(FILES indi_switchbot.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/../share/indi)

# Print configuration
message(STATUS "")
message(STATUS "INDI SwitchBot Driver Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  INDI Include: ${INDI_INCLUDE_DIR}")
message(STATUS "  INDI Libraries: ${INDI_LIBRARIES}")
message(STATUS "  cpp-switchbot: ${CPP_SWITCHBOT_DIR}")
if(BLUEZ_FOUND)
    message(STATUS "  Bluetooth: ${BLUEZ_LIBRARIES}")
else()
    message(STATUS "  Bluetooth: ${BLUETOOTH_LIBRARY}")
endif()
message(STATUS "")
