add_executable(tests operation_tests.cpp chain_tests.cpp util/test_util.h util/test_util.cpp util/magic_list.h document_tests.cpp client_server_simple_tests.cpp)
target_link_libraries(tests ot_variation_lib)

add_executable(simulation_tests util/test_util.h util/test_util.cpp util/magic_list.h client_server_one_threaded_tests.cpp)
target_link_libraries(simulation_tests ot_variation_lib)
