cmake_minimum_required(VERSION 3.5)
project(bot VERSION 1.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Matplot++
include(FetchContent)
FetchContent_Declare(matplotplusplus
        GIT_REPOSITORY https://github.com/alandefreitas/matplotplusplus
        GIT_TAG origin/master) 
        FetchContent_GetProperties(matplotplusplus)
if(NOT matplotplusplus_POPULATED)
    FetchContent_Populate(matplotplusplus)
add_subdirectory(${matplotplusplus_SOURCE_DIR} ${matplotplusplus_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# RapidJSON
include(FetchContent)
FetchContent_Declare(rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_TAG master)
FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif()

# libcurl
include(FetchContent)
FetchContent_Declare(libcurl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG origin/master)
    FetchContent_GetProperties(libcurl)
if(NOT libcurl_POPULATED)
  FetchContent_Populate(libcurl)
  add_subdirectory(${libcurl_SOURCE_DIR} ${libcurl_BINARY_DIR})
endif()

# Source files
file(GLOB CPP_SOURCES src/*.cpp)

# Add executables
add_executable(${PROJECT_NAME} main.cpp ${CPP_SOURCES})

# Link libraries
target_link_directories(${PROJECT_NAME} PRIVATE deps/DPP/library)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl PRIVATE dpp PUBLIC matplot)

# Specify include directories
include_directories(src/include)
target_include_directories(${PROJECT_NAME}
    PRIVATE ${rapidjson_SOURCE_DIR}/include
    PRIVATE deps/DPP/include) 