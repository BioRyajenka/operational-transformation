#set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.16)
project(ot_variation)

#add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
#add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
#add_compile_options("$<$<C_COMPILER_ID:MSVC>:/execution-charset:IBM866>")
#add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:IBM866>")

set(CMAKE_CXX_STANDARD 17)

add_library(ot_variation_lib STATIC src/client/client.h src/server/server.cpp src/server/server.h src/core/operation.cpp src/core/operation.h src/core/operation_transformation.cpp src/client/server_peer.h src/server/client_peer.h src/server/history/operations_history.h src/server/history/simple_history.h src/core/node.h src/core/symbol.h src/client/document.h src/core/chain.h src/client/server_peer.cpp src/server/client_peer.cpp src/client/client.cpp src/client/document.cpp src/core/symbol.cpp src/core/blocking_queue.h src/server/history/jumping_history.h src/server/history/jumping_history.cpp src/server/history/simple_history.cpp)

add_subdirectory(src/testing)
