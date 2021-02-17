//
// Created by Igor on 02.02.2021.
//

#include "document.h"
#include "../testing/util/test_util.h"

void document::apply(const operation &op) {
    // === validation (only for debug) ===
    for (const auto &[node_id, parent_id] : *op.get_deletions()) {
        assert(node_id != symbol::initial.id);
        assert(map.count(node_id));
        assert(map.count(parent_id));
    }
//    for (const auto&[node_id, _] : *op.get_updates()) assert(map.count(node_id));
    for (const auto &[node_id, ch] : *op.get_insertions()) {
        assert(map.count(node_id));
        ch.iterate([this](const auto &s) { assert(!map.count(s.id)); });
    }
    // === end validation ===

    for (const auto &[node_id, _] : *op.get_deletions()) {
        const node<symbol> *n = map.at(node_id);

        data.remove_node(n);
        map.erase(node_id);
    }

    for (const auto &[node_id, ch] : *op.get_insertions()) {
        node<symbol> *n = map.at(node_id);
        const auto &[head, tail] = data.copy_to(n, ch);

        auto cur = head;
        while (cur != tail->next) {
            map[cur->value.id] = cur;

            cur = cur->next;
        }
    }

    // updates should follow after inserts
    for (const auto&[node_id, new_value] : *op.get_updates()) {
        node<symbol> *n = map.at(node_id);

        n->value.value = new_value;
    }
}

node<symbol> const *document::get_node(const node_id_t &node_id) const {
    const auto &it = map.find(node_id);
    return it == map.end() ? nullptr : it->second;
}
