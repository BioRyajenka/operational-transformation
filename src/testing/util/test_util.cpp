//
// Created by Igor on 03.02.2021.
//

#include "test_util.h"

symbol create_symbol(const node_id_t &id, const int &val) {
    symbol res = symbol(1, 0, val);
    res.id = id;
    return res;
}

chain create_chain(std::initializer_list<node_id_t> ids) {
    assert(ids.size());

    chain res(create_symbol(*ids.begin(), 0));
    auto cur = ids.begin();
    ++cur;
    while (cur != ids.end()) {
        res.move_to_the_end(chain(create_symbol(*cur, 0)));
        ++cur;
    }
    return res;
}

bool check_doc_contents(const document &doc, std::initializer_list<node_id_t> expected) {
    auto cur = doc.get_node(0);
    for (auto id : expected) {
        if (cur == nullptr) {printf("ended too early\n");return false;}
        if (id != cur->value.id) {printf("difference: %d %d\n",id, cur->value.id);return false;}
        cur = cur->next;
    }
    if (cur != nullptr) {printf("too long\n");return false;}
    return true;
}

