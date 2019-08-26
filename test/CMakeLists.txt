cmake_minimum_required(VERSION 2.6)

project(TestHTTPClient)

# Locate libcURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(../HTTP)
include_directories(./)
include_directories(./rapidjson)

#Output Setup
add_executable(test_httpclient test.cpp)

#Link setup
target_link_libraries(test_httpclient cpphttpclient ${GTEST_LIBRARIES} pthread curl)
