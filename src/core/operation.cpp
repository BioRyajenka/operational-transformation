//
// Created by Igor on 30.01.2021.
//

#include "operation.h"

static const int HASH_FLAG_INSERT = 1351727964;
static const int HASH_FLAG_UPDATE = -472705117;
static const int HASH_FLAG_DELETE = 3552447;

void operation::apply(const operation &rhs) {
    for (const auto &[node_id, parent_id] : rhs.deletions) del(node_id, parent_id);
    for (const auto &[node_id, new_value] : rhs.updates) update(node_id, new_value);
    for (const auto &[node_id, ch] : rhs.insertions) insert(node_id, ch);
}

void operation::insert(const node_id_t &node_id, const chain &chain_to_copy) {
    assert(!deletions.count(node_id) && "Deleted nodes can't be inserted to");

    chain_to_copy.iterate([this](const auto &s){
        content_hash ^= s.id * s.value;
    });

    const auto &existing_chain = insertions.find(node_id);
    if (existing_chain != insertions.end()) {
        existing_chain->second.copy_to_the_beginning(chain_to_copy);
    } else {
        // check if node_id is in some chain or totally new
        bool chain_found = false;
        for (auto &[n_id, ch] : insertions) {
            auto node = ch.find_node(node_id);
            if (node != nullptr) {
                ch.copy_to(node, chain_to_copy);
                chain_found = true;
                break;
            }
        }
        if (!chain_found) {
            insertions.emplace(node_id, chain_to_copy); // copying here, not moving
        }
    }
}

void operation::update(const node_id_t &node_id, const int &new_value) {
    assert(!deletions.count(node_id) && "Deleted nodes can't be updated");

    auto prev = updates.find(node_id);
    if (prev != updates.end()) {
        content_hash ^= HASH_FLAG_UPDATE * node_id * prev->second; // remove
    }
    content_hash ^= HASH_FLAG_UPDATE * node_id * new_value; // add

    updates.emplace(node_id, new_value).first->second = new_value;
}

void operation::del(const node_id_t &node_id, node_id_t parent_id) {
    assert(!deletions.count(node_id));

    if (updates.count(node_id)) {
        content_hash ^= HASH_FLAG_UPDATE * node_id * updates.at(node_id); // remove
        updates.erase(node_id);
    }

    if (!insertions.count(node_id) && !deletions.count(node_id)) {
        for (auto &[n_id, ch] : insertions) {
            // note: here is bottleneck. one can use hashmap in chain
            // to speedup node lookup, but it will require more memory
            const auto &n = ch.find_node(node_id);
            if (n) {
                content_hash ^= node_id * n->value.value;

                if (n->prev == nullptr && n->next == nullptr) {
                    insertions.erase(n_id);
                } else {
                    ch.remove_node(n);
                }
                return;
            }
        }
    }

    if (deletions.count(parent_id)) {
        parent_id = deletions.at(parent_id);
    }

    for (auto &[node_id, ch] : insertions) {
        if (ch.get_tail()->value.id == parent_id) {
            parent_id = node_id;
            break;
        }
    }

    assert(!deletions.count(parent_id));

    deletions.emplace(node_id, parent_id);

    // case 1: if there is already insertion from node_id. need to rehang to parent_id
    if (insertions.count(node_id)) {
        rehang_insertions(node_id, parent_id);
    }

    // case 2
    for (auto& [n_id, p_id] : deletions) {
        if (p_id == node_id) p_id = parent_id;
    }
}

void operation::rehang_insertions(const node_id_t &from_node_id, const node_id_t &to_node_id) {
    assert(insertions.count(from_node_id));
    assert(!deletions.count(to_node_id));

    chain &source_chain = insertions.at(from_node_id);
    const auto &target_chain = insertions.find(to_node_id);
    if (target_chain != insertions.end()) {
        // chain already exists
        target_chain->second.move_to_the_end(std::move(source_chain));
    } else {
        // to_node may be the end of one of the chains

        bool was_rehanged = false;
        for (auto &[node_id, ch] : insertions) {
            if (ch.get_tail()->value.id == to_node_id) {
                ch.move_to_the_end(std::move(source_chain));
                was_rehanged = true;
                break;
            }
        }

        if (!was_rehanged) {
            // creating new chain
            insertions.emplace(to_node_id, std::move(source_chain));
        }
    }
    insertions.erase(from_node_id);
}

bool operation::empty() const {
    return deletions.empty() && insertions.empty() && updates.empty();
}

ll operation::hash() const {
    return content_hash;
}
