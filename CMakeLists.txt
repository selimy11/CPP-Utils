cmake_minimum_required(VERSION 3.10)
project(CPP-Utils)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(CPP-Utils SHARED src/util.cpp)

target_compile_definitions(CPP-Utils PRIVATE CPPUTILS_EXPORTS)

find_package(OpenSSL REQUIRED)
target_link_libraries(CPP-Utils OpenSSL::Crypto)
