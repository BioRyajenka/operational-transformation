//
// Created by Igor on 02.02.2021.
//

#include "document.h"

void document::apply(const operation &op) {
    static const int HASH_FLAG_INS = 12345678;
    static const int HASH_FLAG_UPD = 8765432;
    static const int HASH_FLAG_DEL = -1852332;

    // === validation (only for debug) ===
    for (const auto &node_id : *op.get_deletions()) assert(node_id != symbol::initial.id && map.count(node_id));
    for (const auto&[node_id, _] : *op.get_updates()) assert(map.count(node_id));
    for (const auto &[node_id, ch] : *op.get_insertions()) {
        assert(map.count(node_id));
        ch.iterate([this](const auto &ins_id) { assert(map.count(ins_id)); });
    }
    // === end validation ===

    // TODO: hashes
    for (const auto &node_id : *op.get_deletions()) {
        const node<symbol> *n = map.at(node_id);
        data.remove_node(n);
        map.erase(node_id);
    }

    for (const auto&[node_id, new_value] : *op.get_updates()) {
        map.at(node_id)->value.value = new_value;
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
            const auto &tmp = cur;
            cur = cur->next;
            c = c->next;

            data.remove_node(tmp);
        }
    }
}

ll document::hash() const {
    return hasher.hash();
}

