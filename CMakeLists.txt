cmake_minimum_required(VERSION 3.16)
project(ot_variation)

set(CMAKE_CXX_STANDARD 14)

add_executable(ot_variation main.cpp src/client/client.cpp src/client/client.h src/testing/tests.h src/server/server.cpp src/server/server.h src/core/operation.cpp src/core/operation.h src/core/change.h src/core/document.cpp src/core/document.h src/client/server_peer.cpp src/client/server_peer.h src/server/client_peer.h src/server/history/operations_history.h src/server/history/simple_history.h)
