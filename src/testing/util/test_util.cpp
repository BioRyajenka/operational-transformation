//
// Created by Igor on 03.02.2021.
//

#include "test_util.h"

symbol create_symbol(const node_id_t id, const int val) {
    symbol res = symbol(1, 0, val);
    res.id = id;
    return res;
}

chain create_chain(std::initializer_list<node_id_t> ids) {
    assert(ids.size());

    chain res(create_symbol(*ids.begin(), *ids.begin()));
    auto cur = ids.begin();
    ++cur;
    while (cur != ids.end()) {
        res.move_to_the_end(chain(create_symbol(*cur, (int)*cur)));
        ++cur;
    }
    return res;
}

bool check_vectors_equal(const std::vector<node_id_t> &a, const std::vector<node_id_t> &b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < (int)a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

std::vector<node_id_t> doc2vec(const document &doc) {
    std::vector<node_id_t> res;
    auto cur = doc.get_node(0);
    while (cur != nullptr) {
        res.push_back(cur->value.id);
        cur = cur->next;
    }
    return res;
}

void print_doc(const std::string& prefix, const document &doc) {
    printf("%s: ", prefix.c_str());
    print_vector(doc2vec(doc));
}

std::vector<node_id_t> chain2vec(const chain &ch) {
    std::vector<node_id_t> res;

    auto cur = ch.get_head();
    while (cur != nullptr) {
        res.push_back(cur->value.id);
        cur = cur->next;
    }

    return res;
}

void print_chain(const std::string& prefix, const chain &ch) {
    printf("%s: ", prefix.c_str());
    const std::vector<node_id_t> &v = chain2vec(ch);
    printf("%u", v[0]);
    for (int j = 1; j < (int) v.size(); j++) printf("->%u", v[j]);
}

bool check_doc_ids(const document &doc, std::initializer_list<node_id_t> expected_list) {
    std::vector<node_id_t> actual = doc2vec(doc);

    std::vector<node_id_t> expected;
    for (const auto &i : expected_list) {
        expected.push_back(i);
    }

    if (!check_vectors_equal(actual, expected)) {
        printf("expected: ");
        print_vector(expected);
        printf("actual  :");
        print_vector(actual);
        return false;
    }

    return true;
}

void print_operation(const std::string &prefix, const operation &op) {
    printf("%s: \n", prefix.c_str());

    printf(" deletions: ");
    for (const auto &[node_id, parent_id]: op.get_deletions()) printf("%u(%u) ", node_id, parent_id);

    printf("\n insertions: ");
    for (const auto &[node_id, ch]: op.get_insertions()) {
        const std::vector<node_id_t> &v = chain2vec(ch);
        printf("[%u: %u", node_id, v[0]);
        for (int i = 1; i < (int) v.size(); i++) printf("->%u", v[i]);
        printf("] ");
    }

    printf("\n updates: ");
    for (const auto &[node_id, new_value]: op.get_updates()) {
        printf("[%u: %d] ", node_id, new_value);
    }
    printf("\n");
}
