cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

add_definitions(-DLINUX)

include_directories(include)
include_directories(include/rapidjson)

add_subdirectory(src)
add_subdirectory(test)


add_executable(main ./example/main.cpp)

link_directories(${PROJECT_SOURCE_DIR}/build/lib)
target_link_libraries(main cpprestclient pthread curl)