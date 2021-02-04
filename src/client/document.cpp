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

        content_hash ^= n->value.id * n->value.value;
        data.remove_node(n);
        map.erase(node_id);
    }

    for (const auto &[node_id, ch] : *op.get_insertions()) {
        node<symbol> *n = map.at(node_id);
        const auto &[head, tail] = data.copy_to(n, ch);

        auto cur = head;
        while (cur != tail->next) {
            content_hash ^= cur->value.id * cur->value.value;
            map[cur->value.id] = cur;

            cur = cur->next;
        }
    }

    // updates should follow after inserts
    for (const auto&[node_id, new_value] : *op.get_updates()) {
        node<symbol> *n = map.at(node_id);

        content_hash ^= node_id * n->value.value; // remove
        content_hash ^= node_id * new_value; // and reinsert
        n->value.value = new_value;
    }
}

node<symbol> const *document::get_node(const node_id_t &node_id) const {
    const auto &it = map.find(node_id);
    return it == map.end() ? nullptr : it->second;
}

void document::undo_insertions(const std::unordered_map<node_id_t, chain> &insertions) {
    for (const auto &[node_id, ch] : insertions) {
        auto &before = map.at(node_id);
        auto &cur = before->next;
        auto c = ch.get_head();

        while (c != nullptr) {
            assert(cur->value.id == c->value.id && cur->value.value == c->value.value);
            const auto tmp = cur;
            cur = cur->next;
            c = c->next;

            content_hash ^= tmp->value.id * tmp->value.value;
            map.erase(tmp->value.id);
            data.remove_node(tmp);
        }
    }
}

ll document::hash() const {
    return content_hash;
}

