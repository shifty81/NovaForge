cmake_minimum_required(VERSION 3.15)
project(NovaForgeCommon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only interface library — shared types used by both client and server
add_library(novaforge_common INTERFACE)
target_include_directories(novaforge_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
