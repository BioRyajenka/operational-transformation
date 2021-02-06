//
// Created by Igor on 03.02.2021.
//

#ifndef OT_VARIATION_TEST_UTIL_H
#define OT_VARIATION_TEST_UTIL_H

#include "../../util.h"
#include "../../core/symbol.h"
#include "../../core/chain.h"
#include "../../client/document.h"
#include <initializer_list>

symbol create_symbol(const node_id_t &id, const int &val);

chain create_chain(std::initializer_list<node_id_t> ids);

bool check_vectors_equal(const std::vector<node_id_t> &a, const std::vector<node_id_t> &b);

std::vector<node_id_t> doc2vec(const document &doc);

bool check_doc_ids(const document &doc, std::initializer_list<node_id_t> expected_list);

template<typename T>
std::enable_if_t<std::is_integral_v<T>> print_vector(const std::vector<T> vec) {
    for (const auto &i : vec) printf("%d ", i);
    printf("\n");
}

void print_doc(const std::string& prefix, const document &doc);

std::vector<node_id_t> chain2vec(const chain &ch);

void print_chain(const std::string& prefix, const chain &ch);

void print_operation(const std::string &prefix, const operation &op);

#endif //OT_VARIATION_TEST_UTIL_H
