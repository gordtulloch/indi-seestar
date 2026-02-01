cmake_minimum_required(VERSION 3.15)
project(cpp-switchbot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# Find BlueZ (Bluetooth library)
find_library(BLUETOOTH_LIBRARY bluetooth)
if(NOT BLUETOOTH_LIBRARY)
    message(WARNING "libbluetooth not found. BLE support will be limited. Install with: sudo apt-get install libbluetooth-dev")
endif()

# Use nlohmann_json (header-only, can be included from system or fetched)
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Library source files
set(SWITCHBOT_SOURCES
    src/switchbot_client.cpp
    src/device.cpp
    src/remote.cpp
    src/scene.cpp
    src/switchbot.cpp
    src/utils.cpp
    src/ble_controller.cpp
)

set(SWITCHBOT_HEADERS
    include/switchbot/switchbot_client.hpp
    include/switchbot/device.hpp
    include/switchbot/remote.hpp
    include/switchbot/scene.hpp
    include/switchbot/switchbot.hpp
    include/switchbot/ble_controller.hpp
    include/switchbot/utils.hpp
)

# Create library
add_library(switchbot STATIC ${SWITCHBOT_SOURCES} ${SWITCHBOT_HEADERS})

target_include_directories(switchbot
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(switchbot
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        CURL::libcurl
        OpenSSL::SSL
        OpenSSL::Crypto
)

if(BLUETOOTH_LIBRARY)
    target_link_libraries(switchbot PRIVATE ${BLUETOOTH_LIBRARY})
endif()

# Example executable
add_executable(switchbot_example examples/example.cpp)
target_link_libraries(switchbot_example PRIVATE switchbot)

# Test bot executable
add_executable(test_bot test_bot.cpp)
target_link_libraries(test_bot PRIVATE switchbot)

# BLE test bot executable
add_executable(test_bot_ble test_bot_ble.cpp)
target_link_libraries(test_bot_ble PRIVATE switchbot)

# Detect bots executable
add_executable(detect_bots detect_bots.cpp)
target_link_libraries(detect_bots PRIVATE switchbot)

# Device info executable
add_executable(device_info device_info.cpp)
target_link_libraries(device_info PRIVATE switchbot)

# Optional: Install targets (commented out to avoid export issues)
# install(TARGETS switchbot
#     EXPORT switchbot-targets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )
# 
# install(DIRECTORY include/switchbot
#     DESTINATION include
# )
# 
# install(EXPORT switchbot-targets
#     FILE switchbot-targets.cmake
#     NAMESPACE switchbot::
#     DESTINATION lib/cmake/switchbot
# )
