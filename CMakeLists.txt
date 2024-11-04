cmake_minimum_required(VERSION 3.29)
project(live_driving)

set(CMAKE_CXX_STANDARD 23)

include(cmake/get_cpm.cmake)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(spdlog CONFIG REQUIRED)

CPMAddPackage(
        NAME safetyhook
        GIT_TAG v0.4.1
        GITHUB_REPOSITORY cursey/safetyhook
        OPTIONS
            "SAFETYHOOK_FETCH_ZYDIS ON"
)

file(GLOB_RECURSE sources src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${sources})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE safetyhook spdlog::spdlog)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)