cmake_minimum_required(VERSION 3.18)

project(BGMRPCModule)

set(BGMRPC_SRC_DIR
    ${CMAKE_CURRENT_LIST_DIR}/BGMRPC
    CACHE PATH "Path to BGMRPC directory")

add_subdirectory(BGMRPC)
add_subdirectory(Deploy)
add_subdirectory(Apps)
add_subdirectory(BGMRPCConsole)
