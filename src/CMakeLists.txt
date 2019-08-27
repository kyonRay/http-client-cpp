cmake_minimum_required(VERSION 2.6)

project(RESTClient)

# Locate libcURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

include_directories(../include)
include_directories(../include/rapidjson)
file(GLOB_RECURSE source_files ./*)
add_library(cpprestclient STATIC ${source_files})
