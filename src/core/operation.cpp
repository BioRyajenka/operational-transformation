//
// Created by Igor on 30.01.2021.
//

#include "operation.h"
#include "../client/document.h"
#include "../testing/util/test_util.h"

static const int HASH_FLAG_INSERT = 1351727964;
static const int HASH_FLAG_UPDATE = -472705117;
static const int HASH_FLAG_DELETE = 3552447;

operation::operation(const operation& op_to_copy) {
    apply(op_to_copy);
//    hasher.reset(op_to_copy.hash());
}

void operation::apply(const operation &rhs) {
    for (const auto &[node_id, parent_id] : rhs.deletions) del(node_id, parent_id);
    for (const auto &[node_id, new_value] : rhs.updates) update(node_id, new_value);
    for (const auto &[node_id, ch] : rhs.insertions) insert(node_id, ch);
}

void operation::insert(const node_id_t &node_id, const chain &chain_to_copy) {
    if (deletions.count(node_id)) {
        print_operation("op", *this);
        fflush(stdout);
    }

    assert(!deletions.count(node_id) && "Deleted nodes can't be inserted to");

//    hasher.apply_change(HASH_FLAG_INSERT ^ (int)node_id ^ (int)chain_to_copy.get_head()->value.id ^ chain_to_copy.get_tail()->value.value);
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

//    hasher.apply_change(HASH_FLAG_UPDATE ^ (int)node_id ^ new_value);
    auto prev = updates.find(node_id);
    if (prev != updates.end()) {
        content_hash ^= HASH_FLAG_UPDATE * node_id * prev->second; // remove
    }
    content_hash ^= HASH_FLAG_UPDATE * node_id * new_value; // add

    updates.emplace(node_id, new_value).first->second = new_value;
}

void operation::del(const node_id_t &node_id, const node_id_t &parent_id) {
    assert(!deletions.count(node_id));
    assert(!deletions.count(parent_id));

//    hasher.apply_change(HASH_FLAG_DELETE & (int)node_id);

    if (updates.count(node_id)) {
        content_hash ^= HASH_FLAG_UPDATE * node_id * updates.at(node_id); // remove
        updates.erase(node_id);
    }

    if (!insertions.count(node_id) && !deletions.count(node_id)) {
        for (auto &[_, ch] : insertions) {
            // note: here is bottleneck. one can use hashmap in chain
            // to speedup node lookup, but it will require more memory
            const auto &n = ch.find_node(node_id);
            if (n) {
                content_hash ^= node_id * n->value.value;
                ch.remove_node(n);
            }
        }
    }

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
        // creating new chain
        insertions.emplace(to_node_id, std::move(source_chain));
    }
    insertions.erase(from_node_id);
}

ll operation::hash() const {
    return content_hash;
}
