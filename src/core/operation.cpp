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
    apply(op_to_copy, nullptr);
//    hasher.reset(op_to_copy.hash());
}

void operation::apply(const operation &rhs, const std::shared_ptr<document> &root_state) {
    for (const auto &node_id : rhs.deletions) del(node_id, root_state);
    for (const auto &[node_id, new_value] : rhs.updates) update(node_id, new_value);
    for (const auto &[node_id, ch] : rhs.insertions) insert(node_id, ch);
}

void operation::insert(const node_id_t &node_id, const chain &chain_to_copy) {
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

void operation::del(const node_id_t &node_id, const std::shared_ptr<document> &root_state) {
    assert(!deletions.count(node_id));

//    hasher.apply_change(HASH_FLAG_DELETE & (int)node_id);

    if (updates.count(node_id)) {
        content_hash ^= HASH_FLAG_UPDATE * node_id * updates.at(node_id); // remove
        updates.erase(node_id);
    }

    if (root_state && !root_state->get_node(node_id)) {
        // should be in one of the chains
        for (auto &[k, ch] : insertions) {
            const auto &node = ch.find_node(node_id);
            if (node) {
                content_hash ^= node->value.id * node->value.value;
                if (!node->prev && !node->next) {
                    // the only node in the chain
                    insertions.erase(k);
                    return;
                }
                ch.remove_node(node);
                break;
            }
        }
        assert(false && "Node_id is not found neither in doc nor in chains");
    } else {
        deletions.insert(node_id);
    }

    if (insertions.count(node_id)) {
        rehang_insertions(node_id, root_state);
    }
}

void operation::rehang_insertions(const node_id_t &node_id, const std::shared_ptr<document> &root_state) {
    assert(root_state);
    assert(deletions.count(node_id));
    assert(insertions.count(node_id));

    auto cur = root_state->get_node(node_id);
    assert(cur);
    while (deletions.count(cur->value.id)) {
        cur = cur->prev;
        assert(cur && "Root node can't be deleted!");
    }
    const auto &new_starting_node_id = cur->value.id;

    chain &source_chain = insertions.at(node_id);
    const auto &target_chain = insertions.find(new_starting_node_id);
    if (target_chain != insertions.end()) {
        // chain already exists
        target_chain->second.move_to_the_end(std::move(source_chain));
    } else {
        // creating new chain
        insertions.emplace(new_starting_node_id, std::move(source_chain));
    }
    insertions.erase(node_id);
}

ll operation::hash() const {
    return content_hash;
}

std::shared_ptr<operation> operation::detach_unprocessable_by_server(const std::unordered_set<node_id_t> &dels) {
    std::shared_ptr<operation> x = nullptr;
    for (auto it = insertions.begin(); it != insertions.end();) {
        if (dels.count(it->first)) {
            if (x == nullptr) x = std::make_shared<operation>();

            it->second.iterate([this, x](const auto &s){
                // removing from 'this'
                content_hash ^= s.id * s.value;
                // adding to x
                x->content_hash ^= s.id * s.value;
            });

            x->insertions.emplace(it->first, std::move(it->second));
            it = insertions.erase(it);
        } else {
            ++it;
        }
    }
    return x;
}
