cmake_minimum_required(VERSION 2.6)

project(HTTPClient)

include_directories(./)
include_directories(../rapidjson)
file(GLOB_RECURSE source_files ./*)
add_library(cpprestclient STATIC ${source_files} ../HTTP/httpclient.cpp)
