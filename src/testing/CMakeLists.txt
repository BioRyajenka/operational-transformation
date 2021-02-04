add_executable(tests operation_tests.cpp chain_tests.cpp hash_counter_tests.cpp util/test_util.h util/test_util.cpp util/magic_list.h document_tests.cpp client_server_simple_tests.cpp client_server_one_threaded_tests.cpp)
target_link_libraries(tests ot_variation_lib)
