cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

add_definitions(-DLINUX)

include_directories(HTTP)
include_directories(RESTwrapper)

add_subdirectory(HTTP)
add_subdirectory(test)
add_subdirectory(RESTwrapper)


add_executable(main main.cpp)

link_directories(${PROJECT_SOURCE_DIR}/build/lib)
target_link_libraries(main cpprestclient cpphttpclient pthread curl)