cmake_minimum_required(VERSION 3.16)
project(ot_variation)

set(CMAKE_CXX_STANDARD 17)

add_executable(ot_variation main.cpp src/client/client.h src/testing/tests.h src/server/server.cpp src/server/server.h src/core/operation.cpp src/core/operation.h src/client/server_peer.h src/server/client_peer.h src/server/history/operations_history.h src/server/history/simple_history.h src/testing/magic_list.h src/core/node.h src/core/symbol.h src/client/document.h src/core/chain.h src/testing/test_document.h src/client/server_peer.cpp src/server/client_peer.cpp src/client/client.cpp)
