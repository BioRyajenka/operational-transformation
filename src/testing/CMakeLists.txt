add_executable(tests operation_tests.cpp chain_tests.cpp hash_counter_tests.cpp util/test_util.h util/test_util.cpp document_tests.cpp)
target_link_libraries(tests ot_variation_lib)
