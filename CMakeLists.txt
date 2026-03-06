# Atlas - Root CMakeLists.txt
# Unified build: Atlas Engine core + Nova Forge game project (client & server)
#
# Architecture:
#   engine/       → AtlasEngine static library (generic, game-agnostic)
#   editor/       → AtlasEditor executable (authoring tool)
#   cpp_client/   → Nova Forge game client (links AtlasEngine)
#   cpp_server/   → Nova Forge game server (links AtlasEngine)
#   atlas_tests/  → Atlas engine unit tests

cmake_minimum_required(VERSION 3.15)
project(Atlas VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard (C++17 for game code, engine uses C++17-compatible subset)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Unified output directories so all executables (client, server, editor) land together
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_CLIENT "Build C++ OpenGL client" ON)
option(BUILD_SERVER "Build C++ dedicated server" ON)
option(BUILD_ATLAS_ENGINE "Build Atlas Engine library" ON)
option(BUILD_ATLAS_EDITOR "Build Atlas Editor" ON)
option(BUILD_ATLAS_TESTS "Build Atlas Engine tests" ON)
option(NOVAFORGE_EDITOR_TOOLS "Embed editor tool layer in the game client (F12 toggle)" ON)

# --- Atlas Engine (core library) ---
if(BUILD_ATLAS_ENGINE)
    message(STATUS "Adding Atlas Engine library...")
    add_subdirectory(engine)
endif()

# --- Atlas Editor ---
if(BUILD_ATLAS_EDITOR AND BUILD_ATLAS_ENGINE)
    message(STATUS "Adding Atlas Editor...")
    add_subdirectory(editor)
endif()

# --- Atlas Engine Tests ---
if(BUILD_ATLAS_TESTS AND BUILD_ATLAS_ENGINE)
    enable_testing()
    message(STATUS "Adding Atlas Engine tests...")
    add_subdirectory(atlas_tests)
endif()

# --- Nova Forge Game Client ---
if(BUILD_CLIENT)
    message(STATUS "Adding C++ Client to solution...")
    add_subdirectory(cpp_client)
endif()

# --- Nova Forge Game Server ---
if(BUILD_SERVER)
    message(STATUS "Adding C++ Server to solution...")
    add_subdirectory(cpp_server)
endif()

message(STATUS "")
message(STATUS "Atlas Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Atlas Engine:  ${BUILD_ATLAS_ENGINE}")
message(STATUS "  Atlas Editor:  ${BUILD_ATLAS_EDITOR}")
message(STATUS "  Atlas Tests:   ${BUILD_ATLAS_TESTS}")
message(STATUS "  Game Client:   ${BUILD_CLIENT}")
message(STATUS "  Game Server:   ${BUILD_SERVER}")
message(STATUS "  Editor Tools:  ${NOVAFORGE_EDITOR_TOOLS}")
message(STATUS "")
message(STATUS "After generation, open the solution file in Visual Studio:")
message(STATUS "  build_vs/Atlas.sln")
message(STATUS "")
