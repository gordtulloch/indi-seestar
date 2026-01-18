cmake_minimum_required(VERSION 3.10)
project(indi-alpaca-tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)

# Include directories
include_directories(${CURL_INCLUDE_DIRS})

# Test executables
add_executable(alpaca_version_test alpaca_version_test.cpp)
add_executable(alpaca_discovery_test alpaca_discovery_test.cpp)
add_executable(play_sound_test play_sound_test.cpp)
add_executable(supported_actions_test supported_actions_test.cpp)
add_executable(findhome_test findhome_test.cpp)
add_executable(slewtoaltaz_test slewtoaltaz_test.cpp)
add_executable(slewtocoordinates_test slewtocoordinates_test.cpp)
add_executable(startup_test startup_test.cpp)
add_executable(telescope_state_test telescope_state_test.cpp)
add_executable(slewasync_test slewasync_test.cpp)
add_executable(park_test park_test.cpp)
add_executable(park_native_test park_native_test.cpp)
add_executable(park_unpark_test park_unpark_test.cpp)
add_executable(telescope_get_all_test telescope_get_all_test.cpp)
add_executable(telescope_complete_test telescope_complete_test.cpp)
add_executable(common_methods_test common_methods_test.cpp)
add_executable(camera_methods_test camera_methods_test.cpp)
add_executable(filterwheel_methods_test filterwheel_methods_test.cpp)
add_executable(focuser_methods_test focuser_methods_test.cpp)
add_executable(site_location_test site_location_test.cpp)

# Link libraries
target_link_libraries(alpaca_version_test ${CURL_LIBRARIES})
target_link_libraries(play_sound_test ${CURL_LIBRARIES})
target_link_libraries(supported_actions_test ${CURL_LIBRARIES})
target_link_libraries(findhome_test ${CURL_LIBRARIES})
target_link_libraries(slewtoaltaz_test ${CURL_LIBRARIES})
target_link_libraries(slewtocoordinates_test ${CURL_LIBRARIES})
target_link_libraries(startup_test ${CURL_LIBRARIES})
target_link_libraries(park_native_test ${CURL_LIBRARIES})
target_link_libraries(park_unpark_test ${CURL_LIBRARIES})
target_link_libraries(telescope_get_all_test ${CURL_LIBRARIES})
target_link_libraries(telescope_complete_test ${CURL_LIBRARIES})
target_link_libraries(common_methods_test ${CURL_LIBRARIES})
target_link_libraries(camera_methods_test ${CURL_LIBRARIES})
target_link_libraries(filterwheel_methods_test ${CURL_LIBRARIES})
target_link_libraries(focuser_methods_test ${CURL_LIBRARIES})
target_link_libraries(site_location_test ${CURL_LIBRARIES})
target_link_libraries(telescope_complete_test ${CURL_LIBRARIES})
target_link_libraries(telescope_get_all_test ${CURL_LIBRARIES})
target_link_libraries(park_native_test ${CURL_LIBRARIES})

install(TARGETS alpaca_version_test alpaca_discovery_test play_sound_test supported_actions_test findhome_test slewtoaltaz_test slewtocoordinates_test startup_test telescope_state_test slewasync_test DESTINATION bin)
install(TARGETS alpaca_version_test alpaca_discovery_test play_sound_test supported_actions_test findhome_test slewtoaltaz_test slewtocoordinates_test startup_test telescope_state_test DESTINATION bin)
