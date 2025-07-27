cmake_minimum_required(VERSION 3.29)
project(live_driving)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(spdlog CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(safetyhook CONFIG REQUIRED)

file(GLOB_RECURSE LIVE_DRIVING_SOURCES src/live_driving/*.cpp)
add_library(${PROJECT_NAME} SHARED ${LIVE_DRIVING_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE
        safetyhook::safetyhook
        spdlog::spdlog
        CURL::libcurl
        rapidjson
        cryptopp::cryptopp
        yaml-cpp::yaml-cpp
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

file(GLOB_RECURSE YAML_TEST_SOURCES src/yaml_test/*.cpp)
add_executable(yaml_test ${YAML_TEST_SOURCES})
target_link_libraries(yaml_test PRIVATE yaml-cpp::yaml-cpp)

file(COPY live_driving.yaml DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})