cmake_minimum_required(VERSION 2.6)

project(HTTPClient)

# Locate libcURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

file(GLOB_RECURSE source_files ./*)
add_library(cpphttpclient STATIC ${source_files})
